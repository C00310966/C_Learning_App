/* CODEQUEST - C/C++ learning app. C11 + raylib only.
 * Build: make (macOS) or Ctrl+Shift+B in VS Code (Windows GCC)
 * UI and quiz interactions live here; curriculum data and progress live in separate C modules.
 */
#include "raylib.h"
#include "curriculum.h"
#include "progress.h"
#include "app.h"
#include "lab_runner.h"
#include "debug_challenges.h"
#include "celebration.h"
#include "exam_simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define COUNT(a) ((int)(sizeof(a)/sizeof((a)[0])))

static Color BG={12,16,26,255}, PANEL={24,31,45,255}, CARD={34,43,60,255};
static Color CQ_WHITE={237,241,250,255}, CQ_MUTED={157,171,194,255}, CQ_LINE={58,70,91,255};
static Color CQ_PURPLE={155,133,255,255}, CQ_GREEN={89,218,187,255}, CQ_ORANGE={245,185,113,255}, CQ_RED={241,112,140,255}, CQ_BLUE={111,181,244,255};
static int lightTheme=0;
static int navCollapsed=0; /* Sidebar can be reduced to an icon rail. */
static Color textOnAccent={13,18,31,255};
static void applyTheme(void){
 if(lightTheme){
  BG=(Color){247,249,253,255}; PANEL=(Color){237,241,249,255}; CARD=(Color){226,233,245,255};
  CQ_WHITE=(Color){32,43,64,255}; CQ_MUTED=(Color){91,105,131,255}; CQ_LINE=(Color){197,208,228,255};
  CQ_PURPLE=(Color){105,79,198,255}; CQ_GREEN=(Color){22,136,108,255}; CQ_ORANGE=(Color){173,104,34,255};
  CQ_RED=(Color){190,65,87,255}; CQ_BLUE=(Color){53,116,191,255};
  textOnAccent=(Color){255,255,255,255};
 }else{
  BG=(Color){12,16,26,255}; PANEL=(Color){24,31,45,255}; CARD=(Color){34,43,60,255};
  CQ_WHITE=(Color){237,241,250,255}; CQ_MUTED=(Color){157,171,194,255}; CQ_LINE=(Color){58,70,91,255};
  CQ_PURPLE=(Color){155,133,255,255}; CQ_GREEN=(Color){89,218,187,255}; CQ_ORANGE=(Color){245,185,113,255};
  CQ_RED=(Color){241,112,140,255}; CQ_BLUE=(Color){111,181,244,255};
  textOnAccent=BG;
 }
}
static void loadTheme(void){FILE*f=fopen("codequest_theme.cfg","r");if(f){int mode=0;if(fscanf(f,"%d",&mode)==1)lightTheme=(mode==1);fclose(f);}applyTheme();}
static void toggleTheme(void){lightTheme=!lightTheme;applyTheme();FILE*f=fopen("codequest_theme.cfg","w");if(f){fprintf(f,"%d\n",lightTheme);fclose(f);}}
static Font font={0};
static Font headingFont={0};
static Font codeFont={0};
static int ownsHeadingFont=0, ownsCodeFont=0;
static int screen=0, selectedTrack=-1, selectedLesson=-1, selectedLab=-1;
/* A quiz is a complete track assessment, not an individual question.
 * The session stores answers so the final review can identify weaknesses. */
static char labCode[LAB_CODE_CAP]="";
static int labLoaded=-1, labCursor=0, labFocus=0, labFirstLine=0;
/* Selection is the half-open interval between anchor and cursor. */
static int labAnchor=0, labDragging=0;
static float labDragScrollTime=0.0f;
/* Scrollbar drag is separate from code/text selection dragging. */
static int labScrollbarDragging=0;
static float labScrollbarGrab=0.0f;
static Rectangle labEditorWheelRect={0};
#define LAB_EDITOR_VISIBLE_LINES 13
#define LAB_EDITOR_LINE_HEIGHT 23.0f
static int labRepeatKey=0;
static float labRepeatTime=0.0f;
static LabRunResult labResult={0};
static int labHasResult=0;
static int labWasCheck=0; /* A successful Run is neutral until Check Solution verifies it. */
static int quizTrack=-1, quizPosition=0, quizCount=0;
static int quizQuestionIds[30], quizSelections[30];
static int quizChoice=-1, quizAnswered=0, quizFinished=0;
static int debugMode=0;
static int examMode=0;

static float scroll=0, maxScroll=0;
static int windowW=1380,windowH=850;
/* A logical UI canvas is scaled to the actual window in both windowed and
 * fullscreen modes. Drawing text at native resolution keeps it sharp. */
static float uiScale=1.0f, userZoom=1.0f;
static Vector2 pointer(void){
 Vector2 mouse=GetMousePosition();
 return (Vector2){mouse.x/uiScale,mouse.y/uiScale};
}
static float computeScale(int physicalW,int physicalH){
 float fit=fminf((float)physicalW/1100.0f,(float)physicalH/720.0f);
 /* At very small window sizes use scroll/compact layout instead of
  * magnifying beyond the available space. */
 float maxSafe=fminf((float)physicalW/1000.0f,(float)physicalH/650.0f);
 return fmaxf(1.0f,fminf(fminf(4.0f,maxSafe),fit*userZoom));
}
/* Stage 8: subtle, frame-rate independent UI easing. Purely visual state. */
static float hoverEase(float current,int active,float speed){
 float dt=fminf(GetFrameTime(),0.05f);
 float blend=1.0f-expf(-speed*dt);
 return current+((active?1.0f:0.0f)-current)*blend;
}
static Color mixColor(Color a,Color b,float t){
 t=fmaxf(0.0f,fminf(1.0f,t));
 return (Color){(unsigned char)(a.r+(b.r-a.r)*t),
                (unsigned char)(a.g+(b.g-a.g)*t),
                (unsigned char)(a.b+(b.b-a.b)*t),
                (unsigned char)(a.a+(b.a-a.a)*t)};
}
/* Cache button hover progress by visible label and layout slot, independent
 * of draw order or clipping. Offscreen buttons do not animate on input. */
typedef struct {unsigned int key;float value;unsigned int frame;} HoverEntry;
static HoverEntry buttonHovers[192]={0};
static unsigned int hoverFrame=0;
static float buttonEase(const char *label,float x,float y,int hovered){
 unsigned int h=2166136261u;
 for(const unsigned char *c=(const unsigned char *)label;*c;c++)h=(h^*c)*16777619u;
 int px=(int)(x*.5f),py=(int)(y*.5f);
 h=(h^(unsigned int)px)*16777619u;
 h=(h^(unsigned int)py)*16777619u;
 h=(h^(unsigned int)screen)*16777619u;
 if(!h)h=1;
 int slot=-1,oldest=0;
 for(int i=0;i<COUNT(buttonHovers);i++){
  if(buttonHovers[i].key==h){slot=i;break;}
  if(buttonHovers[i].key==0){slot=i;break;}
  if(buttonHovers[i].frame<buttonHovers[oldest].frame)oldest=i;
 }
 if(slot<0)slot=oldest;
 HoverEntry *e=&buttonHovers[slot];
 if(e->key!=h){e->key=h;e->value=0;}
 e->frame=hoverFrame;
 e->value=hoverEase(e->value,hovered,13.0f);
 return e->value;
}
static float navHovers[5]={0};
static float journeyHovers[5]={0};
static float badgeHovers[9]={0};
static float quizTrackHovers[5]={0};
static float quizOptionHovers[4]={0};
static float pageReveal=0.0f;
static int lastPageSignature=-1;
static int uiScrollActive=0;
static char searchText[96]="";
static int searchActive=0;

typedef struct {float x,y,w,h;} Rect;
static Rectangle RR(float x,float y,float w,float h){return (Rectangle){x,y,w,h};}
static Color alpha(Color c,float a){c.a=(unsigned char)(255.0f*a);return c;}
static Rectangle clickRect(Rectangle r){if(uiScrollActive)r.y-=scroll;return r;}
static int hit(Rectangle r){return CheckCollisionPointRec(pointer(),clickRect(r)) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);}
/* Consistent font roles: body, editorial heading, and technical monospace. */
static void tx(const char*s,float x,float y,int size,Color color){DrawTextEx(font,s,(Vector2){x,y},(float)size,0.35f,color);}
static void txH(const char*s,float x,float y,int size,Color color){DrawTextEx(headingFont,s,(Vector2){x,y},(float)size,0.15f,color);}
static void txCode(const char*s,float x,float y,int size,Color color){DrawTextEx(codeFont,s,(Vector2){x,y},(float)size,0.0f,color);}
static float tw(const char*s,int size){return MeasureTextEx(font,s,(float)size,0.35f).x;}
static void box(float x,float y,float w,float h,float round,Color c){DrawRectangleRounded(RR(x,y,w,h),round,12,c);}
static void border(float x,float y,float w,float h,float round,Color c){DrawRectangleRoundedLinesEx(RR(x,y,w,h),round,12,1.1f,c);}
/* A subtle status glow behind both feedback and console output. Several low-alpha
 * layers create a soft halo without shaders or external assets. */
static void resultGlow(float x,float y,float w,float h,Color accent){
 box(x-6,y-6,w+12,h+12,.075f,alpha(accent,lightTheme?.035f:.060f));
 box(x-3,y-3,w+6,h+6,.075f,alpha(accent,lightTheme?.055f:.085f));
 box(x,y,w,h,.075f,alpha(accent,lightTheme?.095f:.115f));
 border(x,y,w,h,.075f,alpha(accent,lightTheme?.35f:.29f));
}

static int btn(const char *text,float x,float y,float w,float h,Color color,int filled){
 Rectangle r=RR(x,y,w,h);
 int hover=CheckCollisionPointRec(pointer(),clickRect(r));
 float a=buttonEase(text,x,y,hover);
 /* A quiet brightness shift + hairline highlight; no layout shift on click. */
 Color base=filled?color:PANEL;
 Color bright=filled?ColorBrightness(color,.11f):CARD;
 box(x,y,w,h,.24f,mixColor(base,bright,a));
 if(!filled)border(x,y,w,h,.24f,mixColor(CQ_LINE,alpha(CQ_PURPLE,.72f),a));
 else if(a>.005f)border(x,y,w,h,.24f,alpha(CQ_WHITE,a*.26f));
 tx(text,x+(w-tw(text,15))/2,y+(h-17)/2,15,filled?textOnAccent:CQ_WHITE);
 if(hover)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
 return hit(r);
}
static void pill(const char*s,float x,float y,Color c){float w=tw(s,11)+24;box(x,y,w,25,.45f,alpha(c,.14f));tx(s,x+12,y+6,11,c);}
static void bar(float x,float y,float w,float value,Color c){box(x,y,w,6,1,alpha(CQ_LINE,.90f));if(value>0)box(x,y,fminf(w,fmaxf(4,w*fminf(value,1))),6,1,c);}
static void richLine(const char *s,float x,float y,int sz,Color color,float width,float *outY){
 /* Word wrapping for variable-width fonts; also respects explicit newlines. */
 char line[1024]={0};int n=0;float cy=y;const char*p=s;
 while(*p){
  if(*p=='\n'){line[n]=0;tx(line,x,cy,sz,color);n=0;line[0]=0;cy+=sz+9;p++;continue;}
  const char*e=p;while(*e && *e!=' ' && *e!='\n')e++;
  int len=(int)(e-p); if(len>350)len=350;
  char word[360];memcpy(word,p,(size_t)len);word[len]=0;
  if(n>0 && tw(line,sz)+tw(" ",sz)+tw(word,sz)>width){tx(line,x,cy,sz,color);cy+=sz+9;n=0;line[0]=0;}
  if(n && n+1<(int)sizeof(line)){line[n++]=' ';line[n]=0;}
  if(n+len<(int)sizeof(line)){memcpy(line+n,word,(size_t)len);n+=len;line[n]=0;}
  p=e;if(*p==' ')p++;
 }
 if(n){tx(line,x,cy,sz,color);cy+=sz+9;}*outY=cy;
}
static float paragraph(const char*s,float x,float y,int sz,Color color,float w){float end=y;richLine(s,x,y,sz,color,w,&end);return end;}
static float codePanel(const char *code,float x,float y,float w){
 int lines=1;for(const char*p=code;*p;p++)if(*p=='\n')lines++;
 float height=54+lines*23;box(x,y,w,height,.065f,lightTheme?(Color){222,231,245,255}:(Color){14,22,35,255});
 border(x,y,w,height,.065f,alpha(CQ_LINE,.60f));
 box(x+20,y+17,8,8,1,CQ_RED);box(x+35,y+17,8,8,1,CQ_ORANGE);box(x+50,y+17,8,8,1,CQ_GREEN);
 tx("CODE EXAMPLE",x+w-143,y+13,11,CQ_MUTED);
 char copy[4500];snprintf(copy,sizeof(copy),"%s",code);char*line=strtok(copy,"\n");int i=0;
 while(line){Color c=CQ_WHITE;if(strstr(line,"//")||strstr(line,"/*"))c=CQ_MUTED;
 txCode(line,x+22,y+47+i*23,15,c);i++;line=strtok(NULL,"\n");}
 return height;
}
static int totalLearned(void){int n=0;for(int i=0;i<COUNT(lessons);i++)n+=progress.learned[i]!=0;return n;}
static int totalLabs(void){int n=0;for(int i=0;i<COUNT(labTasks);i++)n+=progress.labDone[i]!=0;return n;}
static int totalQuizzes(void){int n=0;for(int i=0;i<COUNT(quizzes);i++)n+=progress.quizDone[i]!=0;return n;}
static int trackLessonCount(int t){int n=0;for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t)n++;return n;}
static int trackDone(int t){int n=0;for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t)n+=progress.learned[i]!=0;return n;}
static int trackComplete(int t){
 int found=0;
 for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t){found=1;if(!progress.learned[i])return 0;}
 for(int i=0;i<COUNT(labTasks);i++)if(labTasks[i].track==t){found=1;if(!progress.labDone[i])return 0;}
 for(int i=0;i<COUNT(quizzes);i++)if(quizzes[i].track==t){found=1;if(!progress.quizDone[i])return 0;}
 return found;
}
static void checkTrackRewards(void){
 for(int t=0;t<COUNT(tracks);t++)if(trackComplete(t))rewardTrack(t);
}
static int countPerfectQuizzes(void){int n=0;for(int t=0;t<CQ_TRACKS;t++)n+=progress.quizPerfectReward[t]!=0;return n;}
static int achievementUnlocked(int id){
 switch(id){
 case 0:return totalLearned()>=1;
 case 1:return totalLearned()>=10;
 case 2:return totalLabs()>=1;
 case 3:return totalLabs()>=5;
 case 4:return countPerfectQuizzes()>=1;
 case 5:return trackComplete(1);
 case 6:return cqLevel()>=5;
 case 7:for(int t=0;t<CQ_TRACKS;t++)if(!trackComplete(t))return 0;return 1;
 case 8:return cqDebugMastered()==CQ_DEBUG_COUNT;
 default:return 0;
 }
}
static const char *extraLabHints[24]={
 "A century year like 1900 is not a leap year, but 2000 is. Check % 400 before % 100.",
 "Keep a copy of the original integer. Each iteration moves the reversed number left by multiplying by 10.",
 "Handle n < 2 first. Check possible divisors while divisor <= n / divisor.",
 "Read binary digits from right to left; multiply each digit by 1, 2, 4, 8, and so on.",
 "Build each row with an inner loop, then print a newline after that row.",
 "Keep two distinct maxima, and skip numbers equal to the current maximum.",
 "For insertion, move elements from the end towards the insertion index so values are not overwritten.",
 "Two separate indexes are required; using one movie twice must not count.",
 "Mark matched runs before dropping cells. Process each column from bottom to top when applying gravity.",
 "Push a new scope at '{' and pop it at '}'. A later declaration can shadow an earlier name.",
 "Use a temporary variable: temp = *a; *a = *b; *b = temp;.",
 "Start best at *arr, and move a pointer from arr + 1 toward arr + size.",
 "Use left = arr and right = arr + size - 1. Swap *left and *right until they meet.",
 "Read one character at a time through *p. For copies, remember the final null terminator.",
 "Stop on the first mismatching character; if one string ends first, the shorter prefix comes first.",
 "Every new[] should have a matching delete[]. Never return the address of a local stack variable.",
 "If *head is NULL, the new node becomes the head. Otherwise walk until current->next is NULL.",
 "Index zero updates *head. For another position, find the preceding node and link the new node between two neighbors.",
 "When inserting, update node->prev, node->next and the neighboring nodes' reverse links.",
 "Keep the template definition visible where you use it; test with both int and double values.",
 "Return a new Point with x + rhs.x and y + rhs.y; do not mutate the original point.",
 "Call std::sort(values.begin(), values.end()) after including <algorithm>.",
 "std::list::splice transfers nodes, so verify that the source list becomes empty after the move.",
 "Read each word into a std::string, then use counts[word]++ to update the map."
};
static void nav(int s){if(examMode&&!ExamIsFinished()&&s!=2)return;if(examMode&&s!=2){ExamLeave();examMode=0;}if(labLoaded>=0)labSaveDraft(labLoaded,labCode);labFocus=0;screen=s;selectedLesson=-1;selectedLab=-1;scroll=0;searchActive=0;if(s==2){quizTrack=-1;quizFinished=0;}}
static int contains(const char*s,const char*term){if(!term[0])return 1;char a[1100],b[100];snprintf(a,sizeof(a),"%s",s);snprintf(b,sizeof(b),"%s",term);for(char*p=a;*p;p++)if(*p>='A'&&*p<='Z')*p+=32;for(char*p=b;*p;p++)if(*p>='A'&&*p<='Z')*p+=32;return strstr(a,b)!=NULL;}
static void startQuiz(int track){
 if(track<0 || track>=COUNT(tracks))return;
 int ids[COUNT(quizzes)], count=0;
 for(int i=0;i<COUNT(quizzes);i++)if(quizzes[i].track==track)ids[count++]=i;
 if(count==0)return;
 nav(2);
 quizTrack=track;quizCount=count;quizPosition=0;quizChoice=-1;
 quizAnswered=0;quizFinished=0;
 for(int i=0;i<count;i++){quizQuestionIds[i]=ids[i];quizSelections[i]=-1;}
}
static int quizScore(void){
 int score=0;
 for(int i=0;i<quizCount;i++){
  int id=quizQuestionIds[i];
  if(quizSelections[i]==quizzes[id].correct)score++;
 }
 return score;
}
static int quizCountForTrack(int track){
 int count=0;
 for(int i=0;i<COUNT(quizzes);i++)if(quizzes[i].track==track)count++;
 return count;
}
static int quizMasteredForTrack(int track){
 int count=0;
 for(int i=0;i<COUNT(quizzes);i++)if(quizzes[i].track==track&&progress.quizDone[i])count++;
 return count;
}
/* Icon artwork is drawn with raylib primitives, so no extra fonts/assets are needed. */
static void themeMoon(float cx,float cy,float radius,Color foreground,Color cutout){
 DrawCircle((int)cx,(int)cy,radius,foreground);
 DrawCircle((int)(cx+radius*.47f),(int)(cy-radius*.24f),radius*.83f,cutout);
}
static void themeSun(float cx,float cy,float radius,Color foreground){
 DrawCircle((int)cx,(int)cy,radius*.54f,foreground);
 for(int i=0;i<8;i++){
  float angle=(float)i*3.14159265358979323846f/4.0f;
  Vector2 a={cx+cosf(angle)*radius*.85f,cy+sinf(angle)*radius*.85f};
  Vector2 b={cx+cosf(angle)*radius*1.24f,cy+sinf(angle)*radius*1.24f};
  DrawLineEx(a,b,1.8f,foreground);
 }
}
static void drawThemeSwitch(float x,float y){
 const float w=92.0f,h=38.0f,r=15.0f;
 Rectangle rect=RR(x,y,w,h);
 int hovered=CheckCollisionPointRec(pointer(),rect);
 Color track=lightTheme?(Color){213,224,244,255}:(Color){40,52,78,255};
 Color outline=lightTheme?(Color){168,181,210,255}:(Color){76,91,124,255};
 Color knob=lightTheme?(Color){255,255,255,255}:(Color){140,111,255,255};
 /* Knob moves smoothly between dark (left) and light (right). */
 static float knobX=-1.0f;
 float target=lightTheme?x+w-20.0f:x+20.0f;
 if(knobX<0 || knobX<x-2 || knobX>x+w+2)knobX=target;
 float blend=fminf(1.0f,GetFrameTime()*16.0f);
 knobX+=(target-knobX)*blend;
 box(x,y,w,h,.95f,hovered?ColorBrightness(track,.07f):track);
 border(x,y,w,h,.95f,outline);
 /* Unselected symbol remains visible; the active symbol is inside the knob. */
 if(lightTheme)themeMoon(x+20,y+h*.5f,6.7f,CQ_MUTED,track);
 else themeSun(x+w-20,y+h*.5f,6.5f,CQ_MUTED);
 DrawCircle((int)knobX,(int)(y+h*.5f),r,knob);
 if(lightTheme)themeSun(knobX,y+h*.5f,7.0f,(Color){188,112,28,255});
 else themeMoon(knobX,y+h*.5f,7.5f,(Color){248,246,255,255},knob);
 if(hovered)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
 if(hit(rect))toggleTheme();
}
/* Stage 2.1: consistent 24px line icons, hand-drawn with raylib.
 * Matching stroke widths, round end caps, and an optical 24x24 canvas
 * keep each destination legible in the full sidebar and collapsed rail. */
