#include <iostream>
#include <vector>
#include <algorithm>
int main()
{
    std::vector<int> values = {4, 1, 3};
    // TODO: use std::sort then iterate and print each value
    for (int x : values)
        std::cout << x << " ";
    std::cout << "\n";
}
