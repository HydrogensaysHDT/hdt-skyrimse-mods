#include "hdtSkinnedMeshAlgorithm.h"
#include "hdtCollider.h"

namespace hdt
{

	SkinnedMeshAlgorithm::SkinnedMeshAlgorithm(const btCollisionAlgorithmConstructionInfo& ci)
		: btCollisionAlgorithm(ci)
	{
	}
	
	static const CollisionResult zero;

	template<class T0, class T1>
	struct CollisionCheck
	{
		typedef typename T0::ShapeProp SP0;
		typedef typename T1::ShapeProp SP1;

		CollisionCheck(T0* a, T1* b, CollisionResult* r)
		{
			v0 = a->m_owner->m_vpos.data();
			v1 = b->m_owner->m_vpos.data();
			c0 = &a->m_tree;
			c1 = &b->m_tree;
			sp0 = &a->m_shapeProp;
			sp1 = &b->m_shapeProp;
			results = r;
			numResults = 0;
		}

		VertexPos* v0;
		VertexPos* v1;
		ColliderTree* c0;
		ColliderTree* c1;
		SP0* sp0;
		SP1* sp1;

		std::atomic_long numResults;
		CollisionResult* results;

		bool checkCollide(Collider* a, Collider* b, CollisionResult& res);
		
		bool addResult(const CollisionResult& res)
		{
			int p = numResults.fetch_add(1);
			if (p < SkinnedMeshAlgorithm::MaxCollisionCount)
			{
				results[p] = res;
				return true;
			}
			else return false;
		}

		int operator()()
		{
			std::vector<std::pair<ColliderTree*, ColliderTree*>> pairs;
			pairs.reserve(c0->colliders.size() + c1->colliders.size());
			c0->checkCollisionL(c1, pairs);
			if (pairs.empty()) return 0;

			decltype(auto) func = [this](const std::pair<ColliderTree*, ColliderTree*>& pair)
			{
				if (numResults >= SkinnedMeshAlgorithm::MaxCollisionCount)
					return;

				auto a = pair.first, b = pair.second;

				auto aabbA = a->aabbMe;
				auto aabbB = b->aabbMe;
				auto abeg = a->aabb;
				auto bbeg = b->aabb;
				auto asize = b->isKinematic ? a->dynCollider : a->numCollider;
				auto bsize = a->isKinematic ? b->dynCollider : b->numCollider;
				auto aend = abeg + asize;
				auto bend = bbeg + bsize;

				CollisionResult result;
				CollisionResult temp;
				bool hasResult = false;

				// std::string(MSVC implements) has a small internal buffer which decreased memory allocations 
				thread_local std::vector<Aabb*> list;
				if (asize > bsize)
				{
					list.reserve(std::max<size_t>(bsize, list.capacity()));
					for (auto i = bbeg; i < bend; ++i)
					{
						if (i->collideWith(aabbA))
							list.push_back(i);
					}

					for (auto i = abeg; i < aend; ++i)
					{
						//if (numResults >= SkinnedMeshAlgorithm::MaxCollisionCount)
						//	break;

						if (!i->collideWith(aabbB))
							continue;

						for (auto j : list)
						{
							if (!i->collideWith(*j))
								continue;
							if (checkCollide(&a->cbuf[i - abeg], &b->cbuf[j - bbeg], temp))
							{
								if (!hasResult || result.depth > temp.depth)
								{
									hasResult = true;
									result = temp;
								}
							}
						}
					}
				}
				else
				{
					list.reserve(std::max<size_t>(bsize, list.capacity()));
					for (auto i = abeg; i < aend; ++i)
					{
						if (i->collideWith(aabbB))
							list.push_back(i);
					}

					for (auto j = bbeg; j < bend; ++j)
					{
						//if (numResults >= SkinnedMeshAlgorithm::MaxCollisionCount)
						//	break;

						if (!j->collideWith(aabbA))
							continue;

						for (auto i : list)
						{
							if (!i->collideWith(*j))
								continue;
							if (checkCollide(&a->cbuf[i - abeg], &b->cbuf[j - bbeg], temp))
							{
								if (!hasResult || result.depth > temp.depth)
								{
									hasResult = true;
									result = temp;
								}
							}
						}
					}
				}
				list.clear();

				if (hasResult)
				{
					addResult(result);
				}
			};

			if (pairs.size() >= std::thread::hardware_concurrency())
				concurrency::parallel_for_each(pairs.begin(), pairs.end(), func);
			else for (auto& i : pairs) func(i);
			//if (pairs.size() >= 4)
			//{
			//	concurrency::task_group taskGroup;
			//	for (auto& i : pairs)
			//		taskGroup.run([&]() { func(i); });
			//	taskGroup.wait();
			//}
			//else for (auto& i : pairs) func(i);

			return numResults;
		}
	};