static void iconLine(float x1,float y1,float x2,float y2,float width,Color ink){
 DrawLineEx((Vector2){x1,y1},(Vector2){x2,y2},width,ink);
 DrawCircleV((Vector2){x1,y1},width*.5f,ink);
 DrawCircleV((Vector2){x2,y2},width*.5f,ink);
}
static void navSymbol(int kind,float cx,float cy,Color ink){
 const float s=2.15f;
 if(kind==0){ /* Home: roof, complete walls, clearly open door. */
  iconLine(cx-10,cy-1,cx,cy-10,s,ink);
  iconLine(cx,cy-10,cx+10,cy-1,s,ink);
  iconLine(cx-7,cy-3,cx-7,cy+9,s,ink);
  iconLine(cx+7,cy-3,cx+7,cy+9,s,ink);
  iconLine(cx-7,cy+9,cx-2.5f,cy+9,s,ink);
  iconLine(cx+2.5f,cy+9,cx+7,cy+9,s,ink);
  iconLine(cx-2.5f,cy+9,cx-2.5f,cy+2,s,ink);
  iconLine(cx+2.5f,cy+2,cx+2.5f,cy+9,s,ink);
  iconLine(cx-2.5f,cy+2,cx+2.5f,cy+2,s,ink);
 }else if(kind==1){ /* Learn: open book with curved lower edges and center fold. */
  iconLine(cx,cy-6,cx,cy+9,s,ink);
  iconLine(cx-1.4f,cy-6,cx-5.7f,cy-8.2f,s,ink);
  iconLine(cx-5.7f,cy-8.2f,cx-10,cy-8.2f,s,ink);
  iconLine(cx-10,cy-8.2f,cx-10,cy+6.5f,s,ink);
  iconLine(cx-10,cy+6.5f,cx-5.7f,cy+6.5f,s,ink);
  iconLine(cx-5.7f,cy+6.5f,cx,cy+9,s,ink);
  iconLine(cx+1.4f,cy-6,cx+5.7f,cy-8.2f,s,ink);
  iconLine(cx+5.7f,cy-8.2f,cx+10,cy-8.2f,s,ink);
  iconLine(cx+10,cy-8.2f,cx+10,cy+6.5f,s,ink);
  iconLine(cx+10,cy+6.5f,cx+5.7f,cy+6.5f,s,ink);
  iconLine(cx+5.7f,cy+6.5f,cx,cy+9,s,ink);
 }else if(kind==2){ /* Quizzes: clipboard question/check, distinct from Progress. */
  DrawRectangleRoundedLinesEx(RR(cx-8.5f,cy-8,17,18),.18f,8,s,ink);
  DrawRectangleRounded(RR(cx-4.5f,cy-11,9,5.4f),.4f,6,PANEL);
  DrawRectangleRoundedLinesEx(RR(cx-4.5f,cy-11,9,5.4f),.4f,6,s,ink);
  iconLine(cx-4.5f,cy+1.5f,cx-1,cy+5,s,ink);
  iconLine(cx-1,cy+5,cx+5,cy-2.5f,s,ink);
 }else if(kind==3){ /* Coding labs: an IDE terminal window, prompt and cursor. */
  DrawRectangleRoundedLinesEx(RR(cx-11,cy-9,22,18),.18f,8,s,ink);
  iconLine(cx-10,cy-3.7f,cx+10,cy-3.7f,1.6f,ink);
  DrawCircleV((Vector2){cx-7,cy-6.4f},.85f,ink);
  DrawCircleV((Vector2){cx-4,cy-6.4f},.85f,ink);
  iconLine(cx-6,cy-0.2f,cx-2.5f,cy+3,s,ink);
  iconLine(cx-2.5f,cy+3,cx-6,cy+6,s,ink);
  iconLine(cx+0.5f,cy+6,cx+6.5f,cy+6,s,ink);
 }else{ /* My progress: rising bar chart, not an ambiguous avatar. */
  iconLine(cx-10,cy+9,cx+10,cy+9,s,ink);
  DrawRectangleRounded(RR(cx-8,cy+2,3.5f,6),.2f,4,ink);
  DrawRectangleRounded(RR(cx-2,cy-3,3.5f,11),.2f,4,ink);
  DrawRectangleRounded(RR(cx+4,cy-8,3.5f,16),.2f,4,ink);
  iconLine(cx-9,cy-4,cx-2.5f,cy-9,s,ink);
  iconLine(cx-2.5f,cy-9,cx+1,cy-6,s,ink);
  iconLine(cx+1,cy-6,cx+7,cy-11,s,ink);
 }
}
/* Tooltips are queued while the sidebar is drawn, then rendered in a final
 * overlay pass. Drawing them inside sidebar() places them UNDER the content
 * camera/scissor pass, making labels disappear behind Home and other pages. */
static const char *pendingNavTooltip=NULL;
static float pendingNavTooltipX=0, pendingNavTooltipY=0;
static void navTooltip(const char *label,float x,float y){
 pendingNavTooltip=label;
 pendingNavTooltipX=x;
 pendingNavTooltipY=y;
}
static void drawNavTooltipOverlay(void){
 if(!pendingNavTooltip)return;
 const char *label=pendingNavTooltip;
 float width=tw(label,13)+28;
 float x=fminf(pendingNavTooltipX,(float)windowW-width-8);
 float y=fminf(pendingNavTooltipY,(float)windowH-40);
 /* A slight shadow makes the floating label legible over bright content. */
 box(x+2,y+3,width,32,.19f,alpha(BG,.65f));
 box(x,y,width,32,.19f,CARD);
 border(x,y,width,32,.19f,CQ_LINE);
 tx(label,x+14,y+8,13,CQ_WHITE);
}
static void sidebar(float side){
 const int collapsed=navCollapsed;
 const float mid=side*.5f;
 DrawRectangle(0,0,(int)side,windowH,PANEL);
 DrawLine((int)side,0,(int)side,windowH,CQ_LINE);
 /* Branding is intentionally quieter than the content itself. */
 box(collapsed?17:19,22,40,40,.24f,CQ_PURPLE);
 tx("{ }",collapsed?23:25,32,18,textOnAccent);
 if(!collapsed){
  txH("codequest",70,25,20,CQ_WHITE);
  tx("C / C++ STUDIO",71,51,10,CQ_MUTED);
 }
 Rectangle toggle=RR(collapsed?19:side-37,84,collapsed?36:27,34);
 int toggleHover=CheckCollisionPointRec(pointer(),toggle);
 if(toggleHover){box(toggle.x,toggle.y,toggle.width,toggle.height,.22f,CARD);SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);}
 /* Collapse/expand control uses a directional chevron rather than a menu icon. */
 float chevronX=toggle.x+toggle.width*.5f;
 float chevronY=toggle.y+toggle.height*.5f;
 float direction=collapsed?1.0f:-1.0f;
 iconLine(chevronX-direction*3.5f,chevronY-5,chevronX+direction*2.0f,chevronY,2.1f,CQ_MUTED);
 iconLine(chevronX+direction*2.0f,chevronY,chevronX-direction*3.5f,chevronY+5,2.1f,CQ_MUTED);
 if(hit(toggle))navCollapsed=!navCollapsed;
 if(!collapsed)tx("EXPLORE",22,112,10,CQ_MUTED);
 const char *labels[]={"Home","Learn","Quizzes","Coding labs","My progress"};
 const float navTop=collapsed?140.0f:144.0f;
 for(int i=0;i<5;i++){
  float y=navTop+i*57;
  Rectangle r=RR(collapsed?12:12,y,side-24,47);
  int active=screen==i;
  int hovered=CheckCollisionPointRec(pointer(),r);
  navHovers[i]=hoverEase(navHovers[i],hovered,14.0f);
  float nh=navHovers[i];
  if(active||nh>.005f){
   Color activeBg=alpha(CQ_PURPLE,lightTheme?.13f:.19f);
   box(r.x,r.y,r.width,r.height,.22f,active?activeBg:alpha(CQ_PURPLE,nh*(lightTheme?.08f:.11f)));
  }
  if(active)box(r.x,y+9,3,29,1,CQ_PURPLE);
  Color ink=active?CQ_PURPLE:mixColor(CQ_MUTED,CQ_WHITE,nh);
  navSymbol(i,collapsed?mid:37,y+23,ink);
  if(!collapsed)tx(labels[i],60,y+14,15,active?CQ_WHITE:ink);
  if(hovered){SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);if(collapsed)navTooltip(labels[i],side+8,y+7);}
  if(hit(r))nav(i);
 }
 /* The course list now lives on the Home learning journey and in Learn.
  * Keeping the sidebar to five destinations gives the content more room. */
 float footerY=(float)windowH-147;
 if(collapsed){
  DrawLine(17,(int)footerY-19,(int)side-17,(int)footerY-19,CQ_LINE);
  Rectangle themeArea=RR(15,footerY-4,side-30,39);
  int hovered=CheckCollisionPointRec(pointer(),themeArea);
  if(hovered){box(themeArea.x,themeArea.y,themeArea.width,themeArea.height,.2f,CARD);SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);navTooltip("Change theme",side+8,footerY);}
  if(lightTheme)themeSun(mid,footerY+15,9,CQ_ORANGE);
  else themeMoon(mid,footerY+15,9,CQ_MUTED,PANEL);
  if(hit(themeArea))toggleTheme();
  Rectangle profile=RR(11,footerY+48,side-22,49);
  hovered=CheckCollisionPointRec(pointer(),profile);
  box(profile.x,profile.y,profile.width,profile.height,.25f,hovered?ColorBrightness(CARD,.1f):CARD);
  DrawCircle((int)mid,(int)(footerY+72),15,alpha(CQ_PURPLE,.27f));
  char lv[12];snprintf(lv,sizeof(lv),"%d",cqLevel());
  tx(lv,mid-tw(lv,15)/2,footerY+62,15,CQ_WHITE);
  if(hovered){SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);navTooltip("Level / Profile",side+8,footerY+58);}
  if(hit(profile))nav(4);
 }else{
  DrawLine(21,(int)footerY-18,(int)side-21,(int)footerY-18,CQ_LINE);
  tx("APPEARANCE",22,footerY-1,10,CQ_MUTED);
  drawThemeSwitch(22,footerY+19);
  tx(lightTheme?"Light":"Dark",125,footerY+30,12,CQ_MUTED);
  Rectangle profile=RR(12,footerY+67,side-24,64);
  int hovered=CheckCollisionPointRec(pointer(),profile);
  box(profile.x,profile.y,profile.width,profile.height,.18f,hovered?ColorBrightness(CARD,.08f):CARD);
  DrawCircle((int)(profile.x+28),(int)(profile.y+26),17,alpha(CQ_PURPLE,.24f));
  char lv[20];snprintf(lv,sizeof(lv),"%d",cqLevel());
  tx(lv,profile.x+28-tw(lv,16)/2,profile.y+17,16,CQ_PURPLE);
  tx(cqRank(cqLevel()),profile.x+56,profile.y+11,13,CQ_WHITE);
  char xp[44];snprintf(xp,sizeof(xp),"%d XP  /  LEVEL %d",progress.xp,cqLevel());
  tx(xp,profile.x+56,profile.y+34,10,CQ_MUTED);
  if(hovered)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
  if(hit(profile))nav(4);
 }
}
static void header(float x,float width,const char*section){
 tx(section,x,37,14,CQ_MUTED);
 char s[60];snprintf(s,sizeof(s),"%d XP",progress.xp);box(x+width-158,25,144,43,.28f,CARD);
 DrawCircle(x+width-131,46,10,CQ_ORANGE);tx("*",x+width-136,36,20,textOnAccent);tx(s,x+width-111,36,17,CQ_WHITE);
}
/* Home screen: a learning workspace, not a statistics dashboard.
 * All featured actions derive their destinations from the existing saved progress. */
static int homeNextLesson(void){
 /* Prefer the next chapter in the learner's most advanced started track. */
 int active=-1;
 for(int i=0;i<COUNT(lessons);i++)if(progress.learned[i])active=lessons[i].track;
 if(active>=0)for(int i=0;i<COUNT(lessons);i++)
  if(lessons[i].track==active&&!progress.learned[i])return i;
 for(int i=0;i<COUNT(lessons);i++)if(!progress.learned[i])return i;
 return -1;
}
static void homeOpenLesson(int id){
 if(id<0)return;
 selectedTrack=lessons[id].track;
 nav(1);
 selectedLesson=id;
 scroll=0;
}
static void homeOpenTrack(int id){selectedTrack=id;nav(1);}
static void homeSymbol(float cx,float cy,float s,Color c){
 /* Four connected endpoints form a subtle coding / learning motif. */
 DrawLineEx((Vector2){cx-s,cy-s*.6f},(Vector2){cx+s,cy+s*.6f},2.2f,alpha(c,.65f));
 DrawLineEx((Vector2){cx-s,cy+s*.6f},(Vector2){cx+s,cy-s*.6f},2.2f,alpha(c,.65f));
 DrawCircleV((Vector2){cx-s,cy-s*.6f},4,c);
 DrawCircleV((Vector2){cx+s,cy+s*.6f},4,c);
 DrawCircleV((Vector2){cx-s,cy+s*.6f},4,c);
 DrawCircleV((Vector2){cx+s,cy-s*.6f},4,c);
 DrawCircleV((Vector2){cx,cy},6,c);
}
/* Journey interaction is local to the Home screen; it never resets curriculum
 * progress or changes the user's selected lesson until an action is clicked. */
static int journeySelected=-1;
static float journeyReveal=0.0f;
static float journeyPulse=0.0f;
static int journeyTrackNext(int track){
 for(int i=0;i<COUNT(lessons);i++)
  if(lessons[i].track==track&&!progress.learned[i])return i;
 for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==track)return i;
 return -1;
}
/* Tiny purpose-drawn glyphs keep this map consistent with the vector sidebar
 * and avoid introducing external icon assets or font glyph dependencies. */
/* One visual system for the five stages: matching colour, consistent strokes
 * and optical centring within the 32px milestone. C++ uses a centred wordmark. */
