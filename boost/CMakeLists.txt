find_package(Boost 1.53.0 REQUIRED COMPONENTS system log)

message(STATUS ${Boost_LIBRARIES})
message(STATUS ${Boost_SYSTEM_LIBRARY})
message(STATUS ${Boost_LOG_LIBRARY})

add_executable(BoostLogTest BoostLogTest.cpp)
target_include_directories(BoostLogTest PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    )
target_link_libraries(BoostLogTest PRIVATE
     ${GTEST_BOTH_LIBRARIES}
     ${CMAKE_THREAD_LIBS_INIT}
     ${Boost_LOG_LIBRARY}
     )
target_compile_features(BoostLogTest PRIVATE
    cxx_constexpr)
target_compile_definitions(BoostLogTest PRIVATE
    BOOST_LOG_DYN_LINK)
    
add_executable(EventfdTest EventfdTest.cpp)
target_include_directories(EventfdTest PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    )
target_link_libraries(EventfdTest PRIVATE
     ${GTEST_BOTH_LIBRARIES}
     ${CMAKE_THREAD_LIBS_INIT}
     ${Boost_LOG_LIBRARY}
     ${Boost_SYSTEM_LIBRARY}
     )
target_compile_features(EventfdTest PRIVATE
    cxx_constexpr)
target_compile_definitions(EventfdTest PRIVATE
    BOOST_LOG_DYN_LINK)
    
add_executable(LockFreeQueueTest LockFreeQueueTest.cpp)
target_include_directories(LockFreeQueueTest PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    )
target_link_libraries(LockFreeQueueTest PRIVATE
     ${GTEST_BOTH_LIBRARIES}
     ${CMAKE_THREAD_LIBS_INIT}
     ${Boost_LOG_LIBRARY}
     ${Boost_SYSTEM_LIBRARY}
     )
target_compile_features(LockFreeQueueTest PRIVATE
    cxx_constexpr)
target_compile_definitions(LockFreeQueueTest PRIVATE
    BOOST_LOG_DYN_LINK)

add_executable(SelectableQueueTest SelectableQueueTest.cpp)
target_include_directories(SelectableQueueTest PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    )
target_link_libraries(SelectableQueueTest PRIVATE
     ${GTEST_BOTH_LIBRARIES}
     ${CMAKE_THREAD_LIBS_INIT}
     ${Boost_LOG_LIBRARY}
     ${Boost_SYSTEM_LIBRARY}
     )
target_compile_features(SelectableQueueTest PRIVATE
    cxx_constexpr)
target_compile_definitions(SelectableQueueTest PRIVATE
    BOOST_LOG_DYN_LINK)

add_test(BoostLogTest BoostLogTest)
add_test(EventfdTest EventfdTest)
add_test(LockFreeQueueTest LockFreeQueueTest)
add_test(SelectableQueueTest SelectableQueueTest)
