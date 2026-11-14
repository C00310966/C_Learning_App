#include <stdio.h>
#include <stdbool.h>
int Reversed(int n) {
    // TODO: reverse decimal digits using % and /
    (void)n;return 0;
}
bool isAPalindrome(int n) {return Reversed(n)==n;}
int main(void) {
    printf("%d %d %d\n",Reversed(123),Reversed(9070),Reversed(5));
    printf("%d %d %d\n",isAPalindrome(121),isAPalindrome(1213),isAPalindrome(7));
}