static void journeyGlyph(int track,float cx,float cy,float r,Color color){
 float k=r/18.0f,st=2.6f*k;
 #define GL(x1,y1,x2,y2) DrawLineEx((Vector2){cx+(x1)*k,cy+(y1)*k},(Vector2){cx+(x2)*k,cy+(y2)*k},st,color)
 switch(track){
  case 0: /* C basics: balanced code brackets and slash */
   GL(-7,-9,-14,0);GL(-14,0,-7,9);
   GL(7,-9,14,0);GL(14,0,7,9);
   GL(3,-10,-3,10);
   break;
  case 1: /* Pointers: crisp cursor arrow with a small address endpoint. */
   /* Offset the pointer shape so its visible bounds sit in the milestone centre. */
   cx+=1.0f*k; cy-=1.5f*k;
   GL(-10,-12,-10,10);
   GL(-10,-12,8,1);
   GL(8,1,-1,3);
   GL(-1,3,4,12);
   GL(4,12,-1,15);
   GL(-1,15,-6,5);
   GL(-6,5,-10,10);
   break;
  case 2: /* Linked lists: three equal-sized nodes and two connectors */
   GL(-8,0,-4,0);GL(4,0,8,0);
   for(int j=-1;j<=1;j++)
    DrawRectangleRoundedLinesEx(RR(cx+(j*11-4)*k,cy-5*k,8*k,10*k),.25f,5,st,color);
   break;
  case 3: /* C++: true centred wordmark, clearer than tiny hand-drawn letters. */
   {
    const char *mark="C++";
    const float sz=19.5f*k;
    Vector2 bounds=MeasureTextEx(headingFont,mark,sz,0.0f);
    DrawTextEx(headingFont,mark,(Vector2){cx-bounds.x*.5f,cy-bounds.y*.5f-1.0f*k},sz,0.0f,color);
   }
   break;
  default: /* STL: evenly centred stacked containers */
   for(int j=-1;j<=1;j++)
    DrawRectangleRoundedLinesEx(RR(cx-12*k,cy+(j*10-4)*k,24*k,8*k),.35f,6,st,color);
   break;
 }
 #undef GL
}
static float drawLearningJourney(float x,float y,float w,int next){
 const int count=COUNT(tracks);
 int current=next>=0?lessons[next].track:count-1;
 if(journeySelected<0||journeySelected>=count)journeySelected=current;
 journeyPulse+=GetFrameTime();
 if(journeyPulse>1000.0f)journeyPulse=0;
 float mapH=w>=710?264.0f:count*76.0f+12.0f;
 float mapTop=y;
 Color mapBg=lightTheme?(Color){234,240,250,255}:(Color){17,24,39,255};
 box(x,y,w,mapH,.055f,mapBg);
 border(x,y,w,mapH,.055f,alpha(CQ_LINE,.64f));
 /* Quiet coordinate-grid motif; the learning path is the visual anchor. */
 for(float xx=x+27;xx<x+w-8;xx+=42)
  DrawLineEx((Vector2){xx,y+10},(Vector2){xx,y+mapH-10},1,alpha(CQ_LINE,lightTheme?.11f:.19f));
 for(float yy=y+23;yy<y+mapH-10;yy+=42)
  DrawLineEx((Vector2){x+12,yy},(Vector2){x+w-12,yy},1,alpha(CQ_LINE,lightTheme?.11f:.19f));
 tx("YOUR LEARNING PATH",x+22,y+14,11,CQ_MUTED);
 if(w>=610)tx("SELECT A TRACK",x+w-135,y+14,11,CQ_MUTED);
 float posX[16],posY[16];
 if(w>=710){
  /* Alternating peaks make the path feel like a journey rather than a list. */
  const float fractions[]={.115f,.305f,.500f,.695f,.885f};
  const float heights[]={.52f,.25f,.60f,.28f,.52f};
  for(int t=0;t<count&&t<16;t++){
   posX[t]=x+w*fractions[t];posY[t]=y+51+(mapH-120)*heights[t];
  }
 }else{
  for(int t=0;t<count&&t<16;t++){
   posX[t]=x+56+(t%2)*16;posY[t]=y+57+t*76;
  }
 }
 /* Paths are drawn behind the interactive milestones. */
 for(int t=1;t<count&&t<16;t++){
  Vector2 a={posX[t-1],posY[t-1]},b={posX[t],posY[t]};
  DrawLineEx(a,b,6,alpha(CQ_LINE,.66f));
  DrawLineEx(a,b,2.2f,alpha(CQ_MUTED,.30f));
  if(trackComplete(t-1))DrawLineEx(a,b,3,alpha(CQ_GREEN,.70f));
 }
 for(int t=0;t<count&&t<16;t++){
  int done=trackDone(t),total=trackLessonCount(t);
  int mastered=trackComplete(t);
  int active=t==current,selected=t==journeySelected;
  Color accent=tracks[t].color;
  float cx=posX[t],cy=posY[t];
  /* Both icon and caption are one generous click target. */
  Rectangle target=w>=710?RR(cx-69,cy-35,138,102):RR(x+20,cy-34,w-40,70);
  int hover=CheckCollisionPointRec(pointer(),clickRect(target));
  if(hover)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
  journeyHovers[t]=hoverEase(journeyHovers[t],hover,10.0f);
  float jh=journeyHovers[t];
  if(selected||jh>.005f){
   DrawCircleV((Vector2){cx,cy},40,alpha(accent,selected?.12f:.07f*jh));
   DrawCircleLines((int)cx,(int)cy,37,alpha(accent,selected?.48f:.27f*jh));
  }
  if(active&&!mastered){
   float pulse=.5f+.5f*sinf(journeyPulse*2.7f);
   DrawCircleLines((int)cx,(int)cy,(float)(38+5*pulse),alpha(CQ_PURPLE,.12f+.15f*pulse));
  }
  DrawCircleV((Vector2){cx,cy},29,mapBg);
  DrawCircleV((Vector2){cx,cy},25,selected?accent:(mastered?CQ_GREEN:CARD));
  if(!selected&&!mastered)DrawCircleLines((int)cx,(int)cy,25,alpha(accent,.63f));
  journeyGlyph(t,cx,cy,16,selected||mastered?textOnAccent:accent);
  if(mastered){
   DrawCircleV((Vector2){cx+22,cy-22},9,CQ_GREEN);
   tx("+",cx+18,cy-30,16,textOnAccent);
  }
  if(w>=710){
   char label[54];
   snprintf(label,sizeof(label),"%s",t==0?"Foundations":t==1?"Pointers":t==2?"Linked lists":t==3?"C++":"STL");
   tx(label,cx-tw(label,15)/2,cy+37,15,selected?CQ_WHITE:CQ_MUTED);
   char status[50];snprintf(status,sizeof(status),"%d / %d lessons",done,total);
   tx(status,cx-tw(status,11)/2,cy+59,11,mastered?CQ_GREEN:CQ_MUTED);
  }else{
   txH(tracks[t].name,x+109,cy-22,17,selected?CQ_WHITE:CQ_MUTED);
   char status[60];snprintf(status,sizeof(status),"%d of %d chapters  %s",done,total,mastered?"- mastered":active?"- current":"");
   tx(status,x+109,cy+7,12,mastered?CQ_GREEN:CQ_MUTED);
   if(selected)DrawRectangleRounded(RR(x+w-23,cy-12,4,25),1,6,accent);
  }
  if(hit(target)){
   journeySelected=t;
   journeyReveal=0;
  }
 }
 y+=mapH+12;
 /* An expandable detail surface stays within the page's normal scroll area. */
 int t=journeySelected;
 int total=trackLessonCount(t),done=trackDone(t);
 float part=total?(float)done/total:0;
 Color accent=tracks[t].color;
 int mastered=trackComplete(t);
 int chosen=journeyTrackNext(t);
 float detailH=169;
 int chipsPerRow=w>=760?4:(w>=550?3:2);
 int chapterCount=0;
 for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t)chapterCount++;
 int rows=(chapterCount+chipsPerRow-1)/chipsPerRow;
 detailH+=rows*42+14;
 journeyReveal=fminf(1.0f,journeyReveal+GetFrameTime()*5.0f);
 box(x,y,w,detailH,.065f,PANEL);
 border(x,y,w,detailH,.065f,alpha(accent,.18f+.22f*journeyReveal));
 box(x+17,y+16,4,38,.9f,accent);
 tx("TRACK DETAILS",x+32,y+10,11,accent);
 txH(tracks[t].name,x+32,y+28,23,CQ_WHITE);
 if(w>=610){
  char counter[70];snprintf(counter,sizeof(counter),"%d / %d chapters complete",done,total);
  tx(counter,x+w-214,y+19,13,CQ_MUTED);
  bar(x+w-214,y+44,184,part,accent);
 }
 tx(tracks[t].subtitle,x+32,y+69,13,CQ_MUTED);
 if(w<610){
  char counter[70];snprintf(counter,sizeof(counter),"%d / %d chapters complete",done,total);
  tx(counter,x+32,y+88,12,CQ_MUTED);
 }
 float buttonsY=y+103;
 if(btn(mastered?"Review track  >":"Explore track  >",x+23,buttonsY,165,39,accent,1))homeOpenTrack(t);
 if(chosen>=0&&btn(mastered?"Review chapter  >":"Next chapter  >",x+201,buttonsY,165,39,CQ_PURPLE,0))homeOpenLesson(chosen);
 tx("CHAPTERS",x+23,y+149,11,CQ_MUTED);
 float chipY=y+170;
 int slot=0;
 float usable=w-46;
 float chipW=(usable-(chipsPerRow-1)*10)/chipsPerRow;
 for(int i=0;i<COUNT(lessons);i++){
  if(lessons[i].track!=t)continue;
  int col=slot%chipsPerRow,row=slot/chipsPerRow;
  float xx=x+23+col*(chipW+10),yy=chipY+row*42;
  Rectangle chip=RR(xx,yy,chipW,37);
  int hov=CheckCollisionPointRec(pointer(),clickRect(chip));
  if(hov)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
  box(xx,yy,chipW,37,.18f,hov?CARD:alpha(CARD,.70f));
  border(xx,yy,chipW,37,.18f,progress.learned[i]?alpha(CQ_GREEN,.66f):alpha(CQ_LINE,.53f));
  /* Show useful chapter names instead of generic numbered buttons. */
  DrawCircleV((Vector2){xx+14,yy+18},4,progress.learned[i]?CQ_GREEN:accent);
  char text[110];snprintf(text,sizeof(text),"%s",lessons[i].title);
  int length=(int)strlen(text);
  while(length>1&&tw(text,12)>chipW-35){text[--length]=0;}
  if(length<(int)strlen(lessons[i].title)&&length>3){
   text[length-3]='.';text[length-2]='.';text[length-1]='.';
  }
  tx(text,xx+25,yy+11,12,progress.learned[i]?CQ_GREEN:CQ_WHITE);
  if(hit(chip))homeOpenLesson(i);
  slot++;
 }
 y+=detailH+20;
 return y;
}

/* Daily challenges: deterministic local-day rotation with three activity formats.
 * Multiple choice uses the app's own answer checking, no compiler required. */
typedef struct {
 const char *kind,*title,*prompt;
 const char *choices[4];
 int answer;
 const char *explanation;
} DailyChallenge;
static const DailyChallenge dailyChallenges[]={
 {"QUICK QUIZ","C fundamentals","Which keyword declares a constant-qualified variable in C?",{"const","static","extern","volatile"},0,"const prevents modification through that declaration."},
 {"PREDICT OUTPUT","Integer arithmetic","What does this print?  printf(\"%d\", 7 / 2);",{"3","3.5","4","2"},0,"Both operands are integers, so division discards the fractional part."},
 {"FIND THE BUG","Array bounds","An int array has five elements. Which access is out of bounds?",{"arr[4]","arr[0]","arr[2]","arr[5]"},3,"Five elements have indices 0 through 4."},
 {"QUICK QUIZ","Pointers","What does the & operator obtain when applied to a variable?",{"Its address","Its size","Its value plus one","Its type"},0,"The address-of operator yields a pointer to that variable."},
 {"PREDICT OUTPUT","Loops","What prints?  for(int i=0;i<3;i++) printf(\"%d\",i);",{"012","123","013","000"},0,"The loop uses i = 0, 1, and 2 before stopping."},
 {"FIND THE BUG","Strings","Which declaration is too small for the C string \"cat\"?",{"char s[3] = \"cat\";","char s[4] = \"cat\";","char s[8] = \"cat\";","char s[] = \"cat\";"},0,"A null terminator requires four bytes in total."},
 {"QUICK QUIZ","Dynamic memory","Which standard C function releases memory from malloc?",{"free","delete","release","clear"},0,"free releases dynamically allocated C memory."},
 {"PREDICT OUTPUT","Increment","What prints?  int a=5; a++; printf(\"%d\",a);",{"6","5","4","10"},0,"The increment expression adds one to a."},
 {"FIND THE BUG","Pointers","Which action is invalid after free(p), without reallocating?",{"Dereference p","Set p to NULL","Assign a new pointer to p","Leave p unused"},0,"Dereferencing a freed pointer is undefined behaviour."},
 {"QUICK QUIZ","Linked lists","What usually connects one node to the next in a linked list?",{"A pointer","An array index","A file handle","A loop counter"},0,"A next pointer links a node to the next one."},
 {"PREDICT OUTPUT","Conditions","What prints?  int x=4; printf(\"%d\", x>5);",{"0","1","4","5"},0,"In C, a false comparison evaluates to integer 0."},
 {"FIND THE BUG","Loops","Which condition stays true as i increases through nonnegative values?",{"i >= 0","i < 5","i != 3","i < 10"},0,"Increasing i from zero never makes i >= 0 false in the normal range."},
 {"QUICK QUIZ","Structures","Which operator accesses a struct member through a pointer?",{"->",".","::","&"},0,"The arrow operator accesses a member through a pointer to a struct."},
 {"PREDICT OUTPUT","Modulo","What prints?  printf(\"%d\", 11 % 4);",{"3","2","4","1"},0,"11 divided by 4 leaves a remainder of 3."},
 {"FIND THE BUG","Input safety","Which scanf format safely limits a char name[10] string read?",{"%9s","%10s","%s","%11s"},0,"%9s leaves one character for the string's terminating null byte."}
};
/* Archive is deliberately practice-only: CQ5 saves have no per-day completion
 * history, so don't pretend that previously played dates have saved results. */
static int dailyChoice=-1,dailyFeedback=0,dailyObservedDay=-1;
static int dailyEarned=0,dailySelectedOffset=0,dailyArchivePage=0;
static void dailySelectOffset(int offset){
 if(offset<0)offset=0;
 if(offset>90)offset=90;
 if(dailySelectedOffset!=offset){dailySelectedOffset=offset;dailyChoice=-1;dailyFeedback=0;dailyEarned=0;scroll=0;}
}
static void dailySync(void){
 int day=cqLocalDay();
 if(day!=dailyObservedDay){dailyObservedDay=day;dailySelectedOffset=0;dailyArchivePage=0;dailyChoice=-1;dailyFeedback=0;dailyEarned=0;}
}
static int dailyProspectiveXP(int day,int complete){
 int streak=cqDailyActiveStreak(day);
 int next=complete?streak:streak+1;
 int bonus=(next-1)*CQ_XP_DAILY_BONUS_STEP;
 if(bonus>CQ_XP_DAILY_MAX_BONUS)bonus=CQ_XP_DAILY_MAX_BONUS;
 return CQ_XP_DAILY_BASE+bonus;
}
static float dailyChallengeCard(float x,float y,float w){
 dailySync();
 int day=cqLocalDay(),streak=cqDailyActiveStreak(day);
 int completed=day>0 && progress.dailyLastDay==day;
 int attempted=day>0 && progress.dailyAttemptDay==day;
 float h=w>=620?148:190;
 box(x,y,w,h,.08f,PANEL);
 border(x,y,w,h,.08f,alpha(CQ_PURPLE,.36f));
 box(x+19,y+22,4,34,1,CQ_PURPLE);
 tx("DAILY CHALLENGE",x+33,y+24,12,CQ_PURPLE);
 char status[120];
 snprintf(status,sizeof(status),"%d-day streak",streak);
 txH(status,x+24,y+55,22,CQ_WHITE);
 if(attempted && !completed)snprintf(status,sizeof(status),"Today's attempt used  /  No XP earned");
 else snprintf(status,sizeof(status),completed?"Today's challenge complete  /  +%d XP earned":"Today's reward  /  +%d XP possible",dailyProspectiveXP(day,completed));
 tx(status,x+25,y+92,13,completed?CQ_GREEN:(attempted?CQ_RED:CQ_MUTED));
 float bw=w>=620?195:210;
 float bx=w>=620?x+w-bw-20:x+23;
 float by=w>=620?y+77:y+129;
 if(btn(completed?"View challenges  >":"Play today's challenge  >",bx,by,bw,40,CQ_PURPLE,1)){
  dailySelectOffset(0);nav(5);
 }
 return y+h+23;
}
static void dailyChallengeView(float x,float w,float *bottom){
 dailySync();
 int today=cqLocalDay();
 int selectedDay=today-dailySelectedOffset;
 int historical=dailySelectedOffset>0;
 int completed=!historical && today>0 && progress.dailyLastDay==today;
 int attempted=!historical && today>0 && progress.dailyAttemptDay==today;
 int locked=attempted || completed || dailyFeedback!=0;
 int streak=cqDailyActiveStreak(today);
 const DailyChallenge *q=&dailyChallenges[(selectedDay>0?selectedDay:1)%COUNT(dailyChallenges)];
 char text[160];
 float y=100;
 tx("DAILY PRACTICE",x,y,12,CQ_PURPLE);
 txH("Daily challenges",x,y+26,30,CQ_WHITE);
 tx("Build a habit with a short challenge every day.",x,y+73,14,CQ_MUTED);
 y+=112;
 float statH=90;
 box(x,y,w,statH,.08f,PANEL);
 border(x,y,w,statH,.08f,alpha(CQ_LINE,.6f));
 snprintf(text,sizeof(text),"%d",streak);
 txH(text,x+23,y+17,31,CQ_ORANGE);
 tx("DAY STREAK",x+24,y+59,11,CQ_MUTED);
 if(attempted && !completed)snprintf(text,sizeof(text),"TODAY'S ATTEMPT USED  /  0 XP");
 else snprintf(text,sizeof(text),completed?"TODAY COMPLETED  /  %d XP EARNED":"TODAY'S REWARD  /  %d XP",dailyProspectiveXP(today,completed));
 tx(text,x+(w>=650?210:143),y+36,13,completed?CQ_GREEN:(attempted?CQ_RED:CQ_PURPLE));
 y+=statH+22;
 if(btn(historical?"Back to today  >":"Today's challenge",x,y,185,38,CQ_PURPLE,!historical))dailySelectOffset(0);
 y+=52;
 float contentW=w>=865?w*.64f:w;
 float challengeH=contentW>=570?398:460;
 box(x,y,contentW,challengeH,.075f,PANEL);
 border(x,y,contentW,challengeH,.075f,alpha(CQ_LINE,.75f));
 tx(historical?"ARCHIVE / PRACTICE ONLY":"TODAY'S CHALLENGE",x+23,y+20,12,historical?CQ_MUTED:CQ_PURPLE);
 if(historical){
  snprintf(text,sizeof(text),"%d day%s ago",dailySelectedOffset,dailySelectedOffset==1?"":"s");
  tx(text,x+23,y+46,12,CQ_MUTED);
 }else{
  tx(completed?"Completed — come back tomorrow":attempted?"Attempt finished — come back tomorrow":"One attempt today — choose carefully",x+23,y+46,12,completed?CQ_GREEN:(attempted?CQ_RED:CQ_MUTED));
 }
 txH(q->title,x+23,y+78,24,CQ_WHITE);
 pill(q->kind,x+23,y+114,historical?CQ_BLUE:CQ_PURPLE);
 paragraph(q->prompt,x+23,y+156,15,CQ_WHITE,contentW-46);
 int columns=contentW>=570?2:1;
 float optionsY=y+208;
 float ow=(contentW-46-(columns-1)*12)/columns;
 int rotation=selectedDay>0?(selectedDay/COUNT(dailyChallenges))%4:0;
 for(int i=0;i<4;i++){
  int choice=(i+rotation)%4;
  float ox=x+23+(i%columns)*(ow+12);
  float oy=optionsY+(i/columns)*(columns==2?50:42);
  snprintf(text,sizeof(text),"%c   %s",'A'+i,q->choices[choice]);
  int correctShown=locked && choice==q->answer;
  int chosenShown=(attempted && progress.dailyAttemptChoice>=0 && progress.dailyAttemptChoice==choice) || dailyChoice==choice;
  /* Show the original answer, but never allow changing it after submission. */
  if(btn(text,ox,oy,ow,36,correctShown?CQ_GREEN:(locked && chosenShown && !correctShown?CQ_RED:CQ_PURPLE),chosenShown || correctShown)){
   if(!locked)dailyChoice=choice;
  }
 }
 float actionY=y+(columns==2?344:403);
 if(completed){
  tx("COMPLETED TODAY",x+23,actionY+7,13,CQ_GREEN);
  paragraph(q->explanation,x+23,actionY+31,12,CQ_MUTED,contentW-46);
 }else if(attempted){
  tx("INCORRECT — NO ATTEMPTS REMAIN",x+23,actionY+7,13,CQ_RED);
  paragraph(q->explanation,x+23,actionY+31,12,CQ_MUTED,contentW-46);
 }else{
  if(!locked && btn(historical?"Check practice":"Submit answer",x+23,actionY,154,37,CQ_GREEN,dailyChoice>=0)){
   int correct=dailyChoice==q->answer;
   if(historical || cqDailySubmit(today,dailyChoice,correct)){
    dailyFeedback=correct?1:-1;
    if(correct){
     celebrationCorrect(x+23+contentW*.5f,actionY-scroll-50.0f);
     if(!historical)dailyEarned=dailyProspectiveXP(today,1);
    }
   }
  }
  if(dailyFeedback==1){
   tx(historical?"Correct! Practice complete, no XP awarded.":"Correct! Daily reward earned.",x+190,actionY+9,12,CQ_GREEN);
  }else if(dailyFeedback==-1){
   tx(historical?"Incorrect — select another date to practise.":"Incorrect — no second attempt today.",x+190,actionY+9,12,CQ_RED);
  }
  if(dailyFeedback!=0)paragraph(q->explanation,x+23,actionY+42,12,CQ_MUTED,contentW-46);
 }
 y+=challengeH+23;
 if(w>=865){
  /* Recent dates sit alongside the featured challenge on wide screens. */
  float ax=x+contentW+18,aw=w-contentW-18,ay=y-challengeH-23;
  box(ax,ay,aw,challengeH,.075f,PANEL);
  border(ax,ay,aw,challengeH,.075f,alpha(CQ_LINE,.65f));
  txH("Previous challenges",ax+17,ay+19,18,CQ_WHITE);
  tx("Replay for practice. No XP.",ax+17,ay+48,12,CQ_MUTED);
  for(int i=0;i<7;i++){
   int offset=1+dailyArchivePage*7+i;
   if(offset>90)break;
   const DailyChallenge *a=&dailyChallenges[(today-offset>0?today-offset:1)%COUNT(dailyChallenges)];
   snprintf(text,sizeof(text),"%d day%s ago  /  %s",offset,offset==1?"":"s",a->title);
   if(btn(text,ax+14,ay+81+i*35,aw-28,30,CQ_PURPLE,dailySelectedOffset==offset))dailySelectOffset(offset);
  }
  if(dailyArchivePage>0 && btn("< Newer",ax+16,ay+346,98,34,CQ_PURPLE,0))dailyArchivePage--;
  if(dailyArchivePage<12 && btn("Older >",ax+aw-113,ay+346,98,34,CQ_PURPLE,0))dailyArchivePage++;
 }else{
  box(x,y,w,374,.075f,PANEL);
  txH("Previous challenges",x+17,y+17,20,CQ_WHITE);
  tx("Practice earlier dates without XP rewards.",x+17,y+45,12,CQ_MUTED);
  for(int i=0;i<7;i++){
   int offset=1+dailyArchivePage*7+i;
   if(offset>90)break;
   const DailyChallenge *a=&dailyChallenges[(today-offset>0?today-offset:1)%COUNT(dailyChallenges)];
   snprintf(text,sizeof(text),"%d day%s ago  /  %s",offset,offset==1?"":"s",a->title);
   if(btn(text,x+14,y+75+i*35,w-28,30,CQ_PURPLE,dailySelectedOffset==offset))dailySelectOffset(offset);
  }
  if(dailyArchivePage>0 && btn("< Newer",x+16,y+334,105,32,CQ_PURPLE,0))dailyArchivePage--;
  if(dailyArchivePage<12 && btn("Older >",x+w-120,y+334,105,32,CQ_PURPLE,0))dailyArchivePage++;
  y+=394;
 }
 *bottom=y+35;
}

