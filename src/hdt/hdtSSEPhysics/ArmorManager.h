#pragma once

#include "../hdtSSEUtils/NetImmerseUtils.h"
#include "../hdtSSEUtils/FrameworkUtils.h"
#include "../hdtSSEFramework/IEventListener.h"
#include "../hdtSSEFramework/HookArmor.h"
#include "../hdtSSEFramework/HookEngine.h"

#include "hdtSkyrimMesh.h"

#include <mutex>

namespace hdt
{
	class ArmorManager
		: public IEventListener<ArmorAttachEvent>
		, public IEventListener<FrameEvent>
		, public IEventListener<ShutdownEvent>
	{
	public:
		ArmorManager();
		~ArmorManager();
		
		static ArmorManager* instance();
		static IDStr generatePrefix(NiAVObject* armor);

		virtual void onEvent(const ArmorAttachEvent& e) override;
		virtual void onEvent(const FrameEvent& e) override;
		virtual void onEvent(const ShutdownEvent&) override;

	protected:
		struct Armor
		{
			Ref<IString>						prefix;
			Ref<NiAVObject>						armorWorn;
			std::unordered_map<IDStr, IDStr>	renameMap;
			std::string							physicsFile;
			Ref<SkyrimMesh>						physics;
		};

		struct Skeleton
		{
			Ref<NiNode>			skeleton;
			Ref<NiNode>			npc;
			std::vector<Armor>	armors;

			void cleanArmor();
			void clear();

			static void doSkeletonMerge(NiNode* dst, NiNode* src, IString* prefix, std::unordered_map<IDStr, IDStr>& map);
			static void doSkeletonClean(NiNode* dst, IString* prefix);
			static NiNode* cloneNodeTree(NiNode* src, IString * prefix, std::unordered_map<IDStr, IDStr>& map);
			static void renameTree(NiNode* root, IString * prefix, std::unordered_map<IDStr, IDStr>& map);
		};

		bool					m_shutdown = false;
		std::recursive_mutex	m_lock;
		std::vector<Skeleton>	m_skeletons;

		Skeleton& getSkeletonData(NiNode* skeleton);

	};
}