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

Save progress = {0};

void persist(void) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return;
    if (fwrite("CQ4!", 1, 4, f) != 4 || fwrite(&progress, sizeof(progress), 1, f) != 1) {
        /* A failed save is retried on the next normal save. */
    }
    fclose(f);
}

void loadProgress(void) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return;
    char magic[4] = {0};
    if (fread(magic, 1, 4, f) == 4) {
        if (memcmp(magic, "CQ4!", 4) == 0) {
            Save loaded = {0};
            if (fread(&loaded, sizeof(loaded), 1, f) == 1) progress = loaded;
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
        progress.quizCorrect > progress.quizAttempts) memset(&progress, 0, sizeof(progress));
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
