#pragma once

#include <skse64/skse64/GameObjects.h>
#include <unordered_set>
#include <mutex>

namespace hdt
{
	class CustomEnchManager
	{
	public:

		struct NODE
		{
			NODE() :form(0) {}
			NODE(uint32_t f) :form(f) {}

			void release() const;
			inline operator int()const { return form; }

			uint32_t form;
		};

		struct CustomEnch
		{
			int enchFormID;
			std::unordered_set<NODE, std::hash<uint32_t>> forms;
		};

		void addEnchType(EnchantmentItem* item);
		void enchant(TESObjectARMO* form, uint32_t type);
		void remove(TESObjectARMO* form);

		void load();
		void clear();
		void save();

		static CustomEnchManager* instance();

	private:

		CustomEnchManager(void) {}
		~CustomEnchManager(void) {}

		std::recursive_mutex m_lock;
		std::vector<CustomEnch> enchInfo;
	};
}
