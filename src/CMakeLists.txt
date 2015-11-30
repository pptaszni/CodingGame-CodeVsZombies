set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11")


set(CMAKE_EXE_LINKER_FLAGS "-lm")

add_library(GameController STATIC GameController.cpp)
add_executable(main main.cpp)

target_link_libraries(main GameController)
