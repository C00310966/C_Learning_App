#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SAVE_FILE "codequest_progress.dat"
Save progress={0};

void persist(void){FILE*f=fopen(SAVE_FILE,"wb");if(!f)return;fwrite("CQ2!",1,4,f);fwrite(&progress,sizeof(progress),1,f);fclose(f);}
void loadProgress(void){FILE*f=fopen(SAVE_FILE,"rb");if(!f)return;char magic[4];if(fread(magic,1,4,f)==4 && memcmp(magic,"CQ2!",4)==0)fread(&progress,sizeof(progress),1,f);fclose(f);if(progress.xp<0||progress.xp>100000000)memset(&progress,0,sizeof(progress));}
void award(int xp){progress.xp+=xp;int day=(int)(time(NULL)/86400);if(progress.lastDay!=day){progress.streak=progress.lastDay==day-1?progress.streak+1:1;progress.lastDay=day;}persist();}
