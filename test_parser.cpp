/**
 * Simple test program for the C++ class parser
 */

#include "parser.h"
#include "codegen.h"
#include <iostream>

using namespace hybrid;

int main() {
    std::cout << "=== Hybrid Transpiler - Parser Test ===\n\n";

    try {
        // Parse the example file
        std::string input_file = "examples/simple_class.cpp";
        std::cout << "Parsing: " << input_file << "\n\n";

        IR ir = Parser::parseFile(input_file);

        // Display parsed classes
        std::cout << "Parsed " << ir.getClasses().size() << " classes:\n";
        for (const auto& class_decl : ir.getClasses()) {
            std::cout << "  - Class: " << class_decl.name << "\n";
            std::cout << "    Fields: " << class_decl.fields.size() << "\n";
            std::cout << "    Methods: " << class_decl.methods.size() << "\n";

            if (!class_decl.base_classes.empty()) {
                std::cout << "    Base classes: ";
                for (const auto& base : class_decl.base_classes) {
                    std::cout << base << " ";
                }
                std::cout << "\n";
            }
        }

        std::cout << "\n=== Generating Rust Code ===\n\n";
        RustCodeGenerator rust_gen;
        std::string rust_code = rust_gen.generate(ir);
        std::cout << rust_code << "\n";

        std::cout << "\n=== Generating Go Code ===\n\n";
        GoCodeGenerator go_gen;
        std::string go_code = go_gen.generate(ir);
        std::cout << go_code << "\n";

        std::cout << "=== Test Complete ===\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