static void overview(float x,float w,float *bottom){
 const int next=homeNextLesson();
 const int lessonCount=totalLearned(),labCount=totalLabs(),quizCount=totalQuizzes();
 const int level=cqLevel();
 const float space=20.0f;
 float y=104;
 char s[180];
 tx("YOUR WORKSPACE",x,y,12,CQ_PURPLE);
 txH("A little progress, every day.",x,y+24,30,CQ_WHITE);
 tx("Pick up where you left off, or explore something new.",x,y+68,15,CQ_MUTED);
 y+=112;

 /* Feature lesson: dominant surface and a clear primary action. */
 float heroW=w*.635f;
 if(heroW<420)heroW=w;
 float profileX=x+heroW+space;
 float profileW=w-heroW-space;
 const float heroH=272;
 Color featureBg=lightTheme?(Color){233,229,249,255}:(Color){32,32,62,255};
 box(x,y,heroW,heroH,.075f,featureBg);
 border(x,y,heroW,heroH,.075f,alpha(CQ_PURPLE,.30f));
 /* Minimal atmospheric geometry; no external image assets required. */
 DrawCircle((int)(x+heroW-56),(int)(y+43),92,alpha(CQ_PURPLE,lightTheme?.08f:.095f));
 DrawCircleLines((int)(x+heroW-64),(int)(y+74),65,alpha(CQ_BLUE,.21f));
 homeSymbol(x+heroW-77,y+74,22,alpha(CQ_PURPLE,.54f));
 pill(next>=0?"UP NEXT  /  LESSON":"JOURNEY COMPLETE",x+25,y+23,CQ_GREEN);
 if(next>=0){
  snprintf(s,sizeof(s),"%s  /  CHAPTER %02d",tracks[lessons[next].track].name,next+1);
  tx(s,x+26,y+67,12,CQ_MUTED);
  /* Long lesson titles wrap safely in the feature column. */
  paragraph(lessons[next].title,x+25,y+92,25,CQ_WHITE,heroW-65);
  tx("Learn a concept. Try it. Build confidence.",x+26,y+174,14,CQ_MUTED);
  if(btn("Start lesson  >",x+25,y+215,178,42,CQ_GREEN,1))homeOpenLesson(next);
  if(btn("View track",x+216,y+215,131,42,CQ_PURPLE,0))homeOpenTrack(lessons[next].track);
 }else{
  tx("Every lesson, completed.",x+25,y+93,25,CQ_WHITE);
  tx("Put your knowledge to work in a coding lab.",x+25,y+174,14,CQ_MUTED);
  if(btn("Open coding labs  >",x+25,y+215,201,42,CQ_GREEN,1))nav(3);
 }

 /* Companion panel scales out of the way on narrower window sizes. */
 if(profileW>=235){
  box(profileX,y,profileW,heroH,.075f,PANEL);
  border(profileX,y,profileW,heroH,.075f,alpha(CQ_LINE,.75f));
  tx("YOUR DEVELOPER PROFILE",profileX+21,y+22,11,CQ_MUTED);
  DrawCircle((int)(profileX+49),(int)(y+96),27,alpha(CQ_PURPLE,.18f));
  DrawCircleLines((int)(profileX+49),(int)(y+96),24,alpha(CQ_PURPLE,.45f));
  snprintf(s,sizeof(s),"%d",level);tx(s,profileX+49-tw(s,26)/2,y+81,26,CQ_PURPLE);
  snprintf(s,sizeof(s),"Level %d",level);tx(s,profileX+88,y+70,20,CQ_WHITE);
  tx(cqRank(level),profileX+88,y+100,13,CQ_MUTED);
  int start=cqLevelStart(level),needed=cqLevelNext()-start;
  float pct=needed>0?(float)(progress.xp-start)/needed:1;
  snprintf(s,sizeof(s),"%d / %d XP",progress.xp-start,needed);tx(s,profileX+22,y+145,13,CQ_WHITE);
  bar(profileX+22,y+172,profileW-44,pct,CQ_PURPLE);
  snprintf(s,sizeof(s),"%d XP available for extra hints",progress.xpWallet);
  tx(s,profileX+22,y+190,11,CQ_MUTED);
  if(btn("View your progress  >",profileX+19,y+225,profileW-38,32,CQ_PURPLE,0))nav(4);
 }
 y+=heroH+26;
 if(profileW<235){
  /* Narrow viewport: rank remains accessible below the feature. */
  box(x,y,w,66,.095f,PANEL);
  snprintf(s,sizeof(s),"LV %d  /  %s",level,cqRank(level));
  tx(s,x+18,y+12,17,CQ_WHITE);
  float progressPct=(float)(progress.xp-cqLevelStart(level))/fmaxf(1,cqLevelNext()-cqLevelStart(level));
  bar(x+18,y+43,w-170,progressPct,CQ_PURPLE);
  if(btn("Progress  >",x+w-133,y+15,115,36,CQ_PURPLE,0))nav(4);
  y+=79;
 }
 y+=13;
 /* One featured, rotating question per local calendar day. */
 y=dailyChallengeCard(x,y,w);

 /* Compact chapter-like activity strip, not three oversized KPI tiles. */
 txH("At a glance",x,y,21,CQ_WHITE);
 tx("The work you have already put in",x,y+31,13,CQ_MUTED);
 y+=70;
 float metricW=(w-2*space)/3;
 const char *labels[]={"CHAPTERS READ","QUIZZES SOLVED","LABS COMPLETED"};
 int values[]={lessonCount,quizCount,labCount};
 int totals[]={COUNT(lessons),COUNT(quizzes),COUNT(labTasks)};
 Color metricColors[]={CQ_BLUE,CQ_PURPLE,CQ_GREEN};
 for(int i=0;i<3;i++){
  float xx=x+i*(metricW+space);
  box(xx,y,metricW,94,.095f,PANEL);
  box(xx+16,y+16,3,61,1,metricColors[i]);
  tx(labels[i],xx+29,y+17,11,CQ_MUTED);
  snprintf(s,sizeof(s),"%d",values[i]);tx(s,xx+28,y+39,28,CQ_WHITE);
  snprintf(s,sizeof(s),"of %d",totals[i]);tx(s,xx+73,y+53,12,CQ_MUTED);
 }
 y+=118;

 txH("Your learning path",x,y,23,CQ_WHITE);
 tx("Pick a track to explore chapters and continue learning.",x,y+31,14,CQ_MUTED);
 y+=67;
 /* Interactive learning path: select tracks to inspect their chapters.
  * The map uses saved lesson completion; tracks remain freely explorable. */
 y=drawLearningJourney(x,y,w,next);

 /* Secondary practice navigation in one quiet bottom surface. */
 box(x,y,w,99,.095f,PANEL);
 tx("Ready to put it into practice?",x+21,y+18,18,CQ_WHITE);
 if(w>=735){
  tx("Apply concepts in a quiz or build something in the lab.",x+21,y+50,13,CQ_MUTED);
  if(btn("Quizzes  >",x+w-279,y+30,118,41,CQ_PURPLE,0))nav(2);
  if(btn("Coding labs  >",x+w-151,y+30,132,41,CQ_PURPLE,1))nav(3);
  y+=127;
 }else{
  tx("Test your skills in quizzes or coding labs.",x+21,y+49,13,CQ_MUTED);
  if(btn("Quizzes  >",x+15,y+112,140,40,CQ_PURPLE,0))nav(2);
  if(btn("Coding labs  >",x+169,y+112,148,40,CQ_PURPLE,1))nav(3);
  y+=174;
 }
 *bottom=y;
}
static void searchBox(float x,float y,float w){Rectangle r=RR(x,y,w,46);box(x,y,w,46,.15f,PANEL);border(x,y,w,46,.15f,searchActive?CQ_PURPLE:CQ_LINE);tx("/",x+16,y+12,18,CQ_PURPLE);tx(searchText[0]?searchText:"Search lessons or topics...",x+40,y+13,15,searchText[0]?CQ_WHITE:CQ_MUTED);
 if(hit(r))searchActive=1;
 if(searchActive){int key=GetCharPressed();while(key){int n=(int)strlen(searchText);if(key>=32 && key<127 && n<90){searchText[n]=(char)key;searchText[n+1]=0;}key=GetCharPressed();}if(IsKeyPressed(KEY_BACKSPACE)&&strlen(searchText))searchText[strlen(searchText)-1]=0;if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_ESCAPE))searchActive=0;}
}
/* Study pages: compact curriculum index, generous reading rhythm and clear
 * guidance. All chapter text stays in curriculum.c / lesson_details.c / tutorials.c. */
