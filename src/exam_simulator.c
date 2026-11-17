/* Timed, written-code practice exam. C11 + raylib; no external services.
 * Inspired by the supplied Pointers & Linked List coding-exam worksheet.
 * Written solutions require student review, not unsafe automatic execution.
 */
#include "exam_simulator.h"
#include <stdio.h>
#include <string.h>

#define EXAM_SECONDS 3600.0
#define EXAM_QUESTIONS 5
#define ANSWER_CAP 4096
#define ARRAY_SIZE(a) ((int)(sizeof(a)/sizeof((a)[0])))

typedef struct {const char *title, *task, *signature, *example, *solution;} ExamQuestion;
static const ExamQuestion bank[]={
 {"Swap integers", "Write swap_ints using pointers. Swap the values, not the addresses.",
  "void swap_ints(int *a, int *b);", "x=5, y=10  ->  x=10, y=5",
  "void swap_ints(int *a, int *b) {\n    int temp = *a;\n    *a = *b;\n    *b = temp;\n}"},
 {"Swap characters", "Write swap_chars using pointers. Your function must modify both original characters.",
  "void swap_chars(char *a, char *b);", "a='A', b='B'  ->  a='B', b='A'",
  "void swap_chars(char *a, char *b) {\n    char temp = *a;\n    *a = *b;\n    *b = temp;\n}"},
 {"Find the maximum", "Find the largest integer in an array using pointer arithmetic only. Assume size > 0.",
  "int find_max(int *arr, int size);", "{3, 9, 2, 8, 5}  ->  9",
  "int find_max(int *arr, int size) {\n    int max = *arr;\n    for (int i = 1; i < size; i++) {\n        if (*(arr + i) > max) max = *(arr + i);\n    }\n    return max;\n}"},
 {"Reverse an array", "Reverse an integer array in place. Use pointer arithmetic rather than arr[i].",
  "void reverse_array(int *arr, int size);", "{1, 2, 3, 4, 5}  ->  {5, 4, 3, 2, 1}",
  "void reverse_array(int *arr, int size) {\n    int *left = arr, *right = arr + size - 1;\n    while (left < right) {\n        int tmp = *left;\n        *left++ = *right;\n        *right-- = tmp;\n    }\n}"},
 {"String length", "Count characters in a null-terminated C string without strlen or array indexing.",
  "int strlen_ptr(char *str);", "\"Hello\"  ->  5",
  "int strlen_ptr(char *str) {\n    int count = 0;\n    while (*str != '\\0') {\n        count++;\n        str++;\n    }\n    return count;\n}"},
 {"Count vowels", "Count upper- and lower-case vowels in a string, using a character pointer.",
  "int count_vowels(char *str);", "\"OpenAI\"  ->  4",
  "int count_vowels(char *str) {\n    int count = 0;\n    while (*str) {\n        char c = *str++;\n        if (c=='a'||c=='e'||c=='i'||c=='o'||c=='u'||\n            c=='A'||c=='E'||c=='I'||c=='O'||c=='U') count++;\n    }\n    return count;\n}"},
 {"Copy a string", "Copy src into dest using pointers. Assume dest has enough capacity, and copy the terminator.",
  "void copy_string(char *dest, char *src);", "src=\"Hello\"  ->  dest=\"Hello\"",
  "void copy_string(char *dest, char *src) {\n    while ((*dest++ = *src++) != '\\0') {\n    }\n}"},
 {"Concatenate strings", "Append src to the end of dest using pointers. Assume enough space in dest.",
  "void concat_strings(char *dest, char *src);", "\"Hello, \" + \"World!\"  ->  \"Hello, World!\"",
  "void concat_strings(char *dest, char *src) {\n    while (*dest) dest++;\n    while ((*dest++ = *src++) != '\\0') {\n    }\n}"},
 {"Sum an array", "Return the total of all array elements using pointer arithmetic (no arr[i]).",
  "int sum_array(int *arr, int size);", "{1, 2, 3, 4}  ->  10",
  "int sum_array(int *arr, int size) {\n    int sum = 0;\n    for (int i = 0; i < size; i++) sum += *(arr + i);\n    return sum;\n}"},
 {"Compare strings", "Compare two strings lexicographically using pointers, as strcmp does. Do not call strcmp.",
  "int compare_strings(char *str1, char *str2);", "\"apple\" vs \"banana\"  ->  negative",
  "int compare_strings(char *str1, char *str2) {\n    while (*str1 && *str1 == *str2) {\n        str1++; str2++;\n    }\n    return (unsigned char)*str1 - (unsigned char)*str2;\n}"},
 {"Linked-list insertion", "Given the node definition below, insert a new value at the beginning of a singly linked list. Update *head and handle an empty list.",
  "typedef struct Node { int data; struct Node *next; } Node;\nvoid insert_at_beginning(Node **head, int value);",
  "5 -> 8 -> NULL; insert 3  ->  3 -> 5 -> 8 -> NULL",
  "void insert_at_beginning(Node **head, int value) {\n    Node *node = malloc(sizeof *node);\n    if (!node) return;\n    node->data = value;\n    node->next = *head;\n    *head = node;\n}\n/* Include <stdlib.h> for malloc. */"},
 {"Doubly linked-list insertion", "Given a doubly linked Node (data, prev, next), insert a new node at the front. Fix both forward and backward links. Assume typedef Node exists.",
  "void push_front(Node **head, int value);", "old head->prev must point to the new node",
  "void push_front(Node **head, int value) {\n    Node *node = malloc(sizeof *node);\n    if (!node) return;\n    node->data = value;\n    node->prev = NULL;\n    node->next = *head;\n    if (*head) (*head)->prev = node;\n    *head = node;\n}\n/* Include <stdlib.h> for malloc. */"}
};

