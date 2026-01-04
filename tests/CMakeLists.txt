cmake_minimum_required(VERSION 3.15)

# Test executable
add_executable(test_transpiler
    test_main.cpp
    test_type_mapping.cpp
    test_codegen.cpp
)

target_include_directories(test_transpiler PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Link against source files (same as main executable)
target_sources(test_transpiler PRIVATE
    ${CMAKE_SOURCE_DIR}/src/transpiler.cpp
    ${CMAKE_SOURCE_DIR}/src/ir/ir_builder.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/type_mapper.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/simple_cpp_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/codegen/codegen_base.cpp
    ${CMAKE_SOURCE_DIR}/src/codegen/rust/rust_codegen.cpp
    ${CMAKE_SOURCE_DIR}/src/codegen/go/go_codegen.cpp
)

# Link against Clang and LLVM
if(TARGET clang-cpp)
    target_link_libraries(test_transpiler clang-cpp LLVM)
else()
    target_link_libraries(test_transpiler
        clangTooling clangFrontend clangDriver clangSerialization
        clangParse clangSema clangAnalysis clangAST clangBasic
        clangEdit clangLex clangRewrite
    )
    llvm_map_components_to_libnames(test_llvm_libs support core irreader)
    target_link_libraries(test_transpiler ${test_llvm_libs})
endif()

# Add tests to CTest
add_test(NAME TypeMappingTests COMMAND test_transpiler --test-type-mapping)
add_test(NAME CodegenTests COMMAND test_transpiler --test-codegen)
