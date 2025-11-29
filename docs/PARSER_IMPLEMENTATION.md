# C++ Parser Implementation

## Overview

The Hybrid Transpiler now includes a **simple regex-based C++ parser** that can parse basic C++ class structures and convert them to an intermediate representation (IR). This parser serves as a foundation for the transpilation process.

## Architecture

```
C++ Source File
     ↓
SimpleCppParser (regex-based)
     ↓
Intermediate Representation (IR)
     ↓
Code Generators (Rust/Go)
     ↓
Target Code
```

## Components

### 1. SimpleCppParser (`src/parser/simple_cpp_parser.cpp`)

A lightweight parser that uses regular expressions to extract:
- Class declarations
- Base classes (inheritance)
- Member fields
- Member methods
- Access specifiers (public/private/protected)
- Constructors
- Method parameters
- Type information

**Key Methods:**
- `parseFile(filename)` - Parse a C++ source file
- `parseString(source)` - Parse C++ source code from string
- `parseClasses(IR&)` - Extract all class declarations
- `parseClassBody(body, ClassDecl&)` - Parse class members
- `parseMethods(section, access, ClassDecl&)` - Parse method declarations
- `parseFields(section, access, ClassDecl&)` - Parse field declarations

### 2. Parser Interface (`include/parser.h`)

Provides a clean API for parsing:

```cpp
namespace hybrid {
    class Parser {
    public:
        static IR parseFile(const std::string& filename);
        static IR parseString(const std::string& source);
    };
}
```

### 3. Integration (`src/transpiler.cpp`)

The parser is integrated into the main transpiler:

```cpp
bool Transpiler::parseSourceFile(const std::string& input_path) {
    *ir_ = Parser::parseFile(input_path);
    return true;
}
```

## Supported C++ Features

### ✅ Fully Supported

| Feature | Example | Notes |
|---------|---------|-------|
| **Basic Classes** | `class Point { ... };` | Complete support |
| **Fields** | `int x; float y;` | Multiple variables per line |
| **Constructors** | `Point(int x, int y)` | Parameter extraction |
| **Methods** | `int getX() const` | Const methods supported |
| **Access Specifiers** | `public:`, `private:`, `protected:` | Full support |
| **Inheritance** | `class Derived : public Base` | Single/multiple inheritance |
| **Smart Pointers** | `std::unique_ptr<T>`, `std::shared_ptr<T>` | Recognized in types |
| **References** | `const Point&` | Const and mutable |
| **Pointers** | `int*`, `char*` | Raw pointers |
| **Method Bodies** | `{ return x * x; }` | Extracted but not analyzed |

### ⚠️ Partially Supported

| Feature | Limitation |
|---------|-----------|
| **Templates** | Recognized but not instantiated |
| **Nested Classes** | Not yet supported |
| **Complex Expressions** | Not parsed in detail |
| **Operator Overloading** | Detected as regular methods |

### ❌ Not Supported

| Feature | Reason |
|---------|--------|
| **Macros** | Requires preprocessor |
| **Template Metaprogramming** | Too complex for regex |
| **Inline Assembly** | Out of scope |
| **Complex Inheritance** | Virtual inheritance not handled |

## Example Usage

### Input C++ Code

```cpp
class Point {
private:
    int x;
    int y;

public:
    Point(int x, int y) : x(x), y(y) {}

    int getX() const { return x; }
    void setX(int newX) { x = newX; }

    int distanceSquared() const {
        return x * x + y * y;
    }
};
```

### Parser Output (IR)

```
ClassDecl {
    name: "Point"
    is_struct: false
    fields: [
        Variable { name: "x", type: Integer("int") },
        Variable { name: "y", type: Integer("int") }
    ]
    methods: [
        Function {
            name: "Point"
            is_constructor: true
            parameters: [
                Parameter { name: "x", type: Integer },
                Parameter { name: "y", type: Integer }
            ]
        },
        Function {
            name: "getX"
            return_type: Integer
            is_const: true
        },
        Function {
            name: "setX"
            return_type: Void
            parameters: [Parameter { name: "newX", type: Integer }]
        },
        Function {
            name: "distanceSquared"
            return_type: Integer
            is_const: true
        }
    ]
}
```

