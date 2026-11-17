#ifndef CODEQUEST_PROGRESS_H
#define CODEQUEST_PROGRESS_H

#define MAX_LESSONS 64
#define MAX_LABS 64
#define MAX_QUIZZES 128
#define CQ_TRACKS 5
#define CQ_HINT_COST 100
#define CQ_XP_LESSON 30
#define CQ_XP_QUIZ_ANSWER 20
#define CQ_XP_QUIZ_PASS 50
#define CQ_XP_PERFECT_QUIZ 25
#define CQ_XP_LAB 75
#define CQ_XP_TRACK 150
#define CQ_DEBUG_COUNT 12
#define CQ_XP_DEBUG_FIX 20
#define CQ_XP_DEBUG_PASS 50
#define CQ_XP_DEBUG_PERFECT 25
#define CQ_XP_DAILY_BASE 25
#define CQ_XP_DAILY_BONUS_STEP 5
#define CQ_XP_DAILY_MAX_BONUS 25

/* CQ4 adds debugging rewards; CQ2 and CQ3 saves are migrated. */
typedef struct {
    unsigned char learned[MAX_LESSONS];
    unsigned char labDone[MAX_LABS];
    unsigned char quizDone[MAX_QUIZZES];
    int xp, quizCorrect, quizAttempts, streak, lastDay;
    int xpWallet;
    unsigned char extraHint[MAX_LABS];
    unsigned char quizPassReward[CQ_TRACKS];
    unsigned char quizPerfectReward[CQ_TRACKS];
    unsigned char trackReward[CQ_TRACKS];
    unsigned char debugMastered[CQ_DEBUG_COUNT];
    unsigned char debugPassReward;
    unsigned char debugPerfectReward;
    int dailyLastDay;   /* Local calendar-day serial; separate from general activity streak. */
    int dailyStreak;
    int dailyAttemptDay; /* Last attempted local calendar day, even on failure. */
    int dailyAttemptChoice; /* 0-3, saved for displaying the locked answer. */
} Save;

extern Save progress;
void persist(void);
void loadProgress(void);
void award(int xp);
int spendHint(int labId);
void rewardQuizSession(int track, int score, int count);
void rewardTrack(int track);
int rewardDebugFix(int challengeId);
void rewardDebugSession(int score, int count);
int cqDebugMastered(void);
int cqLocalDay(void);
int cqDailyReward(int day); /* Returns XP earned, or zero if already completed today. */
int cqDailySubmit(int day, int answer, int correct); /* One saved attempt per local day. */
int cqDailyActiveStreak(int day);
int cqLevel(void);
int cqLevelStart(int level);
int cqLevelNext(void);
const char *cqRank(int level);

#endif
