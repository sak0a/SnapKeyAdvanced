cmake_minimum_required(VERSION 3.30)
project(SnapKeyAdvanced)

set(CMAKE_CXX_STANDARD 17)#
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# Ressourcen-Datei und ausführbare Datei hinzufügen
add_executable(SnapKeyAdvanced
        src/SnapKeyAdvanced.cpp
        include/nlohmann/json.hpp
        src/assets/resources.rc
        include/resource.h
)

if (MINGW)
    target_link_libraries(SnapKeyAdvanced -mwindows)  # GUI-Anwendung ohne Konsolenfenster
endif()

# MSVC-spezifische Einstellungen
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set_target_properties(SnapKeyAdvanced PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()