	template<> bool CollisionCheck<PerVertexShape, PerVertexShape>::checkCollide(Collider* a, Collider* b, CollisionResult& res)
	{
		auto s0 = v0[a->vertex];
		auto r0 = s0.marginMultiplier() * sp0->margin;
		auto s1 = v0[a->vertex];
		auto r1 = s1.marginMultiplier() * sp0->margin;

		auto ret = checkSphereSphere(s0.pos(), s1.pos(), r0, r1, res);
		res.colliderA = a;
		res.colliderB = b;
		return ret;
	}

	template<> bool CollisionCheck<PerVertexShape, PerTriangleShape>::checkCollide(Collider* a, Collider* b, CollisionResult& res)
	{
		auto s = v0[a->vertex];
		auto r = s.marginMultiplier() * sp0->margin;
		auto p0 = v1[b->vertices[0]];
		auto p1 = v1[b->vertices[1]];
		auto p2 = v1[b->vertices[2]];
		auto margin = (p0.marginMultiplier() + p1.marginMultiplier() + p2.marginMultiplier()) / 3;
		auto penetration = sp1->penetration * margin;
		margin *= sp1->margin;

		CheckTriangle tri(p0.pos(), p1.pos(), p2.pos(), margin, penetration);
		if (!tri.valid) return false;
		auto ret = checkSphereTriangle(s.pos(), r, tri, res);
		res.colliderA = a;
		res.colliderB = b;
		return ret;
	}

	template<> bool CollisionCheck<PerTriangleShape, PerVertexShape>::checkCollide(Collider* a, Collider* b, CollisionResult& res)
	{
		auto s = v1[b->vertex];
		auto r = s.marginMultiplier() * sp1->margin;
		auto p0 = v0[a->vertices[0]];
		auto p1 = v0[a->vertices[1]];
		auto p2 = v0[a->vertices[2]];
		auto margin = (p0.marginMultiplier() + p1.marginMultiplier() + p2.marginMultiplier()) / 3;
		auto penetration = sp0->penetration * margin;
		margin *= sp0->margin;

		CheckTriangle tri(p0.pos(), p1.pos(), p2.pos(), margin, penetration);
		if (!tri.valid) return false;
		auto ret = checkTriangleSphere(s.pos(), r, tri, res);
		res.colliderA = a;
		res.colliderB = b;
		return ret;
	}

	template<class T0, class T1> inline int checkCollide(T0* a, T1* b, CollisionResult* results)
	{
		return CollisionCheck<T0, T1>(a, b, results)();
	}

