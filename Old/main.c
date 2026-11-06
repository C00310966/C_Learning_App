#include "visualizer.h"
#include "raylib.h"
#include "content.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define W 1280
#define H 800
#define MAX_INPUT 95

// Custom application colours
static const Color BG = {10,15,29,255};
static const Color PANEL = {20,29,47,255};
static const Color DEEP = {12,20,34,255};
static const Color LINE = {52,71,99,255};
static const Color TEXT = {218,229,244,255};
static const Color MUTED = {145,167,193,255};

static const Color APP_GOLD = {255,197,62,255};
static const Color CYAN = {80,211,222,255};
static const Color APP_GREEN = {83,219,159,255};
static const Color APP_RED = {249,125,142,255};

typedef enum {
    PAGE_HOME,
    PAGE_LESSON,
    PAGE_QUIZ,
    PAGE_RESULTS,
    PAGE_VISUALIZER
} Page;

static Page page = PAGE_HOME;

static int currentLesson = 0;
static int currentQuestion = 0;
static int score = 0;
static int correctCount = 0;
static int asked = 0;

static bool submitted = false;
static bool answerCorrect = false;

static char input[MAX_INPUT + 1] = "";
static int chosen = -1;

// Draw text
static void Label(const char *s, int x, int y, int size, Color c)
{
    DrawText(s, x, y, size, c);
}

// Draw rounded boxes
static void Box(Rectangle r, Color bg, Color outline)
{
    DrawRectangleRounded(r, .13f, 12, bg);
    DrawRectangleRoundedLinesEx(r, .13f, 12, 1.5f, outline);
}

