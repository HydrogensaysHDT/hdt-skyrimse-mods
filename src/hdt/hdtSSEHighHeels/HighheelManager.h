#pragma once

#include <skse64/skse64/GameObjects.h>
#include <skse64/skse64/NiObjects.h>
#include <cstdint>
#include <vector>
#include <mutex>

namespace hdt
{
	class HighheelEffect
	{
	public:
		HighheelEffect(uint32_t id, float m) : m_formID(id), m_oldRoot(0), m_magnitude(m) {}

		bool update();
		void reset();

		inline uint32_t getFormID() const { return m_formID; }
		inline float getMagnitude() const { return m_magnitude; }
		inline void setMagnitude(float x) { m_magnitude = x; }
		
	protected:
		
		uint32_t	m_formID;
		NiNode*		m_oldRoot;
		float		m_magnitude;
	};

	class HighheelManager
	{
	public:
		HighheelManager();
		~HighheelManager();

		void add(TESObjectREFR* akActor, float magnitude);
		void remove(TESObjectREFR* akActor);
		void update();
		void clear();

		static HighheelManager* instance();

	protected:
		std::vector<HighheelEffect> m_effects;
		std::mutex					m_lock;
	};
}