### Generated Rust Code

```rust
pub struct Point {
    pub x: i32,
    pub y: i32,
}

impl Point {
    pub fn new(x: i32, y: i32) -> Self {
        Self { x, y }
    }

    pub fn get_x(&self) -> i32 {
        // TODO: Implement method body
        0
    }

    pub fn set_x(&mut self, new_x: i32) {
        // TODO: Implement method body
    }

    pub fn distance_squared(&self) -> i32 {
        // TODO: Implement method body
        0
    }
}
```

### Generated Go Code

```go
type Point struct {
    x int32
    y int32
}

func NewPoint(x int32, y int32) Point {
    return Point{x: x, y: y}
}

func (p *Point) GetX() int32 {
    // TODO: Implement method body
    return 0
}

func (p *Point) SetX(newX int32) {
    // TODO: Implement method body
}

func (p *Point) DistanceSquared() int32 {
    // TODO: Implement method body
    return 0
}
```

## Type Mapping

### C++ → Rust

| C++ Type | Rust Type |
|----------|-----------|
| `int` | `i32` |
| `long` | `i64` |
| `float` | `f32` |
| `double` | `f64` |
| `bool` | `bool` |
| `std::unique_ptr<T>` | `Box<T>` |
| `std::shared_ptr<T>` | `Rc<T>` or `Arc<T>` |
| `T*` | `*const T` or `*mut T` |
| `T&` | `&T` |
| `T[]` | `Vec<T>` |

### C++ → Go

| C++ Type | Go Type |
|----------|---------|
| `int` | `int32` |
| `long` | `int64` |
| `float` | `float32` |
| `double` | `float64` |
| `bool` | `bool` |
| `std::unique_ptr<T>` | `*T` |
| `std::shared_ptr<T>` | `*T` |
| `T*` | `*T` |
| `T&` | `*T` |
| `T[]` | `[]T` |

## Limitations

### Current Limitations

1. **Regex-based Parsing**
   - Cannot handle all C++ edge cases
   - No preprocessor support
   - Limited template support

2. **No Body Analysis**
   - Method bodies are extracted but not analyzed
   - Control flow not tracked
   - Cannot infer ownership patterns from code

3. **Simplified Type System**
   - Complex template types may not parse correctly
   - Typedef aliases not resolved
   - Namespace qualification limited

### Future Improvements

1. **Clang LibTooling Integration** (v0.2)
   - Full AST traversal
   - Complete type resolution
   - Preprocessor handling
   - Template instantiation

2. **Advanced Analysis** (v0.3)
   - Ownership inference
   - Lifetime analysis
   - Safety verification
   - Performance optimization hints

## Testing

### Running Tests

```bash
# Build the test program
cd build
cmake ..
make

# Run parser test
./test_parser

# Run full transpiler
./hybrid-transpiler -i examples/simple_class.cpp -o output.rs -t rust
./hybrid-transpiler -i examples/simple_class.cpp -o output.go -t go
```

### Test Cases

1. **Simple Classes** - Basic class with fields and methods
2. **Inheritance** - Classes with base classes
3. **Smart Pointers** - `unique_ptr`, `shared_ptr` usage
4. **References** - Const and mutable references
5. **Constructors** - Various constructor patterns

## Performance

### Parsing Speed

| File Size | Classes | Time |
|-----------|---------|------|
| Small (< 100 LOC) | 1-2 | < 10ms |
| Medium (< 1K LOC) | 5-10 | < 50ms |
| Large (< 10K LOC) | 50+ | < 500ms |

*Note: Times are estimates based on regex complexity*

## Contributing

To improve the parser:

1. Add more regex patterns for edge cases
2. Improve type parsing (templates, namespaces)
3. Add better error messages
4. Enhance comment preservation
5. Add support for C++20 features

## References

- [C++ Language Reference](https://en.cppreference.com/)
- [Regular Expressions in C++](https://en.cppreference.com/w/cpp/regex)
- [Clang LibTooling](https://clang.llvm.org/docs/LibTooling.html) (future)

---

**Implementation Date:** 2025-11-29
**Version:** 0.1.1
**Status:** Beta - Functional for basic classes
