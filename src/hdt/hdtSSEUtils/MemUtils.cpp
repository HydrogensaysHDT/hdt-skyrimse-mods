#include "MemUtils.h"

namespace hdt
{
	bool checkHookPoint(const std::vector<HookPointPattern>& patterns, const char* offset, size_t size)
	{
		for (auto& i : patterns)
		{
			if (i.offset + i.count > size)
				return false;
			if (memcmp(offset + i.offset, i.bytes, i.count))
				return false;
		}
		return true;
	}

	std::pair<char*, size_t> findExecRange()
	{
		auto baseOffset = reinterpret_cast<char*>(GetModuleHandleA(nullptr));

		std::vector<std::pair<char*, size_t>> execMems;
		for (auto i = baseOffset; i < baseOffset + 0x0000'0000'0200'0000ll; )
		{
			MEMORY_BASIC_INFORMATION memInfo;
			VirtualQuery(i, &memInfo, sizeof(memInfo));
			if (memInfo.State == MEM_COMMIT)
			{
				switch (memInfo.Protect)
				{
				case PAGE_EXECUTE:
					VirtualProtect(baseOffset, memInfo.RegionSize, PAGE_EXECUTE_READWRITE, nullptr);
				case PAGE_EXECUTE_READ:
				case PAGE_EXECUTE_READWRITE:
					execMems.push_back(std::make_pair(i, memInfo.RegionSize));
					break;
				default:
					break;
				}
			}
			i += memInfo.RegionSize;
		}
		return execMems.front();
	}

	std::pair<char*, size_t> getExecRange()
	{
		static auto range = findExecRange();
		return range;
	}

	std::vector<intptr_t> findHookPoint(std::vector<HookPointPattern> patterns)
	{
		std::sort(patterns.begin(), patterns.end(), [](const HookPointPattern& a, const HookPointPattern& b) {
			return a.offset < b.offset;
		});

		auto baseOffset = reinterpret_cast<char*>(GetModuleHandleA(nullptr));

		auto range = getExecRange();
		std::vector<intptr_t> results;
		for (auto offset = 0; offset < range.second; offset += 16)
		{
			if (offset + range.first - baseOffset == 0x012AB480)
				offset = offset;

			if (checkHookPoint(patterns, offset + range.first, range.second - offset))
				results.push_back(offset + range.first - baseOffset);
		}
		
		return results;
	}

	void printHookPoint(std::vector<HookPointPattern> patterns, const char* file, int line)
	{
		auto ret = findHookPoint(patterns);
		if (ret.size())
		{
			for (auto i : ret) hdt::LogInfo("%s:%d hook point:%08llx", file, line, i);
		}
		else hdt::LogInfo("%s:%s hook point not found", file, line);
	}
}