static int lessonIndexInTrack(int id){
 int n=0;for(int i=0;i<=id;i++)if(lessons[i].track==lessons[id].track)n++;
 return n;
}
static void learnList(float x,float w,float *bottom){
 float y=95;
 tx("LEARN  /  THE CURRICULUM",x,y,12,CQ_PURPLE);y+=29;
 txH("Build your C skills",x,y,30,CQ_WHITE);y+=47;
 tx("Choose a chapter, follow its examples, then practise what you learned.",x,y,15,CQ_MUTED);y+=43;
 searchBox(x,y,w);y+=62;
 /* Filter chips wrap instead of disappearing on smaller windows. */
 float chipX=x,chipY=y;
 const char *filters[]={"All chapters","Foundations","Pointers","Linked lists","C++","STL"};
 for(int f=0;f<6;f++){
  float cw=f==0?137.0f:(f==3?132.0f:112.0f);
  if(chipX+cw>x+w){chipX=x;chipY+=45;}
  Color accent=f==0?CQ_PURPLE:tracks[f-1].color;
  if(btn(filters[f],chipX,chipY,cw,35,accent,selectedTrack==f-1)){
   selectedTrack=f-1;scroll=0;
  }
  chipX+=cw+9;
 }
 y=chipY+60;
 int matchingTotal=0;
 for(int i=0;i<COUNT(lessons);i++)if((selectedTrack<0||lessons[i].track==selectedTrack)&&
     (contains(lessons[i].title,searchText)||contains(lessons[i].tag,searchText)))matchingTotal++;
 char summary[95];snprintf(summary,sizeof(summary),"%d chapter%s available",matchingTotal,matchingTotal==1?"":"s");
 tx(summary,x,y,13,CQ_MUTED);y+=35;
 for(int t=0;t<COUNT(tracks);t++){
  if(selectedTrack>=0&&t!=selectedTrack)continue;
  int shown=0,trackCount=trackLessonCount(t),done=trackDone(t);
  for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t&&
     (contains(lessons[i].title,searchText)||contains(lessons[i].tag,searchText)))shown++;
  if(!shown)continue;
  Color c=tracks[t].color;
  box(x,y,w,70,.10f,alpha(c,.065f));
  box(x,y,3,70,.5f,c);
  txH(tracks[t].name,x+19,y+11,20,CQ_WHITE);
  tx(tracks[t].subtitle,x+20,y+40,13,CQ_MUTED);
  char fraction[48];snprintf(fraction,sizeof(fraction),"%d / %d",done,trackCount);
  tx(fraction,x+w-74,y+12,13,c);
  bar(x+w-154,y+45,132,trackCount?(float)done/trackCount:0,c);
  y+=83;
  for(int i=0;i<COUNT(lessons);i++){
   if(lessons[i].track!=t||
      !(contains(lessons[i].title,searchText)||contains(lessons[i].tag,searchText)))continue;
   int chapterNo=lessonIndexInTrack(i),complete=progress.learned[i]!=0;
   float h=79;
   Rectangle area=RR(x,y,w,h);
   int hover=CheckCollisionPointRec(pointer(),clickRect(area));
   box(x,y,w,h,.09f,hover?CARD:PANEL);
   border(x,y,w,h,.09f,hover?alpha(c,.62f):alpha(CQ_LINE,.52f));
   box(x+13,y+16,46,46,.18f,alpha(c,.13f));
   char num[12];snprintf(num,sizeof(num),"%02d",chapterNo);
   tx(num,x+25,y+30,16,c);
   /* Limit titles before the right-hand status on narrow windows. */
   char title[130];snprintf(title,sizeof(title),"%s",lessons[i].title);
   while(strlen(title)>4&&tw(title,18)>w-235)title[strlen(title)-1]=0;
   tx(title,x+73,y+13,18,CQ_WHITE);
   tx(lessons[i].tag,x+74,y+45,12,CQ_MUTED);
   if(complete){
    if(w>=540)pill("LEARNED",x+w-115,y+26,CQ_GREEN);
    else tx("OK",x+w-44,y+31,12,CQ_GREEN);
   }else tx("Open  >",x+w-83,y+30,13,c);
   if(hover)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
   if(hit(area)){selectedLesson=i;scroll=0;}
   y+=89;
  }
  y+=17;
 }
 if(!matchingTotal){
  box(x,y,w,96,.10f,PANEL);
  tx("No matching chapters",x+18,y+20,18,CQ_WHITE);
  tx("Try another search or select All chapters.",x+18,y+53,14,CQ_MUTED);
  y+=115;
 }
 *bottom=y+40;
}
static float lessonSection(float x,float y,float w,const char *eyebrow,const char *title,Color accent){
 (void)w;
 box(x,y,29,3,1,accent);
 tx(eyebrow,x,y+13,11,accent);
 txH(title,x,y+35,23,CQ_WHITE);
 return y+77;
}
/* Render the source's numbered teaching notes as separate, scan-friendly steps. */
static float lessonSteps(const char *notes,float x,float y,float w,Color accent){
 if(!notes||!*notes)return y;
 const char *p=notes;int index=0;
 while(*p){
  const char *end=strchr(p,'\n');if(!end)end=p+strlen(p);
  size_t n=(size_t)(end-p);if(n>510)n=510;
  char line[512];memcpy(line,p,n);line[n]=0;
  const char *body=line;
  if(line[0]>='0'&&line[0]<='9'&&line[1]=='.'&&line[2]==' ')body=line+3;
  float endY=paragraph(body,x+42,y+2,15,CQ_WHITE,w-56);
  float h=fmaxf(38,endY-y+11);
  box(x+3,y+3,24,24,.45f,alpha(accent,.14f));
  char digit[8];snprintf(digit,sizeof(digit),"%d",++index);
  tx(digit,x+10,y+7,12,accent);
  y+=h+10;
  p=*end?end+1:end;
 }
 return y;
}
static float lessonExample(float x,float y,float w,const TutorialSection *part,Color accent,int number){
 char label[36];snprintf(label,sizeof(label),"EXAMPLE  %02d",number);
 tx(label,x,y+2,11,accent);y+=25;
 txH(part->heading,x,y,19,CQ_WHITE);y+=35;
 y=paragraph(part->explanation,x,y,15,CQ_MUTED,w-8)+19;
 y+=codePanel(part->example,x,y,w)+13;
 float outcomeEnd=paragraph(part->result,x+17,y+32,14,CQ_WHITE,w-38);
 float outcomeH=fmaxf(61,outcomeEnd-y+13);
 box(x,y,w,outcomeH,.09f,alpha(CQ_GREEN,.07f));
 box(x,y,3,outcomeH,.5f,CQ_GREEN);
 tx("WHAT THIS SHOWS",x+17,y+10,11,CQ_GREEN);
 paragraph(part->result,x+17,y+32,14,CQ_WHITE,w-38);
 return y+outcomeH+33;
}
/* Bite-size concept sections used across the entire 24-chapter curriculum. */
static float lessonCoreTopic(float x,float y,float w,const TutorialSection *topic,Color accent,int index,int total){
 char label[48];snprintf(label,sizeof(label),"CONCEPT  %02d / %02d",index,total);
 tx(label,x+2,y+3,11,accent);y+=28;
 txH(topic->heading,x+2,y,21,CQ_WHITE);y+=38;
 y=paragraph(topic->explanation,x+2,y,15,CQ_MUTED,w-12)+18;
 tx("CODE WALKTHROUGH",x+2,y,11,accent);y+=24;
 y+=codePanel(topic->example,x,y,w)+13;
 float endY=paragraph(topic->result,x+17,y+35,14,CQ_WHITE,w-38);
 float h=fmaxf(64,endY-y+18);
 box(x,y,w,h,.085f,alpha(CQ_GREEN,.07f));
 box(x,y,3,h,.5f,CQ_GREEN);
 tx("WHAT TO NOTICE",x+17,y+12,11,CQ_GREEN);
 paragraph(topic->result,x+17,y+35,14,CQ_WHITE,w-38);
 y+=h+29;
 if(index<total){box(x,y,w,1,.5f,alpha(CQ_LINE,.55f));y+=29;}
 return y;
}
/* A focused set of independently titled concepts instead of one long mixed section. */
static float conditionsLoopLesson(float x,float y,float w,Color accent){
 y=lessonSection(x,y,w,"01  /  CONTROL FLOW","Decisions and repetition",accent);
 y=paragraph("A condition decides whether code runs. A loop repeats code while its condition allows it. Study each construct below in order, then compare when to choose it.",x,y,16,CQ_MUTED,w-6)+20;
 for(int i=0;i<6;i++){
  char label[40];snprintf(label,sizeof(label),"TOPIC  %02d / 06",i+1);
  tx(label,x,y+1,11,accent);y+=26;
  const TutorialSection *part=&conditionsLoopTopics[i];
  txH(part->heading,x,y,21,CQ_WHITE);y+=38;
  y=paragraph(part->explanation,x,y,15,CQ_MUTED,w-8)+16;
  y+=codePanel(part->example,x,y,w)+12;
  float end=paragraph(part->result,x+16,y+32,14,CQ_WHITE,w-36);
  float h=fmaxf(60,end-y+15);
  box(x,y,w,h,.08f,alpha(CQ_GREEN,.075f));
  box(x,y,3,h,.5f,CQ_GREEN);
  tx("EXPECTED OUTPUT & WHY",x+16,y+10,11,CQ_GREEN);
  paragraph(part->result,x+16,y+32,14,CQ_WHITE,w-36);
  y+=h+25;
  if(i<5){box(x,y,w,1,.5f,alpha(CQ_LINE,.65f));y+=27;}
 }
 y=lessonSection(x,y,w,"02  /  PUT IT TOGETHER","Combining conditions and loops",accent);
 const TutorialChapter *chapter=&tutorialChapters[2];
 y=lessonExample(x,y,w,&chapter->sections[0],accent,1);
 y=lessonExample(x,y,w,&chapter->sections[1],accent,2);
 y=lessonSection(x,y,w,"03  /  MORE PRACTICE","Explore further",accent);
 const TutorialChapter *extra=&extraChapters[2];
 for(int i=0;i<2;i++)y=lessonExample(x,y,w,&extra->sections[i],accent,i+3);
 return y;
}
static void lessonView(float x,float w,float *bottom){
 Lesson *l=&lessons[selectedLesson];Color c=tracks[l->track].color;
 float rw=fminf(w,820.0f),rx=x+(w-rw)*.5f;
 float y=95;
 if(btn("<  All chapters",rx,y,154,36,CQ_PURPLE,0)){
  selectedLesson=-1;scroll=0;return;
 }
 int number=lessonIndexInTrack(selectedLesson),total=trackLessonCount(l->track);
 char position[65];snprintf(position,sizeof(position),"%s  /  %02d OF %02d",tracks[l->track].name,number,total);
 tx(position,rx+rw- fminf(rw-160,tw(position,12)+8),y+12,12,CQ_MUTED);
 y+=57;
 /* Open editorial header rather than a large empty banner. */
 tx("CHAPTER GUIDE",rx,y,11,c);y+=27;
 txH(l->title,rx,y,30,CQ_WHITE);y+=53;
 pill(l->tag,rx,y,c);
 if(progress.learned[selectedLesson])pill("COMPLETED",rx+tw(l->tag,11)+39,y,CQ_GREEN);
 y+=45;
 bar(rx,y,rw,total?(float)(number-1)/total:0,c);y+=25;
 y=paragraph(l->intro,rx,y,17,CQ_WHITE,rw-5)+25;
 box(rx,y,rw,1,.5f,alpha(CQ_LINE,.75f));y+=30;
 const LessonDetail *d=&lessonDetails[selectedLesson];
 if(selectedLesson==2){
  y=conditionsLoopLesson(rx,y,rw,c);
 }else{
  y=lessonSection(rx,y,rw,"01  /  LEARN THE IDEA","The essential concept",c);
  y=paragraph(l->explain,rx,y,16,CQ_MUTED,rw-6)+25;
  if(d->concept&&*d->concept){
   tx("BREAK IT DOWN",rx,y,11,c);y+=29;
   y=lessonSteps(d->concept,rx,y,rw,c)+17;
  }
  tx("QUICK REFERENCE",rx,y,11,c);y+=25;
  y+=codePanel(l->code,rx,y,rw)+31;
  y=lessonSection(rx,y,rw,"02  /  CORE TOPICS","Learn it step by step",c);
  y=paragraph("Start with these individual ideas. Read the example, predict what it does, then check the explanation before moving on.",rx,y,15,CQ_MUTED,rw-8)+25;
  const TutorialChapter *core=&coreTopics[selectedLesson];
  for(int i=0;i<2;i++)y=lessonCoreTopic(rx,y,rw,&core->sections[i],c,i+1,2);
  const TutorialChapter *chapter=&tutorialChapters[selectedLesson];
  y=lessonSection(rx,y,rw,"03  /  WORKED EXAMPLES","Put the ideas to work",c);
  for(int i=0;i<2;i++)y=lessonExample(rx,y,rw,&chapter->sections[i],c,i+1);
  const TutorialChapter *extra=&extraChapters[selectedLesson];
  y=lessonSection(rx,y,rw,"04  /  GO FURTHER","Build your understanding",c);
  for(int i=0;i<2;i++)y=lessonExample(rx,y,rw,&extra->sections[i],c,i+3);
 }
 if(d->mistakes&&*d->mistakes){
  y=lessonSection(rx,y,rw,"05  /  CHECK YOUR UNDERSTANDING","Watch out for",CQ_ORANGE);
  box(rx,y,3,42,.5f,CQ_ORANGE);
  y=paragraph(d->mistakes,rx+18,y+3,15,CQ_MUTED,rw-24)+27;
  txH("Try it yourself",rx,y,21,CQ_WHITE);y+=39;
  y=paragraph(d->practice,rx,y,16,CQ_WHITE,rw-8)+29;
 }
 float reminderEnd=paragraph(l->tip,rx+18,y+38,15,CQ_WHITE,rw-40);
 float reminderH=fmaxf(88,reminderEnd-y+17);
 box(rx,y,rw,reminderH,.10f,alpha(c,.085f));
 border(rx,y,rw,reminderH,.10f,alpha(c,.25f));
 tx("KEY TAKEAWAY",rx+18,y+12,11,c);
 paragraph(l->tip,rx+18,y+38,15,CQ_WHITE,rw-40);
 y+=reminderH+29;
 box(rx,y,rw,1,.5f,CQ_LINE);y+=26;
 if(progress.learned[selectedLesson])pill("CHAPTER COMPLETED  +30 XP",rx,y,CQ_GREEN);
 else if(btn("Mark as learned  +30 XP",rx,y,229,40,CQ_GREEN,1)){
  progress.learned[selectedLesson]=1;award(CQ_XP_LESSON);checkTrackRewards();
 }
 y+=57;
 int next=-1;
 for(int i=selectedLesson+1;i<COUNT(lessons);i++)if(lessons[i].track==l->track){next=i;break;}
 if(next<0)for(int i=selectedLesson+1;i<COUNT(lessons);i++){next=i;break;}
 /* Compact primary action. Keep the chapter title as a separate, readable cue. */
 if(next>=0){
  tx("UP NEXT",rx,y,11,CQ_MUTED);y+=25;
  tx(lessons[next].title,rx,y,16,CQ_WHITE);y+=34;
  if(btn("Next lesson  >",rx,y,157,40,c,1)){
   selectedLesson=next;scroll=0;return;
  }
  if(rw>=490&&btn("Practise this topic",rx+170,y,188,40,CQ_PURPLE,0)){
   startQuiz(l->track);return;
  }
  y+=55;
  if(rw<490&&btn("Practise this topic",rx,y,188,40,CQ_PURPLE,0)){
   startQuiz(l->track);return;
  }
 }else{
  if(btn("Practise this topic",rx,y,188,40,CQ_PURPLE,0)){
   startQuiz(l->track);return;
  }
  y+=54;
 }
 *bottom=y+66;
}
/* Topic-based quiz menu: one assessment per learning track. */
static void quizMenu(float x,float w,float *bottom){
 float rw=fminf(w,990),rx=x+(w-rw)*.5f,y=101;
 tx("PRACTICE  /  CHALLENGE ROOM",rx,y,12,CQ_PURPLE);y+=29;
 txH("Test what you've learned",rx,y,31,CQ_WHITE);y+=48;
 tx("Choose a topic, answer each question and review what you can improve.",rx,y,15,CQ_MUTED);y+=51;
 box(rx,y,rw,115,.11f,alpha(CQ_PURPLE,.11f));
 border(rx,y,rw,115,.11f,alpha(CQ_PURPLE,.36f));
 tx("NEW  /  60-MINUTE CODING EXAM",rx+20,y+13,12,CQ_PURPLE);
 txH("Exam Simulator",rx+20,y+37,22,CQ_WHITE);
 tx("5 written coding problems  /  pointers and linked lists",rx+20,y+74,14,CQ_MUTED);
 if(rw>=675){
  if(btn("Start exam  >",rx+rw-179,y+33,157,46,CQ_PURPLE,1)){
   ExamStart();examMode=1;scroll=0;return;
  }
 }else if(hit(RR(rx,y,rw,115))){ExamStart();examMode=1;scroll=0;return;}
 y+=136;
 box(rx,y,rw,102,.11f,alpha(CQ_ORANGE,.10f));
 border(rx,y,rw,102,.11f,alpha(CQ_ORANGE,.32f));
 tx("DEBUG THE CODE",rx+20,y+17,12,CQ_ORANGE);
 char debugStats[115];snprintf(debugStats,sizeof(debugStats),"Find the bug  /  %d of %d mastered",cqDebugMastered(),CQ_DEBUG_COUNT);
 tx(debugStats,rx+20,y+48,16,CQ_WHITE);
 if(rw>=650){
  if(btn("Open challenges  >",rx+rw-207,y+29,186,44,CQ_ORANGE,1)){debugMode=1;scroll=0;return;}
 }else{
  if(hit(RR(rx,y,rw,102))){debugMode=1;scroll=0;return;}
 }
 y+=129;
 char stats[115];snprintf(stats,sizeof(stats),"%d / %d QUESTIONS MASTERED",totalQuizzes(),COUNT(quizzes));
 tx(stats,rx,y,12,CQ_MUTED);y+=25;
 bar(rx,y,rw,(float)totalQuizzes()/COUNT(quizzes),CQ_PURPLE);y+=38;
 txH("Choose your challenge",rx,y,23,CQ_WHITE);y+=47;
 for(int t=0;t<COUNT(tracks);t++){
  int count=quizCountForTrack(t),done=quizMasteredForTrack(t);if(!count)continue;
  Rectangle r=RR(rx,y,rw,104);
  int hover=CheckCollisionPointRec(pointer(),clickRect(r));
  quizTrackHovers[t]=hoverEase(quizTrackHovers[t],hover,11.0f);
  float qh=quizTrackHovers[t];
  box(rx,y,rw,104,.11f,mixColor(PANEL,CARD,qh));
  border(rx,y,rw,104,.11f,mixColor(CQ_LINE,alpha(tracks[t].color,.9f),qh));
  box(rx+18,y+21,58,58,.22f,alpha(tracks[t].color,.15f));
  journeyGlyph(t,rx+47,y+50,18.0f,tracks[t].color);
  txH(tracks[t].name,rx+92,y+16,20,CQ_WHITE);
  char details[95];snprintf(details,sizeof(details),"%d questions  /  %d mastered",count,done);
  tx(details,rx+92,y+51,14,CQ_MUTED);
  bar(rx+92,y+83,fmaxf(65,rw-215),(float)done/count,tracks[t].color);
  if(rw>530)pill("START  >",rx+rw-119,y+38,tracks[t].color);
  if(hit(r)){startQuiz(t);return;}
  y+=117;
 }
 *bottom=y+44;
}
static void quizSummary(float x,float w,float *bottom){
 float readW=fminf(w,840.0f);x+=(w-readW)*.5f;w=readW;
 float y=102;
 int score=quizScore();int pct=quizCount?score*100/quizCount:0;
 Color accent=score==quizCount?CQ_GREEN:pct>=70?CQ_PURPLE:CQ_ORANGE;
 pill("QUIZ COMPLETE",x,y,accent);y+=48;
 txH("Your quiz results",x,y,32,CQ_WHITE);y+=60;
 box(x,y,w,215,.09f,PANEL);border(x,y,w,215,.09f,alpha(accent,.32f));
 tx(tracks[quizTrack].name,x+23,y+19,20,CQ_WHITE);
 char result[100];snprintf(result,sizeof(result),"%d / %d",score,quizCount);
 tx(result,x+25,y+57,49,accent);
 snprintf(result,sizeof(result),"%d%% correct",pct);
 tx(result,x+237,y+79,20,CQ_MUTED);
 bar(x+25,y+145,w-50,(float)score/quizCount,accent);
 tx(score==quizCount?"Excellent! Every question was correct.":
    pct>=70?"Good work! Review the questions you missed.":
    "Keep practising. Your review is below.",x+25,y+169,15,CQ_MUTED);
 y+=246;
 if(score<quizCount){
  txH("Areas to improve",x,y,25,CQ_WHITE);y+=40;
  y=paragraph("These are the questions you missed. Review the correct answers and explanations, then revisit this learning track before retrying.",x,y,15,CQ_MUTED,w-22)+24;
  int missed=0;
  for(int i=0;i<quizCount;i++){
   int id=quizQuestionIds[i];Quiz *q=&quizzes[id];
   if(quizSelections[i]==q->correct)continue;
   missed++;
   char heading[80];snprintf(heading,sizeof(heading),"REVIEW QUESTION %d",i+1);
   float start=y;
   float qy=paragraph(q->question,x+20,y+52,17,CQ_WHITE,w-50)+18;
   char correctAnswer[220];snprintf(correctAnswer,sizeof(correctAnswer),"Correct answer: %s",q->answers[q->correct]);
   qy=paragraph(correctAnswer,x+20,qy,16,CQ_GREEN,w-50)+16;
   qy=paragraph(q->why,x+20,qy,15,CQ_MUTED,w-50)+23;
   box(x,start,w,qy-start,.08f,PANEL);
   tx(heading,x+20,start+18,12,CQ_ORANGE);
   /* Text was measured first; draw again on top of its background. */
   float textY=paragraph(q->question,x+20,start+52,17,CQ_WHITE,w-50)+18;
   textY=paragraph(correctAnswer,x+20,textY,16,CQ_GREEN,w-50)+16;
   paragraph(q->why,x+20,textY,15,CQ_MUTED,w-50);
   y=qy+15;
  }
  if(!missed)tx("No mistakes to review.",x,y,16,CQ_GREEN);
  y+=16;
  box(x,y,w,106,.08f,alpha(tracks[quizTrack].color,.12f));
  tx("SUGGESTED NEXT STEP",x+19,y+15,12,tracks[quizTrack].color);
  tx("Review the related lessons",x+19,y+43,19,CQ_WHITE);
  if(btn("Study topic  >",x+w-188,y+35,166,43,tracks[quizTrack].color,1)){
   int t=quizTrack;selectedTrack=t;nav(1);return;
  }
  y+=131;
 }else{
  box(x,y,w,87,.09f,alpha(CQ_GREEN,.10f));
  tx("Great mastery! Try another topic or a coding lab.",x+21,y+31,17,CQ_GREEN);
  y+=112;
 }
 if(btn("Retry quiz",x,y,171,49,CQ_PURPLE,1)){
  int t=quizTrack;startQuiz(t);return;
 }
 if(btn("All quizzes",x+188,y,174,49,CQ_PURPLE,0)){
  quizTrack=-1;quizFinished=0;scroll=0;return;
 }
 *bottom=y+104;
}
static void quizView(float x,float w,float *bottom){
 if(examMode){
  if(!ExamIsRunning()){examMode=0;scroll=0;}
  else{
   ExamStyle style={.x=x,.width=w,.scroll=scroll,.mouse=pointer(),
    .font=font,.headingFont=headingFont,.codeFont=codeFont,
    .panel=PANEL,.card=CARD,.text=CQ_WHITE,.muted=CQ_MUTED,.line=CQ_LINE,
    .accent=CQ_PURPLE,.green=CQ_GREEN,.orange=CQ_ORANGE,.background=BG,
    .accentText=textOnAccent};
   ExamDraw(&style,bottom);return;
  }
 }
 if(debugMode){
  if(btn("< All quizzes",x,96,162,40,CQ_PURPLE,0)){debugMode=0;scroll=0;return;}
  DebugChallengeView v={.x=x,.width=w,.font=font,.mouse=pointer(),
      .scroll=scroll,.scrollPosition=&scroll,.lightTheme=lightTheme};
  DrawDebugChallenges(&v,bottom);return;
 }
 if(quizTrack<0){quizMenu(x,w,bottom);return;}
 if(quizFinished){quizSummary(x,w,bottom);return;}
 float rw=fminf(w,840),rx=x+(w-rw)*.5f,y=100;
 if(btn("<  All quizzes",rx,y,162,38,CQ_PURPLE,0)){
  quizTrack=-1;quizFinished=0;scroll=0;return;
 }
 char counter[75];snprintf(counter,sizeof(counter),"%02d / %02d",quizPosition+1,quizCount);
 tx(counter,rx+rw-73,y+12,15,CQ_MUTED);y+=63;
 tx("CODEQUEST  /  KNOWLEDGE CHECK",rx,y,12,CQ_PURPLE);y+=30;
 txH(tracks[quizTrack].name,rx,y,30,CQ_WHITE);y+=47;
 tx("Choose an answer below. You'll see an explanation before continuing.",rx,y,15,CQ_MUTED);y+=38;
 bar(rx,y,rw,(float)(quizPosition+(quizAnswered?1:0))/quizCount,tracks[quizTrack].color);y+=37;
 Quiz *q=&quizzes[quizQuestionIds[quizPosition]];
 Color c=tracks[q->track].color;
 float qEnd=paragraph(q->question,rx+24,y+57,21,CQ_WHITE,rw-52);
 float questionH=fmaxf(143,qEnd-y+29);
 box(rx,y,rw,questionH,.105f,CARD);
 box(rx,y,4,questionH,.5f,c);
 tx("QUESTION",rx+24,y+19,12,c);
 paragraph(q->question,rx+24,y+57,21,CQ_WHITE,rw-52);
 y+=questionH+24;
 tx("SELECT ONE ANSWER",rx+3,y,11,CQ_MUTED);y+=26;
 for(int i=0;i<4;i++){
  Color background=PANEL,outline=CQ_LINE;
  if(quizAnswered&&i==q->correct){background=alpha(CQ_GREEN,.14f);outline=CQ_GREEN;}
  else if(quizAnswered&&i==quizChoice){background=alpha(CQ_RED,.14f);outline=CQ_RED;}
  else if(!quizAnswered&&quizChoice==i){background=alpha(CQ_PURPLE,.16f);outline=CQ_PURPLE;}
  Rectangle option=RR(rx,y,rw,69);
  int hover=CheckCollisionPointRec(pointer(),clickRect(option));
  quizOptionHovers[i]=hoverEase(quizOptionHovers[i],hover&&!quizAnswered,13.0f);
  float oh=quizOptionHovers[i];
  if(!quizAnswered&&quizChoice!=i){
   background=mixColor(background,CARD,oh);
   outline=mixColor(outline,alpha(CQ_PURPLE,.65f),oh);
  }
  box(rx,y,rw,69,.13f,background);border(rx,y,rw,69,.13f,outline);
  box(rx+17,y+17,35,35,.27f,alpha(outline,.18f));
  char letter[3]={(char)('A'+i),0};
  tx(letter,rx+29,y+23,15,quizChoice==i?CQ_WHITE:CQ_MUTED);
  tx(q->answers[i],rx+68,y+22,17,CQ_WHITE);
  if(quizAnswered && i==q->correct)tx("CORRECT",rx+rw-91,y+26,11,CQ_GREEN);
  if(!quizAnswered&&hit(option))quizChoice=i;
  y+=80;
 }
 y+=10;
 if(quizAnswered){
  int right=quizChoice==q->correct;
  float feedbackEnd=paragraph(q->why,rx+21,y+57,15,CQ_WHITE,rw-45);
  float height=fmaxf(118,feedbackEnd-y+26);
  Color status=right?CQ_GREEN:CQ_ORANGE;
  box(rx,y,rw,height,.10f,alpha(status,.10f));
  border(rx,y,rw,height,.10f,alpha(status,.33f));
  tx(right?"Nicely done!":"Let's review",rx+21,y+17,19,status);
  paragraph(q->why,rx+21,y+57,15,CQ_WHITE,rw-45);
  y+=height+22;
  int last=(quizPosition==quizCount-1);
  if(btn(last?"Finish & view results  >":"Continue  >",rx+rw-(last?252:174),y,last?252:174,49,CQ_PURPLE,1)){
   if(last){quizFinished=1;rewardQuizSession(quizTrack,quizScore(),quizCount);checkTrackRewards();}
   else {quizPosition++;quizChoice=-1;quizAnswered=0;}
   scroll=0;return;
  }
 }else{
  if(btn("Check answer",rx+rw-189,y,189,49,CQ_PURPLE,quizChoice>=0)&&quizChoice>=0){
   quizAnswered=1;quizSelections[quizPosition]=quizChoice;progress.quizAttempts++;
   if(quizChoice==q->correct){
    celebrationCorrect(rx+rw*.5f,y-scroll-80.0f);progress.quizCorrect++;
    int id=quizQuestionIds[quizPosition];
    if(!progress.quizDone[id]){progress.quizDone[id]=1;award(CQ_XP_QUIZ_ANSWER);checkTrackRewards();}
    else persist();
   }else persist();
  }
 }
 *bottom=y+102;
}
static void openLab(int id){
 if(labLoaded>=0)labSaveDraft(labLoaded,labCode);
 labLoaded=id;labCursor=0;labAnchor=0;labDragging=0;labScrollbarDragging=0;labRepeatKey=0;labFirstLine=0;labFocus=0;labHasResult=0;
 if(!labLoadDraft(id,labCode,sizeof(labCode)))labLoadStarter(id,labCode,sizeof(labCode));
}
static int lineStart(int wanted){int line=0;for(int i=0;labCode[i];i++){if(line==wanted)return i;if(labCode[i]=='\n')line++;}return (line==wanted)?(int)strlen(labCode):-1;}
static int lineOf(int pos){int n=0;for(int i=0;i<pos&&labCode[i];i++)if(labCode[i]=='\n')n++;return n;}
static int lineColumn(int pos){int n=0;for(int i=pos-1;i>=0&&labCode[i]!='\n';i--)n++;return n;}
static int editHasSelection(void){return labCursor!=labAnchor;}
static int editSelStart(void){return labCursor<labAnchor?labCursor:labAnchor;}
static int editSelEnd(void){return labCursor>labAnchor?labCursor:labAnchor;}
static void editRemoveSelection(void){
 if(!editHasSelection())return;
 int begin=editSelStart(),end=editSelEnd();
 memmove(labCode+begin,labCode+end,strlen(labCode+end)+1);
 labCursor=labAnchor=begin;labHasResult=0;
}
static void editInsert(const char *value){
 size_t n=strlen(labCode),add=strlen(value);
 size_t selected=(size_t)(editSelEnd()-editSelStart());
 if(n-selected+add>=sizeof(labCode))return;
 editRemoveSelection();n=strlen(labCode);
 memmove(labCode+labCursor+add,labCode+labCursor,n-(size_t)labCursor+1);
 memcpy(labCode+labCursor,value,add);labCursor+=(int)add;
 labAnchor=labCursor;labHasResult=0;
}
static void editDelete(int before){
 if(editHasSelection()){editRemoveSelection();return;}
 size_t n=strlen(labCode);
 if(before){if(labCursor<=0)return;labCursor--;}
 else if((size_t)labCursor>=n)return;
 memmove(labCode+labCursor,labCode+labCursor+1,n-(size_t)labCursor);
 labAnchor=labCursor;labHasResult=0;
}
/* Implements initial key press, then OS-like repeat without relying on the
 * installed raylib version supporting IsKeyPressedRepeat(). */
