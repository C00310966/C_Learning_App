#include <stdio.h>
void concat_strings(char *dst,char *src){
    // TODO: append src at the terminating '\0' of dst
    (void)dst;(void)src;
}
int compare_strings(char *a,char *b){
    // TODO: return negative, zero, or positive lexicographically
    (void)a;(void)b;return 0;
}
int main(void){char dst[64]="Hello";concat_strings(dst," World");
    int a=compare_strings("apple","banana"),b=compare_strings("cat","cat"),c=compare_strings("dog","cat");
    printf("%s\n%d %d %d\n",dst,a<0,b==0,c>0);}
