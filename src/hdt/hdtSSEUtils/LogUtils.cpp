#include "LogUtils.h"
#include <algorithm>

namespace hdt
{
	ILogListener::ILogListener()
	{
		LogManager::instance()->addListener(this);
	}

	ILogListener::~ILogListener()
	{
		LogManager::instance()->removeListener(this);
	}

	const char* LevelName[] = { "", "ERROR", "WARNING", "INFO", "DEBUG" };
	
	void LogManager::writeRaw(const char* fmt, ...)
	{
		std::lock_guard<std::mutex> l(m_lock);

		va_list arg;
		va_start(arg, fmt);
		vsprintf_s(m_buffer, fmt, arg);
		va_end(arg);
		strcat_s(m_buffer, "\n");

		for (auto& i : m_listeners)
			i->onLog(m_buffer);
	}

	void LogManager::write(int level, const char* fmt, ...)
	{
		std::lock_guard<std::mutex> l(m_lock);

		if (level > m_maxLevel) return;

		time_t rawtm;
		tm timeinfo;

		time(&rawtm);
		localtime_s(&timeinfo, &rawtm);

		char timeBuffer[50];
		strftime(timeBuffer, sizeof(timeBuffer), "[%c]", &timeinfo);
		strcat_s(timeBuffer, LevelName[level]);
		strcat_s(timeBuffer, ": ");

		va_list arg;
		va_start(arg, fmt);
		vsprintf_s(m_buffer, fmt, arg);
		va_end(arg);

		std::string str = timeBuffer;
		str += m_buffer;
		if (!str.empty() && str.back() != '\n')
			str.push_back('\n');

		for (auto& i : m_listeners)
			i->onLog(str.c_str());
	}

	LogManager* LogManager::instance()
	{
		static LogManager s;
		return &s;
	}

	void LogManager::addListener(ILogListener* listener)
	{
		m_listeners.push_back(listener);
	}

	void LogManager::removeListener(ILogListener* listener)
	{
		m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
	}

	LogToDebug::LogToDebug()
	{
	}
	
	void LogToDebug::onLog(const char* str)
	{
		OutputDebugStringA(str);
	}

	static LogToDebug s_logToDebug;

	LogToFile::LogToFile(const char* file)
		: m_fout(file)
	{
	}

	LogToFile::LogToFile(const wchar_t* file)
		: m_fout(file)
	{
	}

	void LogToFile::onLog(const char* str)
	{
		m_fout << str;
		m_fout << std::flush;
	}
}
