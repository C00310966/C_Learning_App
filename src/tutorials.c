#include "curriculum.h"
/* Additional tutorial chapters written for CodeQuest. */
/* Conditions & loops: one self-contained example per language construct.
 * Keep these as separate sections so learners can revisit individual topics. */
const TutorialSection conditionsLoopTopics[6] = {
 {"if statements",
  "Use if when a block should run only if a condition is true. The expression inside parentheses is evaluated first; a zero value is false and a nonzero value is true. Braces make the controlled block explicit. If the condition is false, execution skips the block.",
  "#include <stdio.h>\n\nint main(void) {\n    int temperature = 32;\n    if (temperature > 30) {\n        printf(\"It is hot\\n\");\n    }\n    return 0;\n}",
  "Output: It is hot. Because 32 > 30 is true, the printf runs. If temperature were 20, this program would print nothing."},
 {"else if statements",
  "Use else if to test a different condition only when the preceding if (and any earlier else if) was false. Conditions are checked top to bottom; only the first matching branch runs. This is useful for grading ranges and classifying values.",
  "#include <stdio.h>\n\nint main(void) {\n    int score = 75;\n    if (score >= 90) {\n        printf(\"Excellent\\n\");\n    } else if (score >= 60) {\n        printf(\"Pass\\n\");\n    }\n    return 0;\n}",
  "Output: Pass. The first comparison fails, but 75 >= 60 is true. The remaining branches (if any) are skipped once a branch matches."},
 {"else statements",
  "Use else as the fallback when none of the preceding conditions in the same if chain was true. It has no condition of its own. You can use if/else for two outcomes, or put else after one or more else if branches.",
  "#include <stdio.h>\n\nint main(void) {\n    int number = 7;\n    if (number % 2 == 0) {\n        printf(\"Even\\n\");\n    } else {\n        printf(\"Odd\\n\");\n    }\n    return 0;\n}",
  "Output: Odd. The remainder of 7 divided by 2 is 1, so the condition is false and the else block runs."},
 {"for loops",
  "A for loop is useful when you know how to initialise a counter, when to stop, and how to update it. The three parts are initialisation (once), condition (before each iteration), and update (after each iteration). The condition can be false immediately, producing zero iterations.",
  "#include <stdio.h>\n\nint main(void) {\n    for (int i = 1; i <= 3; i++) {\n        printf(\"%d\\n\", i);\n    }\n    return 0;\n}",
  "Output: 1, 2, 3 on separate lines. i begins at 1; the body runs while i <= 3, and i++ increments after each pass."},
 {"while loops",
  "A while loop checks its condition BEFORE every iteration. Use it when repetition depends on a changing condition rather than a predetermined number of passes. Update the controlling value inside the loop; otherwise the condition could remain true indefinitely.",
  "#include <stdio.h>\n\nint main(void) {\n    int count = 3;\n    while (count > 0) {\n        printf(\"%d\\n\", count);\n        count--;\n    }\n    return 0;\n}",
  "Output: 3, 2, 1 on separate lines. Once count becomes 0, the next condition check fails and the loop ends. A while loop may execute zero times."},
 {"do while loops",
  "A do while loop checks its condition AFTER running the body, so the body always executes at least once. It suits actions that must happen before deciding whether to repeat, such as asking for input. Unlike while, a semicolon is required after the closing while condition.",
  "#include <stdio.h>\n\nint main(void) {\n    int attempts = 0;\n    do {\n        attempts++;\n        printf(\"Attempt %d\\n\", attempts);\n    } while (attempts < 2);\n    return 0;\n}",
  "Output: Attempt 1 and Attempt 2 on separate lines. The body would still run once if the condition were false on its first check."}
};

