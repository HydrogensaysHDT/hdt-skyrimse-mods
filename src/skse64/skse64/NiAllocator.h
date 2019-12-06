#pragma once

typedef void * (* _NiAllocate)(UInt32 size);
extern const _NiAllocate NiAllocate;

typedef void (* _NiFree)(void * memory);
extern const _NiFree NiFree;

// 04
class NiAllocator
{
public:
	virtual ~NiAllocator();

	virtual void * Allocate(UInt32 unk1, UInt32 unk2, UInt32 unk3, UInt32 unk4, UInt32 unk5, UInt32 unk6, UInt32 unk7) = 0;
	virtual void Free(void * mem, UInt32 unk1, SInt32 unk2) = 0;
	virtual void Unk_03() = 0;
	virtual void Unk_04() = 0;
	virtual void Unk_05() = 0;
	virtual void Unk_06() = 0;
	virtual void Unk_07() = 0;
	virtual void Unk_08() = 0;
	virtual void Unk_09() = 0;
};

// 04
class NiStandardAllocator : public NiAllocator
{
	virtual ~NiStandardAllocator();

	virtual void * Allocate(UInt32 unk1, UInt32 unk2, UInt32 unk3, UInt32 unk4, UInt32 unk5, UInt32 unk6, UInt32 unk7);
	virtual void Free(void * mem, UInt32 unk1, SInt32 unk2);
	virtual void Unk_03();
	virtual void Unk_04();
	virtual void Unk_05();
	virtual void Unk_06();
	virtual void Unk_07();
	virtual void Unk_08();
	virtual void Unk_09();
};

// 04
class BSNiAllocator : public NiAllocator
{
	virtual ~BSNiAllocator();

	virtual void * Allocate(UInt32 unk1, UInt32 unk2, UInt32 unk3, UInt32 unk4, UInt32 unk5, UInt32 unk6, UInt32 unk7);
	virtual void Free(void * mem, UInt32 unk1, SInt32 unk2);
	virtual void Unk_03();
	virtual void Unk_04();
	virtual void Unk_05();
	virtual void Unk_06();
	virtual void Unk_07();
	virtual void Unk_08();
	virtual void Unk_09();
};
