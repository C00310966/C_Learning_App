/* CODEQUEST - C/C++ learning app. C11 + raylib only.
 * Build: make (macOS) or Ctrl+Shift+B in VS Code (Windows GCC)
 * UI and quiz interactions live here; curriculum data and progress live in separate C modules.
 */
#include "raylib.h"
#include "curriculum.h"
#include "progress.h"
#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define COUNT(a) ((int)(sizeof(a)/sizeof((a)[0])))

static const Color BG={13,18,31,255}, PANEL={23,30,47,255}, CARD={29,38,59,255};
static const Color CQ_WHITE={239,244,255,255}, CQ_MUTED={151,165,190,255}, CQ_LINE={51,64,87,255};
static const Color CQ_PURPLE={141,111,255,255}, CQ_GREEN={88,222,165,255}, CQ_ORANGE={255,182,106,255}, CQ_RED={255,117,139,255}, CQ_BLUE={106,187,255,255};
static Font font={0};
static int screen=0, selectedTrack=-1, selectedLesson=-1, selectedLab=-1;
static int quizIndex=-1, quizChoice=-1, quizAnswered=0, quizSerial=0;
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
 tx(text,x+(w-tw(text,15))/2,y+(h-17)/2,15,filled?BG:CQ_WHITE);
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
  char candidate[1024];snprintf(candidate,sizeof(candidate),"%s%s%s",line,n?" ":"",word);
  if(n>0 && tw(candidate,sz)>width){tx(line,x,cy,sz,color);cy+=sz+9;n=0;line[0]=0;}
  if(n && n+1<(int)sizeof(line)){line[n++]=' ';line[n]=0;}
  if(n+len<(int)sizeof(line)){memcpy(line+n,word,(size_t)len);n+=len;line[n]=0;}
  p=e;if(*p==' ')p++;
 }
 if(n){tx(line,x,cy,sz,color);cy+=sz+9;}*outY=cy;
}
static float paragraph(const char*s,float x,float y,int sz,Color color,float w){float end=y;richLine(s,x,y,sz,color,w,&end);return end;}
static float codePanel(const char *code,float x,float y,float w){
 int lines=1;for(const char*p=code;*p;p++)if(*p=='\n')lines++;
 float height=54+lines*23;box(x,y,w,height,.06f,(Color){12,21,34,255});
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
static void nav(int s){screen=s;selectedLesson=-1;selectedLab=-1;scroll=0;searchActive=0;}
static int contains(const char*s,const char*term){if(!term[0])return 1;char a[1100],b[100];snprintf(a,sizeof(a),"%s",s);snprintf(b,sizeof(b),"%s",term);for(char*p=a;*p;p++)if(*p>='A'&&*p<='Z')*p+=32;for(char*p=b;*p;p++)if(*p>='A'&&*p<='Z')*p+=32;return strstr(a,b)!=NULL;}
static void startQuiz(int track){quizChoice=-1;quizAnswered=0;quizSerial++;int count=0;int candidate[COUNT(quizzes)];for(int i=0;i<COUNT(quizzes);i++)if(track<0 || quizzes[i].track==track)candidate[count++]=i;
 if(!count)return;
 int begin=quizSerial%count;quizIndex=candidate[begin];
 for(int j=0;j<count;j++){int k=candidate[(begin+j)%count];if(!progress.quizDone[k]){quizIndex=k;break;}}
 nav(2);
}
static void sidebar(float side){
 DrawRectangle(0,0,(int)side,windowH,PANEL);DrawLine((int)side,0,(int)side,windowH,CQ_LINE);
 box(26,25,42,42,.25f,CQ_PURPLE);tx("{ }",33,34,19,BG);tx("codequest",82,28,24,CQ_WHITE);tx("LEARN C / C++",83,56,10,CQ_MUTED);
 tx("WORKSPACE",29,115,11,CQ_MUTED);
 const char*names[]={"Overview","Learn","Practice quizzes","Coding labs","My progress"};const char*icons[]={"01","02","03","04","05"};
 for(int i=0;i<5;i++){float y=143+i*57;Rectangle r=RR(17,y,side-34,47);int active=screen==i;box(r.x,r.y,r.width,r.height,.18f,active?alpha(CQ_PURPLE,.22f):(CheckCollisionPointRec(pointer(),r)?CARD:PANEL));
  if(active)box(18,y+8,3,31,1,CQ_PURPLE);
  tx(icons[i],31,y+15,13,active?CQ_PURPLE:CQ_MUTED);tx(names[i],77,y+13,16,active?CQ_WHITE:CQ_MUTED);
  if(hit(r))nav(i);
 }
 tx("LEARNING TRACKS",29,465,11,CQ_MUTED);
 for(int t=0;t<COUNT(tracks);t++){float y=496+t*40;DrawCircle(36,y+9,5,tracks[t].color);tx(tracks[t].name,51,y,13,CQ_MUTED);}
 box(18,windowH-136,side-36,111,.14f,CARD);tx("YOUR JOURNEY",35,windowH-117,11,CQ_MUTED);
 char str[96];snprintf(str,sizeof(str),"Level %d  /  %d XP",1+progress.xp/250,progress.xp);tx(str,35,windowH-91,18,CQ_WHITE);
 bar(35,windowH-57,side-72,(progress.xp%250)/250.0f,CQ_PURPLE);
}
static void header(float x,float width,const char*section){
 tx(section,x,37,14,CQ_MUTED);
 char s[60];snprintf(s,sizeof(s),"%d XP",progress.xp);box(x+width-158,25,144,43,.28f,CARD);
 DrawCircle(x+width-131,46,10,CQ_ORANGE);tx("*",x+width-136,36,20,BG);tx(s,x+width-111,36,17,CQ_WHITE);
}
static void overview(float x,float w,float *bottom){
 float y=96;
 box(x,y,w,218,.05f,(Color){43,39,88,255});DrawCircle(x+w-105,y+105,105,alpha(CQ_PURPLE,.11f));DrawCircle(x+w-105,y+105,72,alpha(CQ_BLUE,.08f));
 pill("YOUR LEARNING SPACE",x+27,y+26,CQ_PURPLE);tx("Build skills. Ship code.",x+27,y+64,32,CQ_WHITE);
 tx("Master C and C++ with bite-sized lessons, real labs and rewards.",x+27,y+112,16,(Color){201,198,232,255});
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
 tx("EXAMPLE",x,y,12,c);y+=27;y+=codePanel(l->code,x,y,w);y+=29;
 box(x,y,w,108,.08f,alpha(c,.10f));tx("GOOD TO KNOW",x+19,y+16,12,c);
 paragraph(l->tip,x+19,y+43,15,CQ_WHITE,w-40);y+=134;
 if(progress.learned[selectedLesson]){pill("LESSON COMPLETED  +30 XP",x,y,CQ_GREEN);}else if(btn("Mark as learned   +30 XP",x,y,248,48,CQ_GREEN,1)){progress.learned[selectedLesson]=1;award(30);}
 if(btn("Practise with a quiz  >",x+265,y,230,48,CQ_PURPLE,0))startQuiz(l->track);
 *bottom=y+105;
}
static void quizView(float x,float w,float *bottom){float y=101;
 tx("Knowledge check",x,y,30,CQ_WHITE);tx("Choose the best answer. Your feedback appears immediately.",x,y+45,15,CQ_MUTED);y+=95;
 if(quizIndex<0)startQuiz(-1);
 if(quizIndex<0)return;
 Quiz*q=&quizzes[quizIndex];Color c=tracks[q->track].color;
 box(x,y,w,79,.08f,PANEL);pill(tracks[q->track].name,x+20,y+16,c);tx("+20 XP for a first-time correct answer",x+20,y+52,12,CQ_MUTED);
 y+=100;box(x,y,w,127,.08f,CARD);tx("QUESTION",x+24,y+19,12,c);paragraph(q->question,x+24,y+51,23,CQ_WHITE,w-48);y+=146;
 for(int i=0;i<4;i++){Color background=PANEL;Color outline=CQ_LINE;Color txt=CQ_WHITE;
  if(quizAnswered && i==q->correct){background=alpha(CQ_GREEN,.14f);outline=CQ_GREEN;}
  else if(quizAnswered && i==quizChoice && i!=q->correct){background=alpha(CQ_RED,.15f);outline=CQ_RED;}
  else if(!quizAnswered && quizChoice==i){background=alpha(CQ_PURPLE,.16f);outline=CQ_PURPLE;}
  box(x,y,w,65,.12f,background);border(x,y,w,65,.12f,outline);
  box(x+17,y+16,32,32,.2f,alpha(outline,.22f));char a[3]={(char)('A'+i),0};tx(a,x+28,y+22,14,quizAnswered&&i==q->correct?CQ_GREEN:CQ_MUTED);
  tx(q->answers[i],x+66,y+20,17,txt);if(!quizAnswered&&hit(RR(x,y,w,65)))quizChoice=i;
  y+=79;
 }
 if(quizAnswered){int right=quizChoice==q->correct;box(x,y,w,119,.08f,alpha(right?CQ_GREEN:CQ_ORANGE,.10f));tx(right?"Correct! Nice work.":"Not quite. Here's why:",x+19,y+15,19,right?CQ_GREEN:CQ_ORANGE);paragraph(q->why,x+19,y+49,15,CQ_WHITE,w-42);y+=138;
  if(btn("Next question  >",x,y,217,48,CQ_PURPLE,1)){startQuiz(selectedTrack);return;}
 }else{if(btn("Check answer",x,y,180,47,CQ_PURPLE,quizChoice>=0)&&quizChoice>=0){quizAnswered=1;progress.quizAttempts++;if(quizChoice==q->correct){progress.quizCorrect++;if(!progress.quizDone[quizIndex]){progress.quizDone[quizIndex]=1;award(20);}else persist();}else persist();}}
 *bottom=y+110;
}
static void labList(float x,float w,float *bottom){float y=95;tx("Coding lab challenges",x,y,30,CQ_WHITE);tx("Real assignments from your worksheets, built for hands-on practise.",x,y+43,15,CQ_MUTED);y+=99;
 box(x,y,w,99,.07f,alpha(CQ_PURPLE,.12f));tx("HOW LABS WORK",x+20,y+16,12,CQ_PURPLE);paragraph("Read the task, implement it in your own C/C++ editor, test it, then mark it complete. This app does not compile or grade arbitrary code.",x+20,y+42,14,CQ_WHITE,w-42);y+=129;
 for(int t=0;t<COUNT(tracks);t++){int n=0;for(int i=0;i<COUNT(labTasks);i++)if(labTasks[i].track==t)n++;if(!n)continue;
  tx(tracks[t].name,x,y,21,CQ_WHITE);y+=39;
  for(int i=0;i<COUNT(labTasks);i++){if(labTasks[i].track!=t)continue;box(x,y,w,85,.08f,PANEL);
   box(x+17,y+18,45,45,.2f,alpha(tracks[t].color,.19f));tx("</>",x+22,y+29,16,tracks[t].color);
   tx(labTasks[i].title,x+77,y+13,18,CQ_WHITE);tx(labTasks[i].level,x+77,y+47,12,CQ_MUTED);
   if(progress.labDone[i])pill("SOLVED",x+w-101,y+29,CQ_GREEN);else tx("Open  >",x+w-91,y+32,14,CQ_PURPLE);
   if(hit(RR(x,y,w,85))){selectedLab=i;scroll=0;}y+=98;
  }y+=21;
 }*bottom=y;
}
static void labView(float x,float w,float *bottom){Lab*l=&labTasks[selectedLab];float y=96;Color c=tracks[l->track].color;
 if(btn("<  Back to labs",x,y,180,40,CQ_PURPLE,0)){selectedLab=-1;scroll=0;return;}y+=66;
 pill(l->level,x,y,c);y+=45;tx(l->title,x,y,30,CQ_WHITE);y+=57;
 tx("YOUR TASK",x,y,13,c);y+=32;y=paragraph(l->task,x,y,18,CQ_WHITE,w-40)+35;
 tx("SUGGESTED SIGNATURE / STARTING POINT",x,y,12,c);y+=31;y+=codePanel(l->signature,x,y,w);y+=30;
 box(x,y,w,105,.07f,PANEL);tx("TEST CASES / EXPECTED BEHAVIOUR",x+20,y+17,12,CQ_MUTED);paragraph(l->tests,x+20,y+49,16,CQ_WHITE,w-45);y+=129;
 box(x,y,w,112,.07f,alpha(CQ_BLUE,.09f));tx("A LITTLE HINT",x+20,y+18,13,CQ_BLUE);paragraph(l->hint,x+20,y+51,16,CQ_WHITE,w-45);y+=137;
 if(progress.labDone[selectedLab])pill("LAB COMPLETED  +60 XP",x,y,CQ_GREEN);
 else if(btn("I completed this lab   +60 XP",x,y,275,49,CQ_GREEN,1)){progress.labDone[selectedLab]=1;award(60);}
 tx("Completion is self-reported; test your solution first.",x,y+66,13,CQ_MUTED);*bottom=y+127;
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
 loadProgress();
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
  if(IsKeyPressed(KEY_ESCAPE)){if(selectedLesson>=0){selectedLesson=-1;scroll=0;}else if(selectedLab>=0){selectedLab=-1;scroll=0;}else if(screen)nav(0);}
  float side=254,x=side+37,w=windowW-side-75;if(w<450)w=450;
  if(CheckCollisionPointRec(pointer(),RR(side,80,(float)windowW-side,(float)windowH-80))){scroll-=GetMouseWheelMove()*45;}
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
 persist();if(font.texture.id!=GetFontDefault().texture.id)UnloadFont(font);CloseWindow();
}
