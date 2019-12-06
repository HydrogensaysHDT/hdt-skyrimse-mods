#include "stdafx.h"
#include "ArmorManager.h"
#include "hdtSkyrimPhysicsWorld.h"
#include "hdtDefaultBBP.h"

#include "../hdtSSEUtils/LogUtils.h"

namespace hdt
{
	ArmorManager::ArmorManager()
	{
	}


	ArmorManager::~ArmorManager()
	{
	}

	ArmorManager * ArmorManager::instance()
	{
		static ArmorManager s;
		return &s;
	}

	IDStr ArmorManager::generatePrefix(NiAVObject * armor)
	{
		char buffer[128];
		sprintf_s(buffer, "hdtSSEPhysics_AutoRename_%016llX ", (uintptr_t)armor);
		return IDStr(buffer);
	}

	inline bool isFirstPersonSkeleton(NiNode* npc)
	{
		if (!npc) return false;
		if (findNode(npc, "Camera1st [Cam1]")) return true;
		return false;
	}

	void ArmorManager::onEvent(const ArmorAttachEvent & e)
	{
		auto npc = findNode(e.skeleton, "NPC");
		if (!npc) return;

		std::lock_guard<decltype(m_lock)> l(m_lock);
		if (m_shutdown) return;

		if (e.hasAttached)
		{
			auto prefix = generatePrefix(e.armorModel);
			auto& skeleton = getSkeletonData(e.skeleton);
			auto iter = std::find_if(skeleton.armors.begin(), skeleton.armors.end(), [=](Armor& i) {
				return i.prefix == prefix;
			});

			if (iter != skeleton.armors.end())
			{
				iter->armorWorn = e.attachedNode;
				std::unordered_map<IDStr, IDStr> renameMap;
				iter->renameMap.swap(renameMap);

				if (!isFirstPersonSkeleton(e.skeleton))
				{
					auto mesh = SkyrimMeshParser().createMesh(npc, e.attachedNode, iter->physicsFile, std::move(renameMap));

					if (mesh)
					{
						SkyrimPhysicsWorld::get()->addSkinnedMeshSystem(mesh);
						iter->physics = mesh;
					}
				}
			}
		}
		else
		{
			auto prefix = generatePrefix(e.armorModel);
			auto& skeleton = getSkeletonData(e.skeleton);
			skeleton.npc = npc;
			auto iter = std::find_if(skeleton.armors.begin(), skeleton.armors.end(), [=](Armor& i) {
				return i.prefix == prefix;
			});
			if (iter == skeleton.armors.end())
			{
				skeleton.armors.push_back(Armor());
				skeleton.armors.back().prefix = prefix;
				iter = skeleton.armors.end() - 1;
			}
			Skeleton::doSkeletonMerge(npc, e.armorModel, prefix, iter->renameMap);
			iter->physicsFile = scanBBP(e.armorModel);
		}
	}

	void ArmorManager::onEvent(const FrameEvent & e)
	{
		if (!e.frameEnd) return;

		std::lock_guard<decltype(m_lock)> l(m_lock);
		if (m_shutdown) return;

		for (auto& i : m_skeletons)
		{
			if (!i.skeleton->m_parent)
			{
				if (i.skeleton->m_uiRefCount == 1)
				{
					i.clear();
					i.skeleton = nullptr;
				}
				else
				{
					for (auto& j : i.armors)
					{
						if (j.physics && j.physics->m_world)
							j.physics->m_world->removeSkinnedMeshSystem(j.physics);
					}
				}
			}
			else
			{
				auto world = SkyrimPhysicsWorld::get();
				for (auto& j : i.armors)
				{
					if (j.physics && !j.physics->m_world)
						world->addSkinnedMeshSystem(j.physics);
				}
			}
		}
		m_skeletons.erase(std::remove_if(m_skeletons.begin(), m_skeletons.end(), [](Skeleton& i) { return !i.skeleton; }), m_skeletons.end());

		for (auto& i : m_skeletons)
			i.cleanArmor();
	}