static int editorKeyRepeat(int key){
 if(!IsKeyDown(key))return 0;
 if(labRepeatKey!=key){
  if(!IsKeyPressed(key))return 0;
  labRepeatKey=key;labRepeatTime=0.0f;return 1;
 }
 labRepeatTime+=GetFrameTime();
 if(labRepeatTime>=0.40f){
  labRepeatTime-=0.045f;
  if(labRepeatTime<0.35f)labRepeatTime=0.35f;
  return 1;
 }
 return 0;
}
static void editorMove(int newPos,int extend){
 int n=(int)strlen(labCode);
 if(newPos<0)newPos=0;
 if(newPos>n)newPos=n;
 if(!extend&&editHasSelection()){/* Standard editor behavior: arrow collapses selection. */
  labCursor=newPos;
 }else labCursor=newPos;
 if(!extend)labAnchor=labCursor;
}
static void editorInput(void){
 if(!labFocus){labRepeatKey=0;return;}
 int oldCursor=labCursor;
 size_t oldLength=strlen(labCode);
 int ctrl=IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL)||IsKeyDown(KEY_LEFT_SUPER)||IsKeyDown(KEY_RIGHT_SUPER);
 int shift=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT);
 if(ctrl&&IsKeyPressed(KEY_S)){labSaveDraft(labLoaded,labCode);return;}
 if(ctrl&&IsKeyPressed(KEY_A)){labAnchor=0;labCursor=(int)strlen(labCode);return;}
 if(ctrl&&IsKeyPressed(KEY_C)){
  if(editHasSelection()){
   int start=editSelStart(),end=editSelEnd();char copy[LAB_CODE_CAP];
   memcpy(copy,labCode+start,(size_t)(end-start));copy[end-start]=0;SetClipboardText(copy);
  }return;
 }
 if(ctrl&&IsKeyPressed(KEY_X)){
  if(editHasSelection()){
   int start=editSelStart(),end=editSelEnd();char copy[LAB_CODE_CAP];
   memcpy(copy,labCode+start,(size_t)(end-start));copy[end-start]=0;SetClipboardText(copy);
   editRemoveSelection();
  }return;
 }
 if(ctrl&&IsKeyPressed(KEY_V)){
  const char *clip=GetClipboardText();
  if(clip){char clean[LAB_CODE_CAP];int j=0;
   for(int k=0;clip[k]&&j<LAB_CODE_CAP-1;k++){
    if(clip[k]=='\r')continue;
    if(clip[k]=='\n'||clip[k]=='\t'||(unsigned char)clip[k]>=32)clean[j++]=clip[k];
   }clean[j]=0;editInsert(clean);
  }return;
 }
 /* Prioritise one held editing key. Releasing it resets the repeat timer. */
 const int repeatKeys[]={KEY_BACKSPACE,KEY_DELETE,KEY_LEFT,KEY_RIGHT,KEY_UP,KEY_DOWN,KEY_ENTER,KEY_KP_ENTER};
 if(labRepeatKey&&!IsKeyDown(labRepeatKey)){labRepeatKey=0;labRepeatTime=0.0f;}
 int key=0;
 for(int i=0;i<COUNT(repeatKeys);i++){
  if(IsKeyPressed(repeatKeys[i])){labRepeatKey=0;key=repeatKeys[i];break;}
 }
 if(!key&&labRepeatKey)key=labRepeatKey;
 if(key&&editorKeyRepeat(key)){
  if(key==KEY_ENTER||key==KEY_KP_ENTER)editInsert("\n");
  else if(key==KEY_BACKSPACE)editDelete(1);
  else if(key==KEY_DELETE)editDelete(0);
  else if(key==KEY_LEFT){
   if(!shift&&editHasSelection())editorMove(editSelStart(),0);
   else editorMove(labCursor-1,shift);
  }else if(key==KEY_RIGHT){
   if(!shift&&editHasSelection())editorMove(editSelEnd(),0);
   else editorMove(labCursor+1,shift);
  }else if(key==KEY_UP||key==KEY_DOWN){
   int column=lineColumn(labCursor),target=lineOf(labCursor)+(key==KEY_UP?-1:1);
   int pos=lineStart(target);if(pos>=0){while(column--&&labCode[pos]&&labCode[pos]!='\n')pos++;editorMove(pos,shift);}
  }
 }
 if(IsKeyPressed(KEY_HOME))editorMove(labCursor-lineColumn(labCursor),shift);
 if(IsKeyPressed(KEY_END)){int pos=labCursor;while(labCode[pos]&&labCode[pos]!='\n')pos++;editorMove(pos,shift);}
 if(IsKeyPressed(KEY_TAB))editInsert("    ");
 if(!ctrl){for(int ch=GetCharPressed();ch;ch=GetCharPressed())if(ch>=32&&ch<127){char c[2]={(char)ch,0};editInsert(c);}}
 if(oldCursor!=labCursor||oldLength!=strlen(labCode)){
  int line=lineOf(labCursor);
  if(line<labFirstLine)labFirstLine=line;
  if(line>=labFirstLine+LAB_EDITOR_VISIBLE_LINES)labFirstLine=line-LAB_EDITOR_VISIBLE_LINES+1;
 }
}
/* Translate mouse coordinates into the closest character boundary. */
static int editorPositionAt(float mouseX,float mouseY,float x,float y){
 const float lh=23.0f;
 int row=(int)floorf((mouseY-(y+52))/lh);
 if(row<0)row=0;
 if(row>=LAB_EDITOR_VISIBLE_LINES)row=LAB_EDITOR_VISIBLE_LINES-1;
 int target=labFirstLine+row;
 int start=lineStart(target);
 if(start<0){return (int)strlen(labCode);}
 int end=start;while(labCode[end]&&labCode[end]!='\n')end++;
 float relativeX=mouseX-(x+65);
 if(relativeX<=0)return start;
 /* Calculate nearest midpoint in exactly the same font as the drawn text. */
 char prefix[LAB_CODE_CAP];int count=0;float previous=0;
 for(int at=start;at<end&&count<LAB_CODE_CAP-2;at++){
  prefix[count++]=labCode[at];prefix[count]=0;
  float width=tw(prefix,15);
  if(relativeX<(previous+width)/2.0f)return at;
  previous=width;
 }
 return end;
}
static void editorPanel(float x,float y,float w){
 const float h=382,lh=LAB_EDITOR_LINE_HEIGHT;
 const float top=y+52,trackX=x+w-24,trackY=y+49;
 const float trackH=LAB_EDITOR_VISIBLE_LINES*lh+6;
 Color bg=lightTheme?(Color){224,233,247,255}:(Color){12,21,34,255};
 box(x,y,w,h,.07f,bg);border(x,y,w,h,.07f,labFocus?CQ_PURPLE:CQ_LINE);
 tx("CODE EDITOR  |  Drag to select  |  Ctrl+A/C/X/V  |  Ctrl+S save",x+18,y+14,12,CQ_MUTED);
 Rectangle edit=RR(x+9,y+43,w-39,h-54);
 Rectangle track=RR(trackX,trackY,13,trackH);
 Vector2 mouse=pointer();float my=mouse.y+scroll;
 /* This editor owns the mouse wheel when hovered; do not scroll the page too. */
 labEditorWheelRect=RR(x+9,y+43-scroll,w-18,h-54);
 int totalLines=lineOf((int)strlen(labCode))+1;
 int maxFirst=totalLines-LAB_EDITOR_VISIBLE_LINES;
 if(maxFirst<0)maxFirst=0;
 if(labFirstLine>maxFirst)labFirstLine=maxFirst;
 if(labFirstLine<0)labFirstLine=0;
 if(CheckCollisionPointRec(mouse,labEditorWheelRect)){
  float wheel=GetMouseWheelMove();
  if(wheel!=0){labFirstLine-=(int)(wheel*3.0f);if(labFirstLine<0)labFirstLine=0;if(labFirstLine>maxFirst)labFirstLine=maxFirst;}
 }
 float thumbH=trackH;
 float thumbY=trackY;
 if(maxFirst>0){
  thumbH=fmaxf(28.0f,trackH*(float)LAB_EDITOR_VISIBLE_LINES/(float)totalLines);
  if(thumbH>trackH)thumbH=trackH;
  thumbY=trackY+(trackH-thumbH)*(float)labFirstLine/(float)maxFirst;
 }
 Rectangle thumb=RR(trackX,thumbY,13,thumbH);
 if(maxFirst>0&&hit(track)){
  labDragging=0;
  labScrollbarDragging=1;
  if(CheckCollisionPointRec((Vector2){mouse.x,my},thumb)){
   labScrollbarGrab=my-thumbY;
  }else{
   labScrollbarGrab=thumbH*0.5f;
  }
 }
 if(labScrollbarDragging){
  if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT))labScrollbarDragging=0;
  else if(maxFirst>0){
   float fraction=(my-labScrollbarGrab-trackY)/(trackH-thumbH);
   if(fraction<0)fraction=0;
   if(fraction>1)fraction=1;
   labFirstLine=(int)(fraction*(float)maxFirst+0.5f);
  }
 }
 if(!labScrollbarDragging&&hit(edit)){
  labFocus=1;labDragging=1;labDragScrollTime=0.0f;
  labCursor=editorPositionAt(mouse.x,my,x,y);
  labAnchor=labCursor;
 }
 if(labDragging){
  if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT))labDragging=0;
  else{
   /* Continue selecting when the pointer leaves the editor. */
   labDragScrollTime+=GetFrameTime();
   if(labDragScrollTime>=0.08f){
    labDragScrollTime=0.0f;
    if(my<top&&labFirstLine>0)labFirstLine--;
    if(my>top+LAB_EDITOR_VISIBLE_LINES*lh&&labFirstLine<maxFirst)labFirstLine++;
   }
   labCursor=editorPositionAt(mouse.x,my,x,y);
  }
 }
 if(labFocus)editorInput();
 /* Enter/backspace can change the line count; update the scrollbar immediately. */
 totalLines=lineOf((int)strlen(labCode))+1;
 maxFirst=totalLines-LAB_EDITOR_VISIBLE_LINES;
 if(maxFirst<0)maxFirst=0;
 if(labFirstLine>maxFirst)labFirstLine=maxFirst;
 if(labFirstLine<0)labFirstLine=0;
 thumbH=maxFirst>0?fmaxf(28.0f,trackH*(float)LAB_EDITOR_VISIBLE_LINES/(float)totalLines):trackH;
 if(thumbH>trackH)thumbH=trackH;
 thumbY=maxFirst>0?trackY+(trackH-thumbH)*(float)labFirstLine/(float)maxFirst:trackY;
 int currentLine=lineOf(labCursor);
 int selectionStart=editSelStart(),selectionEnd=editSelEnd();
 for(int row=0;row<LAB_EDITOR_VISIBLE_LINES;row++){
  int ln=labFirstLine+row,start=lineStart(ln);if(start<0)break;
  float yy=y+52+row*lh;char num[16];snprintf(num,sizeof(num),"%3d",ln+1);tx(num,x+19,yy,13,CQ_MUTED);
  char line[1024];int j=0;
  while(labCode[start+j]&&labCode[start+j]!='\n'&&j<(int)sizeof(line)-1)j++;
  memcpy(line,labCode+start,(size_t)j);line[j]=0;
  /* Clip long lines visually. The full source remains editable. */
  while(j>0&&tw(line,15)>w-122){line[--j]=0;}
  if(ln==currentLine&&labFocus)box(x+59,yy-2,w-75,21,.02f,alpha(CQ_PURPLE,.12f));
  if(editHasSelection()){
   int lineEnd=start+j;
   int first=selectionStart>start?selectionStart:start;
   int last=selectionEnd<lineEnd?selectionEnd:lineEnd;
   if(last>first){
    char left[1024],through[1024];
    int startCol=first-start,endCol=last-start;
    memcpy(left,line,(size_t)startCol);left[startCol]=0;
    memcpy(through,line,(size_t)endCol);through[endCol]=0;
    float sx=x+65+tw(left,15);
    float sw=tw(through,15)-tw(left,15);
    box(sx,yy-2,sw,21,.02f,alpha(CQ_BLUE,.40f));
   }
   /* Highlight a selected newline with a small block at the end of its line. */
   int fullEnd=start;while(labCode[fullEnd]&&labCode[fullEnd]!='\n')fullEnd++;
   if(labCode[fullEnd]=='\n'&&selectionStart<=fullEnd&&selectionEnd>fullEnd&&fullEnd<=lineEnd){
    box(x+65+tw(line,15),yy-2,8,21,.02f,alpha(CQ_BLUE,.40f));
   }
  }
  Color col=(strstr(line,"//")==line||strstr(line,"/*")==line)?CQ_MUTED:CQ_WHITE;
  tx(line,x+65,yy,15,col);
  if(ln==currentLine&&labFocus){
   int colpos=lineColumn(labCursor);int visible=colpos<j?colpos:j;
   char prefix[1024];memcpy(prefix,line,(size_t)visible);prefix[visible]=0;
   float cx=x+65+tw(prefix,15);
   if(cx<x+w-31)DrawRectangle((int)cx,(int)(yy-1),2,19,CQ_PURPLE);
  }
 }
 box(trackX,trackY,13,trackH,.8f,alpha(CQ_LINE,.60f));
 box(trackX+2,thumbY,9,thumbH,.8f,maxFirst>0?CQ_PURPLE:CQ_MUTED);
 if(maxFirst>0&&CheckCollisionPointRec((Vector2){mouse.x,my},track))SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
 tx("Scroll wheel/drag bar. Hold Enter for newlines; Backspace/Delete repeat. Shift+arrows select.",x+16,y+h-22,11,CQ_MUTED);
}
static void labList(float x,float w,float *bottom){float y=95;txH("Coding lab challenges",x,y,30,CQ_WHITE);tx("Real assignments from your worksheets, built for hands-on practise.",x,y+43,15,CQ_MUTED);y+=99;
 box(x,y,w,99,.07f,alpha(CQ_PURPLE,.12f));tx("HOW LABS WORK",x+20,y+16,12,CQ_PURPLE);paragraph("Write code in the built-in editor, Run it with your local compiler, then Check Solution against reference output tests. C uses GCC/Clang; C++ uses G++/Clang++.",x+20,y+42,14,CQ_WHITE,w-42);y+=129;
 for(int t=0;t<COUNT(tracks);t++){int n=0;for(int i=0;i<COUNT(labTasks);i++)if(labTasks[i].track==t)n++;if(!n)continue;
  tx(tracks[t].name,x,y,21,CQ_WHITE);y+=39;
  for(int i=0;i<COUNT(labTasks);i++){if(labTasks[i].track!=t)continue;box(x,y,w,85,.08f,PANEL);
   box(x+17,y+18,45,45,.2f,alpha(tracks[t].color,.19f));tx("</>",x+22,y+29,16,tracks[t].color);
   tx(labTasks[i].title,x+77,y+13,18,CQ_WHITE);tx(labTasks[i].level,x+77,y+47,12,CQ_MUTED);
   if(progress.labDone[i])pill("SOLVED",x+w-101,y+29,CQ_GREEN);else tx("Open  >",x+w-91,y+32,14,CQ_PURPLE);
   if(hit(RR(x,y,w,85))){selectedLab=i;openLab(i);scroll=0;}y+=98;
  }y+=21;
 }*bottom=y;
}
static void labView(float x,float w,float *bottom){Lab*l=&labTasks[selectedLab];float y=96;Color c=tracks[l->track].color;
 if(labLoaded!=selectedLab)openLab(selectedLab);
 if(btn("<  Back to labs",x,y,180,40,CQ_PURPLE,0)){labSaveDraft(selectedLab,labCode);selectedLab=-1;labFocus=0;scroll=0;return;}y+=66;
 pill(l->level,x,y,c);y+=45;tx(l->title,x,y,30,CQ_WHITE);y+=57;
 tx("YOUR TASK",x,y,13,c);y+=32;y=paragraph(l->task,x,y,17,CQ_WHITE,w-40)+26;
 box(x,y,w,88,.07f,PANEL);tx("REFERENCE TESTS",x+18,y+12,12,CQ_MUTED);paragraph(l->tests,x+18,y+39,14,CQ_WHITE,w-37);y+=109;
 tx("WRITE YOUR SOLUTION",x,y,14,c);y+=28;
 editorPanel(x,y,w);y+=396;
 float bw=(w-24)/3;
 if(btn("Run Code",x,y,bw,45,CQ_BLUE,1)){
  labFocus=0;labSaveDraft(selectedLab,labCode);labResult=labExecute(selectedLab,labCode,0);labHasResult=1;labWasCheck=0;
 }
 if(btn("Check Solution",x+bw+12,y,bw,45,CQ_GREEN,1)){
  labFocus=0;labSaveDraft(selectedLab,labCode);labResult=labExecute(selectedLab,labCode,1);labHasResult=1;labWasCheck=1;
  if(labResult.passed&&!progress.labDone[selectedLab]){progress.labDone[selectedLab]=1;award(CQ_XP_LAB);checkTrackRewards();}
 }
 if(btn("Reset Starter",x+2*(bw+12),y,bw,45,CQ_PURPLE,0)){
  labLoadStarter(selectedLab,labCode,sizeof(labCode));labCursor=0;labAnchor=0;labDragging=0;labScrollbarDragging=0;labFirstLine=0;labHasResult=0;labFocus=0;labSaveDraft(selectedLab,labCode);
 }
 y+=67;
 if(labHasResult){
  /* Passing a reference check is green; failed checks or program errors are
   * red. A successful plain Run is neutral, as it has not been verified. */
  int verified=labWasCheck&&labResult.passed;
  int failed=!labResult.compiled||!labResult.ran||(labWasCheck&&!labResult.passed);
  Color statusColor=verified?CQ_GREEN:(failed?CQ_RED:CQ_BLUE);
  const char *statusTitle=verified?"ALL REFERENCE TESTS PASSED":
      (!labResult.compiled?"COMPILATION FAILED":(!labResult.ran?"PROGRAM FAILED":
      (labWasCheck?"REFERENCE TESTS NOT PASSED":"PROGRAM RAN SUCCESSFULLY")));
  const char *statusText=verified?"Your code matches the expected output.":
      (failed?"Read the messages below and adjust your solution.":"Run completed. Select Check Solution to verify your code.");
  /* Size the shared glow to cover the entire console, including all lines. */
  const char *countp=labResult.output;
  int outputLines=0;
  while(*countp&&outputLines<54){
   int n=0;while(countp[n]&&countp[n]!='\n'&&n<510)n++;
   outputLines++;countp+=n;
   if(*countp=='\n')countp++;
   else if(n==0)break;
  }
  int truncated=*countp!=0;
  float resultH=115+21.0f*outputLines+(truncated?23.0f:0.0f);
  resultGlow(x,y,w,resultH,statusColor);
  tx(statusTitle,x+18,y+14,13,statusColor);
  tx(statusText,x+18,y+43,13,CQ_WHITE);
  box(x+14,y+77,w-28,28,.12f,alpha(statusColor,lightTheme?.09f:.12f));
  tx("CONSOLE / COMPILER MESSAGES",x+27,y+84,12,statusColor);
  float outY=y+115;
  const char *p=labResult.output;int lines=0;
  while(*p&&lines<54){
   char line[512];int n=0;
   while(p[n]&&p[n]!='\n'&&n<510)n++;
   memcpy(line,p,(size_t)n);line[n]=0;
   tx(line,x+21,outY,13,CQ_WHITE);
   outY+=21;lines++;p+=n;
   if(*p=='\n')p++;
   else if(!n)break;
  }
  if(*p){tx("... console output truncated ...",x+21,outY,12,CQ_MUTED);}
  y+=resultH+22;
 }
 box(x,y,w,100,.07f,alpha(CQ_ORANGE,.08f));tx("HELPFUL HINT",x+18,y+11,12,CQ_ORANGE);paragraph(l->hint,x+18,y+39,14,CQ_WHITE,w-36);y+=120;
 if(progress.extraHint[selectedLab]){
  box(x,y,w,108,.08f,alpha(CQ_BLUE,.11f));tx("EXTRA HINT UNLOCKED",x+18,y+12,12,CQ_BLUE);
  paragraph(extraLabHints[selectedLab],x+18,y+42,14,CQ_WHITE,w-35);y+=128;
 }else{
  box(x,y,w,75,.08f,PANEL);
  tx("Need another clue? Unlock an extra hint for 100 XP.",x+17,y+13,14,CQ_WHITE);
  char wallet[72];snprintf(wallet,sizeof(wallet),"Available: %d XP",progress.xpWallet);tx(wallet,x+17,y+42,12,CQ_MUTED);
  if(btn("Unlock hint -100 XP",x+w-218,y+22,198,42,CQ_PURPLE,progress.xpWallet>=CQ_HINT_COST))spendHint(selectedLab);
  y+=95;
 }
 if(progress.labDone[selectedLab])pill("LAB VERIFIED  +75 XP",x,y,CQ_GREEN);
 else tx("Pass the reference tests for 75 XP (first success only).",x,y+8,12,CQ_MUTED);
 *bottom=y+70;
}
/* Stage 7 collectible visuals. Refined to a single cohesive line-icon system. */
static void stroke(float x1,float y1,float x2,float y2,float thick,Color c){
 Vector2 a=(Vector2){x1,y1},b=(Vector2){x2,y2};
 DrawLineEx(a,b,thick,c);
 float r=thick*0.5f;
 DrawCircleV(a,r,c);
 DrawCircleV(b,r,c);
}
static void iconCircle(float cx,float cy,float r,float t,Color c){
 DrawRing((Vector2){cx,cy},r-t*0.5f,r+t*0.5f,0,360,48,c);
}
static void iconHex(float cx,float cy,float r,float t,Color c){
 Vector2 pts[6];
 for(int i=0;i<6;i++){
  float a=-PI/2.0f + (float)i*PI/3.0f;
  pts[i]=(Vector2){cx+cosf(a)*r,cy+sinf(a)*r};
 }
 for(int i=0;i<6;i++)stroke(pts[i].x,pts[i].y,pts[(i+1)%6].x,pts[(i+1)%6].y,t,c);
}
static void iconNode(float x,float y,float r,Color c){
 DrawCircleV((Vector2){x,y},r,c);
}
static Color medalTint(int level){
 if(level>=10)return CQ_ORANGE;
 if(level>=7)return CQ_PURPLE;
 if(level>=4)return CQ_BLUE;
 return CQ_GREEN;
}
static void rankMedallion(float cx,float cy,float r,int level){
 Color c=medalTint(level);
 float t=2.8f;
 DrawCircleV((Vector2){cx,cy},r+8,alpha(c,.06f));
 DrawCircleV((Vector2){cx,cy},r,alpha(c,.12f));
 iconCircle(cx,cy,r-1,3.0f,alpha(c,.82f));
 iconCircle(cx,cy,r-10,1.8f,alpha(c,.34f));
 for(int k=0;k<8;k++){
  float a=-PI/2.0f + (float)k*PI/4.0f;
  float x1=cx+cosf(a)*(r-6), y1=cy+sinf(a)*(r-6);
  float x2=cx+cosf(a)*(r-12),y2=cy+sinf(a)*(r-12);
  stroke(x1,y1,x2,y2,2.0f,alpha(c,.9f));
 }
 iconHex(cx,cy,19,t,c);
 stroke(cx-11,cy,cx-3,cy-8,t,CQ_WHITE);
 stroke(cx-11,cy,cx-3,cy+8,t,CQ_WHITE);
 stroke(cx+11,cy,cx+3,cy-8,t,CQ_WHITE);
 stroke(cx+11,cy,cx+3,cy+8,t,CQ_WHITE);
 stroke(cx-1,cy-11,cx+1,cy+11,t,CQ_WHITE);
 int pips=level<1?1:(level>4?4:level);
 float px0=cx-(pips-1)*6.0f*0.5f;
 for(int i=0;i<pips;i++)DrawCircleV((Vector2){px0+i*6.0f,cy+26},2.5f,c);
}
static void badgeSymbol(int id,float cx,float cy,Color c){
 float t=2.6f;
 switch(id){
 case 0: /* first step / path flag */
  stroke(cx-12,cy+11,cx+10,cy+11,t,c);
  stroke(cx-8,cy+11,cx-8,cy-10,t,c);
  stroke(cx-8,cy-10,cx+8,cy-6,t,c);
  stroke(cx+8,cy-6,cx-8,cy-1,t,c);
  break;
 case 1: /* reading / open book */
  stroke(cx,cy-10,cx,cy+12,1.9f,c);
  stroke(cx-14,cy-8,cx-2,cy-10,t,c);
  stroke(cx-14,cy-8,cx-14,cy+10,t,c);
  stroke(cx-14,cy+10,cx-2,cy+8,t,c);
  stroke(cx+14,cy-8,cx+2,cy-10,t,c);
  stroke(cx+14,cy-8,cx+14,cy+10,t,c);
  stroke(cx+14,cy+10,cx+2,cy+8,t,c);
  break;
 case 2: /* first lab / terminal */
  DrawRectangleRoundedLinesEx(RR(cx-17,cy-12,34,24),.18f,6,2,c);
  stroke(cx-10,cy-4,cx-4,cy+1,t,c);
  stroke(cx-10,cy+6,cx-4,cy+1,t,c);
  stroke(cx+2,cy+6,cx+10,cy+6,t,c);
  break;
 case 3: /* solutions / circled check */
  iconCircle(cx,cy,14,2.1f,c);
  stroke(cx-7,cy+1,cx-1,cy+7,t,c);
  stroke(cx-1,cy+7,cx+9,cy-5,t,c);
  break;
 case 4: /* perfect quiz / clean star */
  for(int k=0;k<10;k++){
   float a=-PI/2.0f + (float)k*PI/5.0f;
   float b=-PI/2.0f + (float)(k+1)*PI/5.0f;
   float ra=(k%2==0)?15.0f:6.5f;
   float rb=((k+1)%2==0)?15.0f:6.5f;
   stroke(cx+cosf(a)*ra,cy+sinf(a)*ra,cx+cosf(b)*rb,cy+sinf(b)*rb,2.2f,c);
  }
  break;
 case 5: /* pointer master / clean cursor arrow */
  /* One clear pointer silhouette, with rounded stroke joins. */
  stroke(cx-10,cy-14,cx-10,cy+11,t,c);
  stroke(cx-10,cy+11,cx-3,cy+5,t,c);
  stroke(cx-3,cy+5,cx+2,cy+15,t,c);
  stroke(cx+2,cy+15,cx+8,cy+12,t,c);
  stroke(cx+8,cy+12,cx+3,cy+2,t,c);
  stroke(cx+3,cy+2,cx+13,cy+1,t,c);
  stroke(cx+13,cy+1,cx-10,cy-14,t,c);
  break;
 case 6: /* level 5 / upward growth */
  stroke(cx-15,cy+11,cx+14,cy+11,1.9f,c);
  DrawRectangleRoundedLinesEx(RR(cx-13,cy+4,6,7),.28f,5,1.8f,c);
  DrawRectangleRoundedLinesEx(RR(cx-3,cy-1,6,12),.28f,5,1.8f,c);
  DrawRectangleRoundedLinesEx(RR(cx+7,cy-7,6,18),.28f,5,1.8f,c);
  stroke(cx-10,cy-7,cx+10,cy-15,2.0f,c);
  stroke(cx+10,cy-15,cx+6,cy-15,2.0f,c);
  stroke(cx+10,cy-15,cx+8,cy-11,2.0f,c);
  break;
 case 7: /* codequest champion / crown */
  stroke(cx-14,cy+8,cx+14,cy+8,2.0f,c);
  stroke(cx-14,cy+8,cx-11,cy-7,2.0f,c);
  stroke(cx-11,cy-7,cx-4,cy+1,2.0f,c);
  stroke(cx-4,cy+1,cx,cy-11,2.0f,c);
  stroke(cx,cy-11,cx+4,cy+1,2.0f,c);
  stroke(cx+4,cy+1,cx+11,cy-7,2.0f,c);
  stroke(cx+11,cy-7,cx+14,cy+8,2.0f,c);
  DrawCircleV((Vector2){cx-11,cy-7},2.2f,c);
  DrawCircleV((Vector2){cx,cy-11},2.2f,c);
  DrawCircleV((Vector2){cx+11,cy-7},2.2f,c);
  break;
 case 8: /* bug hunter / simple standalone bug */
  /* Rounded shell, distinct head, six symmetrical legs. */
  DrawRectangleRoundedLinesEx(RR(cx-9,cy-4,18,20),.48f,12,2.2f,c);
  iconCircle(cx,cy-9,5.5f,2.1f,c);
  stroke(cx,cy-4,cx,cy+14,1.7f,c);
  stroke(cx-8,cy+1,cx-15,cy-3,2.0f,c);
  stroke(cx+8,cy+1,cx+15,cy-3,2.0f,c);
  stroke(cx-9,cy+6,cx-16,cy+6,2.0f,c);
  stroke(cx+9,cy+6,cx+16,cy+6,2.0f,c);
  stroke(cx-8,cy+11,cx-14,cy+15,2.0f,c);
  stroke(cx+8,cy+11,cx+14,cy+15,2.0f,c);
  stroke(cx-3,cy-13,cx-6,cy-16,1.8f,c);
  stroke(cx+3,cy-13,cx+6,cy-16,1.8f,c);
  break;
 }
}
static void badgeMedal(int id,float cx,float cy,int earned,Color c){
 Color ink=earned?c:CQ_MUTED;
 Color rim=earned?alpha(c,.82f):alpha(CQ_LINE,.85f);
 DrawCircleV((Vector2){cx,cy},32,earned?alpha(c,.11f):alpha(CQ_MUTED,.04f));
 iconCircle(cx,cy,29,2.3f,rim);
 iconCircle(cx,cy,23,1.4f,alpha(ink,.26f));
 badgeSymbol(id,cx,cy,ink);
 if(earned){
  DrawCircleV((Vector2){cx+22,cy+22},8.5f,CQ_GREEN);
  stroke(cx+18,cy+22,cx+21,cy+25,2.0f,textOnAccent);
  stroke(cx+21,cy+25,cx+26,cy+19,2.0f,textOnAccent);
 }
}
static void progressView(float x,float w,float *bottom){
 float y=95;
 txH("Developer profile",x,y,30,CQ_WHITE);
 tx("Your experience, ranks and milestones in one place.",x,y+44,15,CQ_MUTED);y+=94;
 int level=cqLevel(),first=cqLevelStart(level),next=cqLevelNext();
 float xpRatio=next>first?(float)(progress.xp-first)/(float)(next-first):1.0f;
 Color rankColor=medalTint(level);
 float profileH=189;
 box(x,y,w,profileH,.09f,PANEL);
 border(x,y,w,profileH,.09f,alpha(rankColor,.37f));
 box(x+16,y+15,142,profileH-30,.10f,alpha(rankColor,.065f));
 rankMedallion(x+87,y+95,53,level);
 char s[160];
 tx("CURRENT RANK",x+181,y+21,12,rankColor);
 txH(cqRank(level),x+181,y+44,24,CQ_WHITE);
 snprintf(s,sizeof(s),"LEVEL %d",level);pill(s,x+181,y+81,rankColor);
 snprintf(s,sizeof(s),"%d / %d XP to next level",progress.xp-first,next-first);
 tx(s,x+181,y+121,13,CQ_MUTED);
 bar(x+181,y+148,w-205,xpRatio,rankColor);
 snprintf(s,sizeof(s),"%d total XP",progress.xp);tx(s,x+181,y+165,11,CQ_MUTED);
 snprintf(s,sizeof(s),"%d available for hints",progress.xpWallet);
 float walletWidth=tw(s,11);
 if(w-walletWidth>205)tx(s,x+w-walletWidth-23,y+165,11,CQ_MUTED);
 y+=216;
 int unlocked=0;for(int i=0;i<9;i++)unlocked+=achievementUnlocked(i);
 txH("Achievements",x,y,24,CQ_WHITE);
 snprintf(s,sizeof(s),"%d / 9 collected",unlocked);
 tx(s,x+w-tw(s,14),y+8,14,CQ_MUTED);y+=48;
 box(x,y,w,5,1,alpha(CQ_LINE,.65f));
 if(unlocked)box(x,y,w*(float)unlocked/9.0f,5,1,CQ_GREEN);
 y+=25;
 const char *names[]={"First Steps","Bookworm","First Solution","Problem Solver","Perfect Score","Pointer Master","Level Five","CodeQuest Champion","Bug Hunter"};
 const char *descs[]={"Complete your first lesson","Complete 10 lessons","Verify your first coding lab","Verify 5 coding labs","Score 100% on a quiz","Complete Pointers & Memory","Reach level 5","Finish all five learning tracks","Master 12 debugging challenges"};
 Color tones[]={CQ_BLUE,CQ_PURPLE,CQ_GREEN,CQ_BLUE,CQ_ORANGE,CQ_PURPLE,CQ_ORANGE,CQ_ORANGE,CQ_GREEN};
 int cols=w>=900?3:2;
 float gap=13,cellW=(w-gap*(cols-1))/cols,cellH=112;
 for(int i=0;i<9;i++){
  int col=i%cols,row=i/cols,earned=achievementUnlocked(i);
  float xx=x+col*(cellW+gap), yy=y+row*(cellH+gap);
  Color accent=tones[i];
  Rectangle area=RR(xx,yy,cellW,cellH);
  int hovered=CheckCollisionPointRec(pointer(),clickRect(area));
  badgeHovers[i]=hoverEase(badgeHovers[i],hovered,10.0f);
  float ah=badgeHovers[i];
  box(xx,yy,cellW,cellH,.09f,earned?alpha(accent,.085f+.045f*ah):mixColor(PANEL,CARD,.13f*ah));
  border(xx,yy,cellW,cellH,.09f,earned?alpha(accent,.39f+.32f*ah):mixColor(alpha(CQ_LINE,.65f),alpha(accent,.48f),ah));
  if(ah>.005f)DrawCircleV((Vector2){xx+45,yy+54},36,alpha(accent,.05f*ah));
  badgeMedal(i,xx+45,yy+54,earned,accent);
  float textX=xx+86;
  tx(names[i],textX,yy+26,14,earned?CQ_WHITE:CQ_MUTED);
  float ty=yy+53;
  paragraph(descs[i],textX,ty,11,CQ_MUTED,cellW-101);
  tx(earned?"UNLOCKED":"LOCKED",textX,yy+92,10,earned?CQ_GREEN:CQ_MUTED);
 }
 y+=((9+cols-1)/cols)*(cellH+gap)+12;
 txH("How XP works",x,y,23,CQ_WHITE);y+=42;
 box(x,y,w,191,.08f,PANEL);
 tx("Lesson +30    |    New correct quiz answer +20    |    Verified lab +75",x+18,y+19,15,CQ_WHITE);
 tx("First quiz pass (70%+) +50    |    First perfect quiz +25",x+18,y+53,15,CQ_WHITE);
 tx("Complete every lesson, lab and quiz question in a track +150",x+18,y+87,15,CQ_WHITE);
 tx("Debug: first correct fix +20  |  first 70%+ session +50  |  first perfect +25",x+18,y+120,13,CQ_WHITE);
 tx("Extra hint -100 spendable XP. Rewards are granted once per milestone.",x+18,y+153,13,CQ_MUTED);
 y+=217;
 txH("Learning tracks",x,y,23,CQ_WHITE);y+=43;
 for(int t=0;t<COUNT(tracks);t++){
  int done=trackDone(t),all=trackLessonCount(t);
  box(x,y,w,82,.09f,PANEL);DrawCircle(x+28,y+32,7,tracks[t].color);
  tx(tracks[t].name,x+46,y+15,17,CQ_WHITE);
  snprintf(s,sizeof(s),"%d / %d lessons%s",done,all,trackComplete(t)?"  |  TRACK MASTERED":"");
  tx(s,x+w-264,y+18,13,CQ_MUTED);
  bar(x+46,y+56,w-92,(float)done/all,tracks[t].color);y+=95;
 }
 y+=20;
 box(x,y,w,147,.08f,PANEL);tx("Quiz performance",x+22,y+20,22,CQ_WHITE);
 float accuracy=progress.quizAttempts?100.0f*progress.quizCorrect/progress.quizAttempts:0.0f;
 snprintf(s,sizeof(s),"%.0f%%",accuracy);tx(s,x+22,y+59,29,CQ_GREEN);
 snprintf(s,sizeof(s),"%d correct / %d attempts",progress.quizCorrect,progress.quizAttempts);
 tx(s,x+123,y+70,15,CQ_MUTED);bar(x+22,y+119,w-44,accuracy/100,CQ_GREEN);
 y+=179;
 int weakest=0;float ratio=2;
 for(int t=0;t<COUNT(tracks);t++){float r=(float)trackDone(t)/trackLessonCount(t);if(r<ratio){ratio=r;weakest=t;}}
 txH("Suggested next step",x,y,23,CQ_WHITE);y+=39;
 box(x,y,w,106,.08f,alpha(tracks[weakest].color,.11f));
 tx(tracks[weakest].name,x+20,y+20,20,CQ_WHITE);
 if(btn("Study this track  >",x+w-208,y+35,187,44,tracks[weakest].color,1)){selectedTrack=weakest;nav(1);}
 *bottom=y+133;
}

