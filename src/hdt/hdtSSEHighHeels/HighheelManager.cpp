#include "stdafx.h"
#include "HighheelManager.h"

#define NOMINMAX
#include <skse64\skse64\GameRTTI.h>
#include <skse64\skse64\GameReferences.h>
#include <skse64\skse64\NiNodes.h>

#include "../hdtSSEUtils/NetImmerseUtils.h"
#include "../hdtSSEUtils/TESUtils.h"
#include "../hdtSSEUtils/LogUtils.h"
#include "Configs.h"

namespace hdt
{
	Actor* getActor(int formID)
	{
		auto form = DYNAMIC_CAST(LookupFormByID(formID), TESForm, Actor);
		return DYNAMIC_CAST(form, TESForm, Actor);
	}

	static float clampHeight(Actor* actor, float height, float lastHeight)
	{
		UInt64 flag = actor->actorState.flags04 | (UInt64)(actor->actorState.flags08) << 32;
		if (flag & ActorState::kState_Swimming)
			return 0;

		float delta = configs::stableModeMaxSwing;
		switch (configs::stableMode)
		{
		case 2:
		STABLE_MODE:
			height = min(max(height, lastHeight - delta), lastHeight + delta);
			break;

		case 1:
			if (flag & (ActorState::kState_Walking | ActorState::kState_Running))
				goto STABLE_MODE;

		default:
			break;
		};
		return height;
	}

	bool HighheelEffect::update()
	{
		auto actor = getActor(m_formID);
		if (!actor || !actor->loadedState || !actor->loadedState->node) return true;

		static const BSFixedString NINODE_Root = "NPC Root [Root]";
		static const BSFixedString NINODE_LFoot = "NPC L Foot [Lft ]";
		static const BSFixedString NINODE_RFoot = "NPC R Foot [Rft ]";
		static const BSFixedString NINODE_LCalf = "NPC L Calf [LClf]";
		static const BSFixedString NINODE_RCalf = "NPC R Calf [RClf]";
		static const BSFixedString HDTSSEHH_CTRL = "HDT SSEHH Controller";

		auto* rootNode = actor->loadedState->node->GetAsNiNode();
		if (!rootNode) return true;
		auto* npc = findNode(rootNode, NINODE_Root);
		if (!npc) return true;

		auto ctrl = findNode(rootNode, HDTSSEHH_CTRL);
		if (!ctrl)
		{
			ctrl = addParentToNode(npc, HDTSSEHH_CTRL.data);
		}

		auto* lft = findNode(npc, NINODE_LFoot);
		auto* rft = findNode(npc, NINODE_RFoot);
		if (getSex(actor))
		{
			auto* lcalf = findNode(npc, NINODE_LCalf);
			auto* rcalf = findNode(npc, NINODE_RCalf);

			float lleglen = distance(lcalf->m_worldTransform.pos, lft->m_worldTransform.pos);
			float rleglen = distance(rcalf->m_worldTransform.pos, rft->m_worldTransform.pos);
			float cosl = (lcalf->m_worldTransform.pos.z - lft->m_worldTransform.pos.z) / lleglen;
			float cosr = (rcalf->m_worldTransform.pos.z - rft->m_worldTransform.pos.z) / rleglen;
			float rate = max(max(cosl, cosr), 0);

			static const float CONST_CALF_L = 55.89939793f;
			float calfScale = (lleglen + rleglen) / CONST_CALF_L;
			calfScale *= npc->m_localTransform.scale;
			calfScale /= npc->m_worldTransform.scale; //World -> Object
			auto actualheightDelta = calfScale * m_magnitude;
			ctrl->m_localTransform.pos.z = clampHeight(actor, actualheightDelta, ctrl->m_localTransform.pos.z);
		}
		else ctrl->m_localTransform.pos.z = 0.0f;

		NiAVObject::ControllerUpdateContext ctx;
		npc->UpdateWorldData(&ctx);
		return false;
	}

	void HighheelEffect::reset()
	{
		auto actor = getActor(m_formID);
		if (!actor || !actor->loadedState || !actor->loadedState->node) return;

		static const BSFixedString NINODE_NPC = "NPC";
		static const BSFixedString HDTSSEHH_CTRL = "HDT SSEHH Controller";

		auto* rootNode = actor->loadedState->node->GetAsNiNode();
		if (!rootNode) return;
		auto* npc = findNode(rootNode, NINODE_NPC);
		if (!npc) return;

		auto ctrl = findNode(rootNode, HDTSSEHH_CTRL);
		if (!ctrl) return;

		ctrl->m_localTransform.pos.z = 0.0f;

		NiAVObject::ControllerUpdateContext ctx;
		npc->UpdateWorldData(&ctx);
	}

	HighheelManager::HighheelManager()
	{
	}

	HighheelManager::~HighheelManager()
	{
	}

	void HighheelManager::add(TESObjectREFR * akActor, float magnitude)
	{
		if (!akActor) return;

		std::lock_guard<std::mutex> l(m_lock);
		int id = akActor->formID;
		
		auto idx = m_effects.begin();
		for (; idx < m_effects.end(); ++idx)
		{
			if (idx->getFormID() == id)
				break;
		}

		if (idx >= m_effects.end())
		{
			m_effects.push_back(HighheelEffect(id, magnitude));
			LogDebug("object added ... 0x%08x", id);
		}
		else idx->setMagnitude(magnitude);
	}

	void HighheelManager::remove(TESObjectREFR * akActor)
	{
		if (!akActor) return;

		std::lock_guard<std::mutex> l(m_lock);
		auto iter = std::find_if(m_effects.begin(), m_effects.end(), [=](const HighheelEffect& r) {
			return r.getFormID() == akActor->formID;
		});
		if (iter >= m_effects.end())
			return;

		iter->reset();
		m_effects.erase(iter);
	}

	void HighheelManager::update()
	{
		std::lock_guard<std::mutex> l(m_lock);

		m_effects.erase(std::remove_if(m_effects.begin(), m_effects.end(), [](HighheelEffect& i) {
			return i.update();
		}), m_effects.end());
	}

	void HighheelManager::clear()
	{
		std::lock_guard<std::mutex> l(m_lock);

		for (auto& i : m_effects)
			i.reset();
		m_effects.clear();
	}

	HighheelManager* HighheelManager::instance()
	{
		static HighheelManager s;
		return &s;
	}
}
