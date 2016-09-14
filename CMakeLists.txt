# If Boost is not found call with -DBOOSTROOT=<path_to_boost>

project (osmid)
cmake_minimum_required (VERSION 3.0)
# Need cmake > 3.1, and debian jessie has 3.0.2
#set(CMAKE_CXX_STANDARD 14)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif(NOT MSVC)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS system program_options)
if(APPLE)
    
elseif(UNIX)
    find_package(ALSA REQUIRED)
endif(APPLE)


include_directories(${Boost_INCLUDE_DIR} JuceLibraryCode JuceLibraryCode/modules)

set(m2o_sources
    src/m2o.cpp
    src/midiin.cpp
    src/oscout.cpp
    src/midiinprocessor.cpp
    src/midicommon.cpp
    src/utils.cpp
)

set(o2m_sources
    src/o2m.cpp
    src/midiout.cpp
    src/oscin.cpp
    src/midicommon.cpp
    src/oscinprocessor.cpp
)

if(APPLE)
    set(juce_sources
        JuceLibraryCode/juce_audio_basics.mm
        JuceLibraryCode/juce_audio_devices.mm
        JuceLibraryCode/juce_audio_formats.mm
        JuceLibraryCode/juce_core.mm
        JuceLibraryCode/juce_data_structures.mm
        JuceLibraryCode/juce_events.mm
        JuceLibraryCode/juce_osc.cpp
    )
else(APPLE)
    set(juce_sources
        JuceLibraryCode/juce_audio_basics.cpp
        JuceLibraryCode/juce_audio_devices.cpp
        JuceLibraryCode/juce_audio_formats.cpp
        JuceLibraryCode/juce_core.cpp
        JuceLibraryCode/juce_data_structures.cpp
        JuceLibraryCode/juce_events.cpp
        JuceLibraryCode/juce_osc.cpp
    )
endif(APPLE)

link_directories(${Boost_LIBRARY_DIR_DEBUG})
# m2o
add_executable(m2o ${m2o_sources} ${juce_sources})
target_link_libraries(m2o debug ${Boost_SYSTEM_LIBRARY_DEBUG} optimized ${Boost_SYSTEM_LIBRARY_RELEASE})
target_link_libraries(m2o debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG} optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE})

#o2m
add_executable(o2m ${o2m_sources} ${juce_sources})
target_link_libraries(o2m debug ${Boost_SYSTEM_LIBRARY_DEBUG} optimized ${Boost_SYSTEM_LIBRARY_RELEASE})
target_link_libraries(o2m debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG} optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE})


add_definitions("-DBOOST_REGEX_NO_LIB")

if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0600 -DJUCER_VS2015_78A5022=1 -DJUCE_APP_VERSION=1.0.0 -DJUCE_APP_VERSION_HEX=0x10000)
    target_link_libraries(m2o winmm Ws2_32)
    target_link_libraries(o2m winmm Ws2_32)
elseif(APPLE)
    add_definitions(-DNDEBUG=1 -DJUCER_XCODE_MAC_F6D2F4CF=1 -DJUCE_APP_VERSION=1.0.0 -DJUCE_APP_VERSION_HEX=0x10000)
    set_target_properties(m2o PROPERTIES XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO")
    set_target_properties(o2m PROPERTIES XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO")
    set(CMAKE_EXE_LINKER_FLAGS "-framework CoreMIDI -framework CoreAudio -framework CoreFoundation -framework Accelerate -framework QuartzCore -framework AudioToolbox -framework IOKit -framework DiscRecording -framework Cocoa")
elseif(UNIX)
    add_definitions(-DLINUX=1 -DNDEBUG=1 -DJUCER_LINUX_MAKE_6D53C8B4=1 -DJUCE_APP_VERSION=1.0.0 -DJUCE_APP_VERSION_HEX=0x10000)
    target_link_libraries(m2o pthread ${ALSA_LIBRARY} dl rt X11)
    target_link_libraries(o2m pthread ${ALSA_LIBRARY} dl rt X11)
endif(MSVC)

if(UNIX)
    install (TARGETS m2o DESTINATION bin)
endif(UNIX)
