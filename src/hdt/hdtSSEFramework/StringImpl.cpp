#include "stdafx.h"
#include "StringImpl.h"

#include <algorithm>

namespace hdt
{
	StringImpl::StringImpl(size_t hash, std::string&& str)
		: m_hash(hash), m_str(std::move(str))
	{
		m_refCount = 0;
	}

	StringImpl::~StringImpl()
	{
	}

	void StringImpl::retain()
	{
		m_refCount.fetch_add(1);
	}

	void StringImpl::release()
	{
		if (m_refCount.fetch_sub(1) == 1)
			delete this;
	}

	StringManager * StringManager::instance()
	{
		static StringManager s;
		return &s;
	}

	StringImpl * StringManager::get(const char * begin, const char * end)
	{
		std::string str(begin, end);
		size_t hash = std::hash<std::string>()(str);
		auto& bucket = m_buckets[hash % BucketCount];

		return bucket.get(hash, std::move(str));
	}

	StringImpl * StringManager::Bucket::get(size_t hash, std::string && str)
	{
		std::lock_guard<decltype(m_lock)> l(m_lock);
		auto iter = std::find_if(m_list.begin(), m_list.end(), [&, hash](StringImpl* i) {
			return i->hash() == hash && i->str() == str;
		});

		if (iter != m_list.end())
			return *iter;
		else
		{
			auto ret = new StringImpl(hash, std::move(str));
			m_list.push_back(ret);
			return ret;
		}
	}
	void StringManager::Bucket::clean()
	{
		std::lock_guard<decltype(m_lock)> l(m_lock);

		m_list.erase(std::remove_if(m_list.begin(), m_list.end(), [](StringImpl* i) { return i->refcount() == 1; }), m_list.end());
	}

	StringManager::StringManager()
	{
		m_gcExit = false;
		m_gcThread = std::thread([this]() 
		{
			while (!m_gcExit)
			{
				for (auto& i : m_buckets)
				{
					if (m_gcExit) break;

					i.clean();

					if (m_gcExit) break;
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				if (m_gcExit) break;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
	}

	StringManager::~StringManager()
	{
		m_gcExit = true;
		if (m_gcThread.joinable())
			m_gcThread.join();
	}
}
