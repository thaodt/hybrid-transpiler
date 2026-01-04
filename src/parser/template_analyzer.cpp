/**
 * Template Analyzer
 * Analyzes C++ templates and prepares for conversion to Rust/Go generics
 */

#include "ir.h"
#include <regex>
#include <algorithm>
#include <sstream>

namespace hybrid {

/**
 * Template Analyzer
 * Detects and analyzes template declarations, specializations, and instantiations
 */
class TemplateAnalyzer {
public:
    /**
     * Analyze template class declaration
     */
    void analyzeTemplateClass(ClassDecl& class_decl, const std::string& template_decl) {
        class_decl.is_template = true;
        parseTemplateParameters(template_decl, class_decl.template_parameters);
    }

    /**
     * Analyze template function declaration
     */
    void analyzeTemplateFunction(Function& func, const std::string& template_decl) {
        func.is_template = true;
        parseTemplateParameters(template_decl, func.template_parameters);
    }

private:
    /**
     * Parse template parameter list
     * e.g., "template<typename T, int N>" -> [T (Type), N (NonType)]
     */
    void parseTemplateParameters(const std::string& decl,
                                 std::vector<TemplateParameter>& params) {
        // Extract content between < and >
        size_t start = decl.find('<');
        size_t end = decl.rfind('>');

        if (start == std::string::npos || end == std::string::npos) {
            return;
        }

        std::string param_list = decl.substr(start + 1, end - start - 1);

        // Split by comma (handling nested templates)
        std::vector<std::string> param_strs = splitTemplateParams(param_list);

        for (const auto& param_str : param_strs) {
            TemplateParameter param = parseTemplateParameter(param_str);
            params.push_back(param);
        }
    }

    /**
     * Parse a single template parameter
     */
    TemplateParameter parseTemplateParameter(const std::string& param_str) {
        TemplateParameter param;

        std::string trimmed = trim(param_str);

        // Check for typename/class (type parameter)
        if (trimmed.find("typename") == 0 || trimmed.find("class") == 0) {
            param.kind = TemplateParameter::TypeParam;

            // Extract name and default value
            size_t name_start = trimmed.find(' ');
            if (name_start != std::string::npos) {
                std::string rest = trimmed.substr(name_start + 1);

                // Check for default value
                size_t eq_pos = rest.find('=');
                if (eq_pos != std::string::npos) {
                    param.name = trim(rest.substr(0, eq_pos));
                    param.default_value = trim(rest.substr(eq_pos + 1));
                } else {
                    param.name = trim(rest);
                }
            }
        }
        // Check for template template parameter
        else if (trimmed.find("template") == 0) {
            param.kind = TemplateParameter::TemplateParam;
            // Parse nested template
            // e.g., "template<typename> class Container"
            size_t class_pos = trimmed.rfind("class");
            if (class_pos != std::string::npos) {
                param.name = trim(trimmed.substr(class_pos + 5));
            }
        }
        // Non-type parameter (e.g., int N, size_t Size)
        else {
            param.kind = TemplateParameter::NonType;

            // Find the last identifier as the name
            std::vector<std::string> tokens = split(trimmed, ' ');
            if (!tokens.empty()) {
                // Check for default value
                std::string last_token = tokens.back();
                size_t eq_pos = last_token.find('=');
                if (eq_pos != std::string::npos) {
                    param.name = trim(last_token.substr(0, eq_pos));
                    param.default_value = trim(last_token.substr(eq_pos + 1));
                } else {
                    param.name = last_token;
                }

                // Type is everything except the last token
                std::string type_str;
                for (size_t i = 0; i < tokens.size() - 1; ++i) {
                    if (i > 0) type_str += " ";
                    type_str += tokens[i];
                }

                // Create type object
                param.param_type = std::make_shared<Type>(TypeKind::Integer); // Simplified
                param.param_type->name = type_str;
            }
        }

        return param;
    }

    /**
     * Split template parameters by comma, handling nested templates
     */
    std::vector<std::string> splitTemplateParams(const std::string& params) {
        std::vector<std::string> result;
        std::string current;
        int depth = 0;

        for (char c : params) {
            if (c == '<') {
                depth++;
                current += c;
            } else if (c == '>') {
                depth--;
                current += c;
            } else if (c == ',' && depth == 0) {
                result.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }

        if (!current.empty()) {
            result.push_back(current);
        }

        return result;
    }

    /**
     * Utility: Trim whitespace
     */
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) {
            return "";
        }
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    /**
     * Utility: Split string by delimiter
     */
    std::vector<std::string> split(const std::string& str, char delim) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;