const TutorialChapter tutorialChapters[24] = {
    { {
        {"Understand the program structure", "A source file is compiled before it can run. #include <stdio.h> exposes the declaration of printf, main is the entry point, and a statement normally ends in a semicolon. Braces group the statements that belong to a function.", "#include <stdio.h>\nint main(void) {\n    printf(\"Welcome!\\n\");\n    return 0;\n}", "Output: Welcome! The \\n escape moves the cursor to a new line. Compile with gcc -std=c11 -Wall -Wextra main.c -o program."},
        {"Compile and diagnose errors", "The preprocessor handles includes first, the compiler checks syntax and types, and the linker resolves external functions. An error during any stage prevents a working executable. Warnings can identify suspicious code even when compilation succeeds.", "gcc -std=c11 -Wall -Wextra -Wpedantic hello.c -o hello\n./hello", "Try removing the semicolon after printf. Read the first diagnostic, restore it, and recompile. On Windows, run hello.exe instead."},
    } },
    { {
        {"Types and printing", "An int represents integers, a double stores approximate floating-point values, and char represents a character code. printf needs a format for every supplied value. Use %d for int, %f for double and %c for char.", "int count = 12;\ndouble average = 17.5;\nchar grade = 'B';\nprintf(\"%d %.1f %c\\n\", count, average, grade);", "Output: 12 17.5 B. The .1 part limits the floating-point display to one digit after the decimal point."},
        {"Integer division and conversions", "When both operands are integer types, division discards the fractional part. Convert an operand to double when you need a fractional result. Initialize variables before reading them and use const for values that must not change.", "int sum = 7, count = 2;\ndouble wrong = sum / count;\ndouble correct = (double)sum / count;\nprintf(\"%.1f %.1f\\n\", wrong, correct);", "Output: 3.0 3.5. The cast affects the arithmetic before the result is assigned."},
    } },
    { {
        {"Choose a branch", "An if statement evaluates a condition. An optional else covers the opposite case. && combines conditions, || allows either one, and ! negates a condition. Use == to compare values; = assigns a new value.", "int year = 2000;\nif (year % 400 == 0 ||\n   (year % 4 == 0 && year % 100 != 0)) {\n    printf(\"Leap year\\n\");\n}", "Output: Leap year. Years divisible by 100 are not leap years unless they are also divisible by 400."},
        {"Trace nested loops", "A loop's body can itself contain another loop. The outer loop chooses the row and the inner loop prints the items in that row. Draw a table of row and column values to debug off-by-one mistakes.", "for (int row = 1; row <= 3; ++row) {\n    for (int col = 0; col < row; ++col)\n        putchar('A');\n    putchar('\\n');\n}", "Output is A, then AA, then AAA on separate lines. This pattern is adapted from the first lab sheet."},
    } },
    { {
        {"Parameters are copies", "C passes argument values into function parameters. Altering an ordinary int parameter changes only that local copy. Return a result when one computed value needs to reach the caller.", "int add_one(int value) {\n    value++;\n    return value;\n}\nint x = 4;\nint y = add_one(x);", "After the call, x remains 4 and y is 5. Passing a pointer instead can let a function modify x directly."},
        {"Test the boundary cases", "Before coding, list expected results for representative and edge-case inputs. A prime check must reject values below 2; a palindrome helper should decide how to handle zero and negative inputs.", "int is_prime(int n) {\n    if (n < 2) return 0;\n    for (int d = 2; d <= n / d; ++d)\n        if (n % d == 0) return 0;\n    return 1;\n}", "is_prime(2) is true, is_prime(9) is false and is_prime(1) is false. Using d <= n/d avoids multiplying d*d near the integer limit."},
    } },
    { {
        {"Arrays have bounds", "An array's element count is fixed at declaration. Subscripts begin at 0, so an array with four elements permits indices 0 through 3. Functions taking arrays should receive an explicit length.", "int find(const int a[], int n, int key) {\n    for (int i = 0; i < n; ++i)\n        if (a[i] == key) return i;\n    return -1;\n}", "Given {4, 9, 2}, searching for 9 returns index 1; searching for 7 returns -1. Never read a[n]."},
        {"Strings need a terminator", "A string is a char array ending in the byte '\\0'. The terminator occupies space but is not part of the visible text. Allocate enough room for all characters and this additional byte.", "char name[6] = \"Alice\";\nprintf(\"%s has %zu letters\\n\",\n       name, sizeof name - 1);", "Output: Alice has 5 letters. The sizeof shortcut works for this fixed-size array, but not for an arbitrary char* parameter."},
    } },
    { {
        {"Rows and columns", "In C, int board[4][6] reserves 24 integers in row-major order. A nested loop normally visits each row before moving to the next one. Each index needs its own bounds check.", "int board[2][3] = {{1, 2, 3}, {4, 5, 6}};\nfor (int r = 0; r < 2; ++r) {\n    for (int c = 0; c < 3; ++c)\n        printf(\"%d \", board[r][c]);\n    putchar('\\n');\n}", "Output: 1 2 3 on the first row and 4 5 6 on the second."},
        {"Finding a run of three", "For the lab grid challenge, check horizontal groups first, then vertical groups if no horizontal match is found. A starting column can only have two neighbors to the right when c + 2 is within the width.", "if (c + 2 < 6 && board[r][c] > 0 &&\n    board[r][c] == board[r][c+1] &&\n    board[r][c] == board[r][c+2]) {\n    /* horizontal run found */\n}", "In the full 4-by-6 lab, apply the specified exclusions and scan order. Check boundaries before reading neighboring cells."},
    } },
    { {
        {"Addresses and dereferencing", "A pointer holds an address. The address-of operator & gets an object's address; the unary * operator accesses the pointed-to object. The pointer must refer to a live, correctly typed object before dereferencing.", "int score = 10;\nint *p = &score;\nprintf(\"%d\\n\", *p);\n*p = 15;\nprintf(\"%d\\n\", score);", "Output: 10 followed by 15. Updating *p changes the same object named score."},
        {"Draw the memory relationship", "Think of two separate locations: one containing score and one containing p. The value stored in p is the address of score, not its current integer value. Print addresses with %p and cast to void*.", "int x = 42;\nint *p = &x;\nprintf(\"%p\\n\", (void*)p);\nprintf(\"%d\\n\", *p);", "The address varies by run; the dereferenced value is 42. Do not assume a particular numeric address."},
    } },
    { {
        {"Why swapping needs a temporary", "To change the caller's variables, pass the address of each variable. The function receives pointer values that identify the caller's objects. Preserve one old value before overwriting it.", "void swap_ints(int *a, int *b) {\n    int saved = *a;\n    *a = *b;\n    *b = saved;\n}\nint x = 3, y = 8;\nswap_ints(&x, &y);", "After the call x is 8 and y is 3. Writing a = b would only modify a local pointer parameter."},
        {"Pointers as output parameters", "A function may return one value normally but update several output variables through pointers. Agree on whether null pointers are allowed; if they are, check for NULL before using *.", "void split_seconds(int total, int *mins, int *secs) {\n    if (!mins || !secs) return;\n    *mins = total / 60;\n    *secs = total % 60;\n}", "For total 125, the outputs are 2 minutes and 5 seconds. This is a useful pattern for Lab 1 helper functions."},
    } },
    { {
        {"Array traversal without indexing", "If arr points to an array element, arr + i points i elements further along within that same array. Dereferencing *(arr + i) reads the element there. Pointer arithmetic advances in units of the pointed-to type.", "int sum_array(const int *arr, int size) {\n    int sum = 0;\n    for (const int *p = arr; p < arr + size; ++p)\n        sum += *p;\n    return sum;\n}", "For {1, 2, 3, 4}, the result is 10. The supplied pointer lab asks you to use pointer arithmetic instead of arr[i]."},
        {"Reverse in place using two pointers", "Keep one pointer at the front of an array and another at the last element. Swap their values and move both inward until they meet. This uses constant extra storage.", "int values[] = {1,2,3,4};\nint *left = values;\nint *right = values + 3;\nwhile (left < right) {\n    int tmp = *left;\n    *left++ = *right;\n    *right-- = tmp;\n}", "The resulting array is {4,3,2,1}. Handle arrays of length zero before computing a pointer to their last element."},
    } },
    { {
        {"How pointer-based strlen works", "Start at the first character. Advance until a zero terminator appears. The number of successful advances is the length in bytes, excluding the terminator. The caller must supply a valid null-terminated string.", "int strlen_ptr(const char *s) {\n    const char *start = s;\n    while (*s != '\\0') ++s;\n    return (int)(s - start);\n}", "strlen_ptr(\"Hello\") returns 5. The result counts bytes, not Unicode characters."},
        {"Copying includes the zero byte", "When copying a C string, keep copying until the terminating '\\0' byte has also been written. The destination must have enough capacity; this lab assumes it does, but production code should accept a capacity parameter.", "void copy_string(char *dest, const char *src) {\n    while ((*dest++ = *src++) != '\\0') {\n        /* continue */\n    }\n}", "After copying \"cat\", the destination holds c, a, t and a zero terminator. The caller must avoid insufficient capacity and overlapping buffers."},
    } },
    { {
        {"Object lifetime versus scope", "An automatic local variable ceases to exist when its block ends. A pointer holding its old address does not keep it alive. Dereferencing that pointer afterward has undefined behavior.", "int *bad_address(void) {\n    int value = 5;\n    return &value; /* WRONG */\n}", "The returned address is dangling. A safe alternative is returning 5 by value, or receiving an output pointer from the caller."},
        {"Recognise three different bugs", "A leak occurs when allocated memory is no longer reachable. A dangling pointer refers to an object whose lifetime ended. Use-after-free occurs when code accesses released storage. The fixes differ.", "int *p = malloc(sizeof *p);\nif (p) {\n    *p = 5;\n    free(p);\n    p = NULL;\n}", "Include <stdlib.h> for malloc and free. Setting p to NULL only clears this variable, not other aliases pointing to the same allocation."},
    } },
    { {
        {"Dynamic allocation in C", "malloc returns storage or NULL on failure. Allocate using the element count multiplied by sizeof *pointer, test the result, then release it with free when it is no longer needed.", "int *a = malloc(4 * sizeof *a);\nif (!a) return 1;\nfor (int i = 0; i < 4; ++i) a[i] = i+1;\nfree(a);\na = NULL;", "Use #include <stdlib.h>. Do not use the array after free. For user-controlled sizes, guard against multiplication overflow."},
        {"C++ ownership differs", "new constructs an object and delete destroys it; new[] and delete[] are paired for arrays. Modern C++ usually prefers automatic resource management through std::vector or std::unique_ptr instead of manual cleanup.", "// C++ example (compile with g++)\n#include <vector>\nstd::vector<int> values(4, 0);\nvalues[0] = 10;", "The vector releases its storage automatically when it is destroyed. CodeQuest itself remains a C application; this lesson teaches C++ syntax."},
    } },
    { {
        {"Build a simple chain", "A singly linked node holds a data value and a link to the following node. The last link is NULL. Unlike array elements, nodes can occupy unrelated memory locations.", "typedef struct Node {\n    int value;\n    struct Node *next;\n} Node;\nNode a = {10, NULL};\nNode b = {20, NULL};\na.next = &b;", "Starting at a and following next yields 10 then 20. These stack-allocated example nodes must not be freed with free."},
        {"Traverse without losing the head", "Use a separate cursor and follow the next link at each iteration. Keep the original head unchanged so later operations can still find the first node.", "for (Node *cur = &a; cur != NULL; cur = cur->next) {\n    printf(\"%d \", cur->value);\n}", "Output: 10 20. This loop runs in O(n) time for n nodes."},
    } },
    { {
        {"Prepending changes the head", "A new node should point at the original head before the head variable is replaced. In C, a function that modifies the caller's head can accept Node **head.", "void push_front(Node **head, Node *node) {\n    node->next = *head;\n    *head = node;\n}", "When the list is empty, *head is NULL and the new node becomes the entire list. Otherwise, it becomes the first node."},
        {"Trace the insertion order", "If you prepend 10, then 20, then 30, the head-to-tail order becomes 30 -> 20 -> 10. Each new node points to what was previously first.", "Node *head = NULL;\nNode a={10,NULL},b={20,NULL},c={30,NULL};\npush_front(&head,&a);\npush_front(&head,&b);\npush_front(&head,&c);", "Expected traversal: 30 20 10. Because the nodes are automatic objects here, do not free them."},
    } },
    { {
        {"Append with an empty-list case", "To append, initialize the new node's next link to NULL. If head is NULL, the new node is the head. Otherwise, walk to the last node and attach the new one.", "void append(Node **head, Node *n) {\n    n->next = NULL;\n    if (!*head) { *head = n; return; }\n    Node *p = *head;\n    while (p->next) p = p->next;\n    p->next = n;\n}", "Appending to an empty list works because no traversal occurs; it takes O(n) for a nonempty list without a tail pointer."},
        {"A tail pointer speeds up appending", "If a list also stores a tail pointer, the last node can be accessed directly. Update both head and tail for an empty list; otherwise connect the old tail to the new node and move tail forward.", "if (tail == NULL) {\n    head = tail = newNode;\n} else {\n    tail->next = newNode;\n    tail = newNode;\n}", "With valid head/tail invariants, appending becomes O(1). Keep tail accurate after removing the final node."},
    } },
    { {
        {"Delete safely", "To remove a node, first save the link to the next node, reconnect the list, then free the removed node only if it was dynamically allocated. A pointer to a removed node must not be dereferenced afterward.", "Node *old = head;\nif (old != NULL) {\n    head = old->next;\n    free(old); /* only if malloc allocated it */\n}", "For a heap-owned list, repeatedly removing the head can free every node without losing access to the remaining nodes."},
        {"Handle middle and missing nodes", "When removing by value, maintain a previous pointer as well as a current pointer. If the match is at the head, update head; if not, set previous->next to current->next. Do nothing if the value is absent.", "Node *prev = NULL, *cur = head;\nwhile (cur && cur->value != target) {\n    prev = cur; cur = cur->next;\n}\nif (cur) {\n    if (prev) prev->next = cur->next;\n    else head = cur->next;\n    free(cur);\n}", "This assumes all removed nodes were allocated with malloc. Test head, middle, tail, empty list and missing-key cases."},
    } },
    { {
        {"Every link has a partner", "A doubly linked node has next and prev links. If A->next is B, B->prev should also be A. Maintain this invariant after each insertion or removal.", "typedef struct DNode {\n    int value;\n    struct DNode *prev, *next;\n} DNode;", "The head node's prev is NULL and the tail node's next is NULL. Traversal works in both directions."},
        {"Insert between two nodes", "When inserting N between A and B, first set N->prev=A and N->next=B, then update A->next=N and B->prev=N. Handle absent neighbors at the head or tail.", "newNode->prev = left;\nnewNode->next = right;\nif (left) left->next = newNode;\nelse head = newNode;\nif (right) right->prev = newNode;\nelse tail = newNode;", "Your uploaded linked-list lab asks for beginning, end and position insertion. Draw both arrow directions for each case."},
    } },
    { {
        {"Encapsulation and constructors", "A C++ class groups data and behaviour. private members hide implementation details, while public members expose the supported operations. A constructor establishes the starting state.", "class Counter {\n    int count;\npublic:\n    Counter() : count(0) {}\n    void increment() { ++count; }\n    int value() const { return count; }\n};", "The value() method is const because it doesn't change the object. Compile this example with a C++ compiler, not gcc -std=c11."},
        {"Copying and ownership", "If a class owns a raw allocated pointer, copying it without care can cause two objects to delete the same storage. Prefer standard containers and smart pointers to reduce manual ownership code.", "#include <vector>\nclass Scores {\n    std::vector<int> marks;\npublic:\n    void add(int x) { marks.push_back(x); }\n};", "The vector automatically manages its memory when Scores is copied, moved or destroyed."},
    } },
    { {
        {"Function templates", "A C++ template defines an algorithm in terms of a type parameter. The compiler creates suitable instantiations for the used types when the operations in the template are valid for them.", "template <typename T>\nT maximum(T a, T b) {\n    return a > b ? a : b;\n}\nint a = maximum(2, 9);\ndouble b = maximum(2.5, 3.5);", "a is 9 and b is 3.5. Both arguments of this template call must allow T to be deduced consistently."},
        {"Class templates", "A class template can keep data and functions generic. When creating an object, specify the type in angle brackets, such as Adder<int> or Adder<double>.", "template <typename T>\nclass Adder {\n    T total{};\npublic:\n    void add(T x) { total += x; }\n    T sum() const { return total; }\n};", "Create Adder<int> to sum integers or Adder<double> for doubles. T must support += and default initialization."},
    } },
    { {
        {"Overload versus override operators", "Function overloading chooses among functions based on parameter types; operator overloading defines what an operator does for a user-defined type. The return type alone cannot distinguish function overloads.", "struct Point {\n    int x, y;\n    Point operator+(const Point& b) const {\n        return {x + b.x, y + b.y};\n    }\n};", "For Point a{1,2}, b{3,4}, the expression a+b produces {4,6}. The member function uses *this as the left operand."},
        {"Prefix and postfix ++", "Prefix ++ usually increments the object and returns the updated object by reference. Postfix ++ uses a dummy int parameter to distinguish its signature and typically returns the old value by value.", "Counter& operator++() {\n    ++count; return *this;\n}\nCounter operator++(int) {\n    Counter old = *this;\n    ++count; return old;\n}", "These member function bodies belong inside a suitable C++ Counter class. The extra int in postfix is only a distinguishing parameter."},
    } },
    { {
        {"Grow a vector", "std::vector stores elements contiguously and changes size dynamically. push_back appends an item. size tells you how many elements are currently held; capacity can be larger.", "#include <vector>\nstd::vector<int> a{3, 5};\na.push_back(8);\nfor (int x : a) std::cout << x << ' ';", "Output: 3 5 8 (with <iostream> included). A vector owns its allocation, so manual delete is unnecessary."},
        {"Iterator boundaries and invalidation", "begin() points at the first element and end() points just after the last. Never dereference end(). Operations that cause a vector reallocation invalidate existing pointers, references and iterators to its elements.", "for (auto it = a.begin(); it != a.end(); ++it) {\n    std::cout << *it << ' ';\n}", "Iterators provide a uniform way to traverse STL containers, but different containers support different iterator operations."},
    } },
    { {
        {"List operations", "std::list is a bidirectional sequence, commonly implemented with a doubly linked list. Use push_front, push_back and iterators to access elements; list has no operator[] and no random-access iterators.", "#include <list>\nstd::list<int> a{3, 1, 2};\na.sort();\nfor (int x : a) std::cout << x << ' ';", "Output: 1 2 3. Use list.sort(), not std::sort(a.begin(), a.end()), because list iterators are not random access."},
        {"Splice moves nodes", "splice transfers elements from one list to another without copying their values. The source loses the moved elements; references and iterators to transferred elements remain valid, and now refer to the destination list.", "std::list<int> a{1, 4};\nstd::list<int> b{2, 3};\na.splice(++a.begin(), b);", "After the splice a is {1,2,3,4} and b is empty. This mirrors the STL-list lecture examples."},
    } },
    { {
        {"Maps associate keys with values", "std::map stores unique keys in sorted order by its comparator. Use insert or emplace to add entries, find to search without insertion, and operator[] to access or insert a default-initialized value.", "#include <map>\n#include <string>\nstd::map<std::string,int> count;\n++count[\"apple\"];\n++count[\"apple\"];", "The entry for apple is 2. count.find(\"pear\") == count.end() if pear is absent, without adding it."},
        {"Choose between set, stack and queue", "set stores unique ordered keys. stack gives last-in, first-out access; queue gives first-in, first-out access. These are different container interfaces, so select one based on the operations needed.", "#include <set>\nstd::set<int> ids{3, 1, 3};\n// ids contains 1 and 3 only", "The duplicate 3 is not stored twice. A set can be useful for distinct values in lab-style duplicate detection."},
    } },
    { {
        {"Time complexity as a guide", "Big-O describes how the work grows as input size increases. Sequential search is O(n). Binary search on a sorted random-access sequence is O(log n), and indexing a vector is O(1).", "#include <algorithm>\n#include <vector>\nstd::vector<int> v{8, 2, 5};\nstd::sort(v.begin(), v.end());\nbool found = std::binary_search(v.begin(),v.end(),5);", "The sorted vector is {2,5,8}, and found is true. Sorting itself costs O(n log n) comparisons in typical standard-library implementations."},
        {"Match algorithms to iterators", "std::sort needs random-access iterators, so it works for vector but not list. A list supplies its own sort member. map supports key lookup without a linear scan.", "std::list<int> items{4, 1, 3};\nitems.sort();\n// Don't call std::sort on list iterators.", "Choose a container by expected access and insertion patterns, and consider iterator invalidation before retaining addresses of elements."},
    } },
};

