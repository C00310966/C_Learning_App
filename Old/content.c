#include "content.h"
#include <stddef.h>

const Lesson LESSONS[] = {
    {"Pointer basics", "Lesson 01 / 05", "A pointer stores the memory address of another object.\nAn int pointer can point to an int variable.\nThe & operator gets the address of a variable.",
     "int x = 10;\nint *p = &x;\n// p stores the address of x", "Remember: p is an address, but x is a value."},
    {"Dereferencing", "Lesson 02 / 05", "Use * on an existing valid pointer to read the object it points to.\nYou can also use *p to update that object.\nThis is different from using * in a pointer declaration.",
     "int x = 15;\nint *p = &x;\nprintf(\"%d\", *p); // 15\n*p = 25;       // x is now 25", "*p accesses the pointed-to value."},
    {"Safe pointers", "Lesson 03 / 05", "Never dereference an uninitialized pointer.\nWhen you don't yet have an object to point to, initialize the\npointer to NULL and only dereference once it is valid.",
     "int *p = NULL;\nint x = 50;\np = &x;\nprintf(\"%d\", *p); // 50", "Check that a pointer is valid before dereferencing it."},
    {"Pointers and arrays", "Lesson 04 / 05", "Array elements occupy consecutive positions in memory.\nIn most expressions, an array name converts to a pointer\nto its first element. Pointer arithmetic moves by elements.",
     "int a[] = {5, 10, 15};\nprintf(\"%d\", *a);       // 5\nprintf(\"%d\", *(a + 2)); // 15", "*(a + i) is equivalent to a[i] for valid indices."},
    {"Functions and double pointers", "Lesson 05 / 05", "Passing a pointer lets a function modify the original object.\nA pointer-to-pointer stores the address of another pointer.\n**pp follows both pointers to reach the original value.",
     "void change(int *p) { *p = 50; }\nint x = 10;\nint *p = &x;\nint **pp = &p;\nchange(&x); // x is now 50", "A function can modify x via &x; **pp reaches x."}
};
const int LESSON_COUNT = sizeof(LESSONS) / sizeof(LESSONS[0]);

// Based on the user's C Pointers Quiz. Added explanations and two code-output exercises.
const Question QUESTIONS[] = {
    {0,QUESTION_CHOICE,"What does a pointer store?","",{"A character","A memory address","Multiple values","Only integers"},"B","A pointer stores an address of a compatible object (or can be NULL)."},
    {0,QUESTION_CHOICE,"Which correctly declares a pointer to int?","",{"int p;","pointer p;","int *p;","*int p;"},"C","The declaration int *p declares p as a pointer to int."},
    {0,QUESTION_CHOICE,"Which operator obtains the address of x?","int x = 10;",{"*","%","&","#"},"C","&x evaluates to the address of x."},
    {0,QUESTION_CHOICE,"What is stored in p?","int x = 10;\nint *p = &x;",{"The value 10","Twice the value of x","The address of x","An undefined value"},"C","p holds &x: the memory address where x lives."},
    {1,QUESTION_OUTPUT,"What is the exact output?","#include <stdio.h>\nint main(void) {\n    int x = 15;\n    int *p = &x;\n    printf(\"%d\", *p);\n    return 0;\n}",{NULL,NULL,NULL,NULL},"15","Dereferencing p with *p reads the value of x, which is 15."},
    {1,QUESTION_CHOICE,"What does printf(\"%p\", (void*)p) display?","int x = 20;\nint *p = &x;",{"The number 20","The number 0","The address of x","A compiler error"},"C","%p prints a pointer value; the actual address varies by run."},
    {1,QUESTION_CHOICE,"After *p = 12, what happens to x?","int x = 5;\nint *p = &x;\n*p = 12;",{"x becomes 12","p becomes the integer 12","x stays 5","Compilation fails"},"A","Assigning through *p updates the original variable x."},
    {1,QUESTION_OUTPUT,"What is the exact output?","#include <stdio.h>\nint main(void) {\n    int x = 25;\n    int *p = &x;\n    printf(\"%d %d\", x, *p);\n    return 0;\n}",{NULL,NULL,NULL,NULL},"25 25","Both x and *p refer to the value 25. The format has one space."},
    {2,QUESTION_CHOICE,"How should an unused int pointer be initialized?","",{"int *p;","int *p = NULL;","int p = NULL;","int *p = 1;"},"B","NULL indicates that p does not currently point to an object."},
    {2,QUESTION_CHOICE,"What's wrong with this code?","int *p;\n*p = 50;",{"Incorrect declaration syntax","p is uninitialized","Ints cannot be assigned","NULL stores the integer"},"B","p has an indeterminate value, so dereferencing it causes undefined behavior."},
    {2,QUESTION_CHOICE,"Which statement stores x's address in p?","int x = 10;\nint *p = NULL;",{"p = x;","*p = x;","p = &x;","&p = x;"},"C","p = &x assigns the address of x to the pointer p."},
    {3,QUESTION_CHOICE,"Pointer arithmetic is commonly used for...","",{"Multiplying addresses","Accessing array elements","Converting int to string","Automatic sorting"},"B","Moving a pointer by an integer number of elements is useful with arrays."},
    {3,QUESTION_OUTPUT,"What is the exact output?","#include <stdio.h>\nint main(void) {\n    int a[] = {5, 10, 15};\n    printf(\"%d\", *(a + 2));\n    return 0;\n}",{NULL,NULL,NULL,NULL},"15","a + 2 refers to a[2], and the dereference retrieves 15."},
    {3,QUESTION_CHOICE,"In most expressions, array a converts to...","int a[] = {1, 2, 3};",{"The array's size","Its final element","A pointer to its first element","The number of bytes"},"C","Most expressions convert an array name into a pointer to element zero."},
    {3,QUESTION_CHOICE,"What does *p produce?","int a[] = {1, 2, 3};\nint *p = a;",{"1","2","3","The address of a"},"A","p points at a[0], and *p reads its value: 1."},
    {4,QUESTION_CHOICE,"Which parameter lets a function modify an int in its caller?","",{"int x","float x","char x","int *x"},"D","A parameter of type int * can point to the caller's original int."},
    {4,QUESTION_CHOICE,"What is x after calling change(&x)?","void change(int *p) { *p = 50; }\nint x = 10;\nchange(&x);",{"10","50","The address of x","A compile error"},"B","The function writes 50 through a pointer to x."},
    {4,QUESTION_CHOICE,"What does a pointer-to-pointer store?","",{"Two separate addresses","Address of another pointer","Address of an entire array","Only character addresses"},"B","For example, int **pp = &p stores the address of pointer p."},
    {4,QUESTION_CHOICE,"What is the value of **pp?","int x = 7;\nint *p = &x;\nint **pp = &p;",{"Address of x","Address of p","7","Undefined"},"C","*pp is p; dereferencing again gives the value of x, 7."},
    {4,QUESTION_CHOICE,"Which declares a pointer to pointer to int?","",{"int p;","int *p;","int **p;","int ***p;"},"C","int **p has two levels of pointer indirection."}
};
const int QUESTION_COUNT = sizeof(QUESTIONS) / sizeof(QUESTIONS[0]);
