/* CODEQUEST - C/C++ learning app. C11 + raylib only.
 * Build: make (macOS) or Ctrl+Shift+B in VS Code (Windows GCC)
 * UI and quiz interactions live here; curriculum data and progress live in separate C modules.
 */
#include "raylib.h"
#include "curriculum.h"
#include "progress.h"
#include "app.h"
#include "lab_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define COUNT(a) ((int)(sizeof(a)/sizeof((a)[0])))

static Color BG={13,18,31,255}, PANEL={23,30,47,255}, CARD={29,38,59,255};
static Color CQ_WHITE={239,244,255,255}, CQ_MUTED={151,165,190,255}, CQ_LINE={51,64,87,255};
static Color CQ_PURPLE={141,111,255,255}, CQ_GREEN={88,222,165,255}, CQ_ORANGE={255,182,106,255}, CQ_RED={255,117,139,255}, CQ_BLUE={106,187,255,255};
static int lightTheme=0;
static float trackScroll=0;
static Color textOnAccent={13,18,31,255};
static void applyTheme(void){
 if(lightTheme){
  BG=(Color){246,248,253,255}; PANEL=(Color){232,237,247,255}; CARD=(Color){219,227,242,255};
  CQ_WHITE=(Color){29,39,61,255}; CQ_MUTED=(Color){91,105,128,255}; CQ_LINE=(Color){191,201,220,255};
  CQ_PURPLE=(Color){105,75,205,255}; CQ_GREEN=(Color){22,137,93,255}; CQ_ORANGE=(Color){177,93,23,255};
  CQ_RED=(Color){192,57,78,255}; CQ_BLUE=(Color){41,109,181,255};
  textOnAccent=(Color){255,255,255,255};
 }else{
  BG=(Color){13,18,31,255}; PANEL=(Color){23,30,47,255}; CARD=(Color){29,38,59,255};
  CQ_WHITE=(Color){239,244,255,255}; CQ_MUTED=(Color){151,165,190,255}; CQ_LINE=(Color){51,64,87,255};
  CQ_PURPLE=(Color){141,111,255,255}; CQ_GREEN=(Color){88,222,165,255}; CQ_ORANGE=(Color){255,182,106,255};
  CQ_RED=(Color){255,117,139,255}; CQ_BLUE=(Color){106,187,255,255};
  textOnAccent=BG;
 }
}
static void loadTheme(void){FILE*f=fopen("codequest_theme.cfg","r");if(f){int mode=0;if(fscanf(f,"%d",&mode)==1)lightTheme=(mode==1);fclose(f);}applyTheme();}
static void toggleTheme(void){lightTheme=!lightTheme;applyTheme();FILE*f=fopen("codequest_theme.cfg","w");if(f){fprintf(f,"%d\n",lightTheme);fclose(f);}}
static Font font={0};
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
static int quizTrack=-1, quizPosition=0, quizCount=0;
static int quizQuestionIds[30], quizSelections[30];
static int quizChoice=-1, quizAnswered=0, quizFinished=0;

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
static int uiScrollActive=0;
static char searchText[96]="";
static int searchActive=0;

