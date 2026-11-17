#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define SAVE_FILE "codequest_progress.dat"

typedef struct {
    unsigned char learned[MAX_LESSONS];
    unsigned char labDone[MAX_LABS];
    unsigned char quizDone[MAX_QUIZZES];
    int xp, quizCorrect, quizAttempts, streak, lastDay;
} LegacySave;

/* Preserve the exact CQ3 layout when upgrading an existing save. */
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
} CQ3Save;

/* Exact CQ4 payload for backwards-compatible loading after adding daily fields. */
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
} CQ4Save;

/* CQ5 had only the success-day and streak; preserve exact binary layout when upgrading. */
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
    int dailyLastDay;
    int dailyStreak;
} CQ5Save;

Save progress = {0};

void persist(void) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return;
    if (fwrite("CQ6!", 1, 4, f) != 4 || fwrite(&progress, sizeof(progress), 1, f) != 1) {
        /* A failed save is retried on the next normal save. */
    }
    fclose(f);
}

void loadProgress(void) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return;
    char magic[4] = {0};
    if (fread(magic, 1, 4, f) == 4) {
        if (memcmp(magic, "CQ6!", 4) == 0) {
            Save loaded = {0};
            if (fread(&loaded, sizeof(loaded), 1, f) == 1) progress = loaded;
        } else if (memcmp(magic, "CQ5!", 4) == 0) {
            CQ5Save old = {0};
            if (fread(&old, sizeof(old), 1, f) == 1) {
                memcpy(&progress, &old, sizeof(old));
                /* A prior successful completion must remain locked after migration. */
                progress.dailyAttemptDay = progress.dailyLastDay;
                progress.dailyAttemptChoice = -1; /* Unknown in older saves. */
            }
        } else if (memcmp(magic, "CQ4!", 4) == 0) {
            CQ4Save old = {0};
            if (fread(&old, sizeof(old), 1, f) == 1)
                memcpy(&progress, &old, sizeof(old));
        } else if (memcmp(magic, "CQ3!", 4) == 0) {
            CQ3Save old = {0};
            if (fread(&old, sizeof(old), 1, f) == 1) {
                /* Copy original fields, not the new tail, which begins zeroed. */
                memcpy(&progress, &old, sizeof(old));
            }
        } else if (memcmp(magic, "CQ2!", 4) == 0) {
            LegacySave old = {0};
            if (fread(&old, sizeof(old), 1, f) == 1) {
                memcpy(progress.learned, old.learned, sizeof(old.learned));
                memcpy(progress.labDone, old.labDone, sizeof(old.labDone));
                memcpy(progress.quizDone, old.quizDone, sizeof(old.quizDone));
                progress.xp = old.xp;
                progress.quizCorrect = old.quizCorrect;
                progress.quizAttempts = old.quizAttempts;
                progress.streak = old.streak;
                progress.lastDay = old.lastDay;
                /* Existing earned XP stays available for extra hints. */
                progress.xpWallet = old.xp;
            }
        }
    }
    fclose(f);
    if (progress.xp < 0 || progress.xp > 100000000 || progress.xpWallet < 0 || progress.xpWallet > 100000000 ||
        progress.xpWallet > progress.xp || progress.quizCorrect < 0 || progress.quizAttempts < 0 ||
        progress.quizCorrect > progress.quizAttempts ||
        progress.dailyLastDay < 0 || progress.dailyStreak < 0 || progress.dailyStreak > 1000000 ||
        progress.dailyAttemptDay < 0 || progress.dailyAttemptChoice < -1 || progress.dailyAttemptChoice > 3) memset(&progress, 0, sizeof(progress));
}

void award(int xp) {
    if (xp <= 0 || progress.xp > INT_MAX - xp || progress.xpWallet > INT_MAX - xp) return;
    progress.xp += xp;
    progress.xpWallet += xp;
    int day = (int)(time(NULL)/86400);
    if (progress.lastDay != day) {
        progress.streak = progress.lastDay == day-1 ? progress.streak+1 : 1;
        progress.lastDay = day;
    }
    persist();
}

/* Day numbers use the user's local calendar date (not UTC's midnight).
 * Comparing calendar dates directly avoids DST causing 23/25-hour days. */
