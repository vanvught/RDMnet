################################### testing ###################################

set(RDMNET_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../../include)
set(RDMNET_SRC ${CMAKE_CURRENT_LIST_DIR}/../../src)

include(GoogleTest)

include_directories(test_shared)

add_subdirectory(core)
add_subdirectory(api)
add_subdirectory(broker)
add_subdirectory(discovery)