typedef struct {float x,y,w,h;} Rect;
static Rectangle RR(float x,float y,float w,float h){return (Rectangle){x,y,w,h};}
static Color alpha(Color c,float a){c.a=(unsigned char)(255.0f*a);return c;}
static Rectangle clickRect(Rectangle r){if(uiScrollActive)r.y-=scroll;return r;}
static int hit(Rectangle r){return CheckCollisionPointRec(pointer(),clickRect(r)) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);}
static void tx(const char*s,float x,float y,int size,Color color){DrawTextEx(font,s,(Vector2){x,y},(float)size,0.4f,color);}
static float tw(const char*s,int size){return MeasureTextEx(font,s,(float)size,0.4f).x;}
static void box(float x,float y,float w,float h,float round,Color c){DrawRectangleRounded(RR(x,y,w,h),round,12,c);}
static void border(float x,float y,float w,float h,float round,Color c){DrawRectangleRoundedLinesEx(RR(x,y,w,h),round,12,1.1f,c);}
static int btn(const char *text,float x,float y,float w,float h,Color color,int filled){
 Rectangle r=RR(x,y,w,h);int hover=CheckCollisionPointRec(pointer(),clickRect(r));
 box(x,y,w,h,.20f,filled?(hover?ColorBrightness(color,.12f):color):(hover?CARD:PANEL));
 if(!filled)border(x,y,w,h,.20f,CQ_LINE);
 tx(text,x+(w-tw(text,15))/2,y+(h-17)/2,15,filled?textOnAccent:CQ_WHITE);
 if(hover)SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
 return hit(r);
}
static void pill(const char*s,float x,float y,Color c){float w=tw(s,11)+24;box(x,y,w,25,.45f,alpha(c,.14f));tx(s,x+12,y+6,11,c);}
static void bar(float x,float y,float w,float value,Color c){box(x,y,w,7,1,CQ_LINE);if(value>0)box(x,y,fmaxf(6,w*fminf(value,1)),7,1,c);}
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
 float height=54+lines*23;box(x,y,w,height,.06f,lightTheme?(Color){224,233,247,255}:(Color){12,21,34,255});
 box(x+20,y+17,8,8,1,CQ_RED);box(x+35,y+17,8,8,1,CQ_ORANGE);box(x+50,y+17,8,8,1,CQ_GREEN);
 tx("CODE EXAMPLE",x+w-143,y+13,11,CQ_MUTED);
 char copy[4500];snprintf(copy,sizeof(copy),"%s",code);char*line=strtok(copy,"\n");int i=0;
 while(line){Color c=CQ_WHITE;if(strstr(line,"//")||strstr(line,"/*"))c=CQ_MUTED;
 tx(line,x+22,y+47+i*23,15,c);i++;line=strtok(NULL,"\n");}
 return height;
}
static int totalLearned(void){int n=0;for(int i=0;i<COUNT(lessons);i++)n+=progress.learned[i]!=0;return n;}
static int totalLabs(void){int n=0;for(int i=0;i<COUNT(labTasks);i++)n+=progress.labDone[i]!=0;return n;}
static int totalQuizzes(void){int n=0;for(int i=0;i<COUNT(quizzes);i++)n+=progress.quizDone[i]!=0;return n;}
static int trackLessonCount(int t){int n=0;for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t)n++;return n;}
static int trackDone(int t){int n=0;for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t)n+=progress.learned[i]!=0;return n;}
static void nav(int s){if(labLoaded>=0)labSaveDraft(labLoaded,labCode);labFocus=0;screen=s;selectedLesson=-1;selectedLab=-1;scroll=0;searchActive=0;if(s==2){quizTrack=-1;quizFinished=0;}}
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
static void sidebar(float side){
 DrawRectangle(0,0,(int)side,windowH,PANEL);DrawLine((int)side,0,(int)side,windowH,CQ_LINE);
 box(26,25,42,42,.25f,CQ_PURPLE);tx("{ }",33,34,19,textOnAccent);tx("codequest",82,28,24,CQ_WHITE);tx("LEARN C / C++",83,56,10,CQ_MUTED);
 drawThemeSwitch(side-110,77);
 tx("WORKSPACE",29,115,11,CQ_MUTED);
 const char*names[]={"Overview","Learn","Practice quizzes","Coding labs","My progress"};const char*icons[]={"01","02","03","04","05"};
 for(int i=0;i<5;i++){float y=143+i*57;Rectangle r=RR(17,y,side-34,47);int active=screen==i;box(r.x,r.y,r.width,r.height,.18f,active?alpha(CQ_PURPLE,.22f):(CheckCollisionPointRec(pointer(),r)?CARD:PANEL));
  if(active)box(18,y+8,3,31,1,CQ_PURPLE);
  tx(icons[i],31,y+15,13,active?CQ_PURPLE:CQ_MUTED);tx(names[i],77,y+13,16,active?CQ_WHITE:CQ_MUTED);
  if(hit(r))nav(i);
 }
 /* Track list has its own clipping and wheel scroll so the fixed journey
  * summary never obscures lower tracks on small/windowed displays. */
 const float listTop=452.0f;
 const float listBottom=(float)windowH-153.0f;
 float listHeight=fmaxf(20.0f,listBottom-listTop);
 float listContent=(float)COUNT(tracks)*47.0f;
 float listMax=fmaxf(0.0f,listContent-listHeight);
 if(trackScroll>listMax)trackScroll=listMax;
 Rectangle listArea=RR(20,listTop,side-40,listHeight);
 if(CheckCollisionPointRec(pointer(),listArea)){
  trackScroll-=GetMouseWheelMove()*32.0f;
  if(trackScroll<0)trackScroll=0;
  if(trackScroll>listMax)trackScroll=listMax;
 }
 tx("LEARNING TRACKS",29,426,11,CQ_MUTED);
 BeginScissorMode((int)(listArea.x*uiScale),(int)(listArea.y*uiScale),
                  (int)(listArea.width*uiScale),(int)(listArea.height*uiScale));
 for(int t=0;t<COUNT(tracks);t++){
  float y=listTop+t*47-trackScroll;
  Rectangle r=RR(24,y,side-57,39);
  if(y+39<listTop||y>listBottom)continue;
  if(selectedTrack==t&&screen==1)box(r.x,r.y,r.width,r.height,.18f,alpha(tracks[t].color,.17f));
  DrawCircle(36,y+19,6,tracks[t].color);
  tx(tracks[t].name,53,y+9,14,CQ_WHITE);
  if(CheckCollisionPointRec(pointer(),r)&&hit(r)){selectedTrack=t;nav(1);}
 }
 EndScissorMode();
 if(listMax>0){
  float thumb=fmaxf(16,listHeight*listHeight/listContent);
  box(side-17,listTop,3,listHeight,1,CQ_LINE);
  box(side-17,listTop+(trackScroll/listMax)*(listHeight-thumb),3,thumb,1,CQ_PURPLE);
 }
 box(18,windowH-136,side-36,111,.14f,CARD);tx("YOUR JOURNEY",35,windowH-117,11,CQ_MUTED);
 char str[96];snprintf(str,sizeof(str),"Level %d  /  %d XP",1+progress.xp/250,progress.xp);tx(str,35,windowH-91,18,CQ_WHITE);
 bar(35,windowH-57,side-72,(progress.xp%250)/250.0f,CQ_PURPLE);
}
static void header(float x,float width,const char*section){
 tx(section,x,37,14,CQ_MUTED);
 char s[60];snprintf(s,sizeof(s),"%d XP",progress.xp);box(x+width-158,25,144,43,.28f,CARD);
 DrawCircle(x+width-131,46,10,CQ_ORANGE);tx("*",x+width-136,36,20,textOnAccent);tx(s,x+width-111,36,17,CQ_WHITE);
}
static void overview(float x,float w,float *bottom){
 float y=96;
 box(x,y,w,218,.05f,lightTheme?(Color){226,220,250,255}:(Color){43,39,88,255});DrawCircle(x+w-105,y+105,105,alpha(CQ_PURPLE,.11f));DrawCircle(x+w-105,y+105,72,alpha(CQ_BLUE,.08f));
 pill("YOUR LEARNING SPACE",x+27,y+26,CQ_PURPLE);tx("Build skills. Ship code.",x+27,y+64,32,CQ_WHITE);
 tx("Master C and C++ with lessons, real labs and rewards.",x+27,y+112,16,CQ_MUTED);
 if(btn("Continue learning  >",x+27,y+151,208,46,CQ_GREEN,1))nav(1);
 tx("</>",x+w-195,y+62,57,alpha(CQ_WHITE,.83f));y+=240;
 float cw=(w-36)/3;const char*labels[]={"LESSONS COMPLETED","QUIZZES MASTERED","LABS FINISHED"};
 int vals[]={totalLearned(),totalQuizzes(),totalLabs()};int totals[]={COUNT(lessons),COUNT(quizzes),COUNT(labTasks)};Color cs[]={CQ_BLUE,CQ_PURPLE,CQ_GREEN};
 for(int i=0;i<3;i++){float xx=x+i*(cw+18);box(xx,y,cw,132,.08f,PANEL);tx(labels[i],xx+21,y+22,12,CQ_MUTED);char s[50];snprintf(s,sizeof(s),"%d / %d",vals[i],totals[i]);tx(s,xx+21,y+46,30,CQ_WHITE);bar(xx+21,y+106,cw-42,(float)vals[i]/totals[i],cs[i]);} y+=159;
 tx("Your learning tracks",x,y,23,CQ_WHITE);tx("Pick a path and keep your momentum going",x,y+34,14,CQ_MUTED);y+=73;
 for(int t=0;t<COUNT(tracks);t++){float yy=y+t*91;box(x,yy,w,75,.10f,PANEL);box(x+15,yy+14,46,46,.21f,alpha(tracks[t].color,.18f));
  char num[10];snprintf(num,sizeof(num),"0%d",t+1);tx(num,x+28,yy+25,17,tracks[t].color);tx(tracks[t].name,x+78,yy+13,18,CQ_WHITE);tx(tracks[t].subtitle,x+78,yy+42,13,CQ_MUTED);
  int count=trackLessonCount(t),done=trackDone(t);char p[50];snprintf(p,sizeof(p),"%d / %d lessons",done,count);tx(p,x+w-170,yy+14,13,CQ_MUTED);bar(x+w-170,yy+43,115,(float)done/count,tracks[t].color);
  if(hit(RR(x,yy,w,75))){selectedTrack=t;nav(1);}
 }y+=COUNT(tracks)*91+15;*bottom=y;
}
static void searchBox(float x,float y,float w){Rectangle r=RR(x,y,w,46);box(x,y,w,46,.15f,PANEL);border(x,y,w,46,.15f,searchActive?CQ_PURPLE:CQ_LINE);tx("/",x+16,y+12,18,CQ_PURPLE);tx(searchText[0]?searchText:"Search lessons or topics...",x+40,y+13,15,searchText[0]?CQ_WHITE:CQ_MUTED);
 if(hit(r))searchActive=1;
 if(searchActive){int key=GetCharPressed();while(key){int n=(int)strlen(searchText);if(key>=32 && key<127 && n<90){searchText[n]=(char)key;searchText[n+1]=0;}key=GetCharPressed();}if(IsKeyPressed(KEY_BACKSPACE)&&strlen(searchText))searchText[strlen(searchText)-1]=0;if(IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_ESCAPE))searchActive=0;}
}
static void learnList(float x,float w,float *bottom){float y=95;tx("Explore the curriculum",x,y,30,CQ_WHITE);tx("Short, focused chapters inspired by your lecture material.",x,y+43,15,CQ_MUTED);y+=89;
 searchBox(x,y,w);y+=68;
 if(btn("All topics",x,y,115,36,selectedTrack<0?CQ_PURPLE:CQ_LINE,selectedTrack<0))selectedTrack=-1;
 for(int t=0;t<COUNT(tracks);t++){float xx=x+126+t*128; if(xx+123>x+w)break; char label[29];snprintf(label,sizeof(label),"%s",t==0?"Foundations":t==1?"Pointers":t==2?"Linked lists":t==3?"C++": "STL");if(btn(label,xx,y,121,36,tracks[t].color,selectedTrack==t))selectedTrack=t;}
 y+=61;
 for(int t=0;t<COUNT(tracks);t++){if(selectedTrack>=0&&t!=selectedTrack)continue;int shown=0;for(int i=0;i<COUNT(lessons);i++)if(lessons[i].track==t&&contains(lessons[i].title,searchText))shown++;
  if(!shown)continue;
  char s[96];snprintf(s,sizeof(s),"%s  /  %d chapters",tracks[t].name,shown);tx(s,x,y,20,CQ_WHITE);y+=37;
  for(int i=0;i<COUNT(lessons);i++){if(lessons[i].track!=t||!contains(lessons[i].title,searchText))continue;
   box(x,y,w,86,.08f,PANEL);box(x+17,y+16,52,52,.15f,alpha(tracks[t].color,.18f));char id[12];snprintf(id,sizeof(id),"%02d",i+1);tx(id,x+29,y+31,17,tracks[t].color);
   tx(lessons[i].title,x+86,y+15,19,CQ_WHITE);tx(lessons[i].tag,x+86,y+48,12,CQ_MUTED);if(progress.learned[i])pill("DONE",x+w-95,y+29,CQ_GREEN);else tx("Open  >",x+w-90,y+32,14,CQ_PURPLE);
   if(hit(RR(x,y,w,86))){selectedLesson=i;scroll=0;}y+=99;
  }y+=22;
 }*bottom=y;
}
static void lessonView(float x,float w,float *bottom){Lesson*l=&lessons[selectedLesson];float y=96;Color c=tracks[l->track].color;
 if(btn("<  Back to lessons",x,y,186,39,CQ_PURPLE,0)){selectedLesson=-1;scroll=0;return;}y+=68;
 pill(tracks[l->track].name,x,y,c);y+=46;tx(l->title,x,y,32,CQ_WHITE);y+=57;
 y=paragraph(l->intro,x,y,17,CQ_WHITE,w-28)+28;
 box(x,y,w,1,.5f,CQ_LINE);y+=28;tx("How it works",x,y,23,CQ_WHITE);y+=43;
 y=paragraph(l->explain,x,y,17,CQ_MUTED,w-28)+30;
 if(lessonDetails[selectedLesson].concept){
  const LessonDetail *d=&lessonDetails[selectedLesson];
  tx("STEP-BY-STEP",x,y,12,c);y+=30;
  y=paragraph(d->concept,x,y,17,CQ_WHITE,w-28)+28;
 }
 tx("EXAMPLE",x,y,12,c);y+=27;y+=codePanel(l->code,x,y,w);y+=29;
 /* Extended tutorial walkthroughs, with practical examples and outcomes. */
 const TutorialChapter *chapter = &tutorialChapters[selectedLesson];
 tx("DEEPER DIVE",x,y,13,c);y+=34;
 for(int section=0;section<2;section++){
  const TutorialSection *part=&chapter->sections[section];
  tx(part->heading,x,y,23,CQ_WHITE);y+=43;
  y=paragraph(part->explanation,x,y,17,CQ_MUTED,w-28)+24;
  y+=codePanel(part->example,x,y,w)+19;
  box(x,y,4,25,.5f,CQ_GREEN);
  y=paragraph(part->result,x+17,y+2,16,CQ_WHITE,w-42)+35;
 }

 /* Further course-aligned notes with worked examples and expected outcomes. */
 const TutorialChapter *extra = &extraChapters[selectedLesson];
 tx("MORE EXAMPLES & EXAM PRACTICE",x,y,13,c);y+=34;
 for(int section=0;section<2;section++){
  const TutorialSection *part=&extra->sections[section];
  tx(part->heading,x,y,23,CQ_WHITE);y+=43;
  y=paragraph(part->explanation,x,y,17,CQ_MUTED,w-28)+24;
  y+=codePanel(part->example,x,y,w)+19;
  box(x,y,4,25,.5f,CQ_GREEN);
  y=paragraph(part->result,x+17,y+2,16,CQ_WHITE,w-42)+35;
 }

 if(lessonDetails[selectedLesson].mistakes){
  const LessonDetail *d=&lessonDetails[selectedLesson];
  tx("COMMON MISTAKES",x,y,21,CQ_WHITE);y+=37;
  y=paragraph(d->mistakes,x,y,16,CQ_MUTED,w-28)+28;
  tx("TRY IT YOURSELF",x,y,21,CQ_WHITE);y+=37;
  y=paragraph(d->practice,x,y,16,CQ_WHITE,w-28)+32;
 }

 box(x,y,w,108,.08f,alpha(c,.10f));tx("GOOD TO KNOW",x+19,y+16,12,c);
 paragraph(l->tip,x+19,y+43,15,CQ_WHITE,w-40);y+=134;
 if(progress.learned[selectedLesson]){pill("LESSON COMPLETED  +30 XP",x,y,CQ_GREEN);}else if(btn("Mark as learned   +30 XP",x,y,248,48,CQ_GREEN,1)){progress.learned[selectedLesson]=1;award(30);}
 if(btn("Practise with a quiz  >",x+265,y,230,48,CQ_PURPLE,0))startQuiz(l->track);
 *bottom=y+105;
}
/* Topic-based quiz menu: one assessment per learning track. */
static void quizMenu(float x,float w,float *bottom){
 float y=96;
 tx("Practice quizzes",x,y,30,CQ_WHITE);
 tx("Pick a quiz, work through every question, then review your score.",x,y+44,15,CQ_MUTED);
 y+=99;
 box(x,y,w,96,.10f,PANEL);
 char stats[112];
 snprintf(stats,sizeof(stats),"%d / %d questions mastered across all quizzes",totalQuizzes(),COUNT(quizzes));
 tx(stats,x+21,y+19,18,CQ_WHITE);
 bar(x+21,y+64,w-42,(float)totalQuizzes()/COUNT(quizzes),CQ_PURPLE);
 y+=126;
 tx("AVAILABLE QUIZZES",x,y,12,CQ_MUTED);y+=36;
 for(int t=0;t<COUNT(tracks);t++){
  int count=quizCountForTrack(t),done=quizMasteredForTrack(t);
  if(!count)continue;
  Rectangle r=RR(x,y,w,125);
  int hovered=CheckCollisionPointRec(pointer(),clickRect(r));
  box(x,y,w,125,.10f,hovered?CARD:PANEL);
  border(x,y,w,125,.10f,hovered?tracks[t].color:CQ_LINE);
  box(x+18,y+23,56,56,.20f,alpha(tracks[t].color,.18f));
  char num[12];snprintf(num,sizeof(num),"%02d",t+1);
  tx(num,x+32,y+42,20,tracks[t].color);
  tx(tracks[t].name,x+92,y+16,21,CQ_WHITE);
  tx(tracks[t].subtitle,x+92,y+47,14,CQ_MUTED);
  char details[120];
  snprintf(details,sizeof(details),"%d questions  |  %d mastered",count,done);
  tx(details,x+92,y+77,14,CQ_MUTED);
  bar(x+92,y+104,fminf(290,w-240),(float)done/count,tracks[t].color);
  pill("START QUIZ  >",x+w-148,y+48,tracks[t].color);
  if(hit(r)){startQuiz(t);return;}
  y+=143;
 }
 *bottom=y+40;
}
static void quizSummary(float x,float w,float *bottom){
 float y=102;
 int score=quizScore();int pct=quizCount?score*100/quizCount:0;
 Color accent=score==quizCount?CQ_GREEN:pct>=70?CQ_PURPLE:CQ_ORANGE;
 pill("QUIZ COMPLETE",x,y,accent);y+=48;
 tx("Your quiz results",x,y,32,CQ_WHITE);y+=60;
 box(x,y,w,215,.09f,PANEL);
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
  tx("Areas to improve",x,y,25,CQ_WHITE);y+=40;
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
 if(quizTrack<0){quizMenu(x,w,bottom);return;}
 if(quizFinished){quizSummary(x,w,bottom);return;}
 float y=101;
 if(btn("<  Quiz menu",x,y,162,40,CQ_PURPLE,0)){
  quizTrack=-1;quizFinished=0;scroll=0;return;
 }
 char counter[80];snprintf(counter,sizeof(counter),"Question %d of %d",quizPosition+1,quizCount);
 tx(counter,x+w-195,y+11,17,CQ_MUTED);
 y+=63;
 tx(tracks[quizTrack].name,x,y,29,CQ_WHITE);
 tx("Choose your answer, check it, then move to the next question.",x,y+43,15,CQ_MUTED);
 y+=100;
 bar(x,y,w,(float)quizPosition/quizCount,tracks[quizTrack].color);y+=29;
 Quiz *q=&quizzes[quizQuestionIds[quizPosition]];
 Color c=tracks[q->track].color;
 box(x,y,w,135,.08f,CARD);
 tx("QUESTION",x+24,y+17,12,c);
 paragraph(q->question,x+24,y+48,22,CQ_WHITE,w-48);
 y+=155;
 for(int i=0;i<4;i++){
  Color background=PANEL,outline=CQ_LINE;
  if(quizAnswered&&i==q->correct){background=alpha(CQ_GREEN,.14f);outline=CQ_GREEN;}
  else if(quizAnswered&&i==quizChoice){background=alpha(CQ_RED,.14f);outline=CQ_RED;}
  else if(!quizAnswered&&quizChoice==i){background=alpha(CQ_PURPLE,.16f);outline=CQ_PURPLE;}
  box(x,y,w,65,.12f,background);border(x,y,w,65,.12f,outline);
  box(x+17,y+16,32,32,.2f,alpha(outline,.20f));
  char letter[3]={(char)('A'+i),0};
  tx(letter,x+28,y+22,14,CQ_MUTED);
  tx(q->answers[i],x+66,y+20,17,CQ_WHITE);
  if(!quizAnswered&&hit(RR(x,y,w,65)))quizChoice=i;
  y+=79;
 }
 if(quizAnswered){
  int right=quizChoice==q->correct;
  float feedbackEnd=paragraph(q->why,x+19,y+55,15,CQ_WHITE,w-40);
  float height=fmaxf(118,feedbackEnd-y+22);
  box(x,y,w,height,.08f,alpha(right?CQ_GREEN:CQ_ORANGE,.11f));
  tx(right?"Correct!":"Review this answer",x+19,y+15,19,right?CQ_GREEN:CQ_ORANGE);
  paragraph(q->why,x+19,y+55,15,CQ_WHITE,w-40);
  y+=height+20;
  const int last=(quizPosition==quizCount-1);
  if(btn(last?"Finish quiz and view results  >":"Next question  >",x,y,last?305:213,49,CQ_PURPLE,1)){
   if(last){quizFinished=1;}else{quizPosition++;quizChoice=-1;quizAnswered=0;}
   scroll=0;return;
  }
 }else{
  if(btn("Check answer",x,y,190,49,CQ_PURPLE,quizChoice>=0)&&quizChoice>=0){
   quizAnswered=1;quizSelections[quizPosition]=quizChoice;
   progress.quizAttempts++;
   if(quizChoice==q->correct){
    progress.quizCorrect++;
    int id=quizQuestionIds[quizPosition];
    if(!progress.quizDone[id]){progress.quizDone[id]=1;award(20);}else persist();
   }else persist();
  }
 }
 *bottom=y+110;
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
static void labList(float x,float w,float *bottom){float y=95;tx("Coding lab challenges",x,y,30,CQ_WHITE);tx("Real assignments from your worksheets, built for hands-on practise.",x,y+43,15,CQ_MUTED);y+=99;
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
  labFocus=0;labSaveDraft(selectedLab,labCode);labResult=labExecute(selectedLab,labCode,0);labHasResult=1;
 }
 if(btn("Check Solution",x+bw+12,y,bw,45,CQ_GREEN,1)){
  labFocus=0;labSaveDraft(selectedLab,labCode);labResult=labExecute(selectedLab,labCode,1);labHasResult=1;
  if(labResult.passed&&!progress.labDone[selectedLab]){progress.labDone[selectedLab]=1;award(60);}
 }
 if(btn("Reset Starter",x+2*(bw+12),y,bw,45,CQ_PURPLE,0)){
  labLoadStarter(selectedLab,labCode,sizeof(labCode));labCursor=0;labAnchor=0;labDragging=0;labScrollbarDragging=0;labFirstLine=0;labHasResult=0;labFocus=0;labSaveDraft(selectedLab,labCode);
 }
 y+=67;
 if(labHasResult){
  box(x,y,w,labResult.passed?72:76,.07f,alpha(labResult.passed?CQ_GREEN:CQ_BLUE,.10f));
  tx(labResult.passed?"ALL REFERENCE TESTS PASSED":labResult.compiled?(labResult.ran?"PROGRAM OUTPUT / TEST FEEDBACK":"RUNTIME ERROR"):"COMPILER OUTPUT",x+17,y+12,13,labResult.passed?CQ_GREEN:CQ_BLUE);
  tx(labResult.passed?"Your code matches the expected output.":"Read the output below and adjust your solution.",x+17,y+41,13,CQ_WHITE);y+=87;
  box(x,y,w,35,.07f,PANEL);tx("CONSOLE / COMPILER MESSAGES",x+17,y+9,12,CQ_MUTED);y+=44;
  /* Keep output scrollable as part of main page and cap for large printouts. */
  const char *p=labResult.output;int lines=0;while(*p&&lines<54){char line[512];int n=0;while(p[n]&&p[n]!='\n'&&n<510)n++;memcpy(line,p,(size_t)n);line[n]=0;tx(line,x+15,y,13,CQ_WHITE);y+=21;lines++;p+=n;if(*p=='\n')p++;else if(!n)break;}
  if(*p){tx("... console output truncated ...",x+15,y,12,CQ_MUTED);y+=23;}
  y+=16;
 }
 box(x,y,w,100,.07f,alpha(CQ_ORANGE,.08f));tx("HELPFUL HINT",x+18,y+11,12,CQ_ORANGE);paragraph(l->hint,x+18,y+39,14,CQ_WHITE,w-36);y+=120;
 if(progress.labDone[selectedLab])pill("LAB VERIFIED  +60 XP",x,y,CQ_GREEN);
 else tx("Complete the reference tests to earn 60 XP. Output checks are not a secure or exhaustive judge.",x,y+8,12,CQ_MUTED);
 *bottom=y+70;
}
static void progressView(float x,float w,float *bottom){float y=95;tx("Your learning journey",x,y,30,CQ_WHITE);tx("Every small win counts. Keep building your skills.",x,y+44,15,CQ_MUTED);y+=102;
 float cell=(w-36)/3;const char*lab[]={"TOTAL XP","CURRENT LEVEL","LEARNING STREAK"};char vals[3][40];snprintf(vals[0],40,"%d",progress.xp);snprintf(vals[1],40,"%d",1+progress.xp/250);snprintf(vals[2],40,"%d day%s",progress.streak,progress.streak==1?"":"s");
 for(int i=0;i<3;i++){float xx=x+i*(cell+18);box(xx,y,cell,122,.08f,PANEL);tx(lab[i],xx+20,y+23,12,CQ_MUTED);tx(vals[i],xx+20,y+52,27,i==0?CQ_ORANGE:i==1?CQ_PURPLE:CQ_GREEN);}y+=159;
 box(x,y,w,135,.08f,PANEL);tx("Progress to next level",x+22,y+24,20,CQ_WHITE);char s[70];snprintf(s,sizeof(s),"%d / 250 XP",progress.xp%250);tx(s,x+w-132,y+28,14,CQ_MUTED);bar(x+22,y+85,w-44,(progress.xp%250)/250.0f,CQ_PURPLE);y+=165;
 tx("Topics you've studied",x,y,22,CQ_WHITE);y+=45;
 for(int t=0;t<COUNT(tracks);t++){int done=trackDone(t),all=trackLessonCount(t);box(x,y,w,82,.09f,PANEL);DrawCircle(x+30,y+33,7,tracks[t].color);tx(tracks[t].name,x+48,y+16,17,CQ_WHITE);
  snprintf(s,sizeof(s),"%d of %d completed",done,all);tx(s,x+w-173,y+18,13,CQ_MUTED);bar(x+48,y+56,w-95,(float)done/all,tracks[t].color);y+=95;}
 y+=21;box(x,y,w,147,.08f,PANEL);tx("Quiz performance",x+22,y+20,22,CQ_WHITE);
 float accuracy=progress.quizAttempts?100.0f*progress.quizCorrect/progress.quizAttempts:0.0f;snprintf(s,sizeof(s),"%.0f%%",accuracy);tx(s,x+22,y+59,29,CQ_GREEN);
 snprintf(s,sizeof(s),"%d correct / %d attempts",progress.quizCorrect,progress.quizAttempts);tx(s,x+123,y+70,15,CQ_MUTED);bar(x+22,y+119,w-44,accuracy/100,CQ_GREEN);
 y+=179;tx("Focus areas",x,y,22,CQ_WHITE);y+=42;
 int weakest=0;float ratio=2;for(int t=0;t<COUNT(tracks);t++){float r=(float)trackDone(t)/trackLessonCount(t);if(r<ratio){ratio=r;weakest=t;}}
 box(x,y,w,115,.08f,alpha(tracks[weakest].color,.11f));tx("SUGGESTED NEXT STEP",x+20,y+15,12,tracks[weakest].color);tx(tracks[weakest].name,x+20,y+43,20,CQ_WHITE);
 if(btn("Study this track  >",x+w-208,y+43,187,44,tracks[weakest].color,1)){selectedTrack=weakest;nav(1);}
 *bottom=y+147;
}

