#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct Node{int data;struct Node*next;}Node;
bool insertAtPosition(Node **head,int index,int value){
    // TODO: insert at zero-based index; reject invalid positions
    (void)head;(void)index;(void)value;return false;
}
int main(void){Node *head=NULL;insertAtPosition(&head,0,10);insertAtPosition(&head,1,20);
    printf("%d\n",insertAtPosition(&head,1,15));
    for(Node*p=head;p;p=p->next)printf("%d ",p->data);puts("NULL");
    while(head){Node*n=head->next;free(head);head=n;}}
