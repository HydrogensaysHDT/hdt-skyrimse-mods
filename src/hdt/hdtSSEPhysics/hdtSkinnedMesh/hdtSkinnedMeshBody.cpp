#include "hdtSkinnedMeshBody.h"
#include "hdtSkinnedMeshShape.h"

#include <ppl.h>

namespace hdt
{
#ifdef ENABLE_CL

	static std::string sourceCode = R"__KERNEL(
typedef struct Matrix4x3
{
	float4 r[3];
} Matrix4x3;

float4 mul(Matrix4x3 a, float4 b)
{
	float4 ret = {dot(a.r[0], b), dot(a.r[1], b), dot(a.r[2], b), 1};
	return ret;
}

typedef struct Vertex
{
	float4 skinPos;
	float weight[4];
	uint boneIdx[4];
} Vertex;

typedef struct Bone
{
	Matrix4x3	t;
	float		margin;
	float		reserved[3];
}Bone;

__kernel void updateVertices(
	__global Bone* bones,
	__global Vertex* vertices,
	__global float4* out)
{
	int idx = get_global_id(0);
	Vertex v = vertices[idx];
	float4 pos = 0;
	for(int i=0; i<4; ++i)
	{
		if(v.weight[i] > FLT_EPSILON)
		{
			float4 p = {v.skinPos.x, v.skinPos.y, v.skinPos.z, 1};
			p = mul(bones[v.boneIdx[i]].t, p);
			p.w = bones[v.boneIdx[i]].margin;
			p *= v.weight[i];
			pos += p;
		}
	}
	out[idx] = pos;
}
	)__KERNEL";

	hdtCLKernel SkinnedMeshBody::m_kernel;

	void SkinnedMeshBody::internalUpdateCL()
	{
		for (int i = 0; i < m_skinnedBones.size(); ++i)
		{
			auto boneT = m_skinnedBones[i]->m_currentTransform;
			m_bones[i].m_vertexToWorld = boneT * m_vertexToBone[i];
			m_bones[i].m_maginMultipler = m_skinnedBones[i]->m_marginMultipler * boneT.getScale();
		}

		auto cl = hdtCL::instance();
		if (cl && !m_bonesCL())
		{
			m_bonesCL = cl->createBuffer(sizeof(Bone)* m_bones.size(), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, nullptr);
			m_verticesCL = cl->createBuffer(sizeof(Vertex)* m_vertices.size(), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, nullptr);
			m_vposCL = cl->createBuffer(sizeof(VertexPos)* m_vpos.size(), CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, nullptr);
			cl->writeBuffer(m_verticesCL, m_vertices.data(), sizeof(Vertex)* m_vertices.size(), true);
		}

		auto e0 = cl->writeBufferE(m_bonesCL, m_bones.data(), m_bones.size() * sizeof(Bone));
		m_kernel.setArg(0, m_bonesCL);
		m_kernel.setArg(1, m_verticesCL);
		m_kernel.setArg(2, m_vposCL);
		auto e1 = m_kernel.runE({ m_vertices.size() }, { e0 });
		m_eDoneCL = cl->readBufferE(m_vpos.data(), m_vposCL, m_vpos.size() * sizeof(VertexPos), { e1 });
	}

