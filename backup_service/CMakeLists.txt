add_executable(backup_service
    main.cpp
 )

target_include_directories(backup_service
    PRIVATE ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/cppzmq
)
target_link_libraries(backup_service
    PUBLIC libzmq)
