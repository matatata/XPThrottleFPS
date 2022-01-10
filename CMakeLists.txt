cmake_minimum_required (VERSION 3.0)
project(XPThrottleFPS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(UNIX AND NOT APPLE)
set(LINUX TRUE)
endif()

if(UNIX)
    set(PLATFORM_COMPILE_OPTIONS -Wall -Wextra -Wfloat-equal -pedantic)
else()
    set(PLATFORM_COMPILE_OPTIONS /W3 /wd4996)
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
    set(PLATFORM_CORE_DEFINITIONS
        APL=1
        LIN=0
        IBM=0)
else()
if(LINUX)
    set(PLATFORM_CORE_DEFINITIONS
        APL=0
        LIN=1
        IBM=0)
else()
    set(PLATFORM_CORE_DEFINITIONS
        APL=0
        LIN=0
        IBM=1)
    set(PLATFORM_DEBUG_OPTIONS "-MTd")
    set(PLATFORM_RELEASE_OPTIONS "-MT")
endif()
endif()

set(DEFINITIONS
    XPLM200=1
    XPLM210=1
    XPLM300=1
    XPLM301=1
    ${PLATFORM_CORE_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}/SDK/CHeaders/XPLM
                    ${CMAKE_SOURCE_DIR}/SDK/CHeaders/Widgets
                    ${PLATFORM_INCLUDE_DIRECTORIES})


set(SOURCES
    XPThrottleFPS.c)


add_library(XPThrottleFPS MODULE ${SOURCES})

target_compile_options(XPThrottleFPS PRIVATE ${PLATFORM_COMPILE_OPTIONS})
target_compile_options(XPThrottleFPS PUBLIC "$<$<CONFIG:DEBUG>:${PLATFORM_DEBUG_OPTIONS}>")
target_compile_options(XPThrottleFPS PUBLIC "$<$<CONFIG:RELEASE>:${PLATFORM_RELEASE_OPTIONS}>")
target_compile_definitions(XPThrottleFPS PRIVATE ${DEFINITIONS})


if (APPLE)
    #target_link_libraries(XPThrottleFPS "-framework OpenGL")
    target_link_libraries(XPThrottleFPS "-framework CoreFoundation")
    target_link_libraries(XPThrottleFPS "-framework XPWidgets")
    target_link_libraries(XPThrottleFPS "-framework XPLM")
    set_target_properties(XPThrottleFPS PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks, -Wl,-F${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac\
    -Wl,-exported_symbol -Wl,_XPluginStart\
    -Wl,-exported_symbol -Wl,_XPluginEnable\
    -Wl,-exported_symbol -Wl,_XPluginReceiveMessage\
    -Wl,-exported_symbol -Wl,_XPluginDisable\
    -Wl,-exported_symbol -Wl,_XPluginStop")
else()
if(LINUX)
    target_compile_options(XPThrottleFPS PRIVATE "-fvisibility=hidden")
else()
    target_link_libraries(XPThrottleFPS "SDK/Libraries/Win/XPWidgets_64")
    target_link_libraries(XPThrottleFPS "SDK/Libraries/Win/XPLM_64")
endif()
endif()

set_target_properties(XPThrottleFPS PROPERTIES PREFIX "")
set_target_properties(XPThrottleFPS PROPERTIES SUFFIX ".xpl")

