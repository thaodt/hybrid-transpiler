/**
 * @file ffi_example_rust.rs
 * @brief Expected Rust FFI bindings for ffi_example.cpp
 *
 * This file shows what the hybrid-transpiler should generate when
 * creating Rust FFI bindings for the C++ code.
 */

use std::ffi::c_void;

// Link to the C++ library
#[link(name = "ffi_example")]
extern "C" {
    // Simple function
    fn add(a: i32, b: i32) -> i32;

    // Function with pointer
    fn increment_array(array: *mut i32, length: usize);

    // Point struct functions
    fn create_point(x: f32, y: f32) -> Point;
    fn point_distance(p: *const Point) -> f32;

    // Calculator class methods (opaque pointer pattern)
    fn calculator_new(initial_value: i32) -> *mut c_void;
    fn calculator_delete(calc: *mut c_void);
    fn calculator_get_value(calc: *const c_void) -> i32;
    fn calculator_set_value(calc: *mut c_void, value: i32);
    fn calculator_add(calc: *mut c_void, value: i32);
    fn calculator_multiply(calc: *mut c_void, value: i32);
}

// FFI-compatible struct
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct Point {
    pub x: f32,
    pub y: f32,
}

// Safe Rust wrappers
impl Point {
    /// Create a new Point
    pub fn new(x: f32, y: f32) -> Self {
        unsafe { create_point(x, y) }
    }

    /// Calculate distance from origin
    pub fn distance(&self) -> f32 {
        unsafe { point_distance(self as *const Point) }
    }
}

/// Safe wrapper for Calculator class
pub struct Calculator {
    ptr: *mut c_void,
}

impl Calculator {
    /// Create a new Calculator
    pub fn new(initial_value: i32) -> Self {
        let ptr = unsafe { calculator_new(initial_value) };
        Calculator { ptr }
    }

    /// Get the current value
    pub fn get_value(&self) -> i32 {
        unsafe { calculator_get_value(self.ptr) }
    }

    /// Set the value
    pub fn set_value(&mut self, value: i32) {
        unsafe { calculator_set_value(self.ptr, value) }
    }

    /// Add to the current value
    pub fn add(&mut self, value: i32) {
        unsafe { calculator_add(self.ptr, value) }
    }

    /// Multiply the current value
    pub fn multiply(&mut self, value: i32) {
        unsafe { calculator_multiply(self.ptr, value) }
    }
}

// Implement Drop to automatically clean up the C++ object
impl Drop for Calculator {
    fn drop(&mut self) {
        unsafe {
            calculator_delete(self.ptr);
        }
    }
}

// Safe wrapper for add function
pub fn add_numbers(a: i32, b: i32) -> i32 {
    unsafe { add(a, b) }
}

// Safe wrapper for increment_array
pub fn increment_slice(slice: &mut [i32]) {
    unsafe {
        increment_array(slice.as_mut_ptr(), slice.len());
    }
}

// Example usage
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add() {
        assert_eq!(add_numbers(5, 3), 8);
    }

    #[test]
    fn test_point() {
        let p = Point::new(3.0, 4.0);
        assert_eq!(p.distance(), 25.0);
    }

    #[test]
    fn test_calculator() {
        let mut calc = Calculator::new(10);
        assert_eq!(calc.get_value(), 10);

        calc.add(5);
        assert_eq!(calc.get_value(), 15);

        calc.multiply(2);
        assert_eq!(calc.get_value(), 30);

        calc.set_value(100);
        assert_eq!(calc.get_value(), 100);
    }

    #[test]
    fn test_increment_array() {
        let mut numbers = vec![1, 2, 3, 4, 5];
        increment_slice(&mut numbers);
        assert_eq!(numbers, vec![2, 3, 4, 5, 6]);
    }
}

// Example main function
fn main() {
    println!("=== Rust FFI Example ===\n");

    // Simple function
    let result = add_numbers(10, 20);
    println!("add(10, 20) = {}", result);

    // Point struct
    let p = Point::new(3.0, 4.0);
    println!("\nPoint {{ x: {}, y: {} }}", p.x, p.y);
    println!("Distance: {}", p.distance());

    // Calculator class
    let mut calc = Calculator::new(5);
    println!("\nCalculator initial value: {}", calc.get_value());

    calc.add(10);
    println!("After add(10): {}", calc.get_value());

    calc.multiply(3);
    println!("After multiply(3): {}", calc.get_value());

    // Array increment
    let mut numbers = vec![1, 2, 3, 4, 5];
    println!("\nOriginal array: {:?}", numbers);
    increment_slice(&mut numbers);
    println!("After increment: {:?}", numbers);
}
