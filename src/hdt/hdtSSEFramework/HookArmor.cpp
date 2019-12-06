#include "stdafx.h"
#include "HookArmor.h"
#include "HookUtils.h"
#include "../hdtSSEUtils/NetImmerseUtils.h"
#include "FrameworkImpl.h"

#include <common\IPrefix.h>

#include <skse64\skse64\GameData.h>
#include <skse64\skse64\NiNodes.h>
#include "../hdtSSEUtils/MemUtils.h"

namespace hdt
{
	struct Unk001CB0E0
	{
		MEMBER_FN_PREFIX(Unk001CB0E0);

		DEFINE_MEMBER_FN_HOOK(unk001CB0E0, NiAVObject*, 0x001cafb0, NiNode* armor, NiNode* skeleton, void* unk3, char unk4, char unk5, void* unk6);
		NiAVObject* unk001CB0E0(NiNode* armor, NiNode* skeleton, void* unk3, char unk4, char unk5, void* unk6)
		{
			ArmorAttachEvent event;
			event.armorModel = armor;
			event.skeleton = skeleton;
			FrameworkImpl::instance()->getArmorAttachEventDispatcher()->dispatch(event);

			auto ret = CALL_MEMBER_FN(this, unk001CB0E0)(armor, skeleton, unk3, unk4, unk5, unk6);

			event.attachedNode = ret;
			event.hasAttached = true;
			FrameworkImpl::instance()->getArmorAttachEventDispatcher()->dispatch(event);

			return ret;
		}
	};

	void hookArmor()
	{
		printHookPoint({
			HookPointPattern{0, 0x62, "\x44\x89\x4c\x24\x20\x4c\x89\x44\x24\x18\x48\x89\x54\x24\x10\x48\x89\x4c\x24\x08\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8d\x6c\x24\xf9\x48\x81\xec\xf8\x00\x00\x00\x48\xc7\x45\xf7\xfe\xff\xff\xff\x4d\x63\xf1\x4c\x8b\xea\x48\x8b\xf1\x33\xdb\x48\x89\x5c\x24\x38\x44\x8b\xe3\x48\x89\x5c\x24\x40\x4c\x89\x44\x24\x68\x48\x89\x5c\x24\x60\x48\x89\x5c\x24\x58\x48\x81\xc1\x70\x27\x00\x00"},
		}, __FILE__, __LINE__);
		DetourAttach((void**)Unk001CB0E0::_unk001CB0E0_GetPtrAddr(), (void*)GetFnAddr(&Unk001CB0E0::unk001CB0E0));
	}

	void unhookArmor()
	{
		DetourDetach((void**)Unk001CB0E0::_unk001CB0E0_GetPtrAddr(), (void*)GetFnAddr(&Unk001CB0E0::unk001CB0E0));
	}
}