	void SkinnedMeshAlgorithm::MergeBuffer::doMerge(SkinnedMeshShape* a, SkinnedMeshShape* b, CollisionResult* collision, int count)
	{
		for (int i = 0; i < count; ++i)
		{
			auto& res = collision[i];
			if (res.depth >= -FLT_EPSILON) break;

			auto flexible = std::max(res.colliderA->flexible, res.colliderB->flexible);
			if (flexible < FLT_EPSILON) return;

			for (int ib = 0; ib < a->getBonePerCollider(); ++ib)
			{
				auto w0 = a->getColliderBoneWeight(res.colliderA, ib);
				int boneIdx0 = a->getColliderBoneIndex(res.colliderA, ib);
				if (w0 <= a->m_owner->m_skinnedBones[boneIdx0].weightThreshold) continue;

				for (int jb = 0; jb < b->getBonePerCollider(); ++jb)
				{
					auto w1 = b->getColliderBoneWeight(res.colliderB, jb);
					int boneIdx1 = b->getColliderBoneIndex(res.colliderB, jb);
					if (w1 <= b->m_owner->m_skinnedBones[boneIdx1].weightThreshold) continue;

					if (a->m_owner->m_skinnedBones[boneIdx0].isKinematic && b->m_owner->m_skinnedBones[boneIdx1].isKinematic)
						continue;

					float w = flexible * res.depth;
					float w2 = w * w;
					auto c = get(boneIdx0, boneIdx1);
					c->weight += w2;
					c->normal += res.normOnB * w * w2;
					c->pos[0] += res.posA * w2;
					c->pos[1] += res.posB * w2;
				}
			}
		}
	}
	void SkinnedMeshAlgorithm::MergeBuffer::apply(SkinnedMeshBody* body0, SkinnedMeshBody* body1, CollisionDispatcher* dispatcher)
	{
		for (int i = 0; i < body0->m_skinnedBones.size(); ++i)
		{
			if (!body1->canCollideWith(body0->m_skinnedBones[i].ptr)) continue;
			for (int j = 0; j < body1->m_skinnedBones.size(); ++j)
			{
				if (!body0->canCollideWith(body1->m_skinnedBones[j].ptr)) continue;
				if (get(i, j)->weight < FLT_EPSILON) continue;

				if (body0->m_skinnedBones[i].isKinematic && body1->m_skinnedBones[j].isKinematic) continue;

				auto rb0 = body0->m_skinnedBones[i].ptr;
				auto rb1 = body1->m_skinnedBones[j].ptr;
				if (rb0 == rb1) continue;

				auto c = get(i, j);
				float invWeight = 1.0f / c->weight;

				auto maniford = dispatcher->getNewManifold(&rb0->m_rig, &rb1->m_rig);
				auto worldA = c->pos[0] * invWeight;
				auto worldB = c->pos[1] * invWeight;
				auto localA = rb0->m_rig.getWorldTransform().invXform(worldA);
				auto localB = rb1->m_rig.getWorldTransform().invXform(worldB);
				auto normal = c->normal * invWeight;
				if (normal.fuzzyZero()) continue;
				auto depth = -normal.length();
				normal = -normal.normalized();

				if (depth >= -FLT_EPSILON) continue;

				btManifoldPoint newPt(localA, localB, normal, depth);
				newPt.m_positionWorldOnA = worldA;
				newPt.m_positionWorldOnB = worldB;
				newPt.m_combinedFriction = rb0->m_rig.getFriction() * rb1->m_rig.getFriction();
				newPt.m_combinedRestitution = rb0->m_rig.getRestitution() * rb1->m_rig.getRestitution();
				newPt.m_combinedRollingFriction = rb0->m_rig.getRollingFriction() * rb1->m_rig.getRollingFriction();
				maniford->addManifoldPoint(newPt);
			}
		}
	}

	template<class T0, class T1> void SkinnedMeshAlgorithm::processCollision(T0* shape0, T1* shape1, MergeBuffer& merge, CollisionResult* collision)
	{
		int count = std::min(checkCollide(shape0, shape1, collision), MaxCollisionCount);
		if (count > 0)
			merge.doMerge(shape0, shape1, collision, count);
	}

	void SkinnedMeshAlgorithm::processCollision(SkinnedMeshBody* body0, SkinnedMeshBody* body1, CollisionDispatcher* dispatcher)
	{
		MergeBuffer merge;
		merge.alloc(body0->m_skinnedBones.size(), body1->m_skinnedBones.size());

		auto collision = new CollisionResult[MaxCollisionCount];
		if (body0->m_shape->asPerTriangleShape() && body1->m_shape->asPerTriangleShape())
		{
			processCollision(body0->m_shape->asPerTriangleShape(), body1->m_shape->asPerVertexShape(), merge, collision);
			processCollision(body0->m_shape->asPerVertexShape(), body1->m_shape->asPerTriangleShape(), merge, collision);
		}
		else if (body0->m_shape->asPerTriangleShape())
			processCollision(body0->m_shape->asPerTriangleShape(), body1->m_shape->asPerVertexShape(), merge, collision);
		else if (body1->m_shape->asPerTriangleShape())
			processCollision(body0->m_shape->asPerVertexShape(), body1->m_shape->asPerTriangleShape(), merge, collision);
		else processCollision(body0->m_shape->asPerVertexShape(), body1->m_shape->asPerVertexShape(), merge, collision);

		delete[] collision;
		merge.apply(body0, body1, dispatcher);
		merge.release();
	}

	void SkinnedMeshAlgorithm::registerAlgorithm(btCollisionDispatcher * dispatcher)
	{
		static CreateFunc s_gimpact_cf;
		dispatcher->registerCollisionCreateFunc(CUSTOM_CONCAVE_SHAPE_TYPE, CUSTOM_CONCAVE_SHAPE_TYPE, &s_gimpact_cf);
	}
}