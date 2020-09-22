# STV-Logger
**Table of content**

[1.1 Features](#11-features)  
[1.2 Example](#12-example)  
[1.3 Initialization](#13-initialization)  
[1.4 Purge](#14-purge)  
  
  
[2 Variadic C++ templates](#2-variadic-c-templates)  



Strongly Typed Variadic Logger in C++17

I always wanted to have a nice logger in C++ like we have in Java. So here it is! This project is based on CMake. You must use it to generate the visual studio project in the **bin** folder. The log is generated in this folder.

## 1.1 Features

- You can format your log message in the same way as a `printf`
- You can pass a **`std::string`** to **`%s`**
- You can pass **`bool`** to **`%s`** it will display "true" or "false"
- You can pass **`std::thread::id`** to **`%lld`** (it will be converted to **uint64_t** which is **unsigned long long**)
- The log format is hard coded in `LogManager::writeLog` but you can easily change it.

## 1.2 Example

input:

```c++
m_logger->error("Error receiving packet: %d", 100);
```

output:

```C++
[2020/09/07 17:04:11] [MyClass] [00004704] [ERROR   ] Error receiving packet: 100
```

The log contains a time stamp, the current log name, the thread id and the level.

## 1.3 Initialization

You must write those two lines before using the logger

```c++
LogManager::Instance().setLevel(LogLevel::LL_DEBUG);
LogManager::Instance().open("./STV-Logger"); // create a file like this: STV-Logger-2020-09-07 17-23-43.log
```

In any class you will use a logger with a name of your choice (the name of the class is a good one):

```c++
 std::unique_ptr<Logger> m_logger = LogManager::Instance().getLogger("MyClass");
```

## 1.4 Purge

This implementation DELETE old logs on each start.  See `LogManager::open` to change this if you don't like it.

# 2 Variadic C++ templates

The interesting part of this code is related to the use of variadic recursive c++ template. 

```c++
template<typename First, typename... Rest>
void log(LogLevel level, const char* format, First firstValue, Rest... rest) const
{
    if (!LogManager::Instance().isLoggable(level))
        return;
    std::string r = recurseLog(format, firstValue, rest...);
    LogManager::Instance().writeLog(m_name, level, r);
}
void log(LogLevel level, const char* format) const;
```

The method `recurseLog` is responsible to build the final string that we send to the `LogManager`. The `LogManager` will save it to disk.

At some point in the recursion, each parameter of the message is formatted individually by `Logger::format`

```c++
// NOTE: despite the "..." we have always one single parameter after fmt here.
std::string Logger::format(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	const int size = vsnprintf(NULL, 0, fmt, args) + 1;
	va_end(args);

	char* buffer = new char[size];
	va_start(args, fmt);
	vsnprintf(buffer, size, fmt, args); // formatting occur here
	va_end(args);
	
	std::string result = buffer;
	delete[] buffer;
	buffer = nullptr;
	return result;
}
```

As you can see, the funny thing is that this code mix ultra old API like **va_list** with more recent ones like variadic templates. It's just because I really don't want to re-implement **vsnprintf** which is great as it is.