	void ArmorManager::onEvent(const ShutdownEvent &)
	{
		m_shutdown = true;
		std::lock_guard<decltype(m_lock)> l(m_lock);
		
		m_skeletons.clear();
	}

	ArmorManager::Skeleton& ArmorManager::getSkeletonData(NiNode * skeleton)
	{
		auto iter = std::find_if(m_skeletons.begin(), m_skeletons.end(), [=](Skeleton& i) {
			return i.skeleton == skeleton;
		});
		if (iter != m_skeletons.end())
			return *iter;
		m_skeletons.push_back(Skeleton());
		m_skeletons.back().skeleton = skeleton;
		return m_skeletons.back();
	}

	void ArmorManager::Skeleton::doSkeletonMerge(NiNode * dst, NiNode * src, IString * prefix, std::unordered_map<IDStr, IDStr>& map)
	{
		for (int i = 0; i < src->m_children.m_arrayBufLen; ++i)
		{
			auto srcChild = castNiNode(src->m_children.m_data[i]);
			if (!srcChild) continue;

			if (!srcChild->m_name)
			{
				doSkeletonMerge(dst, srcChild, prefix, map);
				continue;
			}

			auto dstChild = findNode(dst, srcChild->m_name);
			if (dstChild)
			{
				doSkeletonMerge(dstChild, srcChild, prefix, map);
			}
			else
			{
				dst->AttachChild(cloneNodeTree(srcChild, prefix, map), false);
			}
		}
	}

	NiNode * ArmorManager::Skeleton::cloneNodeTree(NiNode * src, IString * prefix, std::unordered_map<IDStr, IDStr>& map)
	{
		NiCloningProcess c;
		auto ret = (NiNode*)src->CreateClone(c);
		src->ProcessClone(&c);

		renameTree(src, prefix, map);
		renameTree(ret, prefix, map);

		return ret;
	}

	void ArmorManager::Skeleton::renameTree(NiNode* root, IString * prefix, std::unordered_map<IDStr, IDStr>& map)
	{
		if (root->m_name)
		{
			std::string newName(prefix->cstr(), prefix->size());
			newName += root->m_name;
			if (map.insert(std::make_pair<IDStr, IDStr>(root->m_name, newName)).second)
				LogDebug("Rename Bone %s -> %s", root->m_name, newName.c_str());
			setNiNodeName(root, newName.c_str());
		}

		for (int i = 0; i < root->m_children.m_arrayBufLen; ++i)
		{
			auto child = castNiNode(root->m_children.m_data[i]);
			if (child)
				renameTree(child, prefix, map);
		}
	}
	
	void ArmorManager::Skeleton::doSkeletonClean(NiNode * dst, IString * prefix)
	{
		for (int i = dst->m_children.m_arrayBufLen - 1; i >= 0; --i)
		{
			auto child = castNiNode(dst->m_children.m_data[i]);
			if (!child) continue;
			
			if (child->m_name && !strncmp(child->m_name, prefix->cstr(), prefix->size()))
			{
				dst->RemoveAt(i++);
			}
			else
			{
				doSkeletonClean(child, prefix);
			}
		}
	}

	void ArmorManager::Skeleton::cleanArmor()
	{
		for (auto& i : armors)
		{
			if (!i.armorWorn) continue;
			if (i.armorWorn->m_parent) continue;
			
			SkyrimPhysicsWorld::get()->removeSkinnedMeshSystem(i.physics);
			if (npc) doSkeletonClean(npc, i.prefix);
			i.prefix = nullptr;
		}

		armors.erase(std::remove_if(armors.begin(), armors.end(), [](Armor& i) { return !i.prefix; }), armors.end());
	}

	void ArmorManager::Skeleton::clear()
	{
		SkyrimPhysicsWorld::get()->removeSystemByNode(npc);
		armors.clear();
	}
}
