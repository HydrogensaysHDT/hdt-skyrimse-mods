#pragma once

#include "stdafx.h"
#include "hdtConvertNi.h"
#include "hdtSkyrimBone.h"
#include "hdtSkyrimShape.h"
#include "hdtSkinnedMesh\hdtSkinnedMeshSystem.h"
#include "hdtSkinnedMesh\hdtGeneric6DofConstraint.h"
#include "hdtSkinnedMesh\hdtStiffSpringConstraint.h"
#include "hdtSkinnedMesh\hdtConeTwistConstraint.h"

namespace hdt
{
	class SkyrimMesh : public SkinnedMeshSystem
	{
	public:
		SkyrimMesh(NiNode* skeleton);

		SkinnedMeshBone* findBone(IDStr name);
		SkinnedMeshBody* findBody(IDStr name);
		int findBoneIdx(hdt::IDStr name);

		virtual void readTransform(float timeStep);
		virtual void writeTransform();

		Ref<NiNode> m_skeleton;
		Ref<NiNode> m_oldRoot;
		bool	m_initialized = false;

		// angular velocity damper
		btQuaternion m_lastRootRotation;
	};

	class XMLReader;
	class SkyrimMeshParser
	{
	public:
		SkyrimMeshParser();
		Ref<SkyrimMesh> createMesh(NiNode* skeleton, NiAVObject* model, const std::string& filepath, std::unordered_map<IDStr, IDStr> renameMap);

	protected:

		IDStr getRenamedBone(IDStr name);

		Ref<SkyrimMesh> m_mesh;
		NiNode* m_skeleton;
		NiAVObject* m_model;
		XMLReader* m_reader;
		std::unordered_map<IDStr, IDStr> m_renameMap;

		NiNode* findObjectByName(const hdt::IDStr& name);
		SkyrimBone* getOrCreateBone(const hdt::IDStr& name);

		std::string m_filePath;

		bool findBones(const IDStr& bodyAName, const IDStr& bodyBName, SkyrimBone*& bodyA, SkyrimBone*& bodyB);

		struct BoneTemplate : public btRigidBody::btRigidBodyConstructionInfo
		{
			static btEmptyShape emptyShape[1];
			BoneTemplate() :btRigidBodyConstructionInfo(0, 0, emptyShape) {
				m_centerOfMassTransform = btTransform::getIdentity();
				m_marginMultipler = 1.f;
			}

			std::shared_ptr<btCollisionShape> m_shape;
			std::vector<hdt::IDStr> m_canCollideWithBone;
			std::vector<hdt::IDStr> m_noCollideWithBone;
			btTransform m_centerOfMassTransform;
			float m_marginMultipler;
			float m_gravityFactor = 1.0f;
			U32 m_collisionFilter = 0;
		};
		std::unordered_map<IDStr, BoneTemplate> m_boneTemplates;

		enum FrameType
		{
			FrameInA,
			FrameInB,
			FrameInLerp,
			AWithXPointToB,
			AWithYPointToB,
			AWithZPointToB
		};

		bool parseFrameType(const std::string& name, FrameType& type, btTransform& frame);
		static void calcFrame(FrameType type, const btTransform& frame, const btQsTransform& trA, const btQsTransform& trB, btTransform& frameA, btTransform& frameB);

		struct GenericConstraintTemplate
		{
			FrameType frameType = FrameInB;
			bool useLinearReferenceFrameA = false;
			btTransform frame = btTransform::getIdentity();
			btVector3 linearLowerLimit = btVector3(1, 1, 1);
			btVector3 linearUpperLimit = btVector3(-1, -1, -1);
			btVector3 angularLowerLimit = btVector3(1, 1, 1);
			btVector3 angularUpperLimit = btVector3(-1, -1, -1);
			btVector3 linearStiffness = btVector3(0, 0, 0);
			btVector3 angularStiffness = btVector3(0, 0, 0);
			btVector3 linearDamping = btVector3(0, 0, 0);
			btVector3 angularDamping = btVector3(0, 0, 0);
			btVector3 linearEquilibrium = btVector3(0, 0, 0);
			btVector3 angularEquilibrium = btVector3(0, 0, 0);
			btVector3 linearBounce = btVector3(0, 0, 0);
			btVector3 angularBounce = btVector3(0, 0, 0);
		};
		std::unordered_map<IDStr, GenericConstraintTemplate> m_genericConstraintTemplates;

		struct StiffSpringConstraintTemplate
		{
			float minDistanceFactor = 1;
			float maxDistanceFactor = 1;
			float stiffness = 0;
			float damping = 0;
			float equilibriumFactor = 0.5;
		};
		std::unordered_map<IDStr, StiffSpringConstraintTemplate> m_stiffSpringConstraintTemplates;

		struct ConeTwistConstraintTemplate
		{
			btTransform frame = btTransform::getIdentity();
			FrameType frameType = FrameInB;
			bool angularOnly = false;
			float swingSpan1 = 0;
			float swingSpan2 = 0;
			float twistSpan = 0;
			float limitSoftness = 1.0f;
			float biasFactor = 0.3f;
			float relaxationFactor = 1.0f;
		};
		std::unordered_map<IDStr, ConeTwistConstraintTemplate> m_coneTwistConstraintTemplates;
		std::unordered_map<IDStr, std::shared_ptr<btCollisionShape>> m_shapes;

		Ref<SkyrimShape> generateMeshBody(const std::string& name);

		void readFrameLerp(btTransform& tr);
		void readBoneTemplate(BoneTemplate& dest);
		void readGenericConstraintTemplate(GenericConstraintTemplate& dest);
		void readStiffSpringConstraintTemplate(StiffSpringConstraintTemplate& dest);
		void readConeTwistConstraintTemplate(ConeTwistConstraintTemplate& dest);

		const BoneTemplate& getBoneTemplate(const IDStr& name);
		const GenericConstraintTemplate& getGenericConstraintTemplate(const IDStr& name);
		const StiffSpringConstraintTemplate& getStiffSpringConstraintTemplate(const IDStr& name);
		const ConeTwistConstraintTemplate& getConeTwistConstraintTemplate(const IDStr& name);

		void readBone();
		Ref<SkyrimShape> readPerVertexShape();
		Ref<SkyrimShape> readPerTriangleShape();
		Ref<Generic6DofConstraint> readGenericConstraint();
		Ref<StiffSpringConstraint> readStiffSpringConstraint();
		Ref<ConeTwistConstraint> readConeTwistConstraint();
		Ref<ConstraintGroup> readConstraintGroup();
		std::shared_ptr<btCollisionShape> readShape();

		template<typename ... Args> void Error(const char* fmt, Args ... args);
		template<typename ... Args> void Warning(const char* fmt, Args ... args);

		std::vector<std::shared_ptr<btCollisionShape>> m_shapeRefs;
	};
}
