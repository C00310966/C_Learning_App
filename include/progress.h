#ifndef CODEQUEST_PROGRESS_H
#define CODEQUEST_PROGRESS_H

#define MAX_LESSONS 64
#define MAX_LABS 64
#define MAX_QUIZZES 128

typedef struct {
    unsigned char learned[MAX_LESSONS];
    unsigned char labDone[MAX_LABS];
    unsigned char quizDone[MAX_QUIZZES];
    int xp, quizCorrect, quizAttempts, streak, lastDay;
} Save;

extern Save progress;
void persist(void);
void loadProgress(void);
void award(int xp);

#endif
