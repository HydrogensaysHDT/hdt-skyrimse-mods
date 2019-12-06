#pragma once

#include "../hdtSSEFramework/IFramework.h"

namespace hdt
{
	IFramework* getFramework();

	class IDStr : public Ref<IString>
	{
		typedef Ref<IString> MyBase;
	public:
		IDStr() {}
		IDStr(const char* str) : MyBase(getFramework()->getString(str)) {}
		IDStr(const std::string& str) : MyBase(getFramework()->getString(str)) {}
		IDStr(const IDStr& str) : MyBase(str) {}
		IDStr(IDStr&& str) : MyBase(std::move(str)) {}

		inline IDStr& operator =(const IDStr& rhs) { MyBase::operator=(rhs); return *this; }
		inline IDStr& operator =(IDStr&& rhs) { MyBase::operator=(std::move(rhs)); return *this; }
	};

	inline bool operator ==(const IDStr& a, const IDStr& b) { return a() == b(); }
	inline bool operator !=(const IDStr& a, const IDStr& b) { return a() != b(); }

	bool checkFrameworkVersion(uint32_t major, uint32_t minor);
}

namespace std
{
	template<>
	struct hash<hdt::IDStr> : public hash<hdt::IString*> {};
}
