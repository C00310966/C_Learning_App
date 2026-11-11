#include <stdio.h>
#include <stdbool.h>
bool twoMovies(int flightLength, int movieLengths[], int size)
{
    // TODO: find two DIFFERENT indices that sum to flightLength
    (void)flightLength;
    (void)movieLengths;
    (void)size;
    return false;
}
int main(void)
{
    int a[] = {50, 70, 90}, b[] = {60}, c[] = {60, 60};
    printf("%d %d %d %d\n", twoMovies(120, a, 3), twoMovies(110, a, 3), twoMovies(120, b, 1), twoMovies(120, c, 2));
}
