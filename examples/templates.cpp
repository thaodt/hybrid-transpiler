/**
 * C++ Template Examples
 * Demonstrates various template patterns and their conversion
 */

#include <iostream>
#include <vector>
#include <memory>

/**
 * Example 1: Simple template class
 */
template<typename T>
class Box {
private:
    T value;

public:
    Box(T v) : value(v) {}

    T get() const {
        return value;
    }

    void set(T v) {
        value = v;
    }
};

/**
 * Example 2: Template function
 */
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

/**
 * Example 3: Multiple template parameters
 */
template<typename K, typename V>
class Pair {
private:
    K key;
    V value;

public:
    Pair(K k, V v) : key(k), value(v) {}

    K getKey() const { return key; }
    V getValue() const { return value; }
};

/**
 * Example 4: Non-type template parameters
 */
template<typename T, size_t N>
class Array {
private:
    T data[N];

public:
    size_t size() const {
        return N;
    }

    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }
};

/**
 * Example 5: Template with default parameters
 */
template<typename T = int>
class Container {
private:
    T value;

public:
    Container() : value(T()) {}
    Container(T v) : value(v) {}

    T get() const { return value; }
};

/**
 * Example 6: Function template with multiple parameters
 */
template<typename Ret, typename Arg>
Ret convert(Arg arg) {
    return static_cast<Ret>(arg);
}

/**
 * Example 7: Template class with template methods
 */
template<typename T>
class SmartContainer {
private:
    std::vector<T> data;

public:
    void add(const T& item) {
        data.push_back(item);
    }

    template<typename U>
    U sumAs() const {
        U sum = U();
        for (const auto& item : data) {
            sum += static_cast<U>(item);
        }
        return sum;
    }

    size_t size() const {
        return data.size();
    }
};

/**
 * Example 8: Variadic templates (C++11)
 */
template<typename... Args>
class Tuple {
    // Simplified - real implementation would use recursive inheritance
};

/**
 * Example 9: Template specialization
 */
template<typename T>
class Serializer {
public:
    static std::string serialize(const T& value) {
        return "generic serialization";
    }
};

// Specialization for int
template<>
class Serializer<int> {
public:
    static std::string serialize(const int& value) {
        return std::to_string(value);
    }
};

/**
 * Example 10: Const generic-like pattern
 */
template<typename T, size_t Rows, size_t Cols>
class Matrix {
private:
    T data[Rows][Cols];

public:
    size_t rows() const { return Rows; }
    size_t cols() const { return Cols; }

    T& at(size_t r, size_t c) {
        return data[r][c];
    }
};

/**
 * Example 11: Template alias (C++11)
 */
template<typename T>
using Vec = std::vector<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

/**
 * Example 12: CRTP (Curiously Recurring Template Pattern)
 */
template<typename Derived>
class Base {
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

class Derived : public Base<Derived> {
public:
    void implementation() {
        // Actual implementation
    }
};

/**
 * Example 13: Template function with constraints (concept-like)
 */
template<typename T>
T add(T a, T b) {
    // In real code, would use C++20 concepts
    // requires std::is_arithmetic_v<T>
    return a + b;
}

/**
 * Example 14: Template with iterator pattern
 */
template<typename Iterator>
void printRange(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
        // Print *it
    }
}

/**
 * Example 15: Template with dependent types
 */
template<typename Container>
class ContainerWrapper {
public:
    using value_type = typename Container::value_type;
    using iterator = typename Container::iterator;

private:
    Container container;

public:
    void add(const value_type& item) {
        container.push_back(item);
    }

    iterator begin() {
        return container.begin();
    }

    iterator end() {
        return container.end();
    }
};

int main() {
    // Example 1: Box
    Box<int> intBox(42);
    Box<std::string> strBox("Hello");

    // Example 2: max function
    int maxInt = max(10, 20);
    double maxDouble = max(3.14, 2.71);

    // Example 3: Pair
    Pair<std::string, int> pair("age", 25);

    // Example 4: Array with size
    Array<int, 5> arr;

    // Example 7: SmartContainer
    SmartContainer<int> container;
    container.add(1);
    container.add(2);
    container.add(3);
    double sum = container.sumAs<double>();

    // Example 10: Matrix
    Matrix<double, 3, 3> matrix;

    return 0;
}
