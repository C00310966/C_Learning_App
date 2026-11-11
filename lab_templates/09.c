#include <stdio.h>
#include <string.h>
void FindAllVariablesInScope(char out[], char code[], int line)
{
    // TODO: track brace nesting and int/char declarations up to line.
    // This is a simplified smoke test for your parser.
    (void)code;
    (void)line;
    strcpy(out, "");
}
int main(void)
{
    char code[] = "int a;\n{\nint b;\n}\n";
    char out[256] = {0};
    FindAllVariablesInScope(out, code, 3);
    printf("%s\n", out);
}
