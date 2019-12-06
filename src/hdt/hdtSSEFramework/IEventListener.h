#pragma once

#include <memory>
#include <functional>

namespace hdt
{
	template <class Event = void>
	class IEventListener
	{
	public:
		virtual void onEvent(const Event&) = 0;
	};

	template <>
	class IEventListener<void>
	{
	public:
		virtual void onEvent() = 0;
	};

	template <class Event = void>
	class FunctionalEventListener : public IEventListener<Event>
	{
	public:
		FunctionalEventListener(const std::function<void(Event)>& func) : m_func(func) {}
		FunctionalEventListener(std::function<void(Event)>&& func) : m_func(std::move(func)) {}

		virtual void onEvent() override { m_func(); }

	private:
		std::function<void(Event)> m_func;
	};

	template <>
	class FunctionalEventListener<void> : public IEventListener<void>
	{
	public:
		FunctionalEventListener(const std::function<void()>& func) : m_func(func) {}
		FunctionalEventListener(std::function<void()>&& func) : m_func(std::move(func)) {}

		virtual void onEvent() override { m_func(); }

	private:
		std::function<void()> m_func;
	};
}
