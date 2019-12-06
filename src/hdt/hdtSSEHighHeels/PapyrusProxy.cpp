#include "stdafx.h"
#include "PapyrusProxy.h"
#include "HighheelManager.h"
#include "Configs.h"
#include "CustomEnchManager.h"

#include "../hdtSSEUtils/LogUtils.h"

#include <skse64/skse64/GameReferences.h>

namespace hdt
{
	// =============== Highheel Manager ================

	float GetMagnitude(ActiveEffect* object)
	{
		return object->magnitude;
	}

	void KeepUpdate(StaticFunctionTag*, TESObjectREFR* akActor, float magnitude)
	{
		if (!akActor) return;

		HighheelManager::instance()->add(akActor, magnitude);
	}

	void StopUpdate(StaticFunctionTag*, TESObjectREFR* akActor)
	{
		if (!akActor) return;
		HighheelManager::instance()->remove(akActor);
	}

	void AdapterClear(StaticFunctionTag*)
	{
		HighheelManager::instance()->clear();
	}

	void ReloadConfigs(StaticFunctionTag*)
	{
		configs::reloadAll();
	}

	// ========================= Configs =============================
#define FAST_CREATE_PROPERTY( name, type, valname ) \
	static type get##name(StaticFunctionTag*){ return valname; }\
	static void set##name(StaticFunctionTag*, type val){ valname.set(val); }

	FAST_CREATE_PROPERTY(GlobalOffset, float, configs::offset)

	static bool getOptionalFlgs(StaticFunctionTag*, SInt32 bit)
	{
		int mask = (1 << bit);
		return mask & configs::optionals;
	}

	static void setOptionalFlgs(StaticFunctionTag*, SInt32 bit, bool val)
	{
		int mask = (1 << bit);
		configs::optionals.set(~mask & configs::optionals | (val ? mask : 0));
	}

	static bool isDebugInfoEnable(StaticFunctionTag*)
	{
		return configs::enableDebugInfo;
	}
	static void setDebugInfoEnable(StaticFunctionTag*, bool val)
	{
		return configs::enableDebugInfo.set(val);
	}

	FAST_CREATE_PROPERTY(StableMode, SInt32, configs::stableMode)
	FAST_CREATE_PROPERTY(StableModeMaxSwing, float, configs::stableModeMaxSwing)

	FAST_CREATE_PROPERTY(DecalUpdate, SInt32, configs::decalUpdate)
	FAST_CREATE_PROPERTY(ThreadPriority, SInt32, configs::priority)
	FAST_CREATE_PROPERTY(LogLevel, SInt32, configs::logLevel)

	// ======================== Custom Enchantments ======================

	void EnchManagerClear(StaticFunctionTag*)
	{
		CustomEnchManager::instance()->clear();
		LogInfo("Custom enchantment manager : clear");
	}

	void EnchManagerLoad(StaticFunctionTag*)
	{
		LogInfo("Custom enchantment manager : loading");
		CustomEnchManager::instance()->load();
	}

	void EnchManagerSave(StaticFunctionTag*)
	{
		LogInfo("Custom enchantment manager : saving");
		CustomEnchManager::instance()->save();
	}

	void RegisterCustomEnch(StaticFunctionTag*, EnchantmentItem* ench)
	{
		CustomEnchManager::instance()->addEnchType(ench);
	}

	void SetEnchNthMagnitude(StaticFunctionTag*, EnchantmentItem* ench, UInt32 idx, float val)
	{
		if (!ench || idx<0 || idx>ench->effectItemList.count || !ench->effectItemList[idx]) return;
		ench->effectItemList[idx]->magnitude = val;
	}

	void AttachEnchantment(StaticFunctionTag*, TESObjectARMO* armor, UInt32 idx)
	{
		CustomEnchManager::instance()->enchant(armor, idx);
	}

	void RemoveEnchantment(StaticFunctionTag*, TESObjectARMO* armor)
	{
		CustomEnchManager::instance()->remove(armor);
	}

	const float fPluginVersion = 2.0f;
	float GetCoreVersion(StaticFunctionTag*) { return fPluginVersion; }
	BSFixedString GetInitText(StaticFunctionTag*) { return "hdtSSEHH Init! Welcome to www.9damao.com!"; }

	bool registerPapyrus(VMClassRegistry * registry)
	{
		AutoRegisterFunction(GetMagnitude, "hdtHighHeelEffect", registry);
		AutoRegisterFunction(KeepUpdate, "hdtHighHeelEffect", registry);
		AutoRegisterFunction(StopUpdate, "hdtHighHeelEffect", registry);
		AutoRegisterFunction(AdapterClear, "hdtHighHeelEffect", registry);

		AutoRegisterFunction(getOptionalFlgs, "hdtHighHeelEffect", registry);
		AutoRegisterFunction(isDebugInfoEnable, "hdtHighHeelEffect", registry);

		AutoRegisterFunction(getGlobalOffset, "hdtHighHeelEffect", registry);

		AutoRegisterFunction(getGlobalOffset, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setGlobalOffset, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(getOptionalFlgs, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setOptionalFlgs, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(getStableMode, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setStableMode, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(getStableModeMaxSwing, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setStableModeMaxSwing, "hdtHighHeelMCM", registry);


		AutoRegisterFunction(getDecalUpdate, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setDecalUpdate, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(getThreadPriority, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setThreadPriority, "hdtHighHeelMCM", registry);

		AutoRegisterFunction(isDebugInfoEnable, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setDebugInfoEnable, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(getLogLevel, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(setLogLevel, "hdtHighHeelMCM", registry);

		AutoRegisterFunction(EnchManagerClear, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(EnchManagerLoad, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(EnchManagerSave, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(RegisterCustomEnch, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(SetEnchNthMagnitude, "hdtHighHeelMCM", registry);

		AutoRegisterFunction(AttachEnchantment, "hdtHighHeelMCM", registry);
		AutoRegisterFunction(RemoveEnchantment, "hdtHighHeelMCM", registry);

		AutoRegisterFunction(GetCoreVersion, "hdtHighHeel_LoadGame", registry);
		AutoRegisterFunction(GetInitText, "hdtHighHeel_LoadGame", registry);

		return true;
	}

}
