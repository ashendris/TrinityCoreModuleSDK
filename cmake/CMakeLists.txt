set(TRINITY_SDK_VERSION_MAJOR 1)
set(TRINITY_SDK_VERSION_MINOR 0)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(TRINITY_SDK_COMPILER "standalone")
set(TRINITY_SDK_CRT "default")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TRINITY_SDK_ARCH "x64")
else()
  set(TRINITY_SDK_ARCH "x86")
endif()
set(TRINITY_SDK_ABI_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/trinity/TrinityAbiV1.h")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_trinity_abi.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/trinity_abi_v1.json" "${TRINITY_SDK_ABI_HEADER}"
  COMMAND_ERROR_IS_FATAL ANY)
configure_file(TrinitySDKBuild.h.in generated/trinity/TrinitySDKBuild.h @ONLY)

add_library(TrinitySDK INTERFACE)
add_library(Trinity::SDK ALIAS TrinitySDK)
set_target_properties(TrinitySDK PROPERTIES EXPORT_NAME SDK)
target_include_directories(TrinitySDK INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/trinity>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/trinity>"
  "$<INSTALL_INTERFACE:include>"
  "$<INSTALL_INTERFACE:include/trinity>")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/" DESTINATION include)
install(FILES "${TRINITY_SDK_ABI_HEADER}" "${CMAKE_CURRENT_BINARY_DIR}/generated/trinity/TrinitySDKBuild.h" DESTINATION include/trinity)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/TrinitySDKConfigVersion.cmake" VERSION "${TRINITY_SDK_VERSION_MAJOR}.${TRINITY_SDK_VERSION_MINOR}.0" COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
configure_package_config_file(TrinitySDKConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/TrinitySDKConfig.cmake" INSTALL_DESTINATION lib/cmake/TrinitySDK)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TrinitySDKConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/TrinitySDKConfigVersion.cmake" TrinitySDKModules.cmake DESTINATION lib/cmake/TrinitySDK)
install(TARGETS TrinitySDK EXPORT TrinitySDKTargets)
install(EXPORT TrinitySDKTargets NAMESPACE Trinity:: DESTINATION lib/cmake/TrinitySDK)
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/../tools/sign_module.py" DESTINATION bin)
