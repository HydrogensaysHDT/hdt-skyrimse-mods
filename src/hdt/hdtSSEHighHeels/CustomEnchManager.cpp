#include "stdafx.h"
#include "CustomEnchManager.h"

#include "../hdtSSEUtils/LogUtils.h"
#include <skse64/skse64/GameRTTI.h>
#include <skse64/skse64/GameData.h>
#include <string>

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace hdt
{
	static const char savePath[] = "data/skse/plugins/hdtSSEHHEnch.json";
	static const char PathPlugins[] = "skyrim/plugins.txt";
	static const char PathLoadOrder[] = "skyrim/loadorder.txt";
	static const char MCMModName[] = "hdtHighHeelMCM.esp";
	static const char* PackageName = "hdtHighHeelMCM";

	void CustomEnchManager::NODE::release() const
	{
		TESObjectARMO* armor = DYNAMIC_CAST(LookupFormByID(form), TESForm, TESObjectARMO);
		if (!armor) return;
		armor->enchantable.enchantment = nullptr;
	}
		
	void CustomEnchManager::clear()
	{
		for (auto& i : enchInfo)
			for (auto& j : i.forms)
				j.release();

		enchInfo.clear();
	}

	static void ReadCurrentModList(std::unordered_map<std::string, uint32_t>& modlist)
	{
		modlist.clear();
		modlist.reserve(256);

		auto& list = DataHandler::GetSingleton()->modList.modInfoList;
		for (auto iter = list.Begin(); !iter.End(); ++iter)
		{
			if (iter->modIndex != 0xFF)
				modlist.emplace(iter->name, iter->modIndex);
		}
	}

	void CustomEnchManager::save()
	{
		std::lock_guard<decltype(m_lock)> l(m_lock);

		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

		std::unordered_map<std::string, uint32_t> modlist;
		ReadCurrentModList(modlist);

		writer.StartObject();

		writer.String("modList");
		writer.StartObject();
		for (auto& i : modlist)
		{
			writer.Key(i.first.c_str(), i.first.length());
			writer.Uint(i.second);
		}
		writer.EndObject();

		writer.String("enchInfo");
		writer.StartArray();
		for (int i = 0; i<enchInfo.size(); ++i)
		{
			auto ench = DYNAMIC_CAST(LookupFormByID(enchInfo[i].enchFormID), TESForm, EnchantmentItem);
			if (!ench || !ench->effectItemList.count || !ench->effectItemList[0])
			{
				LogWarning("Error Enchantment %08x ... skip", enchInfo[i].enchFormID);
				continue;
			}

			writer.StartObject();
			writer.String("magnitude");
			writer.Double(ench->effectItemList[0]->magnitude);

			writer.String("forms");
			writer.StartArray();
			for (auto& form : enchInfo[i].forms)
				writer.Int(form);
			writer.EndArray();

			writer.EndObject();
		}
		writer.EndArray();
		writer.EndObject();

		std::ofstream fout(savePath, std::ios::binary);
		fout << buffer.GetString() << std::endl;
	}

	void CustomEnchManager::load()
	{
		std::lock_guard<decltype(m_lock)> l(m_lock);

		std::ifstream fin(savePath, std::ios::binary);
		if (!fin.is_open()) return;

		try {
			std::string content;
			fin.seekg(0, std::ios::end);
			content.resize(fin.tellg());
			fin.seekg(0, std::ios::beg);
			fin.read(&content[0], content.size());

			rapidjson::Document doc;
			doc.Parse(content.c_str());
			if (!doc.IsObject() || !doc.HasMember("modList") || !doc.HasMember("enchInfo"))
				return;

			std::unordered_map<std::string, uint8_t> oldList;
			auto& jsonOldList = doc["modList"];
			if (!jsonOldList.IsObject())
				return;

			LogInfo("Reading modlist from save(%d)", (int)jsonOldList.Size());
			for (auto iter = jsonOldList.MemberBegin(); iter != jsonOldList.MemberEnd(); ++iter)
			{
				if (!iter->value.IsUint()) return;
				oldList.emplace(std::string(iter->name.GetString(), iter->name.GetStringLength()), (uint8_t)iter->value.GetUint());
			}

			LogInfo("Reading modlist from game");
			std::unordered_map<std::string, uint32_t> modlist;
			ReadCurrentModList(modlist);
			std::unordered_map<uint32_t, uint32_t> map;
			for (auto &i : oldList)
			{
				auto iter = modlist.find(i.first);
				if (iter != modlist.end())
				{
					LogRaw("\tMapping mod (%02x) -> (%02x) : %s", i.second, iter->second, i.first.c_str());
					map.emplace(i.second, iter->second);
				}
				else
				{
					LogRaw("\tDeleted mod (%02x) : %s", i.second, i.first.c_str());
				}
			}

			auto& jsonEnchInfo = doc["enchInfo"];
			if (!jsonEnchInfo.IsArray())
				return;

			int numType = jsonEnchInfo.Size();
			if (numType < enchInfo.size())
				LogWarning("More enchantments(%d) than saved(%d)", enchInfo.size(), numType);
			else if (numType > enchInfo.size())
			{
				LogWarning("Less enchantments(%d) than saved(%d)", enchInfo.size(), numType);
				numType = enchInfo.size();
			}

			for (size_t i = 0; i<numType; ++i)
			{
				auto& jsonEnch = jsonEnchInfo[i];
				if (!jsonEnch.IsObject() || !jsonEnch.HasMember("magnitude") || !jsonEnch.HasMember("forms"))
					continue;

				enchInfo[i].forms.clear();
				auto ench = DYNAMIC_CAST(LookupFormByID(enchInfo[i].enchFormID), TESForm, EnchantmentItem);

				auto& jsonMagnitude = jsonEnch["magnitude"];
				ench->effectItemList[0]->magnitude = jsonMagnitude.IsNumber() ? jsonMagnitude.GetFloat() : 0.f;
				
				auto& jsonForms = jsonEnch["forms"];
				if (!jsonForms.IsArray()) continue;

				for (size_t j = 0; j < jsonForms.Size(); ++j)
				{
					auto& jsonForm = jsonForms[j];
					if (!jsonForm.IsNumber()) continue;

					uint32_t form = jsonForm.GetInt();
					auto modIndex = form >> 24;
					//if (modIndex == 0xFE)
					//{
					//	modIndex = form >> 16; // esl
					//}

					if (map.find(modIndex) != map.end())
					{
						form = form & 0x00FFFFFF | (map[modIndex] << 24);
						auto armor = DYNAMIC_CAST(LookupFormByID(form), TESForm, TESObjectARMO);
						if (armor)
							enchant(armor, i);
					}
				}
			}
		}
		catch (...) {
			LogWarning("save broken!! clear all list");
			for (auto& i : enchInfo)
			{
				for (auto& j : i.forms)
					j.release();
				i.forms.clear();
			}
		}
	}

	void CustomEnchManager::enchant(TESObjectARMO* form, uint32_t type)
	{
		if (!form) return;

		std::lock_guard<decltype(m_lock)> l(m_lock);

		if (type >= enchInfo.size() || type<0)
		{
			LogError("Failed to enchant %dth enchantment : Out Of Range", type);
			return;
		}

		remove(form);

		auto ench = DYNAMIC_CAST(LookupFormByID(enchInfo[type].enchFormID), TESForm, EnchantmentItem);
		if (!ench)
		{
			LogWarning("Failed to enchant %08x", form->formID);
			return;
		}

		enchInfo[type].forms.insert(form->formID);
		form->enchantable.enchantment = ench;
		LogInfo("Enchanting %08x by %08x", form->formID, enchInfo[type].enchFormID);
	}

	void CustomEnchManager::remove(TESObjectARMO* form)
	{
		if (!form) return;

		std::lock_guard<decltype(m_lock)> l(m_lock);

		form->enchantable.enchantment = nullptr;
		for (auto& i : enchInfo)
			i.forms.erase(form->formID);
	}

	void CustomEnchManager::addEnchType(EnchantmentItem* item)
	{
		if (!item) return;
		if (!item->effectItemList.count || !item->effectItemList[0])
		{
			LogWarning("Error enchantment %08x ... skip", item->formID);
			return;
		}

		std::lock_guard<decltype(m_lock)> l(m_lock);

		enchInfo.push_back(CustomEnch());
		enchInfo.back().enchFormID = item->formID;
		LogInfo("Custom enchantment manager : New enchantment added - %08x", item->formID);
	}

	CustomEnchManager* CustomEnchManager::instance()
	{
		static CustomEnchManager s;
		return &s;
	}
}
