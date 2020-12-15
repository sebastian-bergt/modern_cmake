add_executable(lockfree_swsr_ringbuffer_test lockfree_swsr_ringbuffer_test.cpp)

find_package(Threads REQUIRED)

target_link_libraries(lockfree_swsr_ringbuffer_test PRIVATE lockfree_ringbuffer Threads::Threads)

mc_add_test(lockfree_swsr_ringbuffer_test)


# add_executable(lockfree_ringbuffer_two_on_two_test lockfree_ringbuffer_two_on_two_test.cpp)

# find_package(Threads REQUIRED)

# target_link_libraries(lockfree_ringbuffer_two_on_two_test PRIVATE lockfree_ringbuffer Threads::Threads)

# mc_add_test(lockfree_ringbuffer_two_on_two_test)

# add_executable(sandbox_test sandbox_test.cpp)

# find_package(Threads REQUIRED)

# target_link_libraries(sandbox_test PRIVATE Threads::Threads)

# mc_add_test(sandbox_test)