#include <stdio.h>
#include <stdbool.h>

bool isLeapYear(int year) {
    // TODO: divisible by 400 OR divisible by 4 but not 100
    (void)year;
    return false;
}
int main(void) {
    int years[] = {1600, 1700, 2000, 2016, 2019, 1900, 2024};
    for (int i = 0; i < 7; i++) printf("%d:%d\n",years[i],isLeapYear(years[i]));
}
