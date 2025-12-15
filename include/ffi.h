/**
 * @file ffi.h
 * @brief Foreign Function Interface (FFI) generation for Rust and Go
 *
 * This module analyzes C++ code and generates FFI bindings that allow
 * Rust and Go to call C++ functions directly through C ABI.
 */

#ifndef HYBRID_TRANSPILER_FFI_H
#define HYBRID_TRANSPILER_FFI_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace hybrid_transpiler {
namespace ffi {

/**
 * @brief Represents a function parameter for FFI
 */
struct FFIParameter {
    std::string name;
    std::string cpp_type;      // Original C++ type
    std::string c_type;        // C-compatible type
    std::string rust_type;     // Rust FFI type
    std::string go_type;       // Go FFI type (cgo)
    bool is_pointer;
    bool is_const;
    bool is_reference;
};

/**
 * @brief Represents a function that can be exposed via FFI
 */
struct FFIFunction {
    std::string name;
    std::string mangled_name;   // C++ mangled name
    std::string c_name;         // C-compatible name (extern "C")
    std::string return_type;    // Original C++ return type
    std::string c_return_type;  // C-compatible return type
    std::vector<FFIParameter> parameters;
    bool is_method;             // true if member function
    bool is_static;             // true if static member function
    bool is_const;              // true if const member function
    std::string class_name;     // Class name if member function
    bool is_virtual;            // true if virtual function
    bool can_use_ffi;           // true if FFI-compatible
    std::string reason;         // Reason if not FFI-compatible
};

/**
 * @brief Represents a class/struct for FFI
 */
struct FFIClass {
    std::string name;
    std::vector<FFIFunction> methods;
    std::vector<FFIFunction> static_methods;
    std::vector<FFIParameter> fields;
    bool has_virtual_functions;
    bool is_polymorphic;
    bool is_abstract;
    size_t size;                // Size in bytes
    size_t alignment;           // Alignment requirement
};

/**
 * @brief FFI compatibility analyzer
 *
 * Analyzes C++ functions and classes to determine if they can be
 * safely exposed through C ABI for FFI usage.
 */
class FFIAnalyzer {
public:
    FFIAnalyzer() = default;
    ~FFIAnalyzer() = default;

    /**
     * @brief Analyze a C++ function for FFI compatibility
     * @param function_decl Function declaration to analyze
     * @return FFIFunction with compatibility information
     */
    FFIFunction analyzeFunction(const std::string& function_decl);

    /**
     * @brief Analyze a C++ class for FFI compatibility
     * @param class_decl Class declaration to analyze
     * @return FFIClass with compatibility information
     */
    FFIClass analyzeClass(const std::string& class_decl);

    /**
     * @brief Check if a C++ type is FFI-compatible
     * @param cpp_type C++ type to check
     * @return true if FFI-compatible
     */
    bool isFFICompatible(const std::string& cpp_type);

    /**
     * @brief Convert C++ type to C-compatible type
     * @param cpp_type C++ type
     * @return C-compatible type
     */
    std::string toCType(const std::string& cpp_type);

    /**
     * @brief Convert C++ type to Rust FFI type
     * @param cpp_type C++ type
     * @return Rust FFI type
     */
    std::string toRustFFIType(const std::string& cpp_type);

    /**
     * @brief Convert C++ type to Go FFI type (cgo)
     * @param cpp_type C++ type
     * @return Go FFI type
     */
    std::string toGoFFIType(const std::string& cpp_type);

private:
    /**
     * @brief Type mapping tables
     */
    std::unordered_map<std::string, std::string> cpp_to_c_types_;
    std::unordered_map<std::string, std::string> cpp_to_rust_types_;
    std::unordered_map<std::string, std::string> cpp_to_go_types_;

    /**
     * @brief Initialize type mapping tables
     */
    void initializeTypeMappings();
};

/**
 * @brief Rust FFI code generator
 */
class RustFFIGenerator {
public:
    RustFFIGenerator() = default;
    ~RustFFIGenerator() = default;

    /**
     * @brief Generate Rust FFI bindings for a function
     * @param func FFI function descriptor
     * @return Rust FFI code
     */
    std::string generateFunctionBinding(const FFIFunction& func);

