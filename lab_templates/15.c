#include <stdio.h>
#include <stdlib.h>
int *allocate_five(void)
{
    // TODO: allocate 5 ints with malloc, fill 1..5, return pointer.
    // C equivalent of matching new[] with delete[].
    return NULL;
}
int main(void)
{
    int *p = allocate_five();
    if (!p)
    {
        puts("allocation failed");
        return 1;
    }
    int sum = 0;
    for (int i = 0; i < 5; i++)
        sum += p[i];
    printf("sum=%d\n", sum);
    free(p);
}
