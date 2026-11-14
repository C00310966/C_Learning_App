#include <iostream>
struct DoublyLinkedNode {int data; DoublyLinkedNode *prev,*next;};
void insertAtBeginning(DoublyLinkedNode *&head,int value){
    // TODO: allocate node; connect prev and next in both directions
    (void)head;(void)value;
}
int main(){DoublyLinkedNode *head=nullptr;insertAtBeginning(head,10);insertAtBeginning(head,20);
    DoublyLinkedNode*tail=nullptr;for(auto*p=head;p;p=p->next){std::cout<<p->data<<" ";tail=p;}std::cout<<"NULL\n";
    for(auto*p=tail;p;p=p->prev)std::cout<<p->data<<" ";std::cout<<"NULL\n";
    while(head){auto*n=head->next;delete head;head=n;}}
