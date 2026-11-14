#include <stdio.h>
#include <stdlib.h>
typedef struct Node {int data;struct Node *next;} Node;
void InsertAtEnd(Node **head,int value){
    // TODO: malloc a node, connect it at the end, handle empty list
    (void)head;(void)value;
}
int main(void){Node *head=NULL;InsertAtEnd(&head,10);InsertAtEnd(&head,20);
    for(Node *p=head;p;p=p->next)printf("%d ",p->data);puts("NULL");
    while(head){Node*n=head->next;free(head);head=n;}}
