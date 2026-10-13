cmake_minimum_required(VERSION 3.24)
project(TrinitySDK VERSION 1.0.0 LANGUAGES NONE)

set(TRINITY_SDK_CORE_REVISION "standalone" CACHE STRING "SDK compatibility identifier")
add_subdirectory(cmake)
