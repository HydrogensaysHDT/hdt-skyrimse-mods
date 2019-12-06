#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <vector>

#ifdef _DEBUG
#define DEFAULT_LOG_LEVEL LOG_DEBUG
#else
#define DEFAULT_LOG_LEVEL LOG_INFO
#endif

namespace hdt
{
	class ILogListener
	{
	public:
		ILogListener();
		~ILogListener();

		virtual void onLog(const char* str) = 0;
	};

	class LogManager
	{
	public:

		enum LOGLEVEL {
			LOG_NONE,
			LOG_ERROR,
			LOG_WARNING,
			LOG_INFO,
			LOG_DEBUG
		};

		LogManager() : m_maxLevel(DEFAULT_LOG_LEVEL) {}

		void setLevel(int level) { m_maxLevel = level; }
		void write(int level, const char* fmt, ...);
		void writeRaw(const char* fmt, ...);

		void addListener(ILogListener* listener);
		void removeListener(ILogListener* listener);

		static LogManager* instance();

	private:

		std::mutex	m_lock;
		int			m_maxLevel;
		char		m_buffer[2048];

		std::vector<ILogListener*> m_listeners;
	};

	class LogToDebug : public ILogListener
	{
	public:
		LogToDebug();

		virtual void onLog(const char* str) override;
	};

	class LogToFile : public ILogListener
	{
	public:
		LogToFile(const char* filename);
		LogToFile(const wchar_t* filename);

		virtual void onLog(const char* str) override;

	protected:
		std::ofstream m_fout;
	};

	template <class ... Args>
	inline void LogRaw(const char* fmt, Args... args) { LogManager::instance()->writeRaw(fmt, args...); }

	template <class ... Args>
	inline void LogError(const char* fmt, Args... args) { LogManager::instance()->write(LogManager::LOG_ERROR, fmt, args...); }

	template <class ... Args>
	inline void LogWarning(const char* fmt, Args... args) { LogManager::instance()->write(LogManager::LOG_WARNING, fmt, args...); }

	template <class ... Args>
	inline void LogInfo(const char* fmt, Args... args) { LogManager::instance()->write(LogManager::LOG_INFO, fmt, args...); }

	template <class ... Args>
	inline void LogDebug(const char* fmt, Args... args) { LogManager::instance()->write(LogManager::LOG_DEBUG, fmt, args...); }
}