        while (std::getline(ss, item, delim)) {
            result.push_back(item);
        }

        return result;
    }
};

/**
 * Template Conversion Strategy
 * Determines how to convert C++ templates to target language generics
 */
class TemplateConversionStrategy {
public:
    /**
     * Convert template parameters to Rust generic bounds
     */
    static std::string toRustGenericBounds(const std::vector<TemplateParameter>& params) {
        if (params.empty()) {
            return "";
        }

        std::stringstream ss;
        ss << "<";

        bool first = true;
        for (const auto& param : params) {
            if (!first) ss << ", ";
            first = false;

            if (param.kind == TemplateParameter::TypeParam) {
                ss << param.name;

                // Add trait bounds if constraints exist
                if (!param.constraints.empty()) {
                    ss << ": ";
                    for (size_t i = 0; i < param.constraints.size(); ++i) {
                        if (i > 0) ss << " + ";
                        ss << param.constraints[i];
                    }
                }
            } else if (param.kind == TemplateParameter::NonType) {
                // Rust doesn't have non-type generic parameters in the same way
                // Convert to const generic
                ss << "const " << param.name << ": ";
                if (param.param_type) {
                    ss << convertTypeToRust(param.param_type->name);
                } else {
                    ss << "usize";
                }
            }
        }

        ss << ">";
        return ss.str();
    }

    /**
     * Convert template parameters to Go type parameters
     */
    static std::string toGoTypeParameters(const std::vector<TemplateParameter>& params) {
        if (params.empty()) {
            return "";
        }

        std::stringstream ss;
        ss << "[";

        bool first = true;
        for (const auto& param : params) {
            if (!first) ss << ", ";
            first = false;

            if (param.kind == TemplateParameter::TypeParam) {
                ss << param.name;

                // Add constraints if available
                if (!param.constraints.empty()) {
                    ss << " ";
                    for (size_t i = 0; i < param.constraints.size(); ++i) {
                        if (i > 0) ss << " | ";
                        ss << param.constraints[i];
                    }
                } else {
                    ss << " any";  // Default constraint in Go
                }
            }
            // Go doesn't support non-type parameters
        }

        ss << "]";
        return ss.str();
    }

private:
    static std::string convertTypeToRust(const std::string& cpp_type) {
        if (cpp_type == "int" || cpp_type == "int32_t") return "i32";
        if (cpp_type == "size_t") return "usize";
        if (cpp_type == "unsigned int" || cpp_type == "uint32_t") return "u32";
        return "usize"; // default
    }
};

/**
 * Common template patterns detector
 */
class TemplatePatternDetector {
public:
    /**
     * Detect if this is a container template
     */
    static bool isContainerTemplate(const ClassDecl& class_decl) {
        if (!class_decl.is_template) return false;

        // Check for common container method signatures
        for (const auto& method : class_decl.methods) {
            if (method.name == "push_back" || method.name == "insert" ||
                method.name == "size" || method.name == "begin" || method.name == "end") {
                return true;
            }
        }

        return false;
    }

    /**
     * Detect if this is an algorithm template function
     */
    static bool isAlgorithmTemplate(const Function& func) {
        if (!func.is_template) return false;

        // Check for iterator-style parameters
        for (const auto& param : func.parameters) {
            if (param.type->name.find("Iterator") != std::string::npos ||
                param.type->name.find("iterator") != std::string::npos) {
                return true;
            }
        }

        return false;
    }

    /**
     * Detect SFINAE patterns (Substitution Failure Is Not An Error)
     */
    static bool hasSFINAEPattern(const Function& func) {
        // Check for enable_if, is_same, etc. in return type or parameters
        if (func.return_type->name.find("enable_if") != std::string::npos) {
            return true;
        }

        for (const auto& param : func.parameters) {
            if (param.type->name.find("enable_if") != std::string::npos) {
                return true;
            }
        }

        return false;
    }
};

} // namespace hybrid