    /**
     * @brief Generate Rust FFI bindings for a class
     * @param cls FFI class descriptor
     * @return Rust FFI code
     */
    std::string generateClassBinding(const FFIClass& cls);

    /**
     * @brief Generate safe Rust wrapper for FFI function
     * @param func FFI function descriptor
     * @return Safe Rust wrapper code
     */
    std::string generateSafeWrapper(const FFIFunction& func);

    /**
     * @brief Generate complete Rust FFI module
     * @param functions List of FFI functions
     * @param classes List of FFI classes
     * @param library_name Name of the C++ library
     * @return Complete Rust module code
     */
    std::string generateModule(
        const std::vector<FFIFunction>& functions,
        const std::vector<FFIClass>& classes,
        const std::string& library_name
    );
};

/**
 * @brief Go FFI code generator (cgo)
 */
class GoFFIGenerator {
public:
    GoFFIGenerator() = default;
    ~GoFFIGenerator() = default;

    /**
     * @brief Generate cgo bindings for a function
     * @param func FFI function descriptor
     * @return Go cgo code
     */
    std::string generateFunctionBinding(const FFIFunction& func);

    /**
     * @brief Generate cgo bindings for a class
     * @param cls FFI class descriptor
     * @return Go cgo code
     */
    std::string generateClassBinding(const FFIClass& cls);

    /**
     * @brief Generate Go wrapper for FFI function
     * @param func FFI function descriptor
     * @return Go wrapper code
     */
    std::string generateWrapper(const FFIFunction& func);

    /**
     * @brief Generate complete Go package with cgo
     * @param functions List of FFI functions
     * @param classes List of FFI classes
     * @param library_name Name of the C++ library
     * @return Complete Go package code
     */
    std::string generatePackage(
        const std::vector<FFIFunction>& functions,
        const std::vector<FFIClass>& classes,
        const std::string& library_name
    );
};

/**
 * @brief C wrapper generator for C++ code
 *
 * Generates extern "C" wrappers that expose C++ functionality
 * through C ABI for FFI consumption.
 */
class CWrapperGenerator {
public:
    CWrapperGenerator() = default;
    ~CWrapperGenerator() = default;

    /**
     * @brief Generate C wrapper for a C++ function
     * @param func FFI function descriptor
     * @return C wrapper code
     */
    std::string generateFunctionWrapper(const FFIFunction& func);

    /**
     * @brief Generate C wrapper for a C++ class
     * @param cls FFI class descriptor
     * @return C wrapper code
     */
    std::string generateClassWrapper(const FFIClass& cls);

    /**
     * @brief Generate C header file
     * @param functions List of FFI functions
     * @param classes List of FFI classes
     * @param library_name Name of the library
     * @return C header file content
     */
    std::string generateHeader(
        const std::vector<FFIFunction>& functions,
        const std::vector<FFIClass>& classes,
        const std::string& library_name
    );

    /**
     * @brief Generate C implementation file
     * @param functions List of FFI functions
     * @param classes List of FFI classes
     * @param library_name Name of the library
     * @return C implementation file content
     */
    std::string generateImplementation(
        const std::vector<FFIFunction>& functions,
        const std::vector<FFIClass>& classes,
        const std::string& library_name
    );
};

/**
 * @brief Main FFI generation coordinator
 */
class FFIGenerator {
public:
    FFIGenerator();
    ~FFIGenerator() = default;

    /**
     * @brief Generate FFI bindings for C++ source
     * @param cpp_source C++ source code
     * @param library_name Name of the library
     * @param target_lang Target language ("rust" or "go")
     * @return FFI binding code
     */
    std::string generate(
        const std::string& cpp_source,
        const std::string& library_name,
        const std::string& target_lang
    );

    /**
     * @brief Generate C wrapper layer
     * @param cpp_source C++ source code
     * @param library_name Name of the library
     * @return Pair of (header, implementation)
     */
    std::pair<std::string, std::string> generateCWrapper(
        const std::string& cpp_source,
        const std::string& library_name
    );

private:
    FFIAnalyzer analyzer_;
    RustFFIGenerator rust_generator_;
    GoFFIGenerator go_generator_;
    CWrapperGenerator c_wrapper_generator_;
};

} // namespace ffi
} // namespace hybrid_transpiler

#endif // HYBRID_TRANSPILER_FFI_H
