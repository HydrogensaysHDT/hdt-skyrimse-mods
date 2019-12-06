#include "stdafx.h"
#include "HookEngine.h"
#include "IFramework.h"
#include "HookUtils.h"
#include "FrameworkImpl.h"
#include "../hdtSSEUtils/MemUtils.h"

namespace hdt
{
	struct UnkEngine
	{
		MEMBER_FN_PREFIX(UnkEngine);

		// SleepStub + 0xF
		DEFINE_MEMBER_FN_HOOK(onFrame, void, 0x005b2ff0);

		void onFrame();
	};

	void UnkEngine::onFrame()
	{
		FrameEvent e;
		e.frameEnd = false;
		FrameworkImpl::instance()->getFrameEventDispatcher()->dispatch(e);
		CALL_MEMBER_FN(this, onFrame)();
		e.frameEnd = true;
		//LogDebug("onFrame : %f", hdtGetFramework()->getFrameInterval(false));
		FrameworkImpl::instance()->getFrameEventDispatcher()->dispatch(e);
	}

	auto oldShutdown = (void (*)(bool))(hookGetBaseAddr() + 0x01293d20);
	void shutdown(bool arg0)
	{
		FrameworkImpl::instance()->getShutdownEventDispatcher()->dispatch(ShutdownEvent());
		oldShutdown(arg0);
	}
		
	void hookEngine()
	{
		printHookPoint({
			HookPointPattern{0, 24, "\x40\x57\x48\x83\xec\x30\x48\xc7\x44\x24\x20\xfe\xff\xff\xff\x48\x89\x5c\x24\x40\x48\x8b\xd9\x48"},
			HookPointPattern{0x23, 5, "\xb9\x09\x00\x00\x00"},
			HookPointPattern{0x2E, 15, "\xbf\x00\x80\x00\x00\x66\x85\xc7\x74\x14\xb9\x12\x00\x00\x00"},
			}, __FILE__, __LINE__);
		DetourAttach((void**)UnkEngine::_onFrame_GetPtrAddr(), (void*)GetFnAddr(&UnkEngine::onFrame));

		printHookPoint({
			HookPointPattern{0, 39, "\x48\x8b\xc4\x57\x48\x81\xec\x40\x01\x00\x00\x48\xc7\x44\x24\x30\xfe\xff\xff\xff\x48\x89\x58\x10\x48\x89\x70\x18\x33\xf6\x84\xc9\x0f\x84\x41\x02\x00\x00\x48"},
			}, __FILE__, __LINE__);
		DetourAttach((void**)&oldShutdown, (void*)shutdown);
	}

	void unhookEngine()
	{
		DetourDetach((void**)UnkEngine::_onFrame_GetPtrAddr(), (void*)GetFnAddr(&UnkEngine::onFrame));
		DetourDetach((void**)&oldShutdown, (void*)shutdown);
	}

}
