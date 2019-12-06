#include "stdafx.h"
#include "NetImmerseUtils.h"
#include <skse64/skse64/GameStreams.h>
#include <fstream>

namespace hdt
{
	NiNode* addParentToNode(NiNode * node, const char * name)
	{
		auto parent = node->m_parent;
		auto newParent = NiNode::Create(1);
		node->IncRef();
		if (parent)
		{
			parent->RemoveChild(node);
			parent->AttachChild(newParent, false);
		}
		newParent->AttachChild(node, false);
		setNiNodeName(newParent, name);
		node->DecRef();
		return newParent;
	}

	NiAVObject * findObject(NiAVObject * obj, const BSFixedString & name)
	{
		return obj->GetObjectByName((const char**)&name);
	}

	NiNode * findNode(NiNode * obj, const BSFixedString & name)
	{
		auto ret = obj->GetObjectByName((const char**)&name);
		return ret ? ret->GetAsNiNode() : nullptr;
	}

	std::string readAllFile(const char* path)
	{
		BSResourceNiBinaryStream fin(path);
		if (!fin.IsValid()) return "";

		size_t readed;
		char buffer[4096];
		std::string ret;
		do {
			readed = fin.Read(buffer, sizeof(buffer));
			ret.append(buffer, readed);
		} while (readed == sizeof(buffer));
		return ret;
	}

	std::string readAllFile2(const char* path)
	{
		std::ifstream fin(path, std::ios::binary);
		if (!fin.is_open()) return "";

		fin.seekg(0, std::ios::end);
		auto size = fin.tellg();
		fin.seekg(0, std::ios::beg);
		std::string ret;
		ret.resize(size);
		fin.read(&ret[0], size);
		return ret;
	}

	void updateTransformUpDown(NiNode * node)
	{
		if (!node) return;

		NiAVObject::ControllerUpdateContext ctx;
		ctx.delta = 0.f;
		ctx.flags = NiAVObject::ControllerUpdateContext::kDirty;
		node->UpdateWorldData(&ctx);

		for (int i = 0; i < node->m_children.m_arrayBufLen; ++i)
		{
			auto child = castNiNode(node->m_children.m_data[i]);
			if (child) updateTransformUpDown(child);
		}
	}
}
