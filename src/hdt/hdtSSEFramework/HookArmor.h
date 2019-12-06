#pragma once

class NiAVObject;
class NiNode;

namespace hdt
{
	struct ArmorAttachEvent
	{
		NiNode*		armorModel = nullptr;
		NiNode*		skeleton = nullptr;
		NiAVObject*	attachedNode = nullptr;
		bool		hasAttached = false;
	};

	void hookArmor();
	void unhookArmor();
}
