#pragma once

#include "../hdtSSEUtils/Ref.h"

namespace hdt
{
	// Unified Const String Class
	class IString
	{
	public:
		virtual ~IString() {}
		virtual void retain() = 0;
		virtual void release() = 0;

		virtual const char* cstr() const = 0;
		virtual size_t size() const = 0;
	};

	namespace ref
	{
		inline void retain(IString* str) { str->retain(); }
		inline void release(IString* str) { str->release(); }
	}
}
