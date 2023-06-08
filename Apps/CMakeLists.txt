set(SOURCES Source/main.cpp)

add_executable(Viewer WIN32 ${SOURCES})
target_include_directories(Viewer PUBLIC Include PRIVATE Source)

set_property(TARGET Viewer PROPERTY FOLDER Apps)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Viewer)

target_link_libraries(Viewer Engine)