static struct {
 int running, finished, timedOut, showTimeout, confirmSubmit, position, cursor;
 int ids[EXAM_QUESTIONS];
 char answers[EXAM_QUESTIONS][ANSWER_CAP];
 int reviewed[EXAM_QUESTIONS]; /* -1 not reviewed, 0 needs work, 1 correct */
 double startedAt, stoppedAt;
 int editorScroll;
} exam={0};

static Color fade(Color c,float opacity){c.a=(unsigned char)(255.0f*opacity);return c;}
static void panel(Rectangle r,Color fill,Color edge){
 DrawRectangleRounded(r,.12f,10,fill);
 DrawRectangleRoundedLinesEx(r,.12f,10,1.3f,edge);
}
static void label(Font font,const char *s,float x,float y,int size,Color color){
 DrawTextEx(font,s,(Vector2){x,y},(float)size,.2f,color);
}
static float width(Font font,const char *s,int size){return MeasureTextEx(font,s,(float)size,.2f).x;}
static int inside(const ExamStyle *s,Rectangle r){
 r.y-=s->scroll;
 return CheckCollisionPointRec(s->mouse,r);
}
static int click(const ExamStyle *s,Rectangle r){return inside(s,r)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);}
static int button(const ExamStyle *s,Rectangle r,const char *title,int active){
 Color border=active?s->accent:s->line;
 panel(r,active?s->accent:s->panel,border);
 label(s->font,title,r.x+(r.width-width(s->font,title,15))/2,r.y+(r.height-17)/2,15,active?s->accentText:s->text);
 return click(s,r);
}
static double remaining(void){
 double elapsed=(exam.finished?exam.stoppedAt:GetTime())-exam.startedAt;
 double result=EXAM_SECONDS-elapsed;
 return result>0?result:0;
}
int ExamIsRunning(void){return exam.running;}
int ExamIsFinished(void){return exam.finished;}
int ExamTimeoutPopupVisible(void){return exam.showTimeout;}
void ExamLeave(void){exam.running=0;exam.showTimeout=0;exam.confirmSubmit=0;}

