# If Boost is not found call with -DBOOSTROOT=<path_to_boost>

project (osmid)
cmake_minimum_required (VERSION 3.0)
# Need cmake > 3.1, and debian jessie has 3.0.2
#set(CMAKE_CXX_STANDARD 14)
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
    src/utils.cpp
    JuceLibraryCode/juce_audio_basics.cpp
    JuceLibraryCode/juce_audio_devices.cpp
    JuceLibraryCode/juce_audio_formats.cpp
    JuceLibraryCode/juce_core.cpp
    JuceLibraryCode/juce_data_structures.cpp
    JuceLibraryCode/juce_events.cpp
    JuceLibraryCode/juce_osc.cpp
)

link_directories(${Boost_LIBRARY_DIR_DEBUG})
add_executable(m2o ${m2o_sources})
target_link_libraries(m2o debug ${Boost_SYSTEM_LIBRARY_DEBUG} optimized ${Boost_SYSTEM_LIBRARY_RELEASE})
target_link_libraries(m2o debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG} optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE})

add_definitions("-DBOOST_REGEX_NO_LIB")

if(MSVC)
    add_definitions("-D__WINDOWS_MM__ -D_WIN32_WINNT=0x0600")
    target_link_libraries(m2o winmm Ws2_32)
elseif(APPLE)
    add_definitions("-D__MACOSX_CORE__")
    set(CMAKE_EXE_LINKER_FLAGS "-framework CoreMIDI -framework CoreAudio -framework CoreFoundation")
elseif(UNIX)
    add_definitions("-D__LINUX_ALSA__")
    target_link_libraries(m2o pthread ${ALSA_LIBRARY})
endif(MSVC)

if(UNIX)
    install (TARGETS m2o DESTINATION bin)
endif(UNIX)
