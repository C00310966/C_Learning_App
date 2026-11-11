/* Debug the Code: a self-contained CodeQuest practice activity.
 * Only content and session state live here; the existing Save format is unchanged.
 */
#include "debug_challenges.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define DEBUG_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

typedef struct {
    const char *topic;
    const char *title;
    const char *buggy;
    const char *prompt;
    const char *choices[4];
    int correct;  /* zero-based index */
    const char *explanation;
    const char *fixed;
} DebugChallenge;

static const DebugChallenge challenges[] = {
    {
        "Pointers", "Point to an integer",
        "int x = 10;\nint *p = x;",
        "Which replacement correctly makes p point to x?",
        {"int *p = &x;", "int p = &x;", "int *p = *x;", "int *p = NULL;"}, 0,
        "A pointer holds an address, not the integer itself. The & operator gives the address of x.",
        "int x = 10;\nint *p = &x;"
    },
    {
        "Dereferencing", "Print the value, not its address",
        "int x = 8;\nint *p = &x;\nprintf(\"%d\", p);",
        "You want to print 8. What should the final line be?",
        {"printf(\"%p\", (void *)p);", "printf(\"%d\", &x);", "printf(\"%d\", *p);", "printf(\"%d\", &p);"}, 2,
        "The * operator dereferences p, reading the integer stored at the address it holds.",
        "int x = 8;\nint *p = &x;\nprintf(\"%d\", *p);"
    },
    {
        "Arrays", "Stay inside the array",
        "int a[] = {3, 6, 9};\nfor (int i = 0; i <= 3; i++) {\n    printf(\"%d \", a[i]);\n}",
        "Which change avoids accessing an element outside the array?",
        {"Change i++ to i--", "Change i <= 3 to i < 3", "Change i = 0 to i = 1", "Change a[i] to a[3]"}, 1,
        "An array of three elements has valid indices 0, 1 and 2. i <= 3 also attempts index 3.",
        "int a[] = {3, 6, 9};\nfor (int i = 0; i < 3; i++) {\n    printf(\"%d \", a[i]);\n}"
    },
    {
        "Conditions", "Assignment versus comparison",
        "int score = 10;\nif (score = 5) {\n    printf(\"Five\");\n}",
        "The intention is to compare score with 5. Which fix is correct?",
        {"if (score != 5)", "if (score += 5)", "if (score = 5)", "if (score == 5)"}, 3,
        "A single = assigns a value. Use == to compare two values without changing score.",
        "int score = 10;\nif (score == 5) {\n    printf(\"Five\");\n}"
    },
    {
        "Pointer safety", "Never dereference NULL",
        "int *p = NULL;\n*p = 7;",
        "What must happen before the assignment through *p?",
        {"Use ++p to move past NULL", "Give p a valid address, e.g. int x = 0; p = &x;", "Replace *p with &p", "Print p once to initialise it"}, 1,
        "NULL does not point to a valid object. Make p point to an existing int before writing through it.",
        "int x = 0;\nint *p = &x;\n*p = 7;"
    },
    {
        "Pointer arithmetic", "Find the last array element",
        "int a[] = {5, 10, 15};\nint *p = a;\nprintf(\"%d\", *(p + 3));",
        "You want to print 15. What is the correct offset?",
        {"*(p + 2)", "*(p + 4)", "*(p - 1)", "*(p + 3)"}, 0,
        "Pointer arithmetic advances by elements. The final element of this three-element array is a[2].",
        "int a[] = {5, 10, 15};\nint *p = a;\nprintf(\"%d\", *(p + 2));"
    },
    {
        "Functions", "Pass an address to a function",
        "void doubleIt(int *n) { *n *= 2; }\nint x = 4;\ndoubleIt(x);",
        "How do you call doubleIt so that it changes x?",
        {"doubleIt(*x);", "doubleIt(4);", "doubleIt(&x);", "doubleIt(NULL);"}, 2,
        "The function accepts int *, so pass the address of x using &x. It can then modify x through *n.",
        "void doubleIt(int *n) { *n *= 2; }\nint x = 4;\ndoubleIt(&x);"
    },
    {
        "Linked lists", "Set the head pointer",
        "struct Node {\n    int value;\n    struct Node *next;\n};\nstruct Node first = {10, NULL};\nstruct Node *head = first;",
        "What should be stored in the head pointer?",
        {"first.value", "first.next", "*first", "&first"}, 3,
        "head is a pointer to struct Node. The address &first is the correct value for this local example.",
        "struct Node first = {10, NULL};\nstruct Node *head = &first;"
    },
    {
        "Loops", "Stop an infinite loop",
        "int i = 0;\nwhile (i < 3) {\n    printf(\"%d \", i);\n}",
        "Which statement should be added inside the loop?",
        {"i++;", "i = 0;", "i--;", "while (i > 3);"}, 0,
        "Without changing i, the loop condition remains true forever. Incrementing i eventually makes it false.",
        "int i = 0;\nwhile (i < 3) {\n    printf(\"%d \", i);\n    i++;\n}"
    },
    {
        "Dynamic memory", "Avoid use-after-free",
        "int *p = malloc(sizeof *p);\nif (p) {\n    *p = 42;\n    free(p);\n    printf(\"%d\", *p);\n}",
        "How should the code be changed to print 42 safely?",
        {"Call free(p) twice", "Move printf before free(p)", "Replace *p with &p", "Move free(p) before the if"}, 1,
        "After free(p), the allocated object no longer exists. Read its value before freeing the memory.",
        "int *p = malloc(sizeof *p);\nif (p) {\n    *p = 42;\n    printf(\"%d\", *p);\n    free(p);\n}"
    },
    {
        "Double pointers", "Follow both levels of indirection",
        "int x = 7;\nint *p = &x;\nint **pp = &p;\nprintf(\"%d\", *pp);",
        "Which expression prints the integer 7?",
        {"pp", "&pp", "**pp", "*pp"}, 2,
        "*pp gives the pointer p; **pp dereferences p once more to read the integer x.",
        "int x = 7;\nint *p = &x;\nint **pp = &p;\nprintf(\"%d\", **pp);"
    },
    {
        "C++ vectors", "Fix the vector index",
        "std::vector<int> v = {4, 8, 12};\nstd::cout << v[3];",
        "Which change prints 12 without accessing past the end?",
        {"Change v[3] to v[2]", "Change v[3] to v[-1]", "Change v[3] to v[4]", "Change v[3] to v[0]"}, 0,
        "A three-element vector has indices 0 through 2. Index 3 is outside its valid range.",
        "std::vector<int> v = {4, 8, 12};\nstd::cout << v[2];"
    }
};

