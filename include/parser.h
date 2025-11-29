#ifndef HYBRID_PARSER_H
#define HYBRID_PARSER_H

#include "ir.h"
#include <string>

namespace hybrid {

/**
 * C++ Parser Interface
 *
 * Provides methods to parse C++ source code and generate IR
 */
class Parser {
public:
    /**
     * Parse C++ source file
     *
     * @param filename Path to C++ source file
     * @return Intermediate representation of the parsed code
     */
    static IR parseFile(const std::string& filename);

    /**
     * Parse C++ source string
     *
     * @param source C++ source code as string
     * @return Intermediate representation of the parsed code
     */
    static IR parseString(const std::string& source);
};

} // namespace hybrid

#endif // HYBRID_PARSER_H
