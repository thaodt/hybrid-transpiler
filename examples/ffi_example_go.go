/**
 * @file ffi_example_go.go
 * @brief Expected Go FFI bindings (cgo) for ffi_example.cpp
 *
 * This file shows what the hybrid-transpiler should generate when
 * creating Go FFI bindings for the C++ code.
 */

package main

/*
#cgo CFLAGS: -I../include
#cgo LDFLAGS: -L../lib -lffi_example -lstdc++

#include <stdlib.h>
#include <stdint.h>

// C declarations (these would be in a generated header)
int32_t add(int32_t a, int32_t b);
void increment_array(int32_t* array, size_t length);

typedef struct {
    float x;
    float y;
} Point;

Point create_point(float x, float y);
float point_distance(const Point* p);

void* calculator_new(int32_t initial_value);
void calculator_delete(void* calc);
int32_t calculator_get_value(const void* calc);
void calculator_set_value(void* calc, int32_t value);
void calculator_add(void* calc, int32_t value);
void calculator_multiply(void* calc, int32_t value);
*/
import "C"
import (
	"fmt"
	"unsafe"
)

// Point represents a 2D point (matching C struct)
type Point struct {
	X float32
	Y float32
}

// NewPoint creates a new Point
func NewPoint(x, y float32) Point {
	cPoint := C.create_point(C.float(x), C.float(y))
	return Point{
		X: float32(cPoint.x),
		Y: float32(cPoint.y),
	}
}

// Distance calculates the distance from the origin
func (p *Point) Distance() float32 {
	cPoint := C.Point{x: C.float(p.X), y: C.float(p.Y)}
	return float32(C.point_distance(&cPoint))
}

// Calculator wraps the C++ Calculator class
type Calculator struct {
	ptr unsafe.Pointer
}

// NewCalculator creates a new Calculator with an initial value
func NewCalculator(initialValue int32) *Calculator {
	ptr := C.calculator_new(C.int32_t(initialValue))
	return &Calculator{ptr: ptr}
}

// Delete frees the Calculator (call this explicitly or use defer)
func (c *Calculator) Delete() {
	if c.ptr != nil {
		C.calculator_delete(c.ptr)
		c.ptr = nil
	}
}

// GetValue returns the current value
func (c *Calculator) GetValue() int32 {
	return int32(C.calculator_get_value(c.ptr))
}

// SetValue sets the current value
func (c *Calculator) SetValue(value int32) {
	C.calculator_set_value(c.ptr, C.int32_t(value))
}

// Add adds a value to the current value
func (c *Calculator) Add(value int32) {
	C.calculator_add(c.ptr, C.int32_t(value))
}

// Multiply multiplies the current value
func (c *Calculator) Multiply(value int32) {
	C.calculator_multiply(c.ptr, C.int32_t(value))
}

// Add wraps the C add function
func Add(a, b int32) int32 {
	return int32(C.add(C.int32_t(a), C.int32_t(b)))
}

// IncrementSlice increments all elements in a slice
func IncrementSlice(slice []int32) {
	if len(slice) == 0 {
		return
	}
	C.increment_array((*C.int32_t)(unsafe.Pointer(&slice[0])), C.size_t(len(slice)))
}

func main() {
	fmt.Println("=== Go FFI Example ===\n")

	// Simple function
	result := Add(10, 20)
	fmt.Printf("Add(10, 20) = %d\n", result)

	// Point struct
	p := NewPoint(3.0, 4.0)
	fmt.Printf("\nPoint { X: %.1f, Y: %.1f }\n", p.X, p.Y)
	fmt.Printf("Distance: %.1f\n", p.Distance())

	// Calculator class
	calc := NewCalculator(5)
	defer calc.Delete() // Ensure cleanup

	fmt.Printf("\nCalculator initial value: %d\n", calc.GetValue())

	calc.Add(10)
	fmt.Printf("After Add(10): %d\n", calc.GetValue())

	calc.Multiply(3)
	fmt.Printf("After Multiply(3): %d\n", calc.GetValue())

	// Array increment
	numbers := []int32{1, 2, 3, 4, 5}
	fmt.Printf("\nOriginal slice: %v\n", numbers)
	IncrementSlice(numbers)
	fmt.Printf("After increment: %v\n", numbers)
}

// Example tests
func ExampleAdd() {
	result := Add(5, 3)
	fmt.Println(result)
	// Output: 8
}

func ExamplePoint() {
	p := NewPoint(3.0, 4.0)
	fmt.Printf("%.1f", p.Distance())
	// Output: 25.0
}

func ExampleCalculator() {
	calc := NewCalculator(10)
	defer calc.Delete()

	fmt.Println(calc.GetValue())
	calc.Add(5)
	fmt.Println(calc.GetValue())
	calc.Multiply(2)
	fmt.Println(calc.GetValue())
	// Output:
	// 10
	// 15
	// 30
}
