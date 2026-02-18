# CMakeList.txt : CMake project for Homework 1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Homework 1")

# Add source to this project's executable.
cmake_minimum_required(VERSION 3.10)

project("Homework 1" LANGUAGES CXX)

add_executable(CMakeTarget main.cpp)

set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
