#include "Logger.h"
#include <ctime>
#include <iostream>
#include <filesystem>
#include <cstdarg>
#include <sstream>
#include <string>
#ifdef WIN32
#include <Windows.h>
#endif

using namespace std;



/* ----------------------------------------------------------------------------------------------------------
 * Log Manager
 * ----------------------------------------------------------------------------------------------------------
*/

LogManager LogManager::m_instance = LogManager();

LogManager::LogManager() :
	m_logLevel(LogLevel::LL_DEBUG)
{
}

LogManager::~LogManager()
{
}
std::unique_ptr<Logger> LogManager::getLogger(const std::string& name)
{
	auto logger = std::unique_ptr<Logger>(new Logger(name));
	return logger;
}
LogManager& LogManager::Instance()
{
	return m_instance;
}
void LogManager::setLevel(LogLevel level)
{
	m_logLevel = level;
}
void LogManager::open(const std::string& name)
{
	close();

	try
	{
		std::filesystem::path logpath = std::filesystem::path(name).make_preferred(); // make_preferred cleanup slash and backslash
		for (const auto& entry : std::filesystem::directory_iterator(logpath.parent_path()))
		{
			if (entry.path().extension() == ".log")
			{

				if (!std::filesystem::remove(entry.path()))
				{
					std::string msg = "Unable to remove: ";
					msg += entry.path().string();
					writeLog("LogManager", LogLevel::LL_ERROR, msg);
				}
				else
				{
					std::string msg = "Old log removed ";
					msg += entry.path().string();
					writeLog("LogManager", LogLevel::LL_INFO, msg);
				}
			}
		}
	}
	catch (std::exception e)
	{
		std::string msg = "Unexpected error flushing old logs: ";
		msg += e.what();
		writeLog("LogManager", LogLevel::LL_ERROR, msg);
	}
	const string date = currentLogDate("%4.4d-%2.2d-%2.2d %2.2d-%2.2d-%2.2d");
	string filename = name;
	filename += "-";
	filename += date;
	filename += ".log";
	m_logFile.open(filename);

}
void LogManager::flush()
{
	m_logFile.flush();
}
void LogManager::close()
{
	if (m_logFile.is_open())
	{
		m_logFile.close();
	}
}
// convert a std::thread::id to something usable
uint64_t LogManager::getThreadId(const std::thread::id& id) {
	stringstream ss;
	ss << id;
	string sid = ss.str();
	try
	{
		return stoull(sid);
	}
	catch (std::invalid_argument ex)
	{
		return -1; // this happen when the thread id does not represent a running thread
	}
}
bool LogManager::isLoggable(LogLevel level)
{
	return  (level <= m_logLevel);
}
/*
 * send the log entry to the console and to a file
*/
void LogManager::writeLog(const string name, const LogLevel level, const string& message)
{
	if (!isLoggable(level))
		return;

	const char* logFormat = "[%s] [%-8.8s] [%8.8llX] [%-8.8s] %s\n";
	const string date = currentLogDate("%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d");

	string strLevel = "DEBUG";
	if (level == LogLevel::LL_ERROR)
		strLevel = "ERROR";
	if (level == LogLevel::LL_WARNING)
		strLevel = "WARNING";
	if (level == LogLevel::LL_INFO)
		strLevel = "INFO";
	uint64_t threadId = (unsigned long int)getThreadId(this_thread::get_id());
	int finalSize = snprintf(NULL, 0, logFormat, date.c_str(), name.c_str(), threadId, strLevel.c_str(), message.c_str()) + 1;
	char* logBuffer = new char[finalSize];
	snprintf(logBuffer, finalSize, logFormat, date.c_str(), name.c_str(), threadId, strLevel.c_str(), message.c_str());

	cout << logBuffer;
	if (m_logFile.is_open())
	{
		m_logFile << logBuffer;
		m_logFile.flush();
	}
#ifdef WIN32
	::OutputDebugStringA(logBuffer);
#endif
	delete[] logBuffer;
	logBuffer = nullptr;
}

/*
 * the format must accept 6 values: "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d"
 * use %n to skip values
 */
const std::string LogManager::currentLogDate(const std::string& format)
{
	time_t t = time(NULL);
	struct tm tm;
#ifdef WIN32
	gmtime_s(&tm, &t);
#else
	gmtime_r(&t, &tm);
#endif
	const int year = tm.tm_year + 1900;
	const int month = tm.tm_mon + 1;
	const int size = snprintf(NULL, 0, format.c_str(), year, month, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) + 1;
	char* buffer = new char[size];
	snprintf(buffer, size, format.c_str(), year, month, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	std::string result = buffer;
	delete[] buffer;
	buffer = nullptr;
	return result;
}

/* ----------------------------------------------------------------------------------------------------------
 * Logger
 * ----------------------------------------------------------------------------------------------------------
*/

// terminate the variadic template recursion
void Logger::log(LogLevel level, const char* format) const
{
	if (!LogManager::Instance().isLoggable(level))
		return;
	LogManager::Instance().writeLog(m_name, level, this->format(format));
}
void Logger::warn(const char* format) const
{
	log(LogLevel::LL_WARNING, format);
}
void Logger::info(const char* format) const
{
	log(LogLevel::LL_INFO, format);
}
void Logger::error(const char* format) const
{
	log(LogLevel::LL_ERROR, format);
}
void Logger::debug(const char* format) const
{
	log(LogLevel::LL_DEBUG, format);
}


/*
 * Format methods for each single parameters
 */
std::string Logger::format(const char* fmt, std::string& value) const
{
	return format(fmt, value.c_str());
}
std::string Logger::format(const char* fmt, std::thread::id& value) const
{
	return format(fmt, LogManager::Instance().getThreadId(value));
}

// NOTE: despite the "..." we have always one single parameter after fmt here.
std::string Logger::format(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	const int size = vsnprintf(NULL, 0, fmt, args) + 1;
	va_end(args);

	char* buffer = new char[size];
	va_start(args, fmt);
	vsnprintf(buffer, size, fmt, args);
	va_end(args);

	std::string result = buffer;
	delete[] buffer;
	buffer = nullptr;
	return result;
}

Logger::Logger(const std::string& name) :
	m_name(name)
{
}