void RunApp(void){
 SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);InitWindow(windowW,windowH,"CodeQuest | C & C++ Learning Studio");SetWindowMinSize(1000,650);SetTargetFPS(60);
 /* Optional OS font. No images, textures, fonts, or other assets required. */
 const char *paths[]={"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf","/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf","C:/Windows/Fonts/segoeui.ttf","/System/Library/Fonts/Supplemental/Arial.ttf"};
 font=GetFontDefault();for(int i=0;i<COUNT(paths);i++){if(FileExists(paths[i])){Font loaded=LoadFontEx(paths[i],40,NULL,0);if(loaded.texture.id){font=loaded;SetTextureFilter(font.texture,TEXTURE_FILTER_BILINEAR);break;}}}
 loadProgress();loadTheme();
 while(!WindowShouldClose()){
  /* F11 toggles fullscreen. Ctrl + plus/minus adjusts the UI zoom for
   * accessibility; Ctrl + 0 resets it. */
  if(IsKeyPressed(KEY_F11))ToggleFullscreen();
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
  if(IsKeyPressed(KEY_ESCAPE)){if(screen==2&&quizTrack>=0){quizTrack=-1;quizFinished=0;scroll=0;}else if(selectedLesson>=0){selectedLesson=-1;scroll=0;}else if(selectedLab>=0){labSaveDraft(selectedLab,labCode);selectedLab=-1;labFocus=0;scroll=0;}else if(screen)nav(0);}
  float side=254,x=side+37,w=windowW-side-75;if(w<450)w=450;
  if(CheckCollisionPointRec(pointer(),RR(side,80,(float)windowW-side,(float)windowH-80))&&
     !(screen==3&&selectedLab>=0&&CheckCollisionPointRec(pointer(),labEditorWheelRect))){scroll-=GetMouseWheelMove()*45;}
  if(scroll<0)scroll=0;
  if(scroll>maxScroll)scroll=maxScroll;
  BeginDrawing();ClearBackground(BG);
  /* Draw fixed navigation at the same scale as the content. Avoid nested
   * BeginMode2D calls: a nested camera replaces (rather than composes with)
   * the previous camera, which made the content tiny in fullscreen. */
  Camera2D fixedCamera={.offset={0,0},.target={0,0},.rotation=0,.zoom=uiScale};
  BeginMode2D(fixedCamera);
  sidebar(side);
  const char *titles[]={"DASHBOARD / OVERVIEW","LIBRARY / LESSONS","PRACTICE / QUIZ","WORKSHOP / LABS","ANALYTICS / PROGRESS"};
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
  else progressView(x,w,&bottom);
  EndMode2D();
  uiScrollActive=0;
  EndScissorMode();

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
  EndDrawing();
 }
 if(labLoaded>=0)labSaveDraft(labLoaded,labCode);
 persist();
 if(font.texture.id!=GetFontDefault().texture.id)UnloadFont(font);
 CloseWindow();
}