// Reusable button
static bool Button(Rectangle r, const char *s, Color accent)
{
    Vector2 m = GetMousePosition();
    bool hover = CheckCollisionPointRec(m, r);

    Box(r,
        hover ? (Color){42,58,81,255} : PANEL,
        hover ? accent : LINE);

    int fs = 20;

    DrawText(
        s,
        (int)(r.x + (r.width - MeasureText(s, fs)) / 2),
        (int)(r.y + (r.height - fs) / 2),
        fs,
        hover ? accent : TEXT
    );

    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Page headings
static void Heading(const char *tag, const char *title, const char *desc)
{
    Label(tag, 74, 45, 17, APP_GOLD);
    Label(title, 74, 78, 39, TEXT);
    Label(desc, 76, 132, 19, MUTED);

    DrawLine(72, 174, W - 72, 174, LINE);
}

// Display multiline text
static void MultiText(const char *s, int x, int y,
                      int size, int spacing, Color col)
{
    char line[500];
    int z = 0;
    int dy = 0;

    for (const char *p = s;; p++)
    {
        if (*p == '\n' || *p == '\0')
        {
            line[z] = '\0';
            Label(line, x, y + dy, size, col);

            z = 0;
            dy += spacing;

            if (*p == '\0')
                break;
        }
        else if (z < (int)sizeof(line) - 1)
        {
            line[z++] = *p;
        }
    }
}

// Display code examples
static void DrawCode(Rectangle r, const char *code)
{
    Box(r, DEEP, LINE);

    Label("C CODE", (int)r.x + 24, (int)r.y + 17, 15, CYAN);

    DrawLine(
        (int)r.x + 22,
        (int)r.y + 44,
        (int)(r.x + r.width - 22),
        (int)r.y + 44,
        LINE
    );

    MultiText(code, (int)r.x + 27, (int)r.y + 63, 20, 31, TEXT);
}

// Start quiz
static void StartQuiz(void)
{
    page = PAGE_QUIZ;
    currentQuestion = 0;
    score = 0;
    correctCount = 0;
    asked = 0;
    submitted = false;
    chosen = -1;
    input[0] = '\0';
}

// Next question
static void NextQuestion(void)
{
    currentQuestion++;
    submitted = false;
    chosen = -1;
    input[0] = '\0';

    if (currentQuestion >= QUESTION_COUNT)
        page = PAGE_RESULTS;
}

// Check answer
static void Submit(void)
{
    if (submitted)
        return;

    const Question *q = &QUESTIONS[currentQuestion];

    if (q->type == QUESTION_CHOICE)
    {
        if (chosen < 0)
            return;

        answerCorrect = ((char)('A' + chosen) == q->answer[0]);
    }
    else
    {
        if (input[0] == '\0')
            return;

        answerCorrect = strcmp(input, q->answer) == 0;
    }

    submitted = true;
    asked++;

    if (answerCorrect)
    {
        correctCount++;
        score += 10;
    }
}

// Keyboard input
static void TextInput(void)
{
    if (submitted)
        return;

    int key;

    while ((key = GetCharPressed()) != 0)
    {
        size_t n = strlen(input);

        if (key >= 32 && key <= 126 && n < MAX_INPUT)
        {
            input[n] = (char)key;
            input[n + 1] = '\0';
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        size_t n = strlen(input);

        if (n > 0)
            input[n - 1] = '\0';
    }
}

// MAIN MENU
static void Home(void)
{
    Heading(
        "INTERACTIVE LEARNING / C + C++",
        "POINTER ACADEMY",
        "Learn the concept. Read the code. Test what you understand."
    );

    Box((Rectangle){72,212,1136,234}, PANEL, LINE);

    Label("START HERE", 104, 239, 17, APP_GOLD);
    Label("Pointers made understandable", 104, 276, 31, TEXT);

    Label("Five short lessons covering addresses, dereferencing,",
          104, 328, 20, MUTED);

    Label("safe pointers, arrays and functions.",
          104, 360, 20, MUTED);

    if (Button((Rectangle){102,393,310,44},
               "OPEN TUTORIAL", APP_GOLD))
    {
        currentLesson = 0;
        page = PAGE_LESSON;
    }

    Box((Rectangle){72,472,547,220}, PANEL, LINE);

    Label("PRACTICE", 102, 504, 17, CYAN);
    Label("20 pointer questions", 102, 546, 27, TEXT);
    Label("Multiple choice + exact output", 102, 588, 19, MUTED);

    if (Button((Rectangle){102,631,250,43}, "START QUIZ", CYAN))
        StartQuiz();

    Box((Rectangle){641,472,567,220}, PANEL, LINE);

    Label("HOW IT WORKS", 675, 503, 17, APP_GOLD);
    Label("1. Read a short lesson", 675, 547, 19, TEXT);
    Label("2. Predict what the code does", 675, 584, 19, TEXT);
    Label("3. Get feedback and your score", 675, 621, 19, TEXT);

    if (Button((Rectangle){72, 710, 350, 52},
           "POINTER VISUALISER", APP_GOLD))
    {
        ResetVisualizer();
        page = PAGE_VISUALIZER;
    }
}

// POINTER TUTORIAL
static void Tutorial(void)
{
    const Lesson *l = &LESSONS[currentLesson];

    Heading(
        l->subtitle,
        l->title,
        "A guided introduction to C pointers"
    );

    Box((Rectangle){72,206,1136,176}, PANEL, LINE);

    Label("EXPLANATION", 98, 230, 17, CYAN);

    MultiText(l->description, 100, 274, 20, 29, TEXT);

    DrawCode((Rectangle){72,402,1136,230}, l->code);

    Label("KEY IDEA:", 95, 655, 17, APP_GOLD);
    Label(l->takeaway, 207, 653, 18, TEXT);

    if (Button((Rectangle){72,716,196,54}, "MAIN MENU", MUTED))
        page = PAGE_HOME;

    if (Button((Rectangle){735,716,190,54}, "PREVIOUS", CYAN)
        && currentLesson > 0)
    {
        currentLesson--;
    }

    if (Button(
        (Rectangle){941,716,267,54},
        currentLesson == LESSON_COUNT - 1 ? "START QUIZ" : "NEXT LESSON",
        APP_GOLD))
    {
        if (currentLesson == LESSON_COUNT - 1)
            StartQuiz();
        else
            currentLesson++;
    }

    Label(
        TextFormat("LESSON %d / %d", currentLesson + 1, LESSON_COUNT),
        974, 53, 19, APP_GOLD
    );
}

// REUSABLE QUIZ INTERFACE
static void Quiz(void)
{
    const Question *q = &QUESTIONS[currentQuestion];

    Heading(
        "QUIZ / POINTERS",
        TextFormat("QUESTION %d / %d", currentQuestion + 1, QUESTION_COUNT),
        TextFormat("Score: %d     Correct: %d / %d",
                   score, correctCount, asked)
    );

    Box((Rectangle){72,199,1136,74}, PANEL, APP_GOLD);
    Label(q->prompt, 94, 224, 24, TEXT);

    bool hasCode = q->code && q->code[0];
    int answerY = hasCode ? 510 : 305;

    if (hasCode)
        DrawCode((Rectangle){72,288,1136,203}, q->code);

    if (q->type == QUESTION_CHOICE)
    {
        for (int i = 0; i < 4; i++)
        {
            Rectangle r = {
                72 + (float)(i % 2) * 576,
                (float)answerY + (float)(i / 2) * 73,
                560,
                62
            };

            Vector2 m = GetMousePosition();
            bool h = CheckCollisionPointRec(m, r);

            Box(
                r,
                chosen == i ? (Color){41,64,81,255}
                            : (h ? PANEL : DEEP),
                chosen == i ? APP_GOLD : LINE
            );

            DrawText(
                TextFormat("%c", 'A' + i),
                (int)r.x + 19,
                (int)r.y + 19,
                20,
                chosen == i ? APP_GOLD : CYAN
            );

            DrawText(
                q->choices[i],
                (int)r.x + 56,
                (int)r.y + 19,
                19,
                TEXT
            );

            if (!submitted && h &&
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                chosen = i;
            }
        }

        if (!submitted)
        {
            if (IsKeyPressed(KEY_A)) chosen = 0;
            if (IsKeyPressed(KEY_B)) chosen = 1;
            if (IsKeyPressed(KEY_C)) chosen = 2;
            if (IsKeyPressed(KEY_D)) chosen = 3;
        }
    }
    else
    {
        Label("YOUR ANSWER (EXACT OUTPUT)",
              77, answerY, 18, MUTED);

        Rectangle field = {72, (float)answerY + 36, 1136, 63};

        Box(field, DEEP, APP_GOLD);

        Label(
            input[0] ? input : "Type output here (spaces matter)...",
            94,
            answerY + 55,
            22,
            input[0] ? TEXT : MUTED
        );

        TextInput();
    }

    // Display answer feedback
    if (submitted)
    {
        Box(
            (Rectangle){72,658,1136,56},
            PANEL,
            answerCorrect ? APP_GREEN : APP_RED
        );

        Label(
            answerCorrect ? "CORRECT!" : "NOT QUITE",
            89,
            669,
            20,
            answerCorrect ? APP_GREEN : APP_RED
        );

        DrawText(
            q->explanation,
            answerCorrect ? 233 : 240,
            676,
            17,
            TEXT
        );
    }

    if (Button((Rectangle){72,735,185,46}, "END QUIZ", APP_RED))
        page = PAGE_RESULTS;

    if (!submitted)
    {
        if (Button((Rectangle){990,735,218,46},
                   "SUBMIT", APP_GOLD)
            || IsKeyPressed(KEY_ENTER))
        {
            Submit();
        }
    }
    else
    {
        if (Button(
            (Rectangle){953,735,255,46},
            currentQuestion == QUESTION_COUNT - 1
                ? "SEE RESULTS" : "NEXT QUESTION",
            APP_GREEN)
            || IsKeyPressed(KEY_ENTER))
        {
            NextQuestion();
        }
    }
}

// RESULTS SCREEN
static void Results(void)
{
    Heading(
        "SESSION COMPLETE",
        "YOUR RESULTS",
        "Review your score, then practise again or revisit lessons."
    );

    Box((Rectangle){200,237,880,350}, PANEL, LINE);

    Label("FINAL SCORE", 510, 285, 21, APP_GOLD);

    Label(TextFormat("%d POINTS", score),
          464, 345, 46, TEXT);

    Label(
        TextFormat("%d correct out of %d answered", correctCount, asked),
        448, 425, 23, CYAN
    );

    Label(
        asked
            ? TextFormat("Accuracy: %d%%", (correctCount * 100) / asked)
            : "No questions answered",
        506, 483, 20, MUTED
    );

    if (Button((Rectangle){255,630,240,62}, "MAIN MENU", MUTED))
        page = PAGE_HOME;

    if (Button((Rectangle){515,630,240,62}, "TUTORIAL", CYAN))
    {
        currentLesson = 0;
        page = PAGE_LESSON;
    }

    if (Button((Rectangle){775,630,240,62}, "TRY AGAIN", APP_GOLD))
        StartQuiz();
}

// MAIN FUNCTION
int main(void)
{
    InitWindow(W, H, "Pointer Academy - C and C++ Learning App");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BG);

        if (page == PAGE_HOME)
            Home();
        else if (page == PAGE_LESSON)
            Tutorial();
        else if (page == PAGE_QUIZ)
            Quiz();
        else
            Results();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