/* Core topics: short, separately explained concepts before the longer worked examples. */
const TutorialChapter coreTopics[24] = {
    { {
        {"The parts of a C program", "The include directive brings in declarations from a header. main is where execution begins. Curly braces delimit a block, while semicolons terminate statements. Read a program from the top, but remember execution begins at main.", "#include <stdio.h>\nint main(void) {\n    puts(\"Hello, C!\");\n    return 0;\n}", "Prints Hello, C! followed by a newline. puts comes from stdio.h; return 0 indicates success."},
        {"Compile, run, and change", "A source file is plain text, not an executable. Compile it, run the result, then change one line and rebuild. The compiler reports syntax and type problems; warnings can reveal mistakes even when the program builds.", "gcc -std=c11 -Wall -Wextra hello.c -o hello\n./hello", "On Windows, run hello.exe instead of ./hello. If you alter the source, compile again to see the change."},
    } }, /* 0 */
    { {
        {"Declaring and initialising values", "Every variable has a type and a name. An initialiser gives the variable a known starting value. int handles whole numbers, double holds approximate decimal values, and char holds a character code.", "int lives = 3;\ndouble price = 2.50;\nchar grade = 'A';", "lives starts at 3, price at 2.5, and grade contains the character A. Use these values only after they are initialised."},
        {"Printing and calculating", "printf uses format specifiers that must match the argument types: %d for int, %f for double and %c for char. Integer division discards fractions; convert one operand to double to keep them.", "int a = 7, b = 2;\nprintf(\"%d %.1f\\n\", a / b, (double)a / b);", "Prints 3 3.5. The first operation is integer division; the cast makes the second floating-point division."},
    } }, /* 1 */
    { {
        {"", "", "", ""},
        {"", "", "", ""},
    } }, /* 2 */
    { {
        {"Defining and calling a function", "A function has a return type, name, parameter list and body. A call supplies arguments and executes the function. Give functions a specific job instead of repeating the same steps in multiple places.", "int double_it(int value) { return value * 2; }\nint answer = double_it(6);", "answer is 12. The argument 6 is copied into the parameter value."},
        {"Return values versus pointer parameters", "A return statement sends a value back to the caller. Ordinary parameters are copies, so assigning to them does not change the original variable. A pointer parameter can deliberately modify caller-owned state.", "void reset(int *p) { *p = 0; }\nint score = 9;\nreset(&score);", "score becomes 0. &score passes its address and *p writes through the pointer."},
    } }, /* 3 */
    { {
        {"Indexing an array safely", "An array contains elements of one type in contiguous positions. Indexing begins at zero. For n elements, valid indexes run from 0 through n-1, so use i < n, not i <= n.", "int scores[] = {4, 8, 12};\nprintf(\"%d\\n\", scores[1]);", "Prints 8. scores[3] would be outside the array and must not be accessed."},
        {"Strings are character arrays", "A C string finishes with a null character, written \\0. Leave room for it in the array. Functions such as strlen count visible characters but do not count the terminator.", "#include <string.h>\nchar word[] = \"cat\";\nprintf(\"%zu\\n\", strlen(word));", "Prints 3. The array actually has four bytes: c, a, t and the null terminator."},
    } }, /* 4 */
    { {
        {"Rows, columns and indexes", "For int grid[2][3], the first index is the row and the second is the column. Rows are numbered 0 and 1, columns 0 through 2. Always check both dimensions.", "int grid[2][3] = {{1,2,3},{4,5,6}};\nprintf(\"%d\\n\", grid[1][2]);", "Prints 6, the last value in the second row."},
        {"Traversing a grid", "Use one loop for rows and an inner loop for columns. The inner loop runs all its iterations for each outer-loop iteration. Trace the row and column values when debugging.", "for (int r=0; r<2; ++r) {\n    for (int c=0; c<3; ++c)\n        printf(\"(%d,%d) \",r,c);\n    putchar('\\n');\n}", "Prints coordinates (0,0) through (1,2), with one row per line."},
    } }, /* 5 */
    { {
        {"Addresses and dereferencing", "A pointer stores an address, not a copy of the pointed-to value. & obtains an address; * follows a valid pointer to read or change the value there. Initialise pointers before using them.", "int n = 5;\nint *p = &n;\n*p = 8;\nprintf(\"%d\\n\",n);", "Prints 8: changing *p also changes n because both names refer to the same object."},
        {"NULL and pointer safety", "NULL expresses that a C pointer does not currently point to an object. Check before dereferencing. A pointer can also become invalid when the pointed-to object ends its lifetime.", "int *p = NULL;\nif (p != NULL)\n    printf(\"%d\\n\", *p);", "Prints nothing. Dereferencing NULL or a dangling pointer is undefined behaviour."},
    } }, /* 6 */
    { {
        {"Passing an address into a function", "C passes all function arguments by value. Passing a pointer copies an address, so the function can use that address to access the caller's object.", "void add_one(int *value) { ++*value; }\nint n=4;\nadd_one(&n);", "n becomes 5. The pointer parameter enables an intentional change to the original n."},
        {"Swapping two caller-owned values", "To swap two integers, keep one value in a temporary variable and write through two valid pointers. Check how the caller provides each address.", "void swap(int *a,int *b) {\n    int temp=*a; *a=*b; *b=temp;\n}\nint x=2,y=9; swap(&x,&y);", "After the call, x is 9 and y is 2. Swapping local copies would not change them."},
    } }, /* 7 */
    { {
        {"Pointer arithmetic counts elements", "When p points into an array, p+1 means the next element, not necessarily the next byte. The element type determines the distance.", "int a[]={10,20,30};\nint *p=a;\nprintf(\"%d\\n\",*(p+2));", "Prints 30, the same element as a[2]. Keep pointer arithmetic within the array or one past its end."},
        {"Walking an array with pointers", "A pointer can act as an iterator. Compare against a one-past-the-end pointer, but never dereference that end position.", "int a[]={3,6,9};\nfor(int *p=a;p<a+3;++p)\n    printf(\"%d \",*p);", "Prints 3 6 9. The loop stops before p reaches a+3 as a dereference."},
    } }, /* 8 */
    { {
        {"Finding the end of a C string", "A C string ends at the first \\0 byte. A pointer walker increments one char at a time, and should never read beyond the terminator.", "const char *p=\"hello\";\nwhile(*p != '\\0') ++p;", "p now points to the null terminator after the five visible letters."},
        {"Copying without exceeding capacity", "A destination buffer must hold all visible characters plus one null terminator. When writing string-processing helpers, track the destination capacity separately.", "char dst[6];\nconst char *src=\"hello\";\nmemcpy(dst,src,6); /* includes \\0 */", "dst contains \"hello\". Include <string.h>; copying into char dst[5] would not leave room for the terminator."},
    } }, /* 9 */
    { {
        {"Scope and lifetime", "Local automatic variables are typically created when execution enters their block and cease to exist when it exits. A pointer to an expired local variable must never be dereferenced.", "int x=5;\n{ int y=10; x+=y; }\nprintf(\"%d\\n\",x);", "Prints 15. y no longer exists outside the inner braces."},
        {"Allocating and releasing memory", "malloc allocates bytes and returns a pointer or NULL. Check for failure before using it, and release every successful allocation with free exactly once.", "#include <stdlib.h>\nint *p=malloc(sizeof *p);\nif(p) { *p=42; free(p); p=NULL; }", "If allocation succeeds, the integer is written and the allocation is released. p is set to NULL to avoid using a dangling address."},
    } }, /* 10 */
    { {
        {"C allocation versus C++ ownership", "C uses malloc/free. C++ new/delete is different and must not be mixed with malloc/free. For ordinary dynamic sequences in C++, std::vector usually avoids manual ownership.", "/* C */\nint *a=malloc(3*sizeof *a);\nif(a) free(a);", "Include <stdlib.h> in C. Every allocation must be paired with the appropriate deallocation mechanism."},
        {"Array allocation and cleanup", "When allocating several elements, calculate the total size using sizeof *pointer so the calculation stays correct if the element type changes.", "#include <stdlib.h>\nsize_t count=4;\nint *a=malloc(count*sizeof *a);\nif(a) { a[0]=7; free(a); }", "The buffer has room for four int elements if malloc succeeds; all storage is returned by free."},
    } }, /* 11 */
    { {
        {"Representing a node", "A singly linked node contains data and a pointer to the next node. The last node's next pointer is NULL. Nodes are connected by addresses, not necessarily stored together.", "typedef struct Node {\n    int value;\n    struct Node *next;\n} Node;\nNode first={10,NULL};", "first contains 10 and has no successor. Include <stddef.h> for NULL when needed."},
        {"Following the links", "Traversal keeps a cursor separate from the head. At each step, read the current node then move to next. The loop stops at NULL.", "for(Node *cur=head;cur!=NULL;cur=cur->next)\n    printf(\"%d \",cur->value);", "Each node value is visited once. The loop also handles an empty list when head is NULL."},
    } }, /* 12 */
    { {
        {"Prepending changes the head", "To add a node at the front, make its next point to the current head, then update head. Both operations are essential, and an empty list works automatically.", "node->next=head;\nhead=node;", "The newly inserted node becomes the first node; the old list stays connected behind it."},
        {"Changing the caller's head", "When an insertion helper must update the caller's head, accept a pointer-to-pointer in C. The extra * means the function can assign the original pointer.", "void prepend(Node **head,Node *n) {\n    n->next=*head;\n    *head=n;\n}", "Call prepend(&head,newNode). The caller's head now points at newNode."},
    } }, /* 13 */
    { {
        {"Appending to an empty list", "In an empty list, the new node becomes the head. For a nonempty list, traverse until the last node and attach the new node after it.", "node->next=NULL;\nif(head==NULL) head=node;\nelse { /* find tail then set tail->next=node */ }", "The last node always points to NULL. Forgetting this can break traversal."},
        {"Why a tail pointer helps", "Without a tail pointer, appending requires a walk through n nodes (O(n)). Keeping a current tail lets you append in O(1), provided every insertion and removal updates it.", "tail->next=node;\ntail=node;\ntail->next=NULL;", "The new node is the tail. This snippet assumes tail was non-NULL; handle empty lists separately."},
    } }, /* 14 */
    { {
        {"Remove a node safely", "Before freeing a removed node, reconnect the surrounding links. If deleting the head, update head to its successor. Never read fields from a freed node.", "Node *old=head;\nif(old) { head=old->next; free(old); }", "After a successful head removal, the list starts at its former second node. Only free dynamically allocated nodes."},
        {"Separate the head from a traversal cursor", "Searching and deleting are easier when you keep prev and current pointers. The cursor advances until it finds the target or reaches NULL.", "Node *prev=NULL,*cur=head;\nwhile(cur && cur->value!=target) {\n    prev=cur; cur=cur->next;\n}", "cur is the matching node or NULL; prev points to the node immediately before it when a match is not at the head."},
    } }, /* 15 */
    { {
        {"Two directions, two links", "A doubly linked node has prev and next links. Every insertion must update each affected neighbour, so moving forward and backward visits consistent sequences.", "typedef struct DNode {\n    int value;\n    struct DNode *prev,*next;\n} DNode;", "Head->prev should be NULL and tail->next should be NULL. Missing one link update can break reverse traversal."},
        {"Insert between neighbours", "To insert between left and right, connect the new node to both neighbours first, then update the neighbours to point back. Endpoints need separate head/tail handling.", "n->prev=left; n->next=right;\nif(left) left->next=n;\nif(right) right->prev=n;", "The local links are consistent after insertion. If left or right is NULL, update the list head or tail too."},
    } }, /* 16 */
    { {
        {"Class interface and access control", "A C++ class groups data and functions. private data helps protect invariants; public methods define how callers interact with objects. A struct defaults to public, while a class defaults to private.", "class Counter {\n    int value=0;\npublic:\n    void add() { ++value; }\n    int get() const { return value; }\n};", "The user can call add and get, but cannot directly assign to value. Compile this example as C++."},
        {"Constructors establish valid state", "A constructor runs when an object is created. Use its member initialiser list to initialise members, and mark read-only accessors const when appropriate.", "class Score {\n    int points;\npublic:\n    explicit Score(int p):points(p) {}\n    int get() const { return points; }\n};", "Score s(7); creates an object whose get() returns 7. This is C++ syntax."},
    } }, /* 17 */
    { {
        {"Function templates", "A C++ template lets you describe an operation once for many types. The type must support every operation used in the function body.", "template <typename T>\nT twice(T x) { return x+x; }\nint a=twice(3);\ndouble b=twice(1.25);", "a is 6 and b is 2.5. The compiler generates suitable versions when used."},
        {"Class templates", "A class template makes its stored type a parameter. A single class definition can become several distinct types, such as Box<int> and Box<double>.", "template <typename T>\nstruct Box { T value; };\nBox<int> a{10};\nBox<double> b{2.5};", "a.value is an int containing 10; b.value is a double containing 2.5."},
    } }, /* 18 */
    { {
        {"Overload operators for clear meaning", "Operator overloading lets a custom C++ type support familiar expressions. Its behaviour should be intuitive; operators cannot gain new precedence or a different number of operands.", "struct Point {\n    int x,y;\n    Point operator+(Point rhs) const {\n        return {x+rhs.x,y+rhs.y};\n    }\n};", "Point{1,2}+Point{3,4} produces Point{4,6}. This is C++ code."},
        {"Prefix versus postfix increment", "Prefix ++x changes x and usually returns the new value. Postfix x++ produces the old value and then changes x. A postfix overload has a dummy int parameter.", "int x=4;\nint before=x++;\nint after=++x;", "before is 4, after is 6 and x ends at 6. Custom types should generally preserve this familiar meaning."},
    } }, /* 19 */
    { {
        {"A vector is a growing array", "std::vector stores its elements contiguously and manages capacity automatically. size() is the number of elements; push_back adds an element at the end.", "#include <vector>\nstd::vector<int> v{2,4};\nv.push_back(6);", "v contains 2, 4, 6 and v.size() is 3. Compile as C++."},
        {"Iterators mark a range", "begin() points to the first element and end() marks one past the last element. Range-based loops provide a concise way to read elements. Never dereference end().", "for(int value : v)\n    std::cout << value << \" \";", "With v={2,4,6}, prints 2 4 6. Include <iostream> and <vector>."},
    } }, /* 20 */
    { {
        {"How a list differs from a vector", "std::list is typically a doubly linked list. You can insert near a known iterator efficiently, but cannot use an integer index such as list[2].", "#include <list>\nstd::list<int> nums{2,4};\nnums.push_front(1);", "nums holds 1, 2, 4 in iteration order. This example requires C++."},
        {"Splicing moves nodes", "list::splice transfers elements between lists without copying individual values. Iterator/reference validity is retained for transferred elements, now belonging to the destination.", "std::list<int> a{1,2}, b{8};\na.splice(a.end(),b);", "a becomes 1,2,8 and b becomes empty. Include <list>."},
    } }, /* 21 */
    { {
        {"Map keys and values", "std::map associates unique ordered keys with values. operator[] can create a missing key, while find checks without adding anything.", "#include <map>\n#include <string>\nstd::map<std::string,int> scores;\nscores[\"Ada\"]=10;", "The key Ada maps to 10. scores.find(\"Bob\") equals scores.end() until Bob is added."},
        {"Sets and container adapters", "std::set stores each key once. std::stack accesses the newest item first; std::queue accesses the oldest first. Their interfaces reflect different use cases.", "std::set<int> s{3,3,1};\nstd::queue<int> q; q.push(4); q.push(8);", "s holds 1 and 3; q.front() is 4. Include <set> and <queue>."},
    } }, /* 22 */
    { {
        {"What Big-O measures", "Big-O describes how running work grows as input increases. An index lookup is O(1); scanning n items is O(n); a nested scan may be O(n squared).", "for(int i=0;i<n;++i)\n    for(int j=0;j<n;++j)\n        ++count;", "The body executes n*n times, illustrating quadratic growth. This is a loop fragment, not a complete program."},
        {"Match algorithms to containers", "std::sort needs random-access iterators and works with vector, but not list iterators. std::list has its own sort member. Choose algorithms based on supported iterator operations.", "std::vector<int> v{3,1,2};\nstd::sort(v.begin(),v.end());", "v becomes 1,2,3. Include <vector> and <algorithm>; compile as C++."},
    } }, /* 23 */
};
