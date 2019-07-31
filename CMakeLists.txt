project(loxc)
cmake_minimum_required(VERSION 3.14)
set (CMAKE_CXX_STANDARD 17)

find_package( PythonInterp 3 REQUIRED )

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wno-switch")

string(TOUPPER ${CMAKE_BUILD_TYPE} build_affix)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_custom_target(
 stmt ALL
 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/stmt_generator.py && ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/expression_generator.py
 BYPRODUCTS src/stmt.h src/expr.h
 COMMENT "Doing autogeneration."
)

add_executable(loxc src/main.cc src/scan.cc src/token.cc src/val.cc src/op.cc src/parse.cc)

set(summary
    "=================|  Loxc Config Summary  |==================="
    "\nBUILD_TYPE:          ${build_affix}"
    "\nCXX_FLAGS:           ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_affix}}"
    "\n================================================================="
    )

message("\n" ${summary} "\n")
