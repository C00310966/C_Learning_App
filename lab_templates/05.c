#include <stdio.h>
int find2ndLargest(int size,int arr[]){
    // TODO: second DISTINCT greatest value; return -1 if none
    (void)size;(void)arr;return -1;
}
int main(void){
    int a[]={7,2,7,5},b[]={3,3},c[]={9,4,8,6},d[]={1};
    printf("%d\n%d\n%d\n%d\n",find2ndLargest(4,a),find2ndLargest(2,b),find2ndLargest(4,c),find2ndLargest(1,d));
}
