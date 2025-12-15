/**
 * @file ffi_example.cpp
 * @brief Example C++ code for FFI generation
 *
 * This example demonstrates C++ code that can be exposed to Rust and Go
 * through FFI (Foreign Function Interface).
 */

#include <cstdint>
#include <cstddef>

// Simple function - FFI compatible
extern "C" int32_t add(int32_t a, int32_t b) {
    return a + b;
}

// Function with pointer - FFI compatible
extern "C" void increment_array(int32_t* array, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        array[i]++;
    }
}

// Simple struct - FFI compatible
struct Point {
    float x;
    float y;
};

// Function returning struct - FFI compatible
extern "C" Point create_point(float x, float y) {
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

// Function taking struct pointer - FFI compatible
extern "C" float point_distance(const Point* p) {
    return p->x * p->x + p->y * p->y;
}

// Class with opaque handle pattern
class Calculator {
private:
    int32_t value_;

public:
    Calculator() : value_(0) {}
    explicit Calculator(int32_t v) : value_(v) {}

    int32_t getValue() const { return value_; }
    void setValue(int32_t v) { value_ = v; }
    void add(int32_t v) { value_ += v; }
    void multiply(int32_t v) { value_ *= v; }
};

// C API for Calculator class
extern "C" {
    // Constructor
    void* calculator_new(int32_t initial_value) {
        return new Calculator(initial_value);
    }

    // Destructor
    void calculator_delete(void* calc) {
        delete static_cast<Calculator*>(calc);
    }

    // Methods
    int32_t calculator_get_value(const void* calc) {
        return static_cast<const Calculator*>(calc)->getValue();
    }

    void calculator_set_value(void* calc, int32_t value) {
        static_cast<Calculator*>(calc)->setValue(value);
    }

    void calculator_add(void* calc, int32_t value) {
        static_cast<Calculator*>(calc)->add(value);
    }

    void calculator_multiply(void* calc, int32_t value) {
        static_cast<Calculator*>(calc)->multiply(value);
    }
}
