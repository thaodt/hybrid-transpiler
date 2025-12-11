/**
 * Simple C++ Parser
 *
 * A lightweight regex-based parser for basic C++ class structures.
 * This is a simplified parser for demonstration purposes.
 * For production use, integrate with Clang LibTooling.
 */

#include "ir.h"
#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>

namespace hybrid {

class SimpleCppParser {
public:
    /**
     * Parse C++ source file and build IR
     */
    static IR parseFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        return parseString(content);
    }

    /**
     * Parse C++ source string and build IR
     */
    static IR parseString(const std::string& source) {
        IR ir;
        SimpleCppParser parser(source);

        // Parse all classes in the source
        parser.parseClasses(ir);

        return ir;
    }

private:
    std::string source_;

    explicit SimpleCppParser(const std::string& source) : source_(source) {}

    /**
     * Remove C++ comments from source
     */
    std::string removeComments(const std::string& code) const {
        std::string result = code;

        // Remove single-line comments
        result = std::regex_replace(result, std::regex("//[^\n]*"), "");

        // Remove multi-line comments
        result = std::regex_replace(result, std::regex("/\\*.*?\\*/", std::regex::ECMAScript), "");

        return result;
    }

    /**
     * Parse all class declarations
     */
    void parseClasses(IR& ir) {
        std::string cleaned = removeComments(source_);

        // Regex to match class declarations
        // Matches: class ClassName { ... };
        std::regex class_pattern(
            R"(class\s+(\w+)\s*(?::\s*public\s+(\w+(?:\s*,\s*\w+)*))?\s*\{([^}]*(?:\{[^}]*\}[^}]*)*)\};)",
            std::regex::ECMAScript
        );

        auto classes_begin = std::sregex_iterator(cleaned.begin(), cleaned.end(), class_pattern);
        auto classes_end = std::sregex_iterator();

        for (std::sregex_iterator i = classes_begin; i != classes_end; ++i) {
            std::smatch match = *i;

            ClassDecl class_decl;
            class_decl.name = match[1].str();
            class_decl.is_struct = false;

            // Parse base classes if present
            if (match[2].matched) {
                parseBaseClasses(match[2].str(), class_decl);
            }

            // Parse class body
            std::string body = match[3].str();
            parseClassBody(body, class_decl);

            ir.addClass(class_decl);
        }
    }

    /**
     * Parse base class list
     */
    void parseBaseClasses(const std::string& bases_str, ClassDecl& class_decl) {
        std::regex base_pattern(R"(\w+)");
        auto bases_begin = std::sregex_iterator(bases_str.begin(), bases_str.end(), base_pattern);
        auto bases_end = std::sregex_iterator();

        for (std::sregex_iterator i = bases_begin; i != bases_end; ++i) {
            class_decl.base_classes.push_back((*i).str());
        }
    }

    /**
     * Parse class body (fields and methods)
     */
    void parseClassBody(const std::string& body, ClassDecl& class_decl) {
        // Split by access specifiers
        std::vector<std::string> sections;
        std::vector<std::string> access_levels;

        std::regex access_pattern(R"((private|protected|public)\s*:)");

        auto it = std::sregex_iterator(body.begin(), body.end(), access_pattern);
        auto end = std::sregex_iterator();

        size_t last_pos = 0;
        std::string current_access = "private"; // Default for class

        for (; it != end; ++it) {
            std::string section_content = body.substr(last_pos, it->position() - last_pos);
            if (!section_content.empty()) {
                sections.push_back(section_content);
                access_levels.push_back(current_access);
            }

            current_access = (*it)[1].str();
            last_pos = it->position() + it->length();
        }

        // Add remaining section
        if (last_pos < body.length()) {
            sections.push_back(body.substr(last_pos));
            access_levels.push_back(current_access);
        }

        // If no access specifier found, treat entire body as private
        if (sections.empty()) {
            sections.push_back(body);
            access_levels.push_back("private");
        }

        // Parse each section
        for (size_t i = 0; i < sections.size(); ++i) {
            parseSection(sections[i], access_levels[i], class_decl);
        }
    }

    /**
     * Parse a section (fields and methods within an access level)
     */
    void parseSection(const std::string& section, const std::string& access, ClassDecl& class_decl) {
        // Parse field declarations
        parseFields(section, access, class_decl);

        // Parse method declarations/definitions
        parseMethods(section, access, class_decl);
    }

