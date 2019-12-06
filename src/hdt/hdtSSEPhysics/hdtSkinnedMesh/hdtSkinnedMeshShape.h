#pragma once

#include "hdtCollisionAlgorithm.h"
#include "hdtCollider.h"
#include "hdtSkinnedMeshBody.h"

namespace hdt
{
	class PerVertexShape;
	class PerTriangleShape;

	class SkinnedMeshShape : public RefObject
	{
	public:
		BT_DECLARE_ALIGNED_ALLOCATOR();

		SkinnedMeshShape(SkinnedMeshBody* body);
		virtual ~SkinnedMeshShape();

		virtual PerVertexShape* asPerVertexShape(){ return 0; }
		virtual PerTriangleShape* asPerTriangleShape(){ return 0; }

		inline const Aabb& getAabb() const { return m_tree.aabbAll; }

		virtual void clipColliders();
		virtual void finishBuild() = 0;
		virtual void internalUpdate() = 0;
		virtual int getBonePerCollider() = 0;
		virtual void markUsedVertices(bool* flags) = 0;
		virtual void remapVertices(UINT* map) = 0;
		
		//inline int getNumColliders(){ return m_colliders.size(); };
		virtual float getColliderBoneWeight(const Collider* c, int boneIdx) = 0;
		virtual int getColliderBoneIndex(const Collider* c, int boneIdx) = 0;
		virtual btVector3 baryCoord(const Collider* c, const btVector3& p) = 0;
		virtual float baryWeight(const btVector3& w, int boneIdx) = 0;

		//inline float getColliderBoneWeightByIdx(int cidx, int boneIdx){ return getColliderBoneWeight(&m_colliders[cidx], boneIdx); }
		//inline int getColliderBoneIndexByIdx(int cidx, int boneIdx){ return getColliderBoneIndex(&m_colliders[cidx], boneIdx); }

		SkinnedMeshBody*	m_owner;
		vectorA16<Aabb>		m_aabb;
		vectorA16<Collider> m_colliders;
		ColliderTree		m_tree;
		float				m_windEffect = 0.f;

#ifdef ENABLE_CL
		cl::Buffer		m_aabbCL;
		cl::Buffer		m_colliderCL;
		cl::Event		m_eDoneCL;
		virtual void internalUpdateCL() = 0;
#endif
	};

	class PerVertexShape : public SkinnedMeshShape
	{
	public:
		PerVertexShape(SkinnedMeshBody* body);
		virtual ~PerVertexShape();

		virtual PerVertexShape* asPerVertexShape(){ return this; }

		virtual void internalUpdate() override;
		virtual int getBonePerCollider() override { return 4; }
		virtual float getColliderBoneWeight(const Collider* c, int boneIdx) override { return m_owner->m_vertices[c->vertex].m_weight[boneIdx]; }
		virtual int getColliderBoneIndex(const Collider* c, int boneIdx) override { return m_owner->m_vertices[c->vertex].getBoneIdx(boneIdx); }
		virtual btVector3 baryCoord(const Collider* c, const btVector3& p) override { return btVector3(1, 1, 1); }
		virtual float baryWeight(const btVector3& w, int boneIdx) override { return 1; }
		virtual void finishBuild() override;
		virtual void markUsedVertices(bool* flags) override;
		virtual void remapVertices(UINT* map) override;

		void autoGen();

		struct ShapeProp
		{
			float margin = 1.0f;
		} m_shapeProp;

#ifdef ENABLE_CL
		static hdtCLKernel		m_kernel;
		virtual void internalUpdateCL();
#endif
	};


	class PerTriangleShape : public SkinnedMeshShape
	{
	public:
		PerTriangleShape(SkinnedMeshBody* body);
		virtual ~PerTriangleShape();

		virtual PerVertexShape* asPerVertexShape(){ return m_verticesCollision; }
		virtual PerTriangleShape* asPerTriangleShape(){ return this; }

		virtual void internalUpdate() override;
		virtual int getBonePerCollider() override  { return 12; }
		virtual float getColliderBoneWeight(const Collider* c, int boneIdx) override {
			return m_owner->m_vertices[c->vertices[boneIdx/4]].m_weight[boneIdx%4];
		}
		virtual int getColliderBoneIndex(const Collider* c, int boneIdx) override {
			return m_owner->m_vertices[c->vertices[boneIdx / 4]].getBoneIdx(boneIdx % 4);
		}
		virtual btVector3 baryCoord(const Collider* c, const btVector3& p) override {
			return BaryCoord(m_owner->m_vpos[c->vertices[0]].pos(), m_owner->m_vpos[c->vertices[1]].pos(), m_owner->m_vpos[c->vertices[2]].pos(), p);
		}
		virtual float baryWeight(const btVector3& w, int boneIdx) override { return w[boneIdx / 4]; }
		virtual void finishBuild() override;
		virtual void markUsedVertices(bool* flags) override;
		virtual void remapVertices(UINT* map) override;

		void addTriangle(int p0, int p1, int p2);

		struct ShapeProp
		{
			float margin = 1.0f;
			float penetration = 1.f;
		} m_shapeProp;
		
		Ref<PerVertexShape> m_verticesCollision;

#ifdef ENABLE_CL
		static hdtCLKernel		m_kernel;
		virtual void internalUpdateCL();
#endif
	};
}