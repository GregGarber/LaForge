# Add executable 
add_executable (usonic main Wav CIC)

# lib dependencies
target_link_libraries (usonic LINK_PUBLIC m wiringPi pthread)
target_compile_options(usonic PUBLIC -Wall -Wextra -Wpedantic -O2 -g -pg)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

#LDFLAGS = -lm 
#CFLAGS = -Wall -O3 -I.
#CFLAGS = -Wall -g -p -I. -fsanitize=address -fsanitize=leak -fsanitize=undefined -static-libasan