    /**
     * Parse field declarations
     */
    void parseFields(const std::string& section, const std::string& access, ClassDecl& class_decl) {
        // Match: type name; or type name1, name2;
        std::regex field_pattern(
            R"((?:const\s+)?(?:static\s+)?([a-zA-Z_][\w:<>,\[\]\s*&]*?)\s+([a-zA-Z_]\w*(?:\s*,\s*[a-zA-Z_]\w*)*)\s*;)",
            std::regex::ECMAScript
        );

        auto fields_begin = std::sregex_iterator(section.begin(), section.end(), field_pattern);
        auto fields_end = std::sregex_iterator();

        for (std::sregex_iterator it = fields_begin; it != fields_end; ++it) {
            std::smatch match = *it;

            // Skip if this looks like a method (has parentheses before semicolon)
            std::string before_semicolon = match.str();
            if (before_semicolon.find('(') != std::string::npos) {
                continue;
            }

            std::string type_str = match[1].str();
            std::string names_str = match[2].str();

            // Parse multiple variable names (e.g., int x, y;)
            std::regex name_pattern(R"([a-zA-Z_]\w*)");
            auto names_begin = std::sregex_iterator(names_str.begin(), names_str.end(), name_pattern);
            auto names_end = std::sregex_iterator();

            for (std::sregex_iterator name_it = names_begin; name_it != names_end; ++name_it) {
                Variable field;
                field.name = (*name_it).str();
                field.type = parseType(type_str);

                class_decl.fields.push_back(field);
            }
        }
    }

    /**
     * Parse method declarations/definitions
     */
    void parseMethods(const std::string& section, const std::string& access, ClassDecl& class_decl) {
        // Match method signatures (including constructors, virtual, static)
        // Pattern: [virtual] [static] [type] name(params) [const] [= 0] [{ body } | ;]
        std::regex method_pattern(
            R"((virtual\s+)?(static\s+)?(?:([a-zA-Z_][\w:<>,\s*&]*?)\s+)?([a-zA-Z_]\w*)\s*\(([^)]*)\)\s*(const)?\s*(=\s*0)?\s*(?:\{([^}]*(?:\{[^}]*\}[^}]*)*)\}|;))",
            std::regex::ECMAScript
        );

        auto methods_begin = std::sregex_iterator(section.begin(), section.end(), method_pattern);
        auto methods_end = std::sregex_iterator();

        for (std::sregex_iterator it = methods_begin; it != methods_end; ++it) {
            std::smatch match = *it;

            Function method;
            method.name = match[4].str();

            // Check if virtual
            method.is_virtual = match[1].matched;

            // Check if static
            method.is_static = match[2].matched;

            // Check if pure virtual (= 0)
            method.is_pure_virtual = match[7].matched;

            // Check if constructor (no return type and name matches class)
            if (match[3].str().empty() || match[3].str() == class_decl.name) {
                method.is_constructor = true;
                method.return_type = nullptr;
            } else {
                method.return_type = parseType(match[3].str());
            }

            // Parse parameters
            std::string params_str = match[5].str();
            if (!params_str.empty()) {
                parseParameters(params_str, method);
            }

            // Check if const method
            method.is_const = match[6].matched;

            // Store body if present
            if (match[8].matched) {
                method.body = match[8].str();
            }

            class_decl.methods.push_back(method);
        }
    }

    /**
     * Parse function parameters
     */
    void parseParameters(const std::string& params_str, Function& func) {
        // Split by commas (but not inside <>)
        std::vector<std::string> param_strs;
        int angle_depth = 0;
        size_t start = 0;

        for (size_t i = 0; i < params_str.length(); ++i) {
            if (params_str[i] == '<') angle_depth++;
            else if (params_str[i] == '>') angle_depth--;
            else if (params_str[i] == ',' && angle_depth == 0) {
                param_strs.push_back(params_str.substr(start, i - start));
                start = i + 1;
            }
        }
        param_strs.push_back(params_str.substr(start));

        // Parse each parameter
        for (const auto& param_str : param_strs) {
            std::string trimmed = trim(param_str);
            if (trimmed.empty()) continue;

            Parameter param;

            // Simple parameter parsing: type name or just type
            std::regex param_pattern(R"(([a-zA-Z_][\w:<>,\s*&]*?)\s+([a-zA-Z_]\w*)(?:\s*=\s*(.+))?)");
            std::smatch match;

            if (std::regex_match(trimmed, match, param_pattern)) {
                param.type = parseType(match[1].str());
                param.name = match[2].str();

                if (match[3].matched) {
                    param.has_default = true;
                    param.default_value = match[3].str();
                }
            } else {
                // Just type, no name
                param.type = parseType(trimmed);
                param.name = "";
            }

            func.parameters.push_back(param);
        }
    }

