// Example: C++ Inheritance with Virtual Methods

class Shape {
public:
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void draw() const {
        // Default implementation
    }
};

class Circle : public Shape {
private:
    double radius;

public:
    Circle(double r) : radius(r) {}

    virtual double area() const {
        return 3.14159 * radius * radius;
    }

    virtual double perimeter() const {
        return 2 * 3.14159 * radius;
    }

    virtual void draw() const {
        // Draw a circle
    }
};

class Rectangle : public Shape {
private:
    double width;
    double height;

public:
    Rectangle(double w, double h) : width(w), height(h) {}

    virtual double area() const {
        return width * height;
    }

    virtual double perimeter() const {
        return 2 * (width + height);
    }

    virtual void draw() const {
        // Draw a rectangle
    }
};
