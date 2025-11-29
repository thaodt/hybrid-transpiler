#include "transpiler.h"
#include "ir.h"
#include "codegen.h"
#include "parser.h"
#include <fstream>
#include <sstream>

namespace hybrid {

Transpiler::Transpiler(const TranspilerOptions& options)
    : options_(options), ir_(std::make_unique<IR>()) {

    // Create appropriate code generator based on target
    if (options.target == TargetLanguage::Rust) {
        codegen_ = std::make_unique<RustCodeGenerator>();
    } else if (options.target == TargetLanguage::Go) {
        codegen_ = std::make_unique<GoCodeGenerator>();
    }
}

Transpiler::~Transpiler() = default;

bool Transpiler::transpile(const std::string& input_path) {
    // Parse the input file
    if (!parseSourceFile(input_path)) {
        return false;
    }

    // Generate output code
    if (!generateCode(options_.output_path)) {
        return false;
    }

    return true;
}

bool Transpiler::transpileBatch(const std::vector<std::string>& input_paths) {
    for (const auto& path : input_paths) {
        if (!transpile(path)) {
            return false;
        }
    }
    return true;
}

bool Transpiler::parseSourceFile(const std::string& input_path) {
    try {
        // Use the simple C++ parser to parse the source file
        // This will be replaced with full Clang LibTooling in the future
        *ir_ = Parser::parseFile(input_path);

        // TODO (future): Add additional analysis passes:
        // 1. Ownership analysis for smart pointers
        // 2. Lifetime inference for references
        // 3. Safety validation
        // 4. Performance optimization hints

        return true;
    }
    catch (const std::exception& e) {
        last_error_ = "Failed to parse input file: " + std::string(e.what());
        return false;
    }
}

bool Transpiler::generateCode(const std::string& output_path) {
    if (!codegen_) {
        last_error_ = "Code generator not initialized";
        return false;
    }

    // Generate code from IR
    std::string generated_code = codegen_->generate(*ir_);

    // Write to output file
    std::ofstream out_file(output_path);
    if (!out_file.is_open()) {
        last_error_ = "Failed to open output file: " + output_path;
        return false;
    }

    out_file << generated_code;
    out_file.close();

    return true;
}

} // namespace hybrid
