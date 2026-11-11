#include <stdio.h>
void swap_ints(int *a, int *b)
{
    // TODO: swap via dereferenced pointers
    (void)a;
    (void)b;
}
void swap_chars(char *a, char *b)
{
    // TODO: swap via dereferenced pointers
    (void)a;
    (void)b;
}
int main(void)
{
    int a = 5, b = 10;
    char x = 'a', y = 'z';
    swap_ints(&a, &b);
    swap_chars(&x, &y);
    printf("%d %d\n%c %c\n", a, b, x, y);
}
