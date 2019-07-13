# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 3.7.2)
project (laforge)

add_subdirectory (PDMmic)
#target_compile_features(usonic PUBLIC "cxx_std_14")
#target_compile_options(usonic PUBLIC -Wall -Wextra -Wpedantic -Werror -g -p)
target_compile_options(usonic PUBLIC -Wall -Wextra -Wpedantic  -g -pg)
#target_compile_options(usonic Wall  )


