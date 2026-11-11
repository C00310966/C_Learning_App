#include <stdio.h>
int ReplaceWithMinusOne(int grid[4][6])
{
    // TODO: find horizontal/vertical runs of >=3 and replace with -1
    // Return number of replaced cells (this template checks a simple row).
    (void)grid;
    return 0;
}
int main(void)
{
    int a[4][6] = {{1, 1, 1, 2, 3, 4}, {3, 4, 5, 6, 7, 8}, {1, 2, 3, 4, 5, 6}, {2, 3, 4, 5, 6, 7}};
    printf("%d\n", ReplaceWithMinusOne(a));
    for (int j = 0; j < 6; j++)
        printf("%d%c", a[0][j], j == 5 ? '\n' : ' ');
}
