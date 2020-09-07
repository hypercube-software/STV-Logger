cmake_minimum_required(VERSION 3.17)

project (STV-Logger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
			
add_executable(STV-Logger 
	main.cpp
	Logger.cpp
	Logger.h)