#ifndef CODEQUEST_CURRICULUM_H
#define CODEQUEST_CURRICULUM_H

#include "raylib.h"

typedef struct { const char *name, *subtitle; Color color; } Track;
typedef struct { int track; const char *title, *tag, *intro, *explain, *code, *tip; } Lesson;
typedef struct { int track; const char *question, *answers[4], *why; int correct; } Quiz;
typedef struct { int track; const char *title, *level, *task, *signature, *tests, *hint; } Lab;

extern Track tracks[5];
extern Lesson lessons[24];
extern Quiz quizzes[30];
extern Lab labTasks[24];

#endif