#endif

	SkinnedMeshBody::SkinnedMeshBody()
	{
#ifdef ENABLE_CL
		auto cl = hdtCL::instance();
		if (cl)
		{
			if (!m_kernel())
			{
				m_kernel.lock();
				if (!m_kernel())
					m_kernel = hdtCLKernel(cl->compile(sourceCode), "updateVertices");
				m_kernel.unlock();
			}
		}
#endif
		m_collisionShape = &m_bulletShape;
	}

	SkinnedMeshBody::~SkinnedMeshBody()
	{
		//m_bonesGPU.discard_data();
		//m_verticesGPU.discard_data();
	}

	__forceinline __m128 calcVertexState(__m128 skinPos, const Bone& bone, __m128 w)
	{
		auto p = bone.m_vertexToWorld * skinPos;
		p = _mm_blend_ps(p.get128(), _mm_load_ps(bone.m_reserved), 0x8);
		return _mm_mul_ps(w, p.get128());
	}

	void SkinnedMeshBody::internalUpdate()
	{
		for (size_t i = 0; i < m_skinnedBones.size(); ++i)
		{
			auto& v = m_skinnedBones[i];
			auto boneT = v.ptr->m_currentTransform;
			m_bones[i].m_vertexToWorld = btMatrix4x3T(boneT) * v.vertexToBone;
			m_bones[i].m_maginMultipler = v.ptr->m_marginMultipler * boneT.getScale();
		}

		int size = m_vpos.size();

		for (int idx = 0; idx < size; ++idx)
		{
			auto& v = m_vertices[idx];
			auto p = v.m_skinPos.get128();
			auto w = _mm_load_ps(v.m_weight);
			auto flg = _mm_movemask_ps(_mm_cmplt_ps(_mm_set_ps1(FLT_EPSILON), w));
			auto posMargin = calcVertexState(p, m_bones[v.getBoneIdx(0)], setAll0(w));
			if (flg & 0b0010) posMargin += calcVertexState(p, m_bones[v.getBoneIdx(1)], setAll1(w));
			if (flg & 0b0100) posMargin += calcVertexState(p, m_bones[v.getBoneIdx(2)], setAll2(w));
			if (flg & 0b1000) posMargin += calcVertexState(p, m_bones[v.getBoneIdx(3)], setAll3(w));
			m_vpos[idx].set(posMargin);
		}

		m_shape->internalUpdate();
		m_bulletShape.m_aabb = m_shape->m_tree.aabbAll;
	}

	float SkinnedMeshBody::flexible(const Vertex& v)
	{
		float ret = 0.f;
		for (int i = 0; i < 4; ++i)
		{
			if (v.m_weight[i] < FLT_EPSILON) break;
			int boneIdx = v.getBoneIdx(i);
			if (!m_skinnedBones[boneIdx].isKinematic)
				ret += v.m_weight[i];
		}
		return ret;
	}

	int SkinnedMeshBody::addBone(SkinnedMeshBone * bone, const btQsTransform & verticesToBone, const BoundingSphere& boundingSphere)
	{
		m_skinnedBones.push_back(SkinnedBone());
		auto& v = m_skinnedBones.back();
		v.ptr = bone;
		v.vertexToBone = verticesToBone;
		v.localBoundingSphere = boundingSphere;
		v.isKinematic = bone->m_rig.isStaticOrKinematicObject();
		return static_cast<int>(m_skinnedBones.size() - 1);
	}

	void SkinnedMeshBody::finishBuild()
	{
		m_bones.resize(m_skinnedBones.size());

		m_shape->clipColliders();
		m_vpos.resize(m_vertices.size());
		
		m_isKinematic = true;
		for (auto& i : m_skinnedBones)
		{
			i.isKinematic = i.ptr->m_rig.isStaticOrKinematicObject();
			if (!i.isKinematic)
				m_isKinematic = false;
		}
		
		m_shape->finishBuild();

		bool* flags = new bool[m_vertices.size()];
		ZeroMemory(flags, m_vertices.size());
		m_shape->markUsedVertices(flags);

		UINT numUsed = 0;
		std::vector<UINT> map(m_vertices.size());
		for (int i = 0; i < m_vertices.size(); ++i)
		{
			if (flags[i])
			{
				m_vertices[numUsed] = m_vertices[i];
				m_vpos[numUsed] = m_vpos[i];
				map[i] = numUsed++;
			}
		}
		delete[] flags;
		m_shape->remapVertices(map.data());
		m_vertices.resize(numUsed);
		m_vpos.resize(numUsed);

		m_useBoundingSphere = m_shape->m_colliders.size() > 10;
	}

	bool SkinnedMeshBody::canCollideWith(const SkinnedMeshBody* body) const
	{
		if (m_isKinematic && body->m_isKinematic) return false;

		if (m_canCollideWithTags.empty())
		{
			for (auto& i : body->m_tags)
				if (m_noCollideWithTags.find(i) != m_noCollideWithTags.end())
					return false;
			return true;
		}
		else
		{
			for (auto& i : body->m_tags)
				if (m_canCollideWithTags.find(i) != m_canCollideWithTags.end())
					return true;
			return false;
		}
	}

	void SkinnedMeshBody::updateBoundingSphereAabb()
	{
		m_bulletShape.m_aabb.invalidate();
		for (auto& i : m_skinnedBones)
		{
			auto sp = i.localBoundingSphere;
			auto tr = i.ptr->m_currentTransform;
			i.worldBoundingSphere = BoundingSphere(tr * sp.center(), tr.getScale() * sp.radius());
			m_bulletShape.m_aabb.merge(i.worldBoundingSphere.getAabb());
		}

		if (!m_useBoundingSphere)
			internalUpdate();
	}

	bool SkinnedMeshBody::isBoundingSphereCollided(SkinnedMeshBody * rhs)
	{
		if (canCollideWith(rhs) && rhs->canCollideWith(this))
		{
			return true;
			//if (m_useBoundingSphere && rhs->m_useBoundingSphere)
			//{
			//	for (auto& i : m_skinnedBones)
			//	{
			//		for (auto& j : rhs->m_skinnedBones)
			//		{
			//			if (i.isKinematic && j.isKinematic)
			//				continue;
			//			if (i.worldBoundingSphere.isCollide(j.worldBoundingSphere))
			//				return true;
			//		}
			//	}
			//}
			//else if (m_useBoundingSphere)
			//{
			//	for (auto& i : m_skinnedBones)
			//	{
			//		if (rhs->m_bulletShape.m_aabb.collideWithSphere(i.worldBoundingSphere.center(), i.worldBoundingSphere.radius()))
			//			return true;
			//	}
			//}
			//else if (rhs->m_useBoundingSphere)
			//{
			//	for (auto& i : rhs->m_skinnedBones)
			//	{
			//		if (m_bulletShape.m_aabb.collideWithSphere(i.worldBoundingSphere.center(), i.worldBoundingSphere.radius()))
			//			return true;
			//	}
			//}
			//else return true;
		}
		return false;
	}
}