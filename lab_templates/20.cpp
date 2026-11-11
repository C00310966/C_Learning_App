#include <iostream>
struct Point
{
    int x, y;
    Point operator+(const Point &rhs) const
    {
        // TODO: return coordinate-wise sum as a new Point
        (void)rhs;
        return *this;
    }
};
int main()
{
    Point a{1, 2}, b{3, 4};
    Point c = a + b;
    std::cout << c.x << "," << c.y << "\n";
}