void ExamStart(void){
 memset(&exam,0,sizeof(exam));
 /* Pick 4 pointer functions plus one linked-list problem per exam. */
 int pool[10];for(int i=0;i<10;i++)pool[i]=i;
 for(int i=9;i>0;i--){int j=GetRandomValue(0,i),tmp=pool[i];pool[i]=pool[j];pool[j]=tmp;}
 for(int i=0;i<4;i++)exam.ids[i]=pool[i];
 exam.ids[4]=GetRandomValue(10,ARRAY_SIZE(bank)-1);
 for(int i=4;i>0;i--){int j=GetRandomValue(0,i),tmp=exam.ids[i];exam.ids[i]=exam.ids[j];exam.ids[j]=tmp;}
 for(int i=0;i<EXAM_QUESTIONS;i++)exam.reviewed[i]=-1;
 exam.running=1;exam.startedAt=GetTime();
}

static void finishExam(int timedOut){
 if(exam.finished)return;
 exam.finished=1;exam.timedOut=timedOut;exam.showTimeout=timedOut;
 exam.confirmSubmit=0;exam.stoppedAt=GetTime();
}
void ExamUpdate(void){
 if(exam.running&&!exam.finished&&remaining()<=0)finishExam(1);
}

/* Keyboard editing works entirely inside the exam; no compiler is launched. */
static void editAnswer(void){
 char *buf=exam.answers[exam.position];
 int len=(int)strlen(buf);
 if(exam.cursor>len)exam.cursor=len;
 if(IsKeyPressed(KEY_LEFT)&&exam.cursor>0)exam.cursor--;
 if(IsKeyPressed(KEY_RIGHT)&&exam.cursor<len)exam.cursor++;
 if(IsKeyPressed(KEY_HOME)){
  while(exam.cursor>0&&buf[exam.cursor-1]!='\n')exam.cursor--;
 }
 if(IsKeyPressed(KEY_END)){
  while(exam.cursor<len&&buf[exam.cursor]!='\n')exam.cursor++;
 }
 if(IsKeyPressed(KEY_BACKSPACE)&&exam.cursor>0){
  memmove(buf+exam.cursor-1,buf+exam.cursor,(size_t)(len-exam.cursor+1));exam.cursor--;len--;
 }
 if(IsKeyPressed(KEY_DELETE)&&exam.cursor<len){
  memmove(buf+exam.cursor,buf+exam.cursor+1,(size_t)(len-exam.cursor));len--;
 }
 int key;
 while((key=GetCharPressed())!=0){
  if(key>=32&&key<=126&&len<ANSWER_CAP-1){
   memmove(buf+exam.cursor+1,buf+exam.cursor,(size_t)(len-exam.cursor+1));
   buf[exam.cursor++]=(char)key;len++;
  }
 }
 if(IsKeyPressed(KEY_ENTER)&&len<ANSWER_CAP-1){
  memmove(buf+exam.cursor+1,buf+exam.cursor,(size_t)(len-exam.cursor+1));
  buf[exam.cursor++]='\n';len++;
 }
 if(IsKeyPressed(KEY_TAB)&&len<ANSWER_CAP-5){
  memmove(buf+exam.cursor+4,buf+exam.cursor,(size_t)(len-exam.cursor+1));
  memcpy(buf+exam.cursor,"    ",4);exam.cursor+=4;
 }
 if((IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL)||IsKeyDown(KEY_LEFT_SUPER)||IsKeyDown(KEY_RIGHT_SUPER))&&IsKeyPressed(KEY_V)){
  const char *clip=GetClipboardText();
  if(clip){int n=(int)strlen(clip);if(n>ANSWER_CAP-1-len)n=ANSWER_CAP-1-len;
   memmove(buf+exam.cursor+n,buf+exam.cursor,(size_t)(len-exam.cursor+1));
   memcpy(buf+exam.cursor,clip,(size_t)n);exam.cursor+=n;}
 }
}

