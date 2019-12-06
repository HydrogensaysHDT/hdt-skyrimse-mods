#pragma once

#include <common\IPrefix.h>
#include <skse64\skse64\NiNodes.h>
#include <skse64\skse64\NiExtraData.h>
#include <skse64\skse64\NiGeometry.h>
#include "Ref.h"

class NiCloningProcess
{
public:
	uint32_t	unk00 = 0;
	uint32_t	unk04 = 0;
	uint32_t	unk08 = 0;
	uint32_t	unk0C = 0;
	uint32_t	unk10 = 0;
	uint32_t	unk14 = 0;
	uintptr_t	unk18 = RelocationManager::s_baseAddr + 0x01dcc48c;
	void*		unk20 = 0;
	void*		unk28 = 0;
	uint32_t	unk30 = 0;
	uint32_t	unk34 = 0;
	uint32_t	unk38 = 0;
	uint32_t	unk3c = 0;
	uint32_t	unk40 = 0;
	uint32_t	unk44 = 0;
	uintptr_t	unk48 = RelocationManager::s_baseAddr + 0x01dcc488;
	void*		unk50 = 0;
	void*		unk58 = 0;
	uint32_t	cloneType = 1;
	char		appendChar = '$';
	float		unk68 = 1.0f;
	float		unk6c = 1.0f;
	float		unk70 = 1.0f;
	uint32_t	unk74 = 0;
};

namespace hdt
{
	inline void setBSFixedString(const char** ref, const char* value)
	{
		auto tmp = reinterpret_cast<BSFixedString*>(ref);
		CALL_MEMBER_FN(tmp, Set)(value);
	}

	inline void setNiNodeName(NiNode* node, const char* name)
	{
		setBSFixedString(&node->m_name, name);
	}

	inline NiNode* castNiNode(NiAVObject* obj) { return obj ? obj->GetAsNiNode() : nullptr; }
	inline BSTriShape* castBSTriShape(NiAVObject* obj) { return obj ? obj->GetAsBSTriShape() : nullptr; }

	NiNode* addParentToNode(NiNode* node, const char* name);

	NiAVObject* findObject(NiAVObject* obj, const BSFixedString& name);
	NiNode* findNode(NiNode* obj, const BSFixedString& name);

	inline float length(const NiPoint3& a) { return sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
	inline float distance(const NiPoint3& a, const NiPoint3& b) { return length(a - b); }

	namespace ref
	{
		inline void retain(NiRefObject* object) { object->IncRef(); }
		inline void release(NiRefObject* object) { object->DecRef(); }
	}

	std::string readAllFile(const char* path);
	std::string readAllFile2(const char* path);

	void updateTransformUpDown(NiNode* node);
}

