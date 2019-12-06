#pragma once

#include <common/IPrefix.h>
#include <skse64/skse64/PluginAPI.h>
#include <skse64\skse64_common\Utilities.h>

#define DEFINE_MEMBER_FN_LONG_HOOK(className, functionName, retnType, address, ...)		\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);			\
	static inline uintptr_t* _##functionName##_GetPtrAddr(void)						\
	{																				\
		static uintptr_t _address = address + ::hdt::hookGetBaseAddr();				\
		return &_address;															\
	}																				\
																					\
	static inline _##functionName##_type * _##functionName##_GetPtr(void)			\
	{																				\
		return (_##functionName##_type *)_##functionName##_GetPtrAddr();			\
	}

#define DEFINE_MEMBER_FN_HOOK(functionName, retnType, address, ...)	\
	DEFINE_MEMBER_FN_LONG_HOOK(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, __VA_ARGS__)

// baseAddr in 1.5.3 0x00007ff7a4280000
namespace hdt
{
	uintptr_t hookGetBaseAddr();
}
