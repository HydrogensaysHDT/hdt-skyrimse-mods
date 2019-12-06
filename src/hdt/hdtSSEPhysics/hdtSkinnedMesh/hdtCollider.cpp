#include "hdtCollider.h"
#include <algorithm>

namespace hdt
{
	static const _CRT_ALIGN(16) U8 interleaveBits[16] = { 0, 1, 8, 9, 64, 65, 72, 73 };

	void ColliderTree::insertCollider(const std::vector<U32>& keys, const Collider& c)
	{
		ColliderTree* p = this;
		for (int i = 0; i < keys.size() && i < 4; ++i)
		{
			auto key = keys[i];
			auto f = std::find_if(p->children.begin(), p->children.end(), [=](const ColliderTree& n){ return n.key == key; });
			if (f == p->children.end())
			{
				p->children.push_back(ColliderTree(key));
				p = &p->children.back();
			}
			else p = &*f;
		}
		p->colliders.push_back(c);
	}

	void ColliderTree::checkCollisionL(ColliderTree* r, std::vector<std::pair<ColliderTree*, ColliderTree*>>& ret)
	{
		if (isKinematic && r->isKinematic)
			return;

		if (!aabbAll.collideWith(r->aabbAll))
			return;

		if (numCollider && aabbMe.collideWith(r->aabbAll))
		{
			if (aabbMe.collideWith(r->aabbMe))
				ret.push_back(std::make_pair(this, r));

			auto begin = r->children.data();
			auto end = begin + (isKinematic ? r->dynChild : r->children.size());
			for (auto i = begin; i < end; ++i)
				checkCollisionR(i, ret);
		}

		auto begin = children.data();
		auto end = begin + (r->isKinematic ? dynChild : children.size());
		for (auto i = begin; i < end; ++i)
			i->checkCollisionL(r, ret);
	}

	void ColliderTree::checkCollisionR(ColliderTree* r, std::vector<std::pair<ColliderTree*, ColliderTree*>>& ret)
	{
		if (isKinematic && r->isKinematic)
			return;

		if (numCollider)
		{
			if (!aabbMe.collideWith(r->aabbAll))
				return;

			if (aabbMe.collideWith(r->aabbMe))
				ret.push_back(std::make_pair(this, r));

			auto begin = r->children.data();
			auto end = begin + (isKinematic ? r->dynChild : r->children.size());
			for (auto i = begin; i < end; ++i)
				checkCollisionR(i, ret);
		}
	}

	void ColliderTree::clipCollider(const std::function<bool(const Collider&)>& func)
	{
		for (auto& i : children)
			i.clipCollider(func);

		colliders.erase(std::remove_if(colliders.begin(), colliders.end(), func), colliders.end());
		children.erase(std::remove_if(children.begin(), children.end(), [](const ColliderTree& n)->bool{ return n.empty(); }), children.end());
	}

	void ColliderTree::updateKinematic(const std::function<float(const Collider*)>& func)
	{
		U32 k = true;
		for (auto& i : colliders)
		{
			i.flexible = func(&i);
			k &= i.flexible < FLT_EPSILON;
		}

		for (auto& i : children)
		{
			i.updateKinematic(func);
			k &= i.isKinematic;
		}

		std::sort(colliders.begin(), colliders.end(), [](const Collider& a, const Collider& b){ return a.flexible > b.flexible; });
		std::sort(children.begin(), children.end(), [](const ColliderTree& a, const ColliderTree& b){ return a.isKinematic < b.isKinematic; });

		isKinematic = k;

		if (k)
		{
			dynChild = dynCollider = 0;
		}
		else
		{
			for (dynChild = 0; dynChild < children.size(); ++dynChild)
				if (children[dynChild].isKinematic)
					break;

			for (dynCollider = 0; dynCollider < colliders.size(); ++dynCollider)
				if (colliders[dynCollider].flexible < FLT_EPSILON)
					break;
		}
	}

	void ColliderTree::updateAabb()
	{
		if (numCollider)
		{
			register Aabb aabb = *this->aabb;;
			auto aabbEnd = this->aabb + numCollider;
			for (auto i = this->aabb + 1; i < aabbEnd; ++i)
				aabb.merge(*i);
			aabbMe = aabb;
		}

		aabbAll = aabbMe;
		for (auto& i : children)
		{
			i.updateAabb();
			aabbAll.merge(i.aabbAll);
		}
	}

	void ColliderTree::visitColliders(const std::function<void(Collider*)>& func)
	{
		for (auto& i : colliders)
			func(&i);

		for (auto& i : children)
			i.visitColliders(func);
	}

	void ColliderTree::optimize()
	{
		for (auto& i : children)
			i.optimize();

		children.erase(std::remove_if(children.begin(), children.end(), [](const ColliderTree& n){ return n.empty(); }), children.end());
		
		while (children.size() == 1 && children[0].colliders.empty())
		{
			vectorA16<ColliderTree> temp;
			temp.swap(children.front().children);
			children.swap(temp);
		}

		if (children.size() == 1 && colliders.empty())
		{
			colliders = children[0].colliders;

			vectorA16<ColliderTree> temp;
			temp.swap(children[0].children);
			children.swap(temp);
		}
	}

	bool ColliderTree::collapseCollideL(ColliderTree * r)
	{
		if (isKinematic && r->isKinematic)
			return false;

		if (!aabbAll.collideWith(r->aabbAll))
			return false;

		if (numCollider && aabbMe.collideWith(r->aabbAll))
		{
			if (aabbMe.collideWith(r->aabbMe))
				return true;

			auto begin = r->children.data();
			auto end = begin + (isKinematic ? r->dynChild : r->children.size());
			for (auto i = begin; i < end; ++i)
				if (collapseCollideR(i))
					return true;
		}

		auto begin = children.data();
		auto end = begin + (r->isKinematic ? dynChild : children.size());
		for (auto i = begin; i < end; ++i)
			if (i->collapseCollideL(r))
				return true;
		return false;
	}

	bool ColliderTree::collapseCollideR(ColliderTree * r)
	{
		if (isKinematic && r->isKinematic)
			return false;

		if (numCollider)
		{
			if (!aabbMe.collideWith(r->aabbAll))
				return false;

			if (aabbMe.collideWith(r->aabbMe))
				return true;

			auto begin = r->children.data();
			auto end = begin + (isKinematic ? r->dynChild : r->children.size());
			for (auto i = begin; i < end; ++i)
				if (collapseCollideR(i))
					return true;
		}

		return false;
	}

	void ColliderTree::exportColliders(vectorA16<Collider>& exportTo)
	{
		numCollider = colliders.size();
		cbuf = (Collider*)exportTo.size();
		for (auto& i : colliders)
		{
			exportTo.push_back(i);
		}

		for (auto& i : children)
			i.exportColliders(exportTo);
	}

	void ColliderTree::remapColliders(Collider* start, Aabb* startAabb)
	{
		colliders.swap(vectorA16<Collider>());
		auto offset = (size_t)cbuf;
		cbuf = start + offset;
		aabb = startAabb + offset;

		for (auto& i : children)
			i.remapColliders(start, startAabb);
	}
}