static Color DBG_WHITE={239,244,255,255};
static Color DBG_MUTED={151,165,190,255};
static Color DBG_PURPLE={141,111,255,255};
static Color DBG_GREEN={88,222,165,255};
static Color DBG_RED={255,117,139,255};
static Color DBG_ORANGE={255,182,106,255};
static Color DBG_BG={13,18,31,255};
static Color DBG_PANEL={23,30,47,255};
static Color DBG_CARD={29,38,59,255};
static Color DBG_LINE={51,64,87,255};
static Color DBG_CODE_BG={12,21,34,255};
static Color DBG_BUTTON_TEXT={255,255,255,255};
static void updateDebugTheme(int light){
 if(light){
  DBG_WHITE=(Color){29,39,61,255}; DBG_MUTED=(Color){91,105,128,255};
  DBG_PURPLE=(Color){105,75,205,255}; DBG_GREEN=(Color){22,137,93,255};
  DBG_RED=(Color){192,57,78,255}; DBG_ORANGE=(Color){177,93,23,255};
  DBG_BG=(Color){246,248,253,255}; DBG_PANEL=(Color){232,237,247,255};
  DBG_CARD=(Color){219,227,242,255}; DBG_LINE=(Color){191,201,220,255};
  DBG_CODE_BG=(Color){242,246,254,255}; DBG_BUTTON_TEXT=(Color){255,255,255,255};
 }else{
  DBG_WHITE=(Color){239,244,255,255}; DBG_MUTED=(Color){151,165,190,255};
  DBG_PURPLE=(Color){141,111,255,255}; DBG_GREEN=(Color){88,222,165,255};
  DBG_RED=(Color){255,117,139,255}; DBG_ORANGE=(Color){255,182,106,255};
  DBG_BG=(Color){13,18,31,255}; DBG_PANEL=(Color){23,30,47,255};
  DBG_CARD=(Color){29,38,59,255}; DBG_LINE=(Color){51,64,87,255};
  DBG_CODE_BG=(Color){12,21,34,255};DBG_BUTTON_TEXT=DBG_BG;
 }
}


static int current=0, picked=-1, submitted=0, attempts=0, correctAnswers=0, finished=0;
static unsigned char missed[DEBUG_COUNT(challenges)]={0};