static void codeLines(const ExamStyle *s,const char *text,float x,float y,float maxW,int lines,int cursor,int showCursor){
 /* Wrap at screen width and newlines. Always draw within the panel. */
 float lineH=22.0f, xx=x, yy=y;
 float glyph=width(s->codeFont,"M",16);
 if(glyph<7)glyph=9;
 int row=0,lastCursor=-1;
 char ch[2]={0,0};
 for(int i=0;;i++){
  char c=text[i];
  if(showCursor&&i==cursor)lastCursor=i;
  if(c=='\0'){
   if(showCursor&&lastCursor==i&&row<lines){DrawRectangle((int)xx,(int)(yy+1),2,19,s->accent);}
   break;
  }
  if(c=='\n'||xx+glyph>x+maxW){
   if(showCursor&&lastCursor==i&&row<lines)DrawRectangle((int)xx,(int)(yy+1),2,19,s->accent);
   xx=x;yy+=lineH;row++;
   if(row>=lines)break;
   if(c=='\n')continue;
  }
  ch[0]=c;
  if(c!='\r')label(s->codeFont,ch,xx,yy,16,s->text);
  xx+=glyph;
 }
}

static void wrappedText(const ExamStyle *s,const char *text,float x,float y,float maxW,int size,int maxLines){
 char line[220]="";
 const char *p=text;
 for(int row=0;row<maxLines&&*p;row++){
  int len=0;
  while(*p&&*p!='\n'){
   if(len>=205)break;
   line[len]=*p;line[len+1]='\0';
   if(width(s->font,line,size)>maxW){
    if(len>0){line[len]='\0';break;}
   }
   len++;p++;
  }
  label(s->font,line,x,y+row*(size+8),size,s->text);
  if(*p=='\n')p++;
 }
}

