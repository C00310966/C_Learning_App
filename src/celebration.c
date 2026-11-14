/* C11 + raylib: no external sound/image assets required. */
#include "raylib.h"
#include "celebration.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define PARTICLE_COUNT 42
#define SOUND_RATE 44100
#define CHIME_SECONDS 0.38f
#define CQ_PI 3.14159265358979323846f

typedef struct {Vector2 pos, velocity;float life,rotation,spin,size;Color color;} Confetti;
static Confetti pieces[PARTICLE_COUNT];
static int active=0, audioReady=0, soundLoaded=0;
static Sound successSound={0};
static const Color palette[]={
 {123,214,255,255},{163,138,255,255},{255,205,112,255},
 {112,231,179,255},{255,148,182,255},{244,244,255,255}
};
static float randBetween(float lo,float hi){return lo+(hi-lo)*(float)GetRandomValue(0,10000)/10000.0f;}

void celebrationInit(void){
 if(!IsAudioDeviceReady())InitAudioDevice();
 audioReady=IsAudioDeviceReady();
 if(!audioReady)return;
 /* Create an original three-note major arpeggio with soft sine harmonics,
  * a quick attack and decaying tail; no copyrighted audio is reproduced. */
 const unsigned int count=(unsigned int)(SOUND_RATE*CHIME_SECONDS);
 int16_t *pcm=(int16_t*)malloc((size_t)count*sizeof(*pcm));
 if(!pcm)return;
 const float notes[]={659.25f,783.99f,987.77f};
 const float starts[]={0.0f,0.075f,0.150f};
 for(unsigned int i=0;i<count;i++){
  float t=(float)i/SOUND_RATE, mixed=0.0f;
  for(int n=0;n<3;n++){
   float age=t-starts[n];if(age<0)continue;
   float attack=fminf(1.0f,age/0.010f);
   float envelope=attack*expf(-age*11.0f);
   float phase=2.0f*CQ_PI*notes[n]*age;
   mixed+=envelope*(sinf(phase)+0.14f*sinf(2.0f*phase));
  }
  mixed=fmaxf(-1.0f,fminf(1.0f,mixed*0.20f));
  pcm[i]=(int16_t)(mixed*32767.0f);
 }
 Wave wave={0};wave.frameCount=count;wave.sampleRate=SOUND_RATE;
 wave.sampleSize=16;wave.channels=1;wave.data=pcm;
 successSound=LoadSoundFromWave(wave);
 soundLoaded=successSound.frameCount>0;
 /* LoadSoundFromWave copies PCM into the audio buffer. */
 free(pcm);
 if(soundLoaded)SetSoundVolume(successSound,0.55f);
}
void celebrationCorrect(float x,float y){
 for(int i=0;i<PARTICLE_COUNT;i++){
  Confetti *p=&pieces[i];
  p->pos=(Vector2){x+randBetween(-22,22),y+randBetween(-8,8)};
  float angle=randBetween(195,345)*CQ_PI/180.0f;
  float speed=randBetween(125,275);
  p->velocity=(Vector2){cosf(angle)*speed,sinf(angle)*speed};
  p->size=randBetween(3.0f,6.5f);p->rotation=randBetween(0,360);
  p->spin=randBetween(-380,380);p->life=randBetween(0.70f,1.12f);
  p->color=palette[GetRandomValue(0,(int)(sizeof(palette)/sizeof(palette[0]))-1)];
 }
 active=1;
 if(audioReady&&soundLoaded)PlaySound(successSound);
}
void celebrationDraw(float dt){
 if(!active)return;
 if(dt>0.06f)dt=0.06f;
 int visible=0;
 for(int i=0;i<PARTICLE_COUNT;i++){
  Confetti *p=&pieces[i];if(p->life<=0)continue;
  p->life-=dt;if(p->life<=0)continue;
  visible=1;p->velocity.y+=430*dt;
  p->pos.x+=p->velocity.x*dt;p->pos.y+=p->velocity.y*dt;
  p->rotation+=p->spin*dt;
  Color color=p->color;
  if(p->life<0.24f)color.a=(unsigned char)(255.0f*p->life/0.24f);
  DrawRectanglePro((Rectangle){p->pos.x,p->pos.y,p->size,p->size*1.5f},
                   (Vector2){p->size/2,p->size*0.75f},p->rotation,color);
 }
 active=visible;
}
void celebrationClose(void){
 if(soundLoaded){UnloadSound(successSound);soundLoaded=0;}
 if(audioReady){CloseAudioDevice();audioReady=0;}
}