static Color faded(Color c, unsigned char opacity){c.a=opacity;return c;}
static void label(const DebugChallengeView *v,const char *text,float x,float y,int size,Color color){
    DrawTextEx(v->font,text,(Vector2){x,y},(float)size,0.4f,color);
}
static float textWidth(const DebugChallengeView *v,const char *text,int size){
    return MeasureTextEx(v->font,text,(float)size,0.4f).x;
}
static void rectangle(float x,float y,float w,float h,Color color){
    DrawRectangleRounded((Rectangle){x,y,w,h},0.10f,12,color);
}
static void outline(float x,float y,float w,float h,Color color){
    DrawRectangleRoundedLinesEx((Rectangle){x,y,w,h},0.10f,12,1.5f,color);
}
static int clicked(const DebugChallengeView *v,Rectangle rect){
    rect.y-=v->scroll;
    return CheckCollisionPointRec(v->mouse,rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static int button(const DebugChallengeView *v,const char *text,float x,float y,float w,float h,Color color,int filled){
    Rectangle rect={x,y,w,h};
    Rectangle physical=rect;physical.y-=v->scroll;
    int hover=CheckCollisionPointRec(v->mouse,physical);
    rectangle(x,y,w,h,filled?(hover?ColorBrightness(color,0.13f):color):(hover?DBG_CARD:DBG_PANEL));
    if(!filled)outline(x,y,w,h,DBG_LINE);
    label(v,text,x+(w-textWidth(v,text,15))/2,y+(h-17)/2,15,filled?DBG_BUTTON_TEXT:DBG_WHITE);
    if(hover)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return clicked(v,rect);
}
/* Draw text with automatic line wrapping. Returns the next free Y coordinate. */
static float wrapped(const DebugChallengeView *v,const char *text,float x,float y,float maxWidth,int size,Color color){
    char line[900]={0};size_t count=0;
    const char *at=text;
    while(*at){
        if(*at=='\n'){
            label(v,line,x,y,size,color);count=0;line[0]='\0';y+=size+9;at++;continue;
        }
        while(*at==' ')at++;
        const char *end=at;while(*end && *end!=' ' && *end!='\n')end++;
        size_t length=(size_t)(end-at);if(!length)continue;
        if(length>250)length=250;
        char word[252];memcpy(word,at,length);word[length]='\0';
        char candidate[900];snprintf(candidate,sizeof(candidate),"%s%s%s",line,count?" ":"",word);
        if(count && textWidth(v,candidate,size)>maxWidth){
            label(v,line,x,y,size,color);count=0;line[0]='\0';y+=size+9;
        }
        if(count && count+1<sizeof(line)){line[count++]=' ';line[count]='\0';}
        if(count+length<sizeof(line)){memcpy(line+count,word,length);count+=length;line[count]='\0';}
        at=end;
    }
    if(count){label(v,line,x,y,size,color);y+=size+9;}
    return y;
}
static float codeBlock(const DebugChallengeView *v,const char *code,float x,float y,float w,Color color,const char *title){
    int lines=1;for(const char *p=code;*p;p++)if(*p=='\n')lines++;
    float height=56+lines*25;
    rectangle(x,y,w,height,DBG_CODE_BG);
    outline(x,y,w,height,DBG_LINE);
    rectangle(x+16,y+16,9,9,DBG_RED);
    rectangle(x+32,y+16,9,9,DBG_ORANGE);
    rectangle(x+48,y+16,9,9,DBG_GREEN);
    label(v,title,x+73,y+12,13,color);
    char copy[1400];snprintf(copy,sizeof(copy),"%s",code);
    int line=0;char *start=copy;
    for(char *cursor=copy;;cursor++){
        if(*cursor=='\n'||*cursor=='\0'){
            char end=*cursor;*cursor='\0';
            if(*start)label(v,start,x+19,y+42+line*25,16,DBG_WHITE);
            line++;
            if(end=='\0')break;
            start=cursor+1;
        }
    }
    return height;
}
static void resetSession(const DebugChallengeView *v){
    current=0;picked=-1;submitted=0;attempts=0;correctAnswers=0;finished=0;
    memset(missed,0,sizeof(missed));
    if(v->scrollPosition)*v->scrollPosition=0;
}

void DrawDebugChallenges(const DebugChallengeView *v,float *bottom){
    updateDebugTheme(v->lightTheme);
    float x=v->x,w=v->width,y=175;
    if(finished){
        label(v,"Debugging session complete",x,y,29,DBG_WHITE);y+=48;
        label(v,"Review your results and try again to reinforce the concepts.",x,y,15,DBG_MUTED);y+=52;
        rectangle(x,y,w,195,DBG_PANEL);
        label(v,"YOUR RESULT",x+25,y+25,13,DBG_PURPLE);
        char summary[120];snprintf(summary,sizeof(summary),"%d / %d correct",correctAnswers,DEBUG_COUNT(challenges));
        label(v,summary,x+25,y+57,36,DBG_WHITE);
        snprintf(summary,sizeof(summary),"Accuracy: %d%%",100*correctAnswers/DEBUG_COUNT(challenges));
        label(v,summary,x+25,y+113,21,DBG_GREEN);
        label(v,"Results are for this session only; your saved quiz progress is unchanged.",x+25,y+158,13,DBG_MUTED);
        y+=225;
        label(v,"Questions to review",x,y,22,DBG_WHITE);y+=44;
        int found=0;
        for(int i=0;i<DEBUG_COUNT(challenges);i++){
            if(!missed[i])continue;
            char s[180];snprintf(s,sizeof(s),"%02d   %s - %s",i+1,challenges[i].topic,challenges[i].title);
            rectangle(x,y,w,48,DBG_CARD);label(v,s,x+17,y+14,15,DBG_WHITE);y+=58;found++;
        }
        if(!found){label(v,"Excellent work - no incorrect answers this session.",x,y,16,DBG_GREEN);y+=48;}
        y+=15;
        if(button(v,"Try all challenges again",x,y,246,48,DBG_PURPLE,1))resetSession(v);
        *bottom=y+98;
        return;
    }
    const DebugChallenge *q=&challenges[current];
    label(v,"Debug the Code",x,y,29,DBG_WHITE);y+=45;
    label(v,"Read the buggy code, choose the best fix, then learn why it works.",x,y,15,DBG_MUTED);y+=37;
    char counter[100];snprintf(counter,sizeof(counter),"CHALLENGE %d OF %d",current+1,DEBUG_COUNT(challenges));
    label(v,counter,x,y,13,DBG_PURPLE);
    float barX=x+185,barWidth=w-185;
    rectangle(barX,y+4,barWidth,8,DBG_LINE);
    rectangle(barX,y+4,barWidth*((float)current/DEBUG_COUNT(challenges)),8,DBG_PURPLE);
    y+=35;
    rectangle(x,y,w,79,DBG_CARD);
    label(v,q->topic,x+20,y+11,12,DBG_PURPLE);
    label(v,q->title,x+20,y+36,24,DBG_WHITE);y+=95;
    y+=codeBlock(v,q->buggy,x,y,w,DBG_RED,"BUGGY CODE");y+=24;
    y=wrapped(v,q->prompt,x,y,w-20,18,DBG_WHITE)+18;
    for(int i=0;i<4;i++){
        Color accent=DBG_LINE,background=DBG_PANEL;
        if(submitted&&i==q->correct){accent=DBG_GREEN;background=faded(DBG_GREEN,35);}
        else if(submitted&&i==picked){accent=DBG_RED;background=faded(DBG_RED,35);}
        else if(!submitted&&i==picked){accent=DBG_PURPLE;background=faded(DBG_PURPLE,35);}
        float choiceHeight=65;
        rectangle(x,y,w,choiceHeight,background);outline(x,y,w,choiceHeight,accent);
        char letter[3]={(char)('A'+i),'\0','\0'};
        label(v,letter,x+20,y+18,18,(submitted || i==picked)?accent:DBG_MUTED);
        wrapped(v,q->choices[i],x+65,y+15,w-84,16,DBG_WHITE);
        if(!submitted&&clicked(v,(Rectangle){x,y,w,choiceHeight}))picked=i;
        y+=77;
    }
    y+=8;
    if(!submitted){
        label(v,"Select an answer to check your fix.",x,y+11,14,DBG_MUTED);
        if(picked>=0 && button(v,"Check my fix",x+w-182,y,182,48,DBG_PURPLE,1)){
            submitted=1;attempts++;
            if(picked==q->correct)correctAnswers++;
            else missed[current]=1;
        } else if(picked<0)button(v,"Choose a fix first",x+w-182,y,182,48,DBG_MUTED,0);
        y+=77;
    }else{
        int right=picked==q->correct;
        float feedbackTop=y;
        rectangle(x,y,w,135,faded(right?DBG_GREEN:DBG_ORANGE,29));
        label(v,right?"Correct - you found the fix!":"Not quite - here's the correct fix:",x+18,y+16,18,right?DBG_GREEN:DBG_ORANGE);
        float explanationEnd=wrapped(v,q->explanation,x+19,y+49,w-43,15,DBG_WHITE);
        y=fmaxf(feedbackTop+151,explanationEnd+23);
        y+=codeBlock(v,q->fixed,x,y,w,DBG_GREEN,"CORRECTED CODE");y+=24;
        const char *action=current==DEBUG_COUNT(challenges)-1?"See results  >":"Next challenge  >";
        if(button(v,action,x,y,205,49,DBG_PURPLE,1)){
            if(current==DEBUG_COUNT(challenges)-1)finished=1;
            else current++;
            picked=-1;submitted=0;
            if(v->scrollPosition)*v->scrollPosition=0;
        }
        y+=69;
    }
    *bottom=y+45;
}
