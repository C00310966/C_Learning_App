#include <stdio.h>
#include <stdbool.h>
bool isAPrimeNumber(int n) {
    // TODO: n >= 2; try divisors up to n / divisor
    (void)n;return false;
}
int main(void) {
    int numbers[]={1,2,3,4,9,13,25,97};
    for(int i=0;i<8;i++)printf("%d:%d\n",numbers[i],isAPrimeNumber(numbers[i]));
}
