#pragma once

#include "IEventListener.h"

namespace hdt
{
	template <class Event = void>
	class IEventDispatcher
	{
	public:
		
		virtual ~IEventDispatcher() {}

		virtual void addListener(IEventListener<Event>*) = 0;
		virtual void removeListener(IEventListener<Event>*) = 0;
		virtual void dispatch(const Event&) = 0;
	};

	template <>
	class IEventDispatcher<void>
	{
	public:

		virtual ~IEventDispatcher() {}

		virtual void addListener(IEventListener<void>*) = 0;
		virtual void removeListener(IEventListener<void>*) = 0;
		virtual void dispatch() = 0;
	};
}
