#include "stdafx.h"
#include "HookScene.h"
#include "HookUtils.h"
#include "FrameworkImpl.h"
#include <detours.h>
#include <chrono>

namespace hdt
{
/*
	struct HookScene
	{
		MEMBER_FN_PREFIX(HookScene);

		DEFINE_MEMBER_FN_HOOK(updateScene, void, 0x005b1610, uint32_t);

		void updateScene(uint32_t arg0)
		{
			CALL_MEMBER_FN(this, updateScene)(arg0);

			static float timeInterval = 0.f;
			if (timeInterval)
			{
				SceneUpdateEvent e;
				e.timeInterval = timeInterval;
				FrameworkImpl::instance()->getSceneUpdateEventDispatcher()->dispatch(e);
			}
			timeInterval = FrameworkImpl::instance()->getFrameInterval(false);
		}
	};*/
	
	void hookScene()
	{
		//DetourAttach((void**)HookScene::_updateScene_GetPtrAddr(), (void*)GetFnAddr(&HookScene::updateScene));
	}

	void unhookScene()
	{
		//DetourDetach((void**)HookScene::_updateScene_GetPtrAddr(), (void*)GetFnAddr(&HookScene::updateScene));
	}
}
