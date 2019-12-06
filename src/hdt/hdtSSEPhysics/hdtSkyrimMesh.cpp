#include "hdtSkyrimMesh.h"
#include "hdtSkinnedMesh\hdtSkinnedMeshShape.h"
#include "../hdtSSEUtils/NetImmerseUtils.h"
#include "../hdtSSEUtils/FrameworkUtils.h"
#include "../hdtSSEUtils/LogUtils.h"
#include <skse64\skse64\GameStreams.h>
#include "XmlReader.h"

#include <d3d11.h>

namespace hdt
{
	SkinnedMeshBone* SkyrimMesh::findBone(IDStr name)
	{
		for (auto i : m_bones)
			if (i->m_name == name)
				return i;
		return 0;
	}

	SkinnedMeshBody* SkyrimMesh::findBody(IDStr name)
	{
		for (auto i : m_meshes)
			if (i->m_name == name)
				return i;
		return 0;
	}

	int SkyrimMesh::findBoneIdx(IDStr name)
	{
		for (int i = 0; i < m_bones.size(); ++i)
			if (m_bones[i]->m_name == name)
				return i;
		return -1;
	}

	SkyrimMesh::SkyrimMesh(NiNode* skeleton)
		: m_skeleton(skeleton), m_oldRoot(0)
	{
		m_oldRoot = m_skeleton;
	}

	static constexpr float PI = 3.1415926535897932384626433832795f;
	void SkyrimMesh::readTransform(float timeStep)
	{
		auto newRoot = m_skeleton;
		while (newRoot->m_parent)newRoot = newRoot->m_parent;
		if (m_oldRoot != newRoot)
			timeStep = 0;

		if (!m_initialized)
		{
			timeStep = 0;
			m_initialized = true;
		}

		if (!timeStep)
		{
			updateTransformUpDown(m_skeleton);
			m_lastRootRotation = convertNi(m_skeleton->m_worldTransform.rot);
		}
		else
		{
			btQuaternion newRot = convertNi(m_skeleton->m_worldTransform.rot);
			btVector3 rotAxis;
			float rotAngle;
			btTransformUtil::calculateDiffAxisAngleQuaternion(m_lastRootRotation, newRot, rotAxis, rotAngle);
			float limit = 10.f*timeStep;

			if (rotAngle < -limit || rotAngle > limit)
			{
				rotAngle = btClamped(rotAngle, -limit, limit);
				btQuaternion clampedRot(rotAxis, rotAngle);
				m_lastRootRotation = clampedRot * m_lastRootRotation;
				m_skeleton->m_worldTransform.rot = convertBt(m_lastRootRotation);

				for (int i = 0; i < m_skeleton->m_children.m_arrayBufLen; ++i)
				{
					auto node = castNiNode(m_skeleton->m_children.m_data[i]);
					if (node)
					{
						updateTransformUpDown(node);
					}
				}
			}
		}

		SkinnedMeshSystem::readTransform(timeStep);
		m_oldRoot = newRoot;
	}

	void SkyrimMesh::writeTransform()
	{
		SkinnedMeshSystem::writeTransform();
	}

	btEmptyShape SkyrimMeshParser::BoneTemplate::emptyShape[1];

	SkyrimMeshParser::SkyrimMeshParser()
	{
	}

	template<typename ... Args> void SkyrimMeshParser::Error(const char* fmt, Args ... args)
	{
		std::string newfmt = std::string("%s(%d,%d):") + fmt;
		LogError(newfmt.c_str(), m_filePath.c_str(), m_reader->GetRow(), m_reader->GetColumn(), args...);
	}
	template<typename ... Args> void SkyrimMeshParser::Warning(const char* fmt, Args ... args)
	{
		std::string newfmt = std::string("%s(%d,%d):") + fmt;
		LogWarning(newfmt.c_str(), m_filePath.c_str(), m_reader->GetRow(), m_reader->GetColumn(), args...);
	}

	NiNode* SkyrimMeshParser::findObjectByName(const IDStr& name)
	{
		return findNode(m_skeleton, name->cstr());
	}

	SkyrimBone* SkyrimMeshParser::getOrCreateBone(const IDStr& name)
	{
		auto bone = static_cast<SkyrimBone*>(m_mesh->findBone(getRenamedBone(name)));
		if (bone) return bone;

		Warning("Bone %s use before created, create by current default value", name->cstr());
		auto node = findObjectByName(name);
		if (node)
		{
			auto defaultBoneInfo = getBoneTemplate("");
			bone = new SkyrimBone(node->m_name, node, defaultBoneInfo);

			bone->m_localToRig = defaultBoneInfo.m_centerOfMassTransform;
			bone->m_rigToLocal = defaultBoneInfo.m_centerOfMassTransform.inverse();
			bone->m_marginMultipler = defaultBoneInfo.m_marginMultipler;
			bone->m_gravityFactor = defaultBoneInfo.m_gravityFactor;
			bone->readTransform(0);

			m_mesh->m_bones.push_back(bone);
		}
		return bone;
	}

	IDStr SkyrimMeshParser::getRenamedBone(IDStr name)
	{
		auto iter = m_renameMap.find(name);
		if (iter != m_renameMap.end())
			return iter->second;
		return name;
	}