static int daysBeforeYear(int y) {
    y--;
    return 365*y + y/4 - y/100 + y/400;
}
int cqLocalDay(void) {
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    if (!lt) return 0;
    int y = lt->tm_year + 1900;
    return daysBeforeYear(y) - daysBeforeYear(1970) + lt->tm_yday + 1;
}
int cqDailyActiveStreak(int day) {
    if (day <= 0 || progress.dailyLastDay <= 0) return 0;
    if (progress.dailyLastDay == day || progress.dailyLastDay == day-1)
        return progress.dailyStreak;
    return 0;
}
int cqDailyReward(int day) {
    if (day <= 0 || progress.dailyLastDay >= day) return 0;
    int streak = (progress.dailyLastDay == day-1) ? progress.dailyStreak+1 : 1;
    if (streak > 1000000) streak = 1000000;
    int bonus = (streak-1) * CQ_XP_DAILY_BONUS_STEP;
    if (streak > 6 || bonus > CQ_XP_DAILY_MAX_BONUS)
        bonus = CQ_XP_DAILY_MAX_BONUS;
    int xp = CQ_XP_DAILY_BASE + bonus;
    if (progress.xp > INT_MAX-xp || progress.xpWallet > INT_MAX-xp) return 0;
    progress.dailyLastDay = day;
    progress.dailyStreak = streak;
    award(xp); /* Persists progress once, including both streak fields. */
    return xp;
}

/* Submit today's one and only attempt. Incorrect answers are persisted too,
 * preventing retries after leaving the screen or restarting the application. */
int cqDailySubmit(int day, int answer, int correct) {
    if (day <= 0 || day != cqLocalDay() || answer < 0 || answer > 3 ||
        progress.dailyAttemptDay >= day || progress.dailyLastDay >= day) return 0;
    progress.dailyAttemptDay = day;
    progress.dailyAttemptChoice = answer;
    if (correct) {
        (void)cqDailyReward(day); /* Saves both the attempt and XP in one update. */
    } else {
        persist();
    }
    return 1;
}

int spendHint(int labId) {
    if (labId < 0 || labId >= MAX_LABS) return 0;
    if (progress.extraHint[labId]) return 1;
    if (progress.xpWallet < CQ_HINT_COST) return 0;
    progress.xpWallet -= CQ_HINT_COST;
    progress.extraHint[labId] = 1;
    persist();
    return 1;
}

void rewardQuizSession(int track, int score, int count) {
    if (track < 0 || track >= CQ_TRACKS || count <= 0) return;
    if (score * 100 >= count * 70 && !progress.quizPassReward[track]) {
        progress.quizPassReward[track] = 1;
        award(CQ_XP_QUIZ_PASS);
    }
    if (score == count && !progress.quizPerfectReward[track]) {
        progress.quizPerfectReward[track] = 1;
        award(CQ_XP_PERFECT_QUIZ);
    }
    persist();
}

int cqDebugMastered(void) {
    int count = 0;
    for (int i = 0; i < CQ_DEBUG_COUNT; i++)
        if (progress.debugMastered[i]) count++;
    return count;
}

int rewardDebugFix(int challengeId) {
    if (challengeId < 0 || challengeId >= CQ_DEBUG_COUNT || progress.debugMastered[challengeId]) return 0;
    progress.debugMastered[challengeId] = 1;
    award(CQ_XP_DEBUG_FIX);
    return 1;
}

void rewardDebugSession(int score, int count) {
    if (count <= 0 || count != CQ_DEBUG_COUNT || score < 0 || score > count) return;
    if (score * 100 >= count * 70 && !progress.debugPassReward) {
        progress.debugPassReward = 1;
        award(CQ_XP_DEBUG_PASS);
    }
    if (score == count && !progress.debugPerfectReward) {
        progress.debugPerfectReward = 1;
        award(CQ_XP_DEBUG_PERFECT);
    }
}

void rewardTrack(int track) {
    if (track < 0 || track >= CQ_TRACKS || progress.trackReward[track]) return;
    progress.trackReward[track] = 1;
    award(CQ_XP_TRACK);
}

/* A gradually increasing threshold, rather than the old flat 250 XP. */
int cqLevelStart(int level) {
    if (level <= 1) return 0;
    if (level > 1000) level = 1000;
    return 175 * (level-1) + 25 * (level-1) * level;
}
int cqLevel(void) {
    int level = 1;
    while (level < 1000 && progress.xp >= cqLevelStart(level+1)) level++;
    return level;
}
int cqLevelNext(void) { return cqLevelStart(cqLevel()+1); }
const char *cqRank(int level) {
    static const char *names[] = {
        "C Beginner", "Code Explorer", "Logic Builder", "Pointer Apprentice",
        "Memory Navigator", "Linked List Specialist", "C++ Developer",
        "STL Expert", "Code Master", "CodeQuest Champion"
    };
    if (level < 1) level = 1;
    return names[level >= 10 ? 9 : level-1];
}
