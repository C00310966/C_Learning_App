#ifndef CONTENT_H
#define CONTENT_H

typedef struct {
    const char *title;
    const char *subtitle;
    const char *description;
    const char *code;
    const char *takeaway;
} Lesson;

typedef enum { QUESTION_CHOICE, QUESTION_OUTPUT } QuestionType;
typedef struct {
    int lessonIndex;
    QuestionType type;
    const char *prompt;
    const char *code;
    const char *choices[4];
    const char *answer;  // Choice: A/B/C/D; Output: exact printed text
    const char *explanation;
} Question;

extern const Lesson LESSONS[];
extern const int LESSON_COUNT;
extern const Question QUESTIONS[];
extern const int QUESTION_COUNT;

#endif
