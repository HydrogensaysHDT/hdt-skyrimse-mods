#pragma once

#include <skse64/skse64/PapyrusVM.h>
#include <skse64/skse64/PapyrusNativeFunctions.h>

namespace hdt
{
	template <class TBase, class TRet>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*),
		VMClassRegistry * registry
	) {
		return new NativeFunction0<TBase, TRet>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1),
		VMClassRegistry * registry
	) {
		return new NativeFunction1<TBase, TRet, TArg1>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2),
		VMClassRegistry * registry
	) {
		return new NativeFunction2<TBase, TRet, TArg1, TArg2>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3),
		VMClassRegistry * registry
	) {
		return new NativeFunction3<TBase, TRet, TArg1, TArg2, TArg3>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4),
		VMClassRegistry * registry
	) {
		return new NativeFunction4<TBase, TRet, TArg1, TArg2, TArg3, TArg4>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4, TArg5),
		VMClassRegistry * registry
	) {
		return new NativeFunction5<TBase, TRet, TArg1, TArg2, TArg3, TArg4, TArg5>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6),
		VMClassRegistry * registry
	) {
		return new NativeFunction6<TBase, TRet, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6, class TArg7>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7),
		VMClassRegistry * registry
	) {
		return new NativeFunction7<TBase, TRet, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6, class TArg7, class TArg8>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8),
		VMClassRegistry * registry
	) {
		return new NativeFunction8<TBase, TRet, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6, class TArg7, class TArg8, class TArg9>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TArg9),
		VMClassRegistry * registry
	) {
		return new NativeFunction9<TBase, TRet, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TArg9>(Name, Package, callback, registry);
	}

	template <class TBase, class TRet, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6, class TArg7, class TArg8, class TArg9, class TArg10>
	NativeFunction* createNativeFunction(
		const char* Name,
		const char* Package,
		TRet(*callback)(TBase*, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TArg9, TArg10),
		VMClassRegistry * registry
	) {
		return new NativeFunction10<TBase, TRet, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6, TArg7, TArg8, TArg9, TArg10>(Name, Package, callback, registry);
	}
}

#define AutoRegisterFunction( func, package, registry )	registry->RegisterFunction(createNativeFunction(#func, package, func, registry))
#define AutoRegisterFunction_NoWait( func, package, registry ) \
	do {\
		AutoRegisterFunction( func, package, registry );\
		registry->SetFunctionFlags(PackageName, #func, VMClassRegistry::kFunctionFlag_NoWait);\
	} while(false)

