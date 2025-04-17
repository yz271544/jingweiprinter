cmake_minimum_required(VERSION 3.25)
project(jingweiprinter)

include(FetchContent)

#set($ENV{http_proxy} "http://127.0.0.1:10808")
#set($ENV{https_proxy} "http://127.0.0.1:10808")

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

message("CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message("CMAKE_AUTOMOC: ${CMAKE_AUTOMOC}")
message("CMAKE_AUTOUIC: ${CMAKE_AUTOUIC}")
message("CMAKE_AUTORCC: ${CMAKE_AUTORCC}")


if (MSVC)
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    #set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /ignore:4099")

    #add_compile_options(/wd4267)
    add_compile_options(/wd4267 /ignore:4099)
    add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

    #set(CMAKE_TOOLCHAIN_FILE "" CACHE STRING "Clear toolchain" FORCE)
    # 彻底禁用 vcpkg
    set(CMAKE_TOOLCHAIN_FILE "" CACHE STRING "Clear vcpkg" FORCE)
    unset(VCPKG_TARGET_TRIPLET CACHE)
    unset(VCPKG_APPLOCAL_DEPS CACHE)

    # 可选：静态链接
    set(BUILD_SHARED_LIBS OFF)
    # 关键：强制使用 FetchContent 的版本
    set(OATPP_MODULE_LOCATION "BUNDLED")

    set(oatpp_VERSION 1.3.1)
    set(oatpp_FOUND TRUE)
endif()

# 下载 oatpp 1.3.1
FetchContent_Declare(
    oatpp
    GIT_REPOSITORY https://github.com/oatpp/oatpp.git
    GIT_TAG 1.3.1
)

# 下载 oatpp-curl 1.3.0
FetchContent_Declare(
    oatpp-curl
    GIT_REPOSITORY https://github.com/oatpp/oatpp-curl.git
    GIT_TAG 1.3.0
    PATCH_COMMAND 
    ${CMAKE_COMMAND} -E replace 
    "${oatpp-curl_SOURCE_DIR}/CMakeLists.txt" 
    "find_package(oatpp 1.3.0 EXACT)" 
    "find_package(oatpp 1.3.1 EXACT)"
)

# 分步编译
FetchContent_GetProperties(oatpp)
if(NOT oatpp_POPULATED)
    FetchContent_Populate(oatpp)
    add_subdirectory(${oatpp_SOURCE_DIR} ${oatpp_BINARY_DIR})
endif()

FetchContent_GetProperties(oatpp-curl)
if(NOT oatpp-curl_POPULATED)
    FetchContent_Populate(oatpp-curl)
    add_subdirectory(${oatpp-curl_SOURCE_DIR} ${oatpp-curl_BINARY_DIR})
endif()

# 其他依赖（yaml-cpp, spdlog）
FetchContent_MakeAvailable(yaml-cpp spdlog)

message("oatpp_VERSION: ${oatpp_VERSION}")
message("oatpp-curl_VERSION: ${oatpp_VERSION}")

include_directories(src)
add_subdirectory(src)
add_subdirectory(lib/googletest)
add_subdirectory(tests)

