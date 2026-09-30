# Collect all source and header files inside src/
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.c
    src/*.hpp
    src/*.h
)

add_executable(SEngineApp ${APP_SOURCES})

target_link_libraries(SEngineApp PRIVATE SEngine)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${APP_SOURCES})

target_include_directories(SEngineApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

install(TARGETS SEngineApp
    RUNTIME DESTINATION bin
)

include(CompileShaders)
compile_project_shaders(SEngineApp)