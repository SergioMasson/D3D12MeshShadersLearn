set(ENGINE_SOURCES 
    "Source/CoreApp.cpp"
    "Source/p_CoreApp.h"
    "Source/d3dx12.h"
    "Source/Utility.cpp"
    "Source/SystemTime.cpp"
    "Source/Graphics.cpp"
    "Include/Engine/CoreApp.h"
    "Include/Engine/Utility.h"
    "Include/Engine/SystemTime.h"
    "Include/Engine/Graphics.h")

set(ENGINE_NAME Engine)

add_library(${ENGINE_NAME} ${ENGINE_SOURCES})
target_include_directories(${ENGINE_NAME} PUBLIC Include PRIVATE Source)

set_property(TARGET ${ENGINE_NAME} PROPERTY FOLDER Core)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES})