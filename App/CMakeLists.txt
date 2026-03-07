add_executable(SEngineApp main.cpp)

target_link_libraries(SEngineApp PRIVATE SEngine)

install(TARGETS SEngineApp
    RUNTIME DESTINATION bin/$<CONFIG>
)