	Ref<SkyrimMesh> SkyrimMeshParser::createMesh(NiNode* skeleton, NiAVObject* model, const std::string& path, std::unordered_map<IDStr, IDStr> renameMap)
	{
		if (path.empty()) return nullptr;
		auto loaded = readAllFile(path.c_str());
		if (loaded.empty())
			return nullptr;

		m_renameMap = std::move(renameMap);

		m_skeleton = skeleton;
		m_model = model;
		m_filePath = path;
		updateTransformUpDown(m_skeleton);

		XMLReader reader((uint8_t*)loaded.data(), loaded.size());
		m_reader = &reader;

		m_reader->nextStartElement();
		if (m_reader->GetName() != "system")
			return nullptr;

		m_mesh = new SkyrimMesh(skeleton);

		try
		{
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "bone")
					{
						readBone();
						//if(bone) m_mesh->m_bones.push_back(bone);
					}
					else if (name == "bone-default")
					{
						auto clsname = m_reader->getAttribute("name", "");
						auto extends = m_reader->getAttribute("extends", "");
						auto defaultBoneInfo = getBoneTemplate(extends);
						readBoneTemplate(defaultBoneInfo);
						m_boneTemplates[clsname] = defaultBoneInfo;
					}
					else if (name == "per-vertex-shape")
					{
						auto shape = readPerVertexShape();
						if (shape && shape->m_vertices.size())
						{
							m_mesh->m_meshes.push_back(shape);
							shape->m_mesh = m_mesh;
						}
					}
					else if (name == "per-triangle-shape")
					{
						auto shape = readPerTriangleShape();
						if (shape && shape->m_vertices.size())
						{
							m_mesh->m_meshes.push_back(shape);
							shape->m_mesh = m_mesh;
						}
					}
					else if (name == "constraint-group")
					{
						auto constraint = readConstraintGroup();
						if (constraint) m_mesh->m_constraintGroups.push_back(constraint);
					}
					else if (name == "generic-constraint")
					{
						auto constraint = readGenericConstraint();
						if (constraint)	m_mesh->m_constraints.push_back(constraint);
					}
					else if (name == "stiffspring-constraint")
					{
						auto constraint = readStiffSpringConstraint();
						if (constraint)	m_mesh->m_constraints.push_back(constraint);
					}
					else if (name == "conetwist-constraint")
					{
						auto constraint = readConeTwistConstraint();
						if (constraint)	m_mesh->m_constraints.push_back(constraint);
					}
					else if (name == "generic-constraint-default")
					{
						auto clsname = m_reader->getAttribute("name", "");
						auto extends = m_reader->getAttribute("extends", "");
						auto defaultGenericConstraintTemplate = getGenericConstraintTemplate(extends);
						readGenericConstraintTemplate(defaultGenericConstraintTemplate);
						m_genericConstraintTemplates[clsname] = defaultGenericConstraintTemplate;
					}
					else if (name == "stiffspring-constraint-default")
					{
						auto clsname = m_reader->getAttribute("name", "");
						auto extends = m_reader->getAttribute("extends", "");
						auto defaultStiffSpringConstraintTemplate = getStiffSpringConstraintTemplate(extends);
						readStiffSpringConstraintTemplate(defaultStiffSpringConstraintTemplate);
						m_stiffSpringConstraintTemplates[clsname] = defaultStiffSpringConstraintTemplate;
					}
					else if (name == "conetwist-constraint-default")
					{
						auto clsname = m_reader->getAttribute("name", "");
						auto extends = m_reader->getAttribute("extends", "");
						auto defaultConeTwistConstraintTemplate = getConeTwistConstraintTemplate(extends);
						readConeTwistConstraintTemplate(defaultConeTwistConstraintTemplate);
						m_coneTwistConstraintTemplates[clsname] = defaultConeTwistConstraintTemplate;
					}
					else if (name == "shape")
					{
						auto name = m_reader->getAttribute("name");
						auto shape = readShape();
						if (shape)
						{
							m_shapeRefs.push_back(shape);
							m_shapes.insert(std::make_pair(name, shape));
						}
					}
					else
					{
						Warning("unknown element - %s", name.c_str());
						m_reader->skipCurrentElement();
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}
		}
		catch (const std::string& err)
		{
			Error("xml parse error - %s", err.c_str());
			return nullptr;
		}

		if (m_reader->GetErrorCode() != Xml::ErrorCode::None)
		{
			Error("xml parse error - %s", m_reader->GetErrorMessage());
			return nullptr;
		}

		m_mesh->m_skeleton = m_skeleton;
		m_mesh->m_shapeRefs.swap(m_shapeRefs);
		std::sort(m_mesh->m_bones.begin(), m_mesh->m_bones.end(), [](SkinnedMeshBone* a, SkinnedMeshBone* b) {
			return static_cast<SkyrimBone*>(a)->m_depth < static_cast<SkyrimBone*>(b)->m_depth;
		});

		return m_mesh->valid() ? m_mesh : nullptr;
	}

	Ref<ConstraintGroup> SkyrimMeshParser::readConstraintGroup()
	{
		Ref<ConstraintGroup> ret = new ConstraintGroup;

		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();

				if (name == "generic-constraint")
				{
					auto constraint = readGenericConstraint();
					if (constraint)	ret->m_constraints.push_back(constraint);
				}
				else if (name == "stiffspring-constraint")
				{
					auto constraint = readStiffSpringConstraint();
					if (constraint)	ret->m_constraints.push_back(constraint);
				}
				else if (name == "conetwist-constraint")
				{
					auto constraint = readConeTwistConstraint();
					if (constraint)	ret->m_constraints.push_back(constraint);
				}
				else if (name == "generic-constraint-default")
				{
					auto clsname = m_reader->getAttribute("name", "");
					auto extends = m_reader->getAttribute("extends", "");
					auto defaultGenericConstraintTemplate = getGenericConstraintTemplate(extends);
					readGenericConstraintTemplate(defaultGenericConstraintTemplate);
					m_genericConstraintTemplates[clsname] = defaultGenericConstraintTemplate;
				}
				else if (name == "stiffspring-constraint-default")
				{
					auto clsname = m_reader->getAttribute("name", "");
					auto extends = m_reader->getAttribute("extends", "");
					auto defaultStiffSpringConstraintTemplate = getStiffSpringConstraintTemplate(extends);
					readStiffSpringConstraintTemplate(defaultStiffSpringConstraintTemplate);
					m_stiffSpringConstraintTemplates[clsname] = defaultStiffSpringConstraintTemplate;
				}
				else if (name == "conetwist-constraint-default")
				{
					auto clsname = m_reader->getAttribute("name", "");
					auto extends = m_reader->getAttribute("extends", "");
					auto defaultConeTwistConstraintTemplate = getConeTwistConstraintTemplate(extends);
					readConeTwistConstraintTemplate(defaultConeTwistConstraintTemplate);
					m_coneTwistConstraintTemplates[clsname] = defaultConeTwistConstraintTemplate;
				}
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}
		return ret;
	}

	void SkyrimMeshParser::readBoneTemplate(BoneTemplate& cinfo)
	{
		bool clearCollide = true;
		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (name == "mass")
					cinfo.m_mass = m_reader->readFloat();
				else if (name == "inertia")
					cinfo.m_localInertia = m_reader->readVector3();
				else if (name == "centerOfMassTransform")
					cinfo.m_centerOfMassTransform = m_reader->readTransform();
				else if (name == "linearDamping")
					cinfo.m_linearDamping = m_reader->readFloat();
				else if (name == "angularDamping")
					cinfo.m_angularDamping = m_reader->readFloat();
				else if (name == "friction")
					cinfo.m_friction = m_reader->readFloat();
				else if (name == "rollingFriction")
					cinfo.m_rollingFriction = m_reader->readFloat();
				else if (name == "restitution")
					cinfo.m_restitution = m_reader->readFloat();
				else if (name == "margin-multiplier")
					cinfo.m_marginMultipler = m_reader->readFloat();
				else if (name == "shape")
				{
					auto shape = readShape();
					if (shape)
					{
						m_shapeRefs.push_back(shape);
						cinfo.m_collisionShape = shape.get();
					}
					else cinfo.m_collisionShape = BoneTemplate::emptyShape;
				}
				else if (name == "collision-filter")
					cinfo.m_collisionFilter = m_reader->readInt();
				else if (name == "can-collide-with-bone")
				{
					if (clearCollide)
					{
						cinfo.m_canCollideWithBone.clear();
						cinfo.m_noCollideWithBone.clear();
						clearCollide = false;
					}
					cinfo.m_canCollideWithBone.push_back(m_reader->readText());
				}
				else if (name == "no-collide-with-bone")
				{
					if (clearCollide)
					{
						cinfo.m_canCollideWithBone.clear();
						cinfo.m_noCollideWithBone.clear();
						clearCollide = false;
					}
					cinfo.m_noCollideWithBone.push_back(m_reader->readText());
				}
				else if (name == "gravity-factor")
				{
					cinfo.m_gravityFactor = btClamped(m_reader->readFloat(), 0.0f, 1.0f);
				}
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}
	}

	std::shared_ptr<btCollisionShape> SkyrimMeshParser::readShape()
	{
		auto typeStr = m_reader->getAttribute("type");
		if (typeStr == "ref")
		{
			auto shapeName = m_reader->getAttribute("name");
			m_reader->skipCurrentElement();
			auto iter = m_shapes.find(shapeName);
			if (iter != m_shapes.end())
				return iter->second;
			else
			{
				Warning("unknown shape - %s", shapeName.c_str());
				return nullptr;
			}
		}
		else if (typeStr == "box")
		{
			btVector3 halfExtend(0, 0, 0);
			float margin = 0;
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "halfExtend")
						halfExtend = m_reader->readVector3();
					else if (name == "margin")
						margin = m_reader->readFloat();
					else
					{
						Warning("unknown element - %s", name.c_str());
						m_reader->skipCurrentElement();
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}
			auto ret = std::make_shared<btBoxShape>(halfExtend);
			ret->setMargin(margin);
			return ret;
		}
		else if (typeStr == "sphere")
		{
			float radius = 0;
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "radius")
						radius = m_reader->readFloat();
					else
					{
						Warning("unknown element - %s", name.c_str());
						m_reader->skipCurrentElement();
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}
			return std::make_shared<btSphereShape>(radius);
		}
		else if (typeStr == "capsule")
		{
			float radius = 0;
			float height = 0;
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "radius")
						radius = m_reader->readFloat();
					else if (name == "height")
						height = m_reader->readFloat();
					else
					{
						Warning("unknown element - %s", name.c_str());
						m_reader->skipCurrentElement();
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}
			return std::make_shared<btCapsuleShape>(radius, height);
		}
		else if (typeStr == "hull")
		{
			float margin = 0;
			auto ret = std::make_shared<btConvexHullShape>();
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "point")
						ret->addPoint(m_reader->readVector3(), false);
					else if (name == "margin")
						margin = m_reader->readFloat();
					else
					{
						Warning("unknown element - %s", name.c_str());
						m_reader->skipCurrentElement();
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}
			ret->recalcLocalAabb();
			return ret->getNumPoints() ? ret : nullptr;
		}
		else if (typeStr == "cylinder")
		{
			float		height = 0;
			float		radius = 0;
			float		margin = 0;
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "height")
						height = m_reader->readFloat();
					else if (name == "radius")
						radius = m_reader->readFloat();
					else if (name == "margin")
						margin = m_reader->readFloat();
					else
					{
						Warning("unknown element - %s", name.c_str());
						m_reader->skipCurrentElement();
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}

			if (radius >= 0 && height >= 0)
			{
				auto ret = std::make_shared<btCylinderShape>(btVector3(radius, height, radius));
				ret->setMargin(margin);
				return ret;
			}
			else return nullptr;
		}
		else if (typeStr == "compound")
		{
			auto ret = std::make_shared<btCompoundShape>();
			while (m_reader->Inspect())
			{
				if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
				{
					auto name = m_reader->GetName();
					if (name == "child")
					{
						btTransform tr;
						std::shared_ptr<btCollisionShape> shape;

						while (m_reader->Inspect())
						{
							if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
							{
								auto name = m_reader->GetName();
								if (name == "transform")
									tr = m_reader->readTransform();
								else if (name == "shape")
									shape = readShape();
								else
								{
									Warning("unknown element - %s", name.c_str());
									m_reader->skipCurrentElement();
								}
							}
							else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
								break;
						}

						if (shape)
						{
							ret->addChildShape(tr, shape.get());
							m_shapeRefs.push_back(shape);
						}
					}
				}
				else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
					break;
			}
			return ret->getNumChildShapes() ? ret : nullptr;
		}
		else
		{
			Warning("Unknown shape type %s", typeStr.c_str());
			return nullptr;
		}
	}

	void SkyrimMeshParser::readBone()
	{
		IDStr name = getRenamedBone(m_reader->getAttribute("name"));
		IDStr cls = m_reader->getAttribute("template", "");

		if (m_mesh->findBone(name))
		{
			Warning("Bone %s is already exist, skipped", name->cstr());
			return;
		}

		auto node = findObjectByName(name);
		if (!node)
		{
			Warning("Bone %s is not exist, skipped", name->cstr());
			m_reader->skipCurrentElement();
			return;
		}

		BoneTemplate cinfo = m_boneTemplates[cls];
		readBoneTemplate(cinfo);
		auto b = new SkyrimBone(node->m_name, node, cinfo);
		b->m_localToRig = cinfo.m_centerOfMassTransform;
		b->m_rigToLocal = cinfo.m_centerOfMassTransform.inverse();
		b->m_marginMultipler = cinfo.m_marginMultipler;
		b->m_gravityFactor = cinfo.m_gravityFactor;
		//b->m_collisionFilter = cinfo.m_collisionFilter;

		b->readTransform(0);

		m_mesh->m_bones.push_back(b);
	}

	// float32
	// Martin Kallman
	//
	// Fast half-precision to single-precision floating point conversion
	//  - Supports signed zero and denormals-as-zero (DAZ)
	//  - Does not support infinities or NaN
	//  - Few, partially pipelinable, non-branching instructions,
	//  - Core opreations ~6 clock cycles on modern x86-64
	static void float32(float* __restrict out, const uint16_t in) {
		uint32_t t1;
		uint32_t t2;
		uint32_t t3;

		t1 = in & 0x7fff;                       // Non-sign bits
		t2 = in & 0x8000;                       // Sign bit
		t3 = in & 0x7c00;                       // Exponent

		t1 <<= 13;                              // Align mantissa on MSB
		t2 <<= 16;                              // Shift sign bit into position

		t1 += 0x38000000;                       // Adjust bias

		t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

		t1 |= t2;                               // Re-insert sign bit

		*((uint32_t*)out) = t1;
	};

	Ref<SkyrimShape> SkyrimMeshParser::generateMeshBody(const std::string& name)
	{
		//Warning("Skinned Mesh currently not supported");
		auto* g = castBSTriShape(findObject(m_model, name.c_str()));
		if (!g)
		{
			Warning("%s is not a BSTriShape or doesn't exist, skipped", name.c_str());
			m_reader->skipCurrentElement();
			return 0;
		}

		Ref<SkyrimShape> body = new SkyrimShape;
		body->m_name = name;

		if (!g->m_spSkinInstance)
		{
			//auto bone = m_mesh->findBone(name);
			//if (!bone)
			//{
			//	bone = new hdtSkyrimBone(name, g, defaultBoneInfo);
			//	m_mesh->m_bones.push_back(bone);
			//}
			//body->addBone(bone, btQsTransform());

			//for (int i = 0; i < body->m_vertices.size(); ++i)
			//{
			//	body->m_vertices[i].setBoneIdx(0, 0);
			//	body->m_vertices[i].m_weight[0] = 1;
			//}
			Warning("Shape %s has no skin data, skipped", name.c_str());
			m_reader->skipCurrentElement();
			return nullptr;
		}
		else
		{
			NiSkinInstance* skinInstance = g->m_spSkinInstance;
			NiSkinData* skinData = skinInstance->m_spSkinData;
			for (int boneIdx = 0; boneIdx < skinData->m_uiBones; ++boneIdx)
			{
				auto node = skinInstance->m_ppkBones[boneIdx];
				auto boneData = &skinData->m_pkBoneData[boneIdx];
				auto boundingSphere = BoundingSphere(convertNi(boneData->m_kBound.pos), boneData->m_kBound.radius);
				IDStr boneName = node->m_name;
				auto bone = m_mesh->findBone(boneName);
				if (!bone)
				{
					auto defaultBoneInfo = getBoneTemplate("");
					bone = new SkyrimBone(boneName, node->GetAsNiNode(), defaultBoneInfo);
					m_mesh->m_bones.push_back(bone);
				}

				body->addBone(bone, convertNi(boneData->m_kSkinToBone), boundingSphere);
			}

			NiSkinPartition* skinPartition = g->m_spSkinInstance->m_spSkinPartition;
			body->m_vertices.resize(skinPartition->vertexCount);

			// vertices data are all the same in every partitions
			auto partition = skinPartition->m_pkPartitions;
			auto vf = partition->vertexDesc;
			if (NiSkinPartition::GetVertexSize(vf) == sizeof(BSGeometryData::VertexUVSkinned) && NiSkinPartition::GetVertexFlags(vf) == (VF_VERTEX | VF_UV | VF_SKINNED))
			{
				auto vertices = reinterpret_cast<BSGeometryData::VertexUVSkinned*>(partition->shapeData->m_RawVertexData);
				for (int j = 0; j < skinPartition->vertexCount; ++j)
				{
					body->m_vertices[j].m_skinPos = convertNi(vertices[j].pos);
					for (int k = 0; k < partition->m_usBonesPerVertex && k < 4; ++k)
					{
						auto localBoneIndex = vertices[j].boneIndices[k];
						assert(localBoneIndex < body->m_skinnedBones.size());
						body->m_vertices[j].m_boneIdx[k] = localBoneIndex;
						float32(&body->m_vertices[j].m_weight[k], vertices[j].boneWeights[k]);
					}
				}
			}
			else if (NiSkinPartition::GetVertexSize(vf) == sizeof(BSGeometryData::VertexUVNormalTangentSkinned) && NiSkinPartition::GetVertexFlags(vf) == (VF_VERTEX | VF_UV | VF_SKINNED | VF_TANGENT | VF_NORMAL))
			{
				auto vertices = reinterpret_cast<BSGeometryData::VertexUVNormalTangentSkinned*>(partition->shapeData->m_RawVertexData);
				for (int j = 0; j < skinPartition->vertexCount; ++j)
				{
					body->m_vertices[j].m_skinPos = convertNi(vertices[j].pos);
					for (int k = 0; k < partition->m_usBonesPerVertex && k < 4; ++k)
					{
						auto localBoneIndex = vertices[j].boneIndices[k];
						assert(localBoneIndex < body->m_skinnedBones.size());
						body->m_vertices[j].m_boneIdx[k] = localBoneIndex;
						float32(&body->m_vertices[j].m_weight[k], vertices[j].boneWeights[k]);
					}
				}
			}
			else
			{
				Warning("Shape %s  has unsupport vertex format 0x%016llx flag:%x size:%d", name.c_str(), vf, NiSkinPartition::GetVertexFlags(vf), NiSkinPartition::GetVertexSize(vf));
				Warning("support format flag:%x size:%d", name.c_str(), vf, sizeof(BSGeometryData::VertexUVSkinned), (VF_VERTEX | VF_UV | VF_SKINNED));
				Warning("support format flag:%x size:%d", name.c_str(), vf, sizeof(BSGeometryData::VertexUVNormalTangentSkinned), (VF_VERTEX | VF_UV | VF_SKINNED | VF_TANGENT | VF_NORMAL));
				return nullptr;
			}
		}

		for (auto& i : body->m_vertices)
			i.sortWeight();

		return body;
	}

	Ref<SkyrimShape> SkyrimMeshParser::readPerVertexShape()
	{
		auto name = m_reader->getAttribute("name");

		auto body = generateMeshBody(name);
		if (!body) return nullptr;

		auto shape = new PerVertexShape(body);

		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (name == "priority")
				{
					Warning("piority is deprecated and no longer used");
					m_reader->skipCurrentElement();
				}
				else if (name == "margin")
					shape->m_shapeProp.margin = m_reader->readFloat();
				else if (name == "shared")
				{
					auto str = m_reader->readText();
					if (str == "public")
						body->m_shared = SkyrimShape::SHARED_PUBLIC;
					else if (str == "internal")
						body->m_shared = SkyrimShape::SHARED_INTERNAL;
					else if (str == "private")
						body->m_shared = SkyrimShape::SHARED_PRIVATE;
					else
					{
						Warning("unknown shared value, use default value \"public\"");
						body->m_shared = SkyrimShape::SHARED_PUBLIC;
					}
				}
				else if (name == "tag")
					body->m_tags.push_back(m_reader->readText());
				else if (name == "can-collide-with-tag")
					body->m_canCollideWithTags.insert(m_reader->readText());
				else if (name == "no-collide-with-tag")
					body->m_noCollideWithTags.insert(m_reader->readText());
				else if (name == "no-collide-with-bone")
				{
					auto bone = getOrCreateBone(m_reader->readText());
					if (bone) body->m_noCollideWithBones.push_back(bone);
				}
				else if (name == "weight-threshold")
				{
					auto boneName = m_reader->getAttribute("bone");
					float wt = m_reader->readFloat();
					for (int i = 0; i < body->m_skinnedBones.size(); ++i)
						if (body->m_skinnedBones[i].ptr->m_name == getRenamedBone(boneName))
						{
							body->m_skinnedBones[i].weightThreshold = wt;
							break;
						}
				}
				else if (name == "disable-tag")
				{
					body->m_disableTag = m_reader->readText();
				}
				else if (name == "disable-priority")
				{
					body->m_disablePriority = m_reader->readInt();
				}
				else if (name == "wind-effect")
				{
					shape->m_windEffect = m_reader->readFloat();
				}
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}

		shape->autoGen();
		body->finishBuild();

		return body;
	}

	Ref<SkyrimShape> SkyrimMeshParser::readPerTriangleShape()
	{
		auto name = m_reader->getAttribute("name");

		auto body = generateMeshBody(name);
		if (!body) return nullptr;

		auto shape = new PerTriangleShape(body);
		auto* g = castBSTriShape(findObject(m_model, name.c_str()));
		if (g->m_spSkinInstance)
		{
			NiSkinPartition* skinPartition = g->m_spSkinInstance->m_spSkinPartition;
			for (int i = 0; i < skinPartition->m_uiPartitions; ++i)
			{
				auto& partition = skinPartition->m_pkPartitions[i];
				for (int j = 0; j < partition.m_usTriangles; ++j)
					shape->addTriangle(partition.m_pusTriList[j * 3], partition.m_pusTriList[j * 3 + 1], partition.m_pusTriList[j * 3 + 2]);
			}
		}
		else
		{
			Warning("Shape %s has no skin data, skipped", name.c_str());
			return nullptr;
		}

		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (name == "priority")
				{
					Warning("piority is deprecated and no longer used");
					m_reader->skipCurrentElement();
				}
				else if (name == "margin")
					shape->m_shapeProp.margin = m_reader->readFloat();
				else if (name == "shared")
				{
					auto str = m_reader->readText();
					if (str == "public")
						body->m_shared = SkyrimShape::SHARED_PUBLIC;
					else if (str == "internal")
						body->m_shared = SkyrimShape::SHARED_INTERNAL;
					else if (str == "private")
						body->m_shared = SkyrimShape::SHARED_PRIVATE;
					else
					{
						Warning("unknown shared value, use default value \"public\"");
						body->m_shared = SkyrimShape::SHARED_PUBLIC;
					}
				}
				else if (name == "prenetration" || name == "penetration")
					shape->m_shapeProp.penetration = m_reader->readFloat();
				else if (name == "tag")
					body->m_tags.push_back(m_reader->readText());
				else if (name == "no-collide-with-tag")
					body->m_noCollideWithTags.insert(m_reader->readText());
				else if (name == "can-collide-with-tag")
					body->m_canCollideWithTags.insert(m_reader->readText());
				else if (name == "no-collide-with-bone")
				{
					auto bone = getOrCreateBone(m_reader->readText());
					if (bone) body->m_noCollideWithBones.push_back(bone);
				}
				else if (name == "weight-threshold")
				{
					auto boneName = m_reader->getAttribute("bone");
					float wt = m_reader->readFloat();
					for (int i = 0; i < body->m_skinnedBones.size(); ++i)
						if (body->m_skinnedBones[i].ptr->m_name == getRenamedBone(boneName))
						{
							body->m_skinnedBones[i].weightThreshold = wt;
							break;
						}
				}
				else if (name == "disable-tag")
				{
					body->m_disableTag = m_reader->readText();
				}
				else if (name == "disable-priority")
				{
					body->m_disablePriority = m_reader->readInt();
				}
				else if (name == "wind-effect")
				{
					shape->m_windEffect = m_reader->readFloat();
				}
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}

		body->finishBuild();

		return body;
	}

	void SkyrimMeshParser::readFrameLerp(btTransform& tr)
	{
		tr.setIdentity();
		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (name == "translationLerp")
					tr.getOrigin().setX(m_reader->readFloat());
				else if (name == "rotationLerp")
					tr.getOrigin().setY(m_reader->readFloat());
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}
	}

	bool SkyrimMeshParser::parseFrameType(const std::string& name, FrameType& frameType, btTransform& frame)
	{
		if (name == "frameInA")
		{
			frameType = FrameInA;
			frame = m_reader->readTransform();
		}
		else if (name == "frameInB")
		{
			frameType = FrameInB;
			frame = m_reader->readTransform();
		}
		else if (name == "frameInLerp")
		{
			frameType = FrameInLerp;
			readFrameLerp(frame);
		}
		else return false;
		return true;
	}

	void SkyrimMeshParser::readGenericConstraintTemplate(GenericConstraintTemplate& dest)
	{
		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (parseFrameType(name, dest.frameType, dest.frame));
				else if (name == "useLinearReferenceFrameA")
					dest.useLinearReferenceFrameA = m_reader->readBool();
				else if (name == "linearLowerLimit")
					dest.linearLowerLimit = m_reader->readVector3();
				else if (name == "linearUpperLimit")
					dest.linearUpperLimit = m_reader->readVector3();
				else if (name == "angularLowerLimit")
					dest.angularLowerLimit = m_reader->readVector3();
				else if (name == "angularUpperLimit")
					dest.angularUpperLimit = m_reader->readVector3();
				else if (name == "linearStiffness")
					dest.linearStiffness = m_reader->readVector3();
				else if (name == "angularStiffness")
					dest.angularStiffness = m_reader->readVector3();
				else if (name == "linearDamping")
					dest.linearDamping = m_reader->readVector3();
				else if (name == "angularDamping")
					dest.angularDamping = m_reader->readVector3();
				else if (name == "linearEquilibrium")
					dest.linearEquilibrium = m_reader->readVector3();
				else if (name == "angularEquilibrium")
					dest.angularEquilibrium = m_reader->readVector3();
				else if (name == "linearBounce")
					dest.linearBounce = m_reader->readVector3();
				else if (name == "angularBounce")
					dest.angularBounce = m_reader->readVector3();
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}
	}

	bool SkyrimMeshParser::findBones(const IDStr& bodyAName, const IDStr& bodyBName, SkyrimBone*& bodyA, SkyrimBone*& bodyB)
	{
		bodyA = (SkyrimBone*)m_mesh->findBone(bodyAName);
		bodyB = (SkyrimBone*)m_mesh->findBone(bodyBName);

		if (!bodyA)
		{
			auto node = findObjectByName(bodyAName);
			if (node)
			{
				auto defaultBoneInfo = getBoneTemplate("");
				bodyA = new SkyrimBone(node->m_name, node, defaultBoneInfo);
				bodyA->m_localToRig = defaultBoneInfo.m_centerOfMassTransform;
				bodyA->m_rigToLocal = defaultBoneInfo.m_centerOfMassTransform.inverse();
				bodyA->m_marginMultipler = defaultBoneInfo.m_marginMultipler;
				bodyA->m_gravityFactor = defaultBoneInfo.m_gravityFactor;
				bodyA->readTransform(0);
				m_mesh->m_bones.push_back(bodyA);
			}
			else
			{
				Warning("constraint %s <-> %s : bodyA doesn't exist, skipped", bodyAName->cstr(), bodyBName->cstr());
				m_reader->skipCurrentElement();
				return false;
			}
		}
		if (!bodyB)
		{
			auto node = findObjectByName(bodyBName);
			if (node)
			{
				auto defaultBoneInfo = getBoneTemplate("");
				bodyB = new SkyrimBone(node->m_name, node, defaultBoneInfo);
				bodyB->m_localToRig = defaultBoneInfo.m_centerOfMassTransform;
				bodyB->m_rigToLocal = defaultBoneInfo.m_centerOfMassTransform.inverse();
				bodyB->m_marginMultipler = defaultBoneInfo.m_marginMultipler;
				bodyB->m_gravityFactor = defaultBoneInfo.m_gravityFactor;
				bodyB->readTransform(0);
				m_mesh->m_bones.push_back(bodyB);
			}
			else
			{
				Warning("constraint %s <-> %s : bodyB doesn't exist, skipped", bodyAName->cstr(), bodyBName->cstr());
				m_reader->skipCurrentElement();
				return false;
			}
		}
		if (bodyA == bodyB)
		{
			Warning("constraint between same object %s <-> %s, skipped", bodyAName->cstr(), bodyBName->cstr());
			m_reader->skipCurrentElement();
			return false;
		}

		if (bodyA->m_rig.isKinematicObject() && bodyB->m_rig.isKinematicObject())
		{
			Warning("constraint between two kinematic object %s <-> %s, skipped", bodyAName->cstr(), bodyBName->cstr());
			m_reader->skipCurrentElement();
			return false;
		}

		return true;
	}

	btQuaternion rotFromAtoB(const btVector3& a, const btVector3& b)
	{
		auto axis = a.cross(b);
		if (axis.fuzzyZero()) return btQuaternion::getIdentity();
		float sinA = axis.length();
		float cosA = a.dot(b);
		float angle = btAtan2(cosA, sinA);
		return btQuaternion(axis, angle);
	}

	void SkyrimMeshParser::calcFrame(FrameType type, const btTransform& frame, const btQsTransform& trA, const btQsTransform& trB, btTransform& frameA, btTransform& frameB)
	{
		btQsTransform frameInWorld;
		switch (type)
		{
		case FrameInA:
			frameA = frame;
			frameInWorld = trA * frame;
			frameB = (trB.inverse() * frameInWorld).asTransform();
			break;
		case FrameInB:
			frameB = frame;
			frameInWorld = trB * frameB;
			frameA = (trA.inverse() * frameInWorld).asTransform();
			break;
		case FrameInLerp:
		{
			auto trans = trA.getOrigin().lerp(trB.getOrigin(), frame.getOrigin().x());
			auto rot = trA.getBasis().slerp(trB.getBasis(), frame.getOrigin().y());
			frameInWorld = btQsTransform(rot, trans);
			frameA = (trA.inverse() * frameInWorld).asTransform();
			frameB = (trB.inverse() * frameInWorld).asTransform();
			break;
		}
		case AWithXPointToB:
		{
			btMatrix3x3 matr(trA.getBasis());
			frameInWorld = trA;
			auto old = matr.getColumn(0).normalized();
			auto a2b = (trB.getOrigin() - trA.getOrigin()).normalized();
			auto q = rotFromAtoB(old, a2b);
			frameInWorld.getBasis() *= q;
			frameA = (trA.inverse() * frameInWorld).asTransform();
			frameB = (trB.inverse() * frameInWorld).asTransform();
			break;
		}
		case AWithYPointToB:
		{
			btMatrix3x3 matr(trA.getBasis());
			frameInWorld = trA;
			auto old = matr.getColumn(1).normalized();
			auto a2b = (trB.getOrigin() - trA.getOrigin()).normalized();
			auto q = rotFromAtoB(old, a2b);
			frameInWorld.getBasis() *= q;
			frameA = (trA.inverse() * frameInWorld).asTransform();
			frameB = (trB.inverse() * frameInWorld).asTransform();
			break;
		}
		case AWithZPointToB:
		{
			btMatrix3x3 matr(trA.getBasis());
			frameInWorld = trA;
			auto old = matr.getColumn(2).normalized();
			auto a2b = (trB.getOrigin() - trA.getOrigin()).normalized();
			auto q = rotFromAtoB(old, a2b);
			frameInWorld.getBasis() *= q;
			frameA = (trA.inverse() * frameInWorld).asTransform();
			frameB = (trB.inverse() * frameInWorld).asTransform();
			break;
		}


		}
	}

	Ref<Generic6DofConstraint> SkyrimMeshParser::readGenericConstraint()
	{
		auto bodyAName = getRenamedBone(m_reader->getAttribute("bodyA"));
		auto bodyBName = getRenamedBone(m_reader->getAttribute("bodyB"));
		auto clsname = m_reader->getAttribute("template", "");

		SkyrimBone *bodyA, *bodyB;
		if (!findBones(bodyAName, bodyBName, bodyA, bodyB))
			return nullptr;

		auto trA = bodyA->m_currentTransform;
		auto trB = bodyB->m_currentTransform;

		auto cinfo = getGenericConstraintTemplate(clsname);
		readGenericConstraintTemplate(cinfo);
		btTransform frameA, frameB;
		calcFrame(cinfo.frameType, cinfo.frame, trA, trB, frameA, frameB);

		Ref<Generic6DofConstraint> constraint;
		if (cinfo.useLinearReferenceFrameA)
			constraint = new Generic6DofConstraint(bodyB, bodyA, frameB, frameA);
		else
			constraint = new Generic6DofConstraint(bodyA, bodyB, frameA, frameB);

		constraint->setLinearLowerLimit(cinfo.linearLowerLimit);
		constraint->setLinearUpperLimit(cinfo.linearUpperLimit);
		constraint->setAngularLowerLimit(cinfo.angularLowerLimit);
		constraint->setAngularUpperLimit(cinfo.angularUpperLimit);
		for (int i = 0; i < 3; ++i)
		{
			constraint->setStiffness(i, cinfo.linearStiffness[i]);
			constraint->setStiffness(i + 3, cinfo.angularStiffness[i]);
			constraint->setDamping(i, cinfo.linearDamping[i]);
			constraint->setDamping(i + 3, cinfo.angularDamping[i]);
			constraint->setEquilibriumPoint(i, cinfo.linearEquilibrium[i]);
			constraint->setEquilibriumPoint(i + 3, cinfo.angularEquilibrium[i]);
		}
		constraint->getTranslationalLimitMotor()->m_bounce = cinfo.linearBounce;
		constraint->getRotationalLimitMotor(0)->m_bounce = cinfo.angularBounce[0];
		constraint->getRotationalLimitMotor(1)->m_bounce = cinfo.angularBounce[1];
		constraint->getRotationalLimitMotor(2)->m_bounce = cinfo.angularBounce[2];
		/*constraint->getTranslationalLimitMotor()->m_limitSoftness = 1;
		constraint->getRotationalLimitMotor(0)->m_limitSoftness = 1;
		constraint->getRotationalLimitMotor(1)->m_limitSoftness = 1;
		constraint->getRotationalLimitMotor(2)->m_limitSoftness = 1;*/

		return constraint;
	}

	void SkyrimMeshParser::readStiffSpringConstraintTemplate(StiffSpringConstraintTemplate& dest)
	{
		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (name == "minDistanceFactor")
					dest.minDistanceFactor = std::max(m_reader->readFloat(), 0.0f);
				else if (name == "maxDistanceFactor")
					dest.maxDistanceFactor = std::max(m_reader->readFloat(), 0.0f);
				else if (name == "stiffness")
					dest.stiffness = std::max(m_reader->readFloat(), 0.0f);
				else if (name == "damping")
					dest.damping = std::max(m_reader->readFloat(), 0.0f);
				else if (name == "equilibrium")
					dest.equilibriumFactor = btClamped(m_reader->readFloat(), 0.0f, 1.0f);
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}
	}

	void SkyrimMeshParser::readConeTwistConstraintTemplate(ConeTwistConstraintTemplate& dest)
	{
		while (m_reader->Inspect())
		{
			if (m_reader->GetInspected() == XMLReader::Inspected::StartTag)
			{
				auto name = m_reader->GetName();
				if (parseFrameType(name, dest.frameType, dest.frame));
				else if (name == "angularOnly")
					dest.angularOnly = m_reader->readBool();
				else if (name == "swingSpan1" || name == "coneLimit" || name == "limitZ")
					dest.swingSpan1 = std::max(m_reader->readFloat(), 0.f);
				else if (name == "swingSpan2" || name == "planeLimit" || name == "limitY")
					dest.swingSpan2 = std::max(m_reader->readFloat(), 0.f);
				else if (name == "twistSpan" || name == "twistLimit" || name == "limitX")
					dest.twistSpan = std::max(m_reader->readFloat(), 0.f);
				else if (name == "limitSoftness")
					dest.limitSoftness = btClamped(m_reader->readFloat(), 0.f, 1.f);
				else if (name == "biasFactor")
					dest.biasFactor = btClamped(m_reader->readFloat(), 0.f, 1.f);
				else if (name == "relaxationFactor")
					dest.relaxationFactor = btClamped(m_reader->readFloat(), 0.f, 1.f);
				else
				{
					Warning("unknown element - %s", name.c_str());
					m_reader->skipCurrentElement();
				}
			}
			else if (m_reader->GetInspected() == XMLReader::Inspected::EndTag)
				break;
		}
	}

	const SkyrimMeshParser::BoneTemplate & SkyrimMeshParser::getBoneTemplate(const IDStr & name)
	{
		auto iter = m_boneTemplates.find(name);
		if (iter == m_boneTemplates.end())
			return m_boneTemplates[""];
		return iter->second;
	}

	const SkyrimMeshParser::GenericConstraintTemplate & SkyrimMeshParser::getGenericConstraintTemplate(const IDStr & name)
	{
		auto iter = m_genericConstraintTemplates.find(name);
		if (iter == m_genericConstraintTemplates.end())
			return m_genericConstraintTemplates[""];
		return iter->second;
	}

	const SkyrimMeshParser::StiffSpringConstraintTemplate & SkyrimMeshParser::getStiffSpringConstraintTemplate(const IDStr & name)
	{
		auto iter = m_stiffSpringConstraintTemplates.find(name);
		if (iter == m_stiffSpringConstraintTemplates.end())
			return m_stiffSpringConstraintTemplates[""];
		return iter->second;
	}

	const SkyrimMeshParser::ConeTwistConstraintTemplate & SkyrimMeshParser::getConeTwistConstraintTemplate(const IDStr & name)
	{
		auto iter = m_coneTwistConstraintTemplates.find(name);
		if (iter == m_coneTwistConstraintTemplates.end())
			return m_coneTwistConstraintTemplates[""];
		return iter->second;
	}

	Ref<StiffSpringConstraint> SkyrimMeshParser::readStiffSpringConstraint()
	{
		auto bodyAName = getRenamedBone(m_reader->getAttribute("bodyA"));
		auto bodyBName = getRenamedBone(m_reader->getAttribute("bodyB"));
		auto clsname = m_reader->getAttribute("template", "");

		SkyrimBone *bodyA, *bodyB;
		if (!findBones(bodyAName, bodyBName, bodyA, bodyB))
			return nullptr;

		StiffSpringConstraintTemplate cinfo = getStiffSpringConstraintTemplate(clsname);
		readStiffSpringConstraintTemplate(cinfo);

		Ref<StiffSpringConstraint> constraint = new StiffSpringConstraint(bodyA, bodyB);
		constraint->m_minDistance *= cinfo.minDistanceFactor;
		constraint->m_maxDistance *= cinfo.maxDistanceFactor;
		constraint->m_stiffness = cinfo.stiffness;
		constraint->m_damping = cinfo.damping;
		constraint->m_equilibriumPoint = constraint->m_minDistance * cinfo.equilibriumFactor + constraint->m_maxDistance * (1 - cinfo.equilibriumFactor);
		return constraint;
	}

	Ref<ConeTwistConstraint> SkyrimMeshParser::readConeTwistConstraint()
	{
		auto bodyAName = getRenamedBone(m_reader->getAttribute("bodyA"));
		auto bodyBName = getRenamedBone(m_reader->getAttribute("bodyB"));
		auto clsname = m_reader->getAttribute("template", "");

		SkyrimBone *bodyA = nullptr, *bodyB = nullptr;
		if (!findBones(bodyAName, bodyBName, bodyA, bodyB))
			return nullptr;

		auto trA = bodyA->m_currentTransform;
		auto trB = bodyB->m_currentTransform;

		auto cinfo = getConeTwistConstraintTemplate(clsname);
		readConeTwistConstraintTemplate(cinfo);
		btTransform frameA, frameB;
		calcFrame(cinfo.frameType, cinfo.frame, trA, trB, frameA, frameB);

		Ref<ConeTwistConstraint> constraint = new ConeTwistConstraint(bodyA, bodyB, frameA, frameB);
		constraint->setLimit(cinfo.swingSpan1, cinfo.swingSpan2, cinfo.twistSpan, cinfo.limitSoftness, cinfo.biasFactor, cinfo.relaxationFactor);
		constraint->setAngularOnly(cinfo.angularOnly);

		return constraint;
	}
}
