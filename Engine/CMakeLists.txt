find_package(Vulkan REQUIRED)
add_subdirectory(thirdparty)

add_library(SEngine)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS include/*.h)

target_sources(SEngine 
    PRIVATE ${ENGINE_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${ENGINE_HEADERS}
)

target_include_directories(SEngine 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(SEngine
    PUBLIC
        Vulkan::Vulkan #also gives glm
        glfw
        tinyobjloader
        tinygltf
        nlohmann_json
        stb
        OpenAL
        spdlog
        imgui
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
             FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

#Archive for .a / .lib | Library for .so / .dll
install(TARGETS SEngine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    FILE_SET HEADERS DESTINATION include
)