void ExamDraw(const ExamStyle *s,float *bottom){
 float w=s->width<930?s->width:930, x=s->x+(s->width-w)*.5f,y=104;
 const ExamQuestion *q=&bank[exam.ids[exam.position]];
 char clock[40];int secs=(int)remaining();
 snprintf(clock,sizeof(clock),"%02d:%02d",secs/60,secs%60);
 label(s->font,"EXAM SIMULATOR  /  CODING EXAM 2",x,y,13,s->accent);
 label(s->headingFont,exam.finished?"Exam review":"Pointer & linked-list exam",x,y+29,29,s->text);
 panel((Rectangle){x+w-160,y+7,155,62},s->panel,secs<300&&!exam.finished?s->orange:s->line);
 label(s->font,exam.finished?"TIME REMAINING":"TIME LEFT",x+w-145,y+16,11,s->muted);
 label(s->headingFont,clock,x+w-145,y+32,24,secs<300?s->orange:s->accent);
 y+=97;
 label(s->font,exam.finished?"Compare your answers with the sample solutions. Mark each response yourself.":
       "5 written coding questions  |  60 minutes  |  20 marks per question",x,y,15,s->muted);
 y+=44;
 for(int i=0;i<EXAM_QUESTIONS;i++){
  float bw=(w-4*9)/5, bx=x+i*(bw+9);
  char name[25];snprintf(name,sizeof(name),"QUESTION %d",i+1);
  if(button(s,(Rectangle){bx,y,bw,42},name,exam.position==i)){
   exam.position=i;exam.cursor=(int)strlen(exam.answers[i]);exam.editorScroll=0;
  }
 }
 y+=65;
 char num[50];snprintf(num,sizeof(num),"QUESTION %d OF 5",exam.position+1);
 label(s->font,num,x+20,y+18,13,s->accent);
 float qHeight=253;
 panel((Rectangle){x,y,w,qHeight},s->card,s->line);
 label(s->headingFont,q->title,x+20,y+43,25,s->text);
 wrappedText(s,q->task,x+20,y+83,w-45,16,3);
 label(s->font,"PROTOTYPE / NODE DECLARATION",x+20,y+154,11,s->muted);
 codeLines(s,q->signature,x+20,y+177,w-42,3,0,0);
 y+=qHeight+22;
 label(s->font,"EXAMPLE",x+3,y,12,s->accent);
 label(s->codeFont,q->example,x+3,y+23,15,s->muted);
 y+=72;
 if(exam.finished){
  panel((Rectangle){x,y,w,174},s->panel,s->line);
  label(s->font,"YOUR SUBMITTED ANSWER",x+18,y+15,12,s->accent);
  codeLines(s,exam.answers[exam.position][0]?exam.answers[exam.position]:"(No answer)",x+18,y+46,w-40,5,0,0);
  y+=194;
  panel((Rectangle){x,y,w,270},fade(s->green,.12f),s->green);
  label(s->font,"SAMPLE SOLUTION (OTHER CORRECT ANSWERS ARE POSSIBLE)",x+18,y+15,12,s->green);
  codeLines(s,q->solution,x+18,y+43,w-40,10,0,0);
  y+=292;
  label(s->font,"SELF-CHECK: WAS YOUR ANSWER CORRECT?",x,y,12,s->muted);
  if(button(s,(Rectangle){x,y+33,180,43},"Correct  +20",exam.reviewed[exam.position]==1))exam.reviewed[exam.position]=1;
  if(button(s,(Rectangle){x+193,y+33,188,43},"Needs practice",exam.reviewed[exam.position]==0))exam.reviewed[exam.position]=0;
  y+=102;
  int done=0,correct=0;
  for(int i=0;i<EXAM_QUESTIONS;i++){if(exam.reviewed[i]>=0){done++;correct+=exam.reviewed[i];}}
  char marks[110];snprintf(marks,sizeof(marks),"Self-marked: %d / 100 points  |  %d / 5 reviewed",correct*20,done);
  label(s->headingFont,marks,x,y,18,s->text);y+=59;
  if(button(s,(Rectangle){x,y,182,47},"Back to quizzes",0)){ExamLeave();}
 }else{
  panel((Rectangle){x,y,w,335},s->panel,s->accent);
  label(s->font,"YOUR CODE ANSWER  -  click here to type  |  Enter: new line  |  Tab: indent  |  Paste: Cmd/Ctrl+V",x+17,y+13,12,s->muted);
  Rectangle edit=(Rectangle){x+14,y+44,w-28,274};
  DrawRectangleRounded(edit,.06f,6,s->background);
  if(click(s,edit))exam.confirmSubmit=0;
  if(!exam.confirmSubmit)editAnswer();
  codeLines(s,exam.answers[exam.position],edit.x+10,edit.y+9,edit.width-24,11,exam.cursor,1);
  y+=357;
  label(s->font,"Answers remain available while navigating questions. Submit when finished.",x,y,13,s->muted);
  y+=44;
  if(exam.position>0&&button(s,(Rectangle){x,y,180,48},"< Previous",0)){
   exam.position--;exam.cursor=(int)strlen(exam.answers[exam.position]);
  }
  if(exam.position<EXAM_QUESTIONS-1&&button(s,(Rectangle){x+w-185,y,185,48},"Next question >",1)){
   exam.position++;exam.cursor=(int)strlen(exam.answers[exam.position]);
  }
  y+=72;
  if(!exam.confirmSubmit){
   if(button(s,(Rectangle){x,y,211,50},"Submit entire exam",0))exam.confirmSubmit=1;
  }else{
   label(s->font,"Finish now? You will not be able to edit your answers.",x,y-5,15,s->orange);
   if(button(s,(Rectangle){x,y+30,196,45},"Yes, submit",1))finishExam(0);
   if(button(s,(Rectangle){x+209,y+30,177,45},"Keep working",0))exam.confirmSubmit=0;
   y+=28;
  }
 }
 *bottom=y+118;
}

void ExamDrawTimeoutPopup(const ExamStyle *s,int screenW,int screenH){
 if(!exam.showTimeout)return;
 DrawRectangle(0,0,screenW,screenH,fade((Color){0,0,0,255},.77f));
 float w=490,x=(screenW-w)*.5f,y=(screenH-260)*.5f;
 panel((Rectangle){x,y,w,260},s->panel,s->orange);
 label(s->headingFont,"Time is up!",x+38,y+36,34,s->orange);
 label(s->font,"Your 60-minute exam has ended.",x+38,y+98,17,s->text);
 label(s->font,"Your answers have been submitted automatically.",x+38,y+132,15,s->muted);
 Rectangle action={x+38,y+185,w-76,48};
 DrawRectangleRounded(action,.18f,8,s->accent);
 const char *title="Review my answers";
 label(s->font,title,action.x+(action.width-width(s->font,title,16))*.5f,action.y+14,16,s->accentText);
 if(CheckCollisionPointRec(s->mouse,action)&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT))exam.showTimeout=0;
}
