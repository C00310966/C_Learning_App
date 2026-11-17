#ifndef CODEQUEST_EXAM_SIMULATOR_H
#define CODEQUEST_EXAM_SIMULATOR_H

#include "raylib.h"

typedef struct {
    float x, width, scroll;
    Vector2 mouse;
    Font font, headingFont, codeFont;
    Color panel, card, text, muted, line, accent, green, orange, background;
    Color accentText;
} ExamStyle;

void ExamStart(void);
void ExamUpdate(void);
void ExamDraw(const ExamStyle *style, float *bottom);
void ExamDrawTimeoutPopup(const ExamStyle *style, int screenWidth, int screenHeight);
int ExamTimeoutPopupVisible(void);
int ExamIsRunning(void);
int ExamIsFinished(void);
void ExamLeave(void);

#endif
