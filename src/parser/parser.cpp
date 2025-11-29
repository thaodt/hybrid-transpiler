#include "parser.h"
#include "simple_cpp_parser.cpp"

namespace hybrid {

IR Parser::parseFile(const std::string& filename) {
    return SimpleCppParser::parseFile(filename);
}

IR Parser::parseString(const std::string& source) {
    return SimpleCppParser::parseString(source);
}

} // namespace hybrid
