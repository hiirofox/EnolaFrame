cmake_minimum_required(VERSION 3.10)
project(EnolaSAW)

#set(CMAKE_CXX_FLAGS_DEBUG "-Og /MD")
#set(CMAKE_CXX_FLAGS_RELEASE "-Ofast /MD")
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_FLAGS_RELEASE)
    if(MSVC)# MSVC ≈‰÷√
        add_compile_options(
              /GL /Gw /DNDEBUG /EHs-c- /Zc:threadSafeInit-
        )
        add_link_options(
            /LTCG /OPT:REF /OPT:ICF /ENTRY:mainCRTStartup
        )
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    else() # MinGW
        add_compile_options(
         -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti
        )
        add_link_options(
        -Wl,--gc-sections -Wl,--strip-all -s -mwindows
        )
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${WINDOWS_SDK_LIB_PATH})
add_executable(EnolaSAW 
    src/main.cpp 
    src/dbg.cpp
    src/Waveout.cpp
)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    target_link_libraries(EnolaSAW ${OPENGL_LIBRARIES})
else()
    message(FATAL_ERROR "OpenGL not found!")
endif()

