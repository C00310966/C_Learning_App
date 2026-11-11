#include <iostream>
// C++ exercise: insertion and deletion with reference parameters.
bool insertElement(int &size, int &count, int arr[], int value, int index)
{
    // TODO: check capacity/index, shift right, insert, increment count
    (void)size;
    (void)count;
    (void)arr;
    (void)value;
    (void)index;
    return false;
}
bool deleteElement(int &count, int arr[], int index)
{
    // TODO: check index, shift left, decrement count
    (void)count;
    (void)arr;
    (void)index;
    return false;
}
int main()
{
    int a[6] = {2, 4, 6}, capacity = 6, count = 3;
    bool i = insertElement(capacity, count, a, 9, 1);
    std::cout << i << ":" << count;
    for (int k = 0; k < count; k++)
        std::cout << " " << a[k];
    std::cout << "\n";
    bool d = deleteElement(count, a, 2);
    std::cout << d << ":" << count;
    for (int k = 0; k < count; k++)
        std::cout << " " << a[k];
    std::cout << "\n";
}
