#include <stdio.h>
int strlen_ptr(char *s){
    // TODO: traverse using a pointer; exclude the terminating null byte
    (void)s;return 0;
}
int count_vowels(char *s){
    // TODO: count a, e, i, o, u ignoring case
    (void)s;return 0;
}
void copy_string(char *dst,char *src){
    // TODO: copy chars including terminating null byte
    (void)dst;(void)src;
}
int main(void){char s[]="OpenAI",dst[64]={0};copy_string(dst,s);
    printf("%d %d %s\n",strlen_ptr(s),count_vowels(s),dst);
    printf("%d %d\n",strlen_ptr(""),count_vowels("Hello"));}
