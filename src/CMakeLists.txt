# The MIT License (MIT)
#
# Copyright (c) 2017-2018 Alexander Kurbatov

# All the source files for the bot.
file(GLOB_RECURSE BOT_SOURCES "*.cpp" "*.h")

include_directories(SYSTEM
    ${PROJECT_SOURCE_DIR}/s2client-api/include
    ${PROJECT_SOURCE_DIR}/s2client-api/contrib/protobuf/src
    ${PROJECT_BINARY_DIR}/s2client-api/generated
    "."
)

link_directories(${PROJECT_BINARY_DIR}/s2client-api/bin)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX-")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything \
        -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded \
        -Wno-switch-enum -Wno-weak-vtables -Wno-exit-time-destructors \
        -Wno-float-equal -Wno-global-constructors"
    )
elseif (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic \
        -Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor \
        -Wold-style-cast -Wconversion -Wsign-conversion -Winit-self \
        -Wunreachable-code"
    )
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Create the executable.
add_executable(MulleMech ${BOT_SOURCES} blueprints/Barrack.cpp blueprints/Barrack.h blueprints/Factory.cpp blueprints/Factory.h blueprints/Starport.cpp blueprints/Starport.h plugins/BuildInfo.cpp plugins/BuildInfo.h)
target_link_libraries(MulleMech
    sc2api sc2lib sc2utils sc2protocol civetweb libprotobuf
)

if (MINGW)
    target_link_libraries(MulleMech ssp)
endif()

if (APPLE)
    target_link_libraries(MulleMech "-framework Carbon")
endif ()

# Linux specific.
if (UNIX AND NOT APPLE)
    target_link_libraries(MulleMech pthread dl)
endif ()
