#ifndef CODEQUEST_LAB_RUNNER_H
#define CODEQUEST_LAB_RUNNER_H
#include <stddef.h>
#define LAB_CODE_CAP 16384
#define LAB_OUTPUT_CAP 8192
typedef struct {char output[LAB_OUTPUT_CAP]; int compiled, ran, passed, timedOut, supported;} LabRunResult;
/* All programs execute locally with the student's OS permissions. */
int labLoadDraft(int id, char *dst, size_t cap);
void labSaveDraft(int id, const char *code);
void labLoadStarter(int id, char *dst, size_t cap);
LabRunResult labExecute(int id, const char *code, int check);
#endif