void RunApp(void){
 SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);InitWindow(windowW,windowH,"CodeQuest | C & C++ Learning Studio");SetWindowMinSize(1000,650);SetTargetFPS(60);celebrationInit();
 /* Optional OS font. No images, textures, fonts, or other assets required. */
 const char *paths[]={"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf","/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf","C:/Windows/Fonts/segoeui.ttf","/System/Library/Fonts/Supplemental/Arial.ttf"};
 font=GetFontDefault();for(int i=0;i<COUNT(paths);i++){if(FileExists(paths[i])){Font loaded=LoadFontEx(paths[i],40,NULL,0);if(loaded.texture.id){font=loaded;SetTextureFilter(font.texture,TEXTURE_FILTER_BILINEAR);break;}}}
 /* Optional visual hierarchy: headings and code use local system fonts.
  * Fallbacks remain safe when these fonts are unavailable on another OS. */
 headingFont=font;codeFont=font;
 const char *headingPaths[]={
  "C:/Windows/Fonts/seguisb.ttf",
  "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
  "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf"
 };
 const char *monoPaths[]={
  "C:/Windows/Fonts/consola.ttf",
  "/System/Library/Fonts/Supplemental/Menlo.ttc",
  "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
  "/usr/share/fonts/truetype/liberation2/LiberationMono-Regular.ttf"
 };
 for(int i=0;i<COUNT(headingPaths);i++)if(FileExists(headingPaths[i])){
  Font f=LoadFontEx(headingPaths[i],42,NULL,0);
  if(f.texture.id){headingFont=f;ownsHeadingFont=1;SetTextureFilter(f.texture,TEXTURE_FILTER_BILINEAR);break;}
 }
 for(int i=0;i<COUNT(monoPaths);i++)if(FileExists(monoPaths[i])){
  Font f=LoadFontEx(monoPaths[i],38,NULL,0);
  if(f.texture.id){codeFont=f;ownsCodeFont=1;SetTextureFilter(f.texture,TEXTURE_FILTER_BILINEAR);break;}
 }
 loadProgress();checkTrackRewards();loadTheme();
 while(!WindowShouldClose()){
  /* F11 toggles fullscreen. Ctrl + plus/minus adjusts the UI zoom for
   * accessibility; Ctrl + 0 resets it. */
  if(IsKeyPressed(KEY_F11))ToggleFullscreen();
  /* Alt+1..5 jumps between primary destinations; Alt+B collapses the rail. */
  if(IsKeyDown(KEY_LEFT_ALT)||IsKeyDown(KEY_RIGHT_ALT)){
   const int keys[]={KEY_ONE,KEY_TWO,KEY_THREE,KEY_FOUR,KEY_FIVE};
   for(int i=0;i<5;i++)if(IsKeyPressed(keys[i]))nav(i);
   if(IsKeyPressed(KEY_B))navCollapsed=!navCollapsed;
  }
  if(IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL)||
     IsKeyDown(KEY_LEFT_SUPER)||IsKeyDown(KEY_RIGHT_SUPER)){
   if(IsKeyPressed(KEY_EQUAL)||IsKeyPressed(KEY_KP_ADD))userZoom=fminf(1.5f,userZoom+0.1f);
   if(IsKeyPressed(KEY_MINUS)||IsKeyPressed(KEY_KP_SUBTRACT))userZoom=fmaxf(0.8f,userZoom-0.1f);
   if(IsKeyPressed(KEY_ZERO)||IsKeyPressed(KEY_KP_0))userZoom=1.0f;
  }
  int physicalW=GetScreenWidth(),physicalH=GetScreenHeight();
  uiScale=computeScale(physicalW,physicalH);
  windowW=(int)(physicalW/uiScale);windowH=(int)(physicalH/uiScale);
  SetMouseCursor(MOUSE_CURSOR_DEFAULT);
  if(IsKeyPressed(KEY_ESCAPE)&&!ExamTimeoutPopupVisible()){if(screen==2&&examMode){/* Exit only via exam review so answers are not lost. */}else if(screen==2&&debugMode){debugMode=0;scroll=0;}else if(screen==2&&quizTrack>=0){quizTrack=-1;quizFinished=0;scroll=0;}else if(selectedLesson>=0){selectedLesson=-1;scroll=0;}else if(selectedLab>=0){labSaveDraft(selectedLab,labCode);selectedLab=-1;labFocus=0;scroll=0;}else if(screen)nav(0);}
  if(examMode)ExamUpdate();
  /* Navigation rail can be expanded or collapsed without affecting scroll state. */
  float side=navCollapsed?76.0f:204.0f;
  float x=side+32,w=windowW-side-66;if(w<450)w=450;
  if(CheckCollisionPointRec(pointer(),RR(side,80,(float)windowW-side,(float)windowH-80))&&
     !(screen==3&&selectedLab>=0&&CheckCollisionPointRec(pointer(),labEditorWheelRect))){scroll-=GetMouseWheelMove()*45;}
  if(scroll<0)scroll=0;
  if(scroll>maxScroll)scroll=maxScroll;
  pendingNavTooltip=NULL; /* Reset hover overlay for this frame. */
  hoverFrame++;
  /* React to destination/detail changes, including direct chapter shortcuts. */
  int pageSignature=(screen+1)*100000+(selectedLesson+1)*200+(selectedLab+1)*2;
  pageSignature+=screen==2?(quizTrack+2)*41+(debugMode?31:0)+(examMode?67:0):0;
  if(lastPageSignature<0)lastPageSignature=pageSignature;
  if(pageSignature!=lastPageSignature){pageReveal=.62f;lastPageSignature=pageSignature;}
  pageReveal=fmaxf(0.0f,pageReveal-GetFrameTime()*4.8f);
  BeginDrawing();ClearBackground(BG);
  /* Draw fixed navigation at the same scale as the content. Avoid nested
   * BeginMode2D calls: a nested camera replaces (rather than composes with)
   * the previous camera, which made the content tiny in fullscreen. */
  Camera2D fixedCamera={.offset={0,0},.target={0,0},.rotation=0,.zoom=uiScale};
  BeginMode2D(fixedCamera);
  sidebar(side);
  const char *titles[]={"HOME  /  YOUR JOURNEY","LEARN  /  CURRICULUM","PRACTICE  /  QUIZZES","BUILD  /  CODING LABS","PROFILE  /  YOUR PROGRESS","PRACTICE  /  DAILY CHALLENGES"};
  header(x,w,titles[screen]);
  EndMode2D();

  /* Scissor coordinates use Raylib window coordinates. Raylib handles any
   * additional high-DPI framebuffer conversion internally. */
  int clipX=(int)((side+1)*uiScale);
  int clipY=(int)(81*uiScale);
  BeginScissorMode(clipX,clipY,physicalW-clipX,physicalH-clipY);
  float bottom=0;
  uiScrollActive=1;
  Camera2D contentCamera={.offset={0,-scroll*uiScale},.target={0,0},.rotation=0,.zoom=uiScale};
  BeginMode2D(contentCamera);
  if(screen==0)overview(x,w,&bottom);
  else if(screen==1){if(selectedLesson>=0)lessonView(x,w,&bottom);else learnList(x,w,&bottom);}
  else if(screen==2)quizView(x,w,&bottom);
  else if(screen==3){if(selectedLab>=0)labView(x,w,&bottom);else labList(x,w,&bottom);}
  else if(screen==4)progressView(x,w,&bottom);
  else dailyChallengeView(x,w,&bottom);
  EndMode2D();
  uiScrollActive=0;
  EndScissorMode();
  /* Very short content fade on destination changes. It never blocks input
   * and does not fade the fixed navigation or overlay tooltips. */
  if(pageReveal>.005f){
   BeginMode2D(fixedCamera);
   DrawRectangle((int)(side+1),81,windowW-(int)side-1,windowH-81,alpha(BG,pageReveal));
   EndMode2D();
  }

  /* Confetti is rendered above the scrolling content in logical screen coordinates. */
  BeginMode2D(fixedCamera);
  celebrationDraw(GetFrameTime());
  EndMode2D();

  maxScroll=fmaxf(0,bottom-windowH+38);if(scroll>maxScroll)scroll=maxScroll;
  if(maxScroll>0){
   BeginMode2D(fixedCamera);
   float trackHeight=windowH-115;
   float thumb=fmaxf(33,trackHeight*trackHeight/(trackHeight+maxScroll));
   float sy=95+(scroll/maxScroll)*(trackHeight-thumb);
   box(windowW-10,95,4,trackHeight,1,CQ_LINE);
   box(windowW-10,sy,4,thumb,1,CQ_PURPLE);
   EndMode2D();
  }
  /* Draw AFTER the page content and its scissor clip, so icon tooltips
   * always float above Home, lessons, quizzes and lab panels. */
  BeginMode2D(fixedCamera);
  drawNavTooltipOverlay();
  if(examMode&&ExamTimeoutPopupVisible()){
   ExamStyle style={.mouse=pointer(),.font=font,.headingFont=headingFont,
    .panel=PANEL,.text=CQ_WHITE,.muted=CQ_MUTED,.orange=CQ_ORANGE,
    .accent=CQ_PURPLE,.accentText=textOnAccent};
   ExamDrawTimeoutPopup(&style,windowW,windowH);
  }
  EndMode2D();
  EndDrawing();
 }
 if(labLoaded>=0)labSaveDraft(labLoaded,labCode);
 persist();
 celebrationClose();
 if(ownsCodeFont)UnloadFont(codeFont);
 if(ownsHeadingFont)UnloadFont(headingFont);
 if(font.texture.id!=GetFontDefault().texture.id)UnloadFont(font);
 CloseWindow();
}
