#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <thread>
#include <vector>
#include <tuple>
#include <sstream>

/*
 A strongly typed variadic logger

 The formatting of the message works like printf (see // http://www.cplusplus.com/reference/cstdio/printf/)
 you can pass std::string to %s
 you can pass bool to %s it will display "true" or "false"
 you can pass std::thread::id to %lld (it will be converted to uint64_t which is unsigned long long)

 */

#define verbose(...) debug(__FILE__,__LINE__,__VA_ARGS__)

enum class LogLevel
{
	LL_NONE = -1,
	LL_ERROR = 0,
	LL_WARNING = 1,
	LL_INFO = 2,
	LL_DEBUG = 3

};

class Logger;

class LogManager
{
	friend Logger;

public:
	static LogManager& Instance();

	void open(const std::string& filename);
	void flush();
	void close();
	void setLevel(LogLevel level);
	bool isLoggable(LogLevel level);
	std::unique_ptr<Logger> getLogger(const std::string& name);
private:
	LogManager& operator= (const LogManager&) = delete;
	LogManager(const LogManager&) = delete;

	static LogManager m_instance;
	LogManager();
	~LogManager();

	LogLevel m_logLevel;
	std::ofstream m_logFile;
	const std::string currentLogDate(const std::string& format);
	void writeLog(const std::string name, const LogLevel level, const std::string& msg);
	uint64_t getThreadId(const std::thread::id& id);
};

class Logger
{
	friend LogManager;

public:
	// - variadic recursive template for LL_DEBUG   --------------------------
	template<typename First, typename... Rest>
	void debug(const char* format, First firstValue, Rest... rest) const
	{
		log(LogLevel::LL_DEBUG, format, firstValue, rest...);
	}
	void debug(const char* format) const;

	template<typename First, typename... Rest>
	void debug(const char* file, int line, const char* format, First firstValue, Rest... rest) const
	{
		std::ostringstream r;
		r << file;
		r << ':';
		r << line;
		r << ' ';
		r << recurseLog(format, firstValue, rest...);
		LogManager::Instance().writeLog(m_name, LogLevel::LL_DEBUG, r.str());
	}

	// - variadic recursive template for LL_WARNING --------------------------
	template<typename First, typename... Rest>
	void warn(const char* format, First firstValue, Rest... rest) const
	{
		log(LogLevel::LL_WARNING, format, firstValue, rest...);
	}
	void warn(const char* format) const;

	// - variadic recursive template for LL_INFO    --------------------------
	template<typename First, typename... Rest>
	void info(const char* format, First firstValue, Rest... rest) const
	{
		log(LogLevel::LL_INFO, format, firstValue, rest...);
	}
	void info(const char* format) const;

	// - variadic recursive template for LL_ERROR   --------------------------
	template<typename First, typename... Rest>
	void error(const char* format, First firstValue, Rest... rest) const
	{
		log(LogLevel::LL_ERROR, format, firstValue, rest...);
	}
	void error(const char* format) const;

	// - variadic recursive template for all levels -----------------------
	template<typename First, typename... Rest>
	void log(LogLevel level, const char* format, First firstValue, Rest... rest) const
	{
		if (!LogManager::Instance().isLoggable(level))
			return;
		std::string r = recurseLog(format, firstValue, rest...);
		LogManager::Instance().writeLog(m_name, level, r);
	}
	void log(LogLevel level, const char* format) const;

private:

	Logger(const std::string& name);
	Logger(const Logger&) = delete;

	const std::string m_name;

	template<typename First, typename... Rest>
	std::string recurseLog(const char* input_format, First firstValue, Rest... rest) const
	{
		const char* format = input_format;
		std::string result;
		for (; *format != '\0'; format++) {
			if (*format == '%') {
				const char* ptr = format + 1;
				std::string fmt = "%";
				// extract the format specifier
				for (;; ptr++)
				{
					const char c = *ptr;
					if (c == '\0')
					{
						std::ostringstream error;
						error << "invalid format specifier: ";
						error << fmt;
						error << " for format: \"";
						error << input_format;
						error << "\"";
						LogManager::Instance().writeLog(m_name, LogLevel::LL_ERROR, error.str());
						break;
					}
					fmt += c;
					if (c == 'd' || c == 'i' || c == 'u' || c == 'o' || c == 'x' || c == 'X' ||
						c == 'f' || c == 'F' || c == 'e' || c == 'E' || c == 'g' || c == 'G' ||
						c == 'a' || c == 'A' || c == 'c' || c == 's' || c == 'p' || c == 'n' ||
						c == '%')
						break;


				}
				if (fmt == "%%")
				{
					result += '%';
					result += recurseLog(ptr + 1, firstValue, rest...);
				}
				else
				{
					if constexpr (std::is_same_v<First, bool>)
						result += this->format(fmt.c_str(), firstValue ? "true" : "false");
					else
						result += this->format(fmt.c_str(), firstValue);
					result += recurseLog(ptr + 1, rest...);
				}

				return result;
			}
			else
			{
				result += *format;
			}
		}
		return result;
	}

	std::string recurseLog(const char* format) const
	{
		return format;
	}

	std::string format(const char* fmt, ...) const;
	std::string format(const char* fmt, std::string& value) const;
	std::string format(const char* fmt, std::thread::id& value) const;

};
