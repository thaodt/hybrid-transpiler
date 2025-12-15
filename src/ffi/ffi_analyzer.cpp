/**
 * @file ffi_analyzer.cpp
 * @brief FFI compatibility analyzer implementation
 */

#include "ffi.h"
#include <algorithm>
#include <regex>

namespace hybrid_transpiler {
namespace ffi {

void FFIAnalyzer::initializeTypeMappings() {
    // C++ to C type mappings
    cpp_to_c_types_ = {
        {"void", "void"},
        {"bool", "_Bool"},
        {"char", "char"},
        {"unsigned char", "unsigned char"},
        {"short", "short"},
        {"unsigned short", "unsigned short"},
        {"int", "int"},
        {"unsigned int", "unsigned int"},
        {"long", "long"},
        {"unsigned long", "unsigned long"},
        {"long long", "long long"},
        {"unsigned long long", "unsigned long long"},
        {"float", "float"},
        {"double", "double"},
        {"int8_t", "int8_t"},
        {"int16_t", "int16_t"},
        {"int32_t", "int32_t"},
        {"int64_t", "int64_t"},
        {"uint8_t", "uint8_t"},
        {"uint16_t", "uint16_t"},
        {"uint32_t", "uint32_t"},
        {"uint64_t", "uint64_t"},
        {"size_t", "size_t"},
        {"const char*", "const char*"},
        {"char*", "char*"},
    };

    // C++ to Rust FFI type mappings
    cpp_to_rust_types_ = {
        {"void", "()"},
        {"bool", "bool"},
        {"char", "i8"},
        {"unsigned char", "u8"},
        {"short", "i16"},
        {"unsigned short", "u16"},
        {"int", "i32"},
        {"unsigned int", "u32"},
        {"long", "i64"},
        {"unsigned long", "u64"},
        {"long long", "i64"},
        {"unsigned long long", "u64"},
        {"float", "f32"},
        {"double", "f64"},
        {"int8_t", "i8"},
        {"int16_t", "i16"},
        {"int32_t", "i32"},
        {"int64_t", "i64"},
        {"uint8_t", "u8"},
        {"uint16_t", "u16"},
        {"uint32_t", "u32"},
        {"uint64_t", "u64"},
        {"size_t", "usize"},
        {"const char*", "*const i8"},
        {"char*", "*mut i8"},
    };

    // C++ to Go (cgo) type mappings
    cpp_to_go_types_ = {
        {"void", ""},
        {"bool", "C.bool"},
        {"char", "C.char"},
        {"unsigned char", "C.uchar"},
        {"short", "C.short"},
        {"unsigned short", "C.ushort"},
        {"int", "C.int"},
        {"unsigned int", "C.uint"},
        {"long", "C.long"},
        {"unsigned long", "C.ulong"},
        {"long long", "C.longlong"},
        {"unsigned long long", "C.ulonglong"},
        {"float", "C.float"},
        {"double", "C.double"},
        {"int8_t", "int8"},
        {"int16_t", "int16"},
        {"int32_t", "int32"},
        {"int64_t", "int64"},
        {"uint8_t", "uint8"},
        {"uint16_t", "uint16"},
        {"uint32_t", "uint32"},
        {"uint64_t", "uint64"},
        {"size_t", "C.size_t"},
        {"const char*", "*C.char"},
        {"char*", "*C.char"},
    };
}

FFIFunction FFIAnalyzer::analyzeFunction(const std::string& function_decl) {
    FFIFunction func;
    func.can_use_ffi = true;

    // Simple parsing - in real implementation would use Clang AST
    // This is a placeholder that demonstrates the concept

    // Check for exceptions
    if (function_decl.find("throw") != std::string::npos) {
        func.can_use_ffi = false;
        func.reason = "Function may throw exceptions (not compatible with C ABI)";
    }

    // Check for templates
    if (function_decl.find("template") != std::string::npos) {
        func.can_use_ffi = false;
        func.reason = "Template functions require monomorphization";
    }

    // Check for non-POD types in signature
    if (function_decl.find("std::") != std::string::npos) {
        func.can_use_ffi = false;
        func.reason = "Uses C++ standard library types (not C ABI compatible)";
    }

    return func;
}

FFIClass FFIAnalyzer::analyzeClass(const std::string& class_decl) {
    FFIClass cls;

    // Analyze if class has virtual functions
    cls.has_virtual_functions = (class_decl.find("virtual") != std::string::npos);
    cls.is_polymorphic = cls.has_virtual_functions;
    cls.is_abstract = (class_decl.find("= 0") != std::string::npos);

    return cls;
}

bool FFIAnalyzer::isFFICompatible(const std::string& cpp_type) {
    // Remove const, volatile, etc.
    std::string clean_type = cpp_type;
    clean_type.erase(remove(clean_type.begin(), clean_type.end(), ' '), clean_type.end());

    // Check if in mapping table
    return cpp_to_c_types_.find(clean_type) != cpp_to_c_types_.end();
}

std::string FFIAnalyzer::toCType(const std::string& cpp_type) {
    auto it = cpp_to_c_types_.find(cpp_type);
    if (it != cpp_to_c_types_.end()) {
        return it->second;
    }

    // Handle pointers
    if (cpp_type.back() == '*') {
        std::string base_type = cpp_type.substr(0, cpp_type.length() - 1);
        return toCType(base_type) + "*";
    }

    return "void*";  // Fallback for unknown types
}

std::string FFIAnalyzer::toRustFFIType(const std::string& cpp_type) {
    auto it = cpp_to_rust_types_.find(cpp_type);
    if (it != cpp_to_rust_types_.end()) {
        return it->second;
    }

    // Handle pointers
    if (cpp_type.find("*") != std::string::npos) {
        bool is_const = (cpp_type.find("const") != std::string::npos);
        bool is_mut = !is_const;

        return is_mut ? "*mut std::ffi::c_void" : "*const std::ffi::c_void";
    }

    return "*mut std::ffi::c_void";  // Fallback
}

std::string FFIAnalyzer::toGoFFIType(const std::string& cpp_type) {
    auto it = cpp_to_go_types_.find(cpp_type);
    if (it != cpp_to_go_types_.end()) {
        return it->second;
    }

    // Handle pointers
    if (cpp_type.find("*") != std::string::npos) {
        return "unsafe.Pointer";
    }

    return "unsafe.Pointer";  // Fallback
}

} // namespace ffi
} // namespace hybrid_transpiler