    /**
     * Parse type string into Type object
     */
    std::shared_ptr<Type> parseType(const std::string& type_str) {
        std::string trimmed = trim(type_str);

        // Check for const
        bool is_const = false;
        if (trimmed.find("const") == 0) {
            is_const = true;
            trimmed = trim(trimmed.substr(5));
        }

        // Check for pointer
        if (trimmed.back() == '*') {
            trimmed.pop_back();
            auto element = parseType(trimmed);
            auto ptr_type = std::make_shared<Type>(TypeKind::Pointer);
            ptr_type->element_type = element;
            ptr_type->name = trimmed + "*";
            ptr_type->is_const = is_const;
            return ptr_type;
        }

        // Check for reference
        if (trimmed.back() == '&') {
            trimmed.pop_back();
            auto element = parseType(trimmed);
            auto ref_type = std::make_shared<Type>(TypeKind::Reference);
            ref_type->element_type = element;
            ref_type->name = trimmed + "&";
            ref_type->is_const = is_const;
            return ref_type;
        }

        // Check for smart pointers
        if (trimmed.find("std::unique_ptr<") == 0) {
            size_t start = trimmed.find('<') + 1;
            size_t end = trimmed.rfind('>');
            std::string element_type = trimmed.substr(start, end - start);

            auto element = parseType(element_type);
            auto ptr_type = std::make_shared<Type>(TypeKind::Pointer);
            ptr_type->element_type = element;
            ptr_type->name = "std::unique_ptr<" + element_type + ">";
            return ptr_type;
        }

        if (trimmed.find("std::shared_ptr<") == 0) {
            size_t start = trimmed.find('<') + 1;
            size_t end = trimmed.rfind('>');
            std::string element_type = trimmed.substr(start, end - start);

            auto element = parseType(element_type);
            auto ptr_type = std::make_shared<Type>(TypeKind::Pointer);
            ptr_type->element_type = element;
            ptr_type->name = "std::shared_ptr<" + element_type + ">";
            return ptr_type;
        }

        // Check for arrays
        size_t bracket_pos = trimmed.find('[');
        if (bracket_pos != std::string::npos) {
            std::string base_type = trimmed.substr(0, bracket_pos);
            auto element = parseType(base_type);
            auto array_type = std::make_shared<Type>(TypeKind::Array);
            array_type->element_type = element;
            array_type->name = trimmed;
            return array_type;
        }

        // Built-in types
        auto builtin = mapBuiltinType(trimmed);
        if (builtin) {
            builtin->is_const = is_const;
            return builtin;
        }

        // Custom class/struct type
        auto class_type = std::make_shared<Type>(TypeKind::Class);
        class_type->name = trimmed;
        class_type->is_const = is_const;
        return class_type;
    }

    /**
     * Map C++ built-in types
     */
    std::shared_ptr<Type> mapBuiltinType(const std::string& type_name) {
        static const std::map<std::string, TypeKind> builtin_map = {
            {"void", TypeKind::Void},
            {"bool", TypeKind::Bool},
            {"char", TypeKind::Integer},
            {"short", TypeKind::Integer},
            {"int", TypeKind::Integer},
            {"long", TypeKind::Integer},
            {"float", TypeKind::Float},
            {"double", TypeKind::Float},
            {"size_t", TypeKind::Integer}
        };

        auto it = builtin_map.find(type_name);
        if (it != builtin_map.end()) {
            auto type = std::make_shared<Type>(it->second);
            type->name = type_name;
            return type;
        }

        return nullptr;
    }

    /**
     * Trim whitespace from string
     */
    std::string trim(const std::string& str) const {
        size_t start = 0;
        while (start < str.length() && std::isspace(str[start])) {
            start++;
        }

        size_t end = str.length();
        while (end > start && std::isspace(str[end - 1])) {
            end--;
        }

        return str.substr(start, end - start);
    }
};

} // namespace hybrid
