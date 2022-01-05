set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
add_executable(backup_service
    main.cpp
 )

target_include_directories(backup_service
    PRIVATE ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/cppzmq
)
target_link_libraries(backup_service
    PRIVATE redis++::redis++_static
    PRIVATE Threads::Threads
    PUBLIC libzmq)

if (APPLE)
    set (BINARY_INSTALL_PATH
        bin/macos)
else (APPLE)
    set (BINARY_INSTALL_PATH
        bin)
endif (APPLE)
install(TARGETS backup_service
    RUNTIME DESTINATION ${BINARY_INSTALL_PATH})

