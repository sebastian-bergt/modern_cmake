add_library(lockfree_ringbuffer 
  src/lockfree_ringbuffer.cpp 
  include/lockfree_ringbuffer/i_lockfree_ringbuffer.h
  include/lockfree_ringbuffer/lockfree_ringbuffer.ipp 
  include/lockfree_ringbuffer/lockfree_ringbuffer.h 
)

target_include_directories(
  lockfree_ringbuffer
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

add_subdirectory(tests)

mc_install(lockfree_ringbuffer)
