#ifndef CODEQUEST_CELEBRATION_H
#define CODEQUEST_CELEBRATION_H
/* Original, procedural success chime and short, lightweight confetti burst.
 * Coordinates are in CodeQuest's logical screen space (not scroll space). */
void celebrationInit(void);
void celebrationCorrect(float x, float y);
void celebrationDraw(float dt);
void celebrationClose(void);
#endif
