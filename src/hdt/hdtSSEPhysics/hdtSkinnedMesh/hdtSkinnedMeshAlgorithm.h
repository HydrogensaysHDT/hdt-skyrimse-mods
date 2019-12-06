#pragma once

#include "hdtSkinnedMeshShape.h"
#include "hdtDispatcher.h"

namespace hdt
{
	class SkinnedMeshAlgorithm : public btCollisionAlgorithm
	{
	public:
		SkinnedMeshAlgorithm(const btCollisionAlgorithmConstructionInfo& ci);

		virtual void processCollision(const btCollisionObjectWrapper* body0Wrap, const btCollisionObjectWrapper* body1Wrap, const btDispatcherInfo& dispatchInfo, btManifoldResult* resultOut){}
		virtual btScalar calculateTimeOfImpact(btCollisionObject* body0, btCollisionObject* body1, const btDispatcherInfo& dispatchInfo, btManifoldResult* resultOut){ return 1; }	// TOI cost too much
		virtual	void getAllContactManifolds(btManifoldArray& manifoldArray){}

		struct CreateFunc :public 	btCollisionAlgorithmCreateFunc
		{
			virtual	btCollisionAlgorithm* CreateCollisionAlgorithm(btCollisionAlgorithmConstructionInfo& ci, const btCollisionObjectWrapper* body0Wrap, const btCollisionObjectWrapper* body1Wrap)
			{
				void* mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SkinnedMeshAlgorithm));
				return new (mem) SkinnedMeshAlgorithm(ci);
			}
		};

		static void registerAlgorithm(btCollisionDispatcher * dispatcher);

		static const int MaxCollisionCount = 256;

		static void processCollision(SkinnedMeshBody* body0Wrap, SkinnedMeshBody* body1Wrap, CollisionDispatcher* dispatcher);
	protected:
		
		struct CollisionMerge
		{
			btVector3 normal;
			btVector3 pos[2];
			float weight;

			inline CollisionMerge()
			{
				_mm_store_ps(((float*)this), _mm_setzero_ps());
				_mm_store_ps(((float*)this) + 4, _mm_setzero_ps());
				_mm_store_ps(((float*)this) + 8, _mm_setzero_ps());
				_mm_store_ps(((float*)this) + 12, _mm_setzero_ps());
			}
		};

		struct MergeBuffer
		{
			MergeBuffer(){ mergeStride = mergeSize = 0; buffer = 0; }

			inline CollisionMerge* begin() const { return buffer; }
			inline CollisionMerge* end() const { return buffer + mergeSize; }

			void alloc(int x, int y){ mergeStride = y; mergeSize = x*y; buffer = new CollisionMerge[mergeSize]; }
			void release(){ if (buffer) delete[] buffer; }
			
			CollisionMerge* get(int x, int y){ return &buffer[x * mergeStride + y]; }

			void doMerge(SkinnedMeshShape* shape0, SkinnedMeshShape* shape1, CollisionResult* collisions, int count);
			void apply(SkinnedMeshBody* body0, SkinnedMeshBody* body1, CollisionDispatcher* dispatcher);
			
			int mergeStride;
			int mergeSize;
			CollisionMerge* buffer;
		};

		template<class T0, class T1> static void processCollision(T0* shape0, T1* shape1, MergeBuffer& merge, CollisionResult* collision);
	};

}