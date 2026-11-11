#ifndef CODEQUEST_DEBUG_CHALLENGES_H
#define CODEQUEST_DEBUG_CHALLENGES_H

#include "raylib.h"

typedef struct {
    float x;
    float width;
    Font font;
    Vector2 mouse;          /* Mouse in CodeQuest's logical UI coordinates */
    float scroll;           /* Current content scroll in logical pixels */
    int lightTheme;         /* Match CodeQuest light/dark mode */
    float *scrollPosition;  /* Allows next/retry to jump to the top */
} DebugChallengeView;

/* Draw and update a separate, session-based Debug the Code activity. */
void DrawDebugChallenges(const DebugChallengeView *view, float *bottom);

#endif
