#include <iostream>
#include <list>
int main()
{
    std::list<int> first = {1, 2}, second = {8, 9};
    // TODO: splice all elements from second onto the end of first
    for (int x : first)
        std::cout << x << " ";
    std::cout << "\n";
    std::cout << "second-size=" << second.size() << "\n";
}
