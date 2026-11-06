#include "raylib.h"
#include "visualizer.h"
#include <stdio.h>

static int x = 10;
static int *p = &x;

static const Color V_BG = {10, 15, 29, 255};
static const Color V_PANEL = {20, 29, 47, 255};
static const Color V_TEXT = {218, 229, 244, 255};
static const Color V_MUTED = {145, 167, 193, 255};
static const Color V_GOLD = {255, 197, 62, 255};
static const Color V_CYAN = {80, 211, 222, 255};
static const Color V_GREEN = {83, 219, 159, 255};
static const Color V_LINE = {52, 71, 99, 255};

static char feedback[160] =
    "The pointer p stores the address of x.";

void ResetVisualizer(void)
{
    x = 10;
    p = &x;

    snprintf(feedback, sizeof(feedback),
             "The pointer p stores the address of x.");
}

static void Panel(Rectangle r, Color border)
{
    DrawRectangleRounded(r, 0.12f, 12, V_PANEL);
    DrawRectangleRoundedLinesEx(r, 0.12f, 12, 2, border);
}

static bool VisualButton(Rectangle r, const char *label, Color accent)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, r);

    DrawRectangleRounded(
        r, 0.2f, 8,
        hover ? (Color){42, 58, 81, 255} : V_PANEL
    );

    DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2,
                                hover ? accent : V_LINE);

    int fontSize = 19;
    int textWidth = MeasureText(label, fontSize);

    DrawText(label,
             (int)(r.x + (r.width - textWidth) / 2),
             (int)(r.y + (r.height - fontSize) / 2),
             fontSize,
             hover ? accent : V_TEXT);

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool DrawPointerVisualizer(void)
{
    DrawText("INTERACTIVE LEARNING / C POINTERS",
             74, 45, 17, V_GOLD);

    DrawText("POINTER MEMORY VISUALISER",
             74, 78, 36, V_TEXT);

    DrawText("Change the values and watch how pointers behave.",
             76, 132, 19, V_MUTED);

    DrawLine(72, 174, 1208, 174, V_LINE);

    // Code example
    Panel((Rectangle){72, 200, 1136, 120}, V_LINE);

    DrawText("LIVE C CODE", 96, 216, 16, V_CYAN);

    DrawText(TextFormat("int x = %d;", x),
             100, 250, 22, V_TEXT);

    DrawText("int *p = &x;",
             410, 250, 22, V_TEXT);

    DrawText(TextFormat("*p = %d;", *p),
             800, 250, 22, V_GOLD);

    // Variable x
    Rectangle variableBox = {115, 370, 330, 180};
    Panel(variableBox, V_CYAN);

    DrawText("VARIABLE: x", 143, 394, 23, V_CYAN);
    DrawText(TextFormat("%d", x), 235, 443, 48, V_TEXT);
    DrawText("Address: 0x100", 143, 515, 19, V_MUTED);

    // Pointer p
    Rectangle pointerBox = {830, 370, 330, 180};
    Panel(pointerBox, V_GOLD);

    DrawText("POINTER: p", 858, 394, 23, V_GOLD);
    DrawText("0x100", 902, 452, 34, V_TEXT);
    DrawText(TextFormat("*p = %d", *p),
             858, 515, 19, V_MUTED);

    // Arrow from pointer to variable
    DrawLineEx((Vector2){825, 462},
               (Vector2){455, 462},
               5, V_GOLD);

    DrawTriangle(
        (Vector2){448, 462},
        (Vector2){466, 450},
        (Vector2){466, 474},
        V_GOLD
    );

    DrawText("p points to x", 535, 427, 21, V_GOLD);

    // Buttons
    if (VisualButton((Rectangle){72, 585, 205, 55},
                     "x = x + 1", V_CYAN))
    {
        x++;
        snprintf(feedback, sizeof(feedback),
                 "x increased. Dereferencing p now gives %d.", *p);
    }

    if (VisualButton((Rectangle){292, 585, 205, 55},
                     "x = x - 1", V_CYAN))
    {
        x--;
        snprintf(feedback, sizeof(feedback),
                 "x decreased. Dereferencing p now gives %d.", *p);
    }

    if (VisualButton((Rectangle){512, 585, 205, 55},
                     "*p = 25", V_GOLD))
    {
        *p = 25;
        snprintf(feedback, sizeof(feedback),
                 "Writing through *p changed x to %d.", x);
    }

    if (VisualButton((Rectangle){732, 585, 205, 55},
                     "*p = *p + 5", V_GREEN))
    {
        *p += 5;
        snprintf(feedback, sizeof(feedback),
                 "Adding 5 through *p also changed x to %d.", x);
    }

    if (VisualButton((Rectangle){952, 585, 256, 55},
                     "RESET", V_MUTED))
    {
        ResetVisualizer();
    }

    // Explanation
    Panel((Rectangle){72, 661, 1136, 65}, V_LINE);

    DrawText(feedback, 95, 683, 19, V_TEXT);

    // Navigation
    if (VisualButton((Rectangle){72, 741, 200, 43},
                     "BACK TO MENU", V_GOLD))
    {
        return true;
    }

    DrawText("Illustrative address: 0x100",
             885, 753, 16, V_MUTED);

    return false;
}
