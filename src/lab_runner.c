#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
/* Native offline C/C++ compiler runner. No shell or external GUI toolkit.
 * WARNING: compiled student programs run with the current user's permissions.
 * Do not run untrusted code; this is not a sandbox or an anti-cheat judge. */
#include "lab_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define SEP '\\'
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#define MKDIR(path) mkdir(path,0700)
#define SEP '/'
#endif

static void pathFor(char *dst,size_t cap,const char *dir,int id,const char *ext){snprintf(dst,cap,"%s%c%02d.%s",dir,SEP,id,ext);}
static void readFile(const char *path,char *out,size_t cap){FILE *f=fopen(path,"rb");size_t n=0;out[0]=0;if(!f)return;n=fread(out,1,cap-1,f);out[n]=0;fclose(f);}
static int writeFile(const char *path,const char *data){FILE *f=fopen(path,"wb");if(!f)return 0;size_t len=strlen(data);int ok=fwrite(data,1,len,f)==len;fclose(f);return ok;}
static int isCpp(int id){return id==6||id>=18;}
static int pathOk(int id){return id>=0&&id<24;}
static void draftPath(char *dst,size_t cap,int id){snprintf(dst,cap,"codequest_lab_drafts%c%02d.%s",SEP,id,isCpp(id)?"cpp":"c");}
int labLoadDraft(int id,char *dst,size_t cap){if(!pathOk(id)||cap<2)return 0;char p[256];draftPath(p,sizeof(p),id);FILE *f=fopen(p,"rb");if(!f)return 0;size_t n=fread(dst,1,cap-1,f);dst[n]=0;fclose(f);return 1;}
void labSaveDraft(int id,const char *code){if(!pathOk(id))return;MKDIR("codequest_lab_drafts");char p[256];draftPath(p,sizeof(p),id);(void)writeFile(p,code);}
void labLoadStarter(int id,char *dst,size_t cap){if(!pathOk(id)||cap<2){if(cap)dst[0]=0;return;}char p[256];pathFor(p,sizeof(p),"lab_templates",id,isCpp(id)?"cpp":"c");readFile(p,dst,cap);if(!dst[0])snprintf(dst,cap,"/* Starter template missing: %s */\n",p);}
static void append(char *s,size_t cap,const char *more){size_t n=strlen(s);if(n<cap-1)snprintf(s+n,cap-n,"%s",more);}
/* Execute an explicit argv vector; never interpolate student's code into a shell. */
#ifdef _WIN32
static int process(const char *exe,const char *const argv[],const char *out,int ms,int *timedOut){
 char command[2048]="";int off=0;
 for(int i=0;argv[i];i++){int n=snprintf(command+off,sizeof(command)-(size_t)off,"%s\"%s\"",i?" ":"",argv[i]);if(n<0||n>=(int)(sizeof(command)-(size_t)off))return -1;off+=n;}
 HANDLE f=CreateFileA(out,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return -1;
 SetHandleInformation(f,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
 SECURITY_ATTRIBUTES sa={sizeof(sa),NULL,TRUE};
 HANDLE inheritable=NULL;DuplicateHandle(GetCurrentProcess(),f,GetCurrentProcess(),&inheritable,0,TRUE,DUPLICATE_SAME_ACCESS);
 STARTUPINFOA si={0};si.cb=sizeof(si);si.dwFlags=STARTF_USESTDHANDLES;si.hStdOutput=inheritable;si.hStdError=inheritable;HANDLE nul=CreateFileA("NUL",GENERIC_READ,FILE_SHARE_READ,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 si.hStdInput=(nul==INVALID_HANDLE_VALUE)?NULL:nul;
 PROCESS_INFORMATION pi={0};
 BOOL ok=CreateProcessA(exe,command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi);
 CloseHandle(inheritable);CloseHandle(f);if(nul!=INVALID_HANDLE_VALUE)CloseHandle(nul);if(!ok)return -1;
 HANDLE job=CreateJobObjectA(NULL,NULL);if(job){JOBOBJECT_EXTENDED_LIMIT_INFORMATION lim={0};lim.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;SetInformationJobObject(job,JobObjectExtendedLimitInformation,&lim,sizeof(lim));AssignProcessToJobObject(job,pi.hProcess);}
 DWORD wait=WaitForSingleObject(pi.hProcess,(DWORD)ms);if(wait==WAIT_TIMEOUT){*timedOut=1;TerminateProcess(pi.hProcess,1);WaitForSingleObject(pi.hProcess,1000);}
 DWORD code=1;GetExitCodeProcess(pi.hProcess,&code);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);if(job)CloseHandle(job);return wait==WAIT_TIMEOUT?-2:(int)code;
}
static int tempDir(char *out,size_t cap){char base[MAX_PATH];if(!GetTempPathA(MAX_PATH,base))return 0;char p[MAX_PATH];if(!GetTempFileNameA(base,"cqe",0,p))return 0;DeleteFileA(p);if(!CreateDirectoryA(p,NULL))return 0;snprintf(out,cap,"%s",p);return 1;}
#else
static int process(const char *exe,const char *const argv[],const char *out,int ms,int *timedOut){
 pid_t p=fork();if(p<0)return -1;
 if(p==0){setpgid(0,0);FILE*f=fopen(out,"wb");if(!f)_exit(127);dup2(fileno(f),STDOUT_FILENO);dup2(fileno(f),STDERR_FILENO);fclose(f);execvp(exe,(char *const *)argv);_exit(127);}
 int status=0,elapsed=0;
 while(elapsed<ms){pid_t r=waitpid(p,&status,WNOHANG);if(r==p)return WIFEXITED(status)?WEXITSTATUS(status):-1;if(r<0)return -1;struct timespec ts={0,20000000};nanosleep(&ts,NULL);elapsed+=20;}
 *timedOut=1;kill(-p,SIGKILL);kill(p,SIGKILL);waitpid(p,&status,0);(void)exe;(void)argv;return -2;
}
static int tempDir(char *out,size_t cap){char path[]="/tmp/codequest-XXXXXX";char*d=mkdtemp(path);if(!d)return 0;snprintf(out,cap,"%s",d);return 1;}
#endif
static void normalizeNewlines(char *s){char *dst=s;for(char *src=s;*src;src++)if(*src!='\r')*dst++=*src;*dst=0;}
static void trim(char *s){normalizeNewlines(s);size_t n=strlen(s);while(n&&(s[n-1]=='\n'||s[n-1]=='\r'||s[n-1]==' '||s[n-1]=='\t'))s[--n]=0;}
LabRunResult labExecute(int id,const char *code,int check){
 LabRunResult r={0};if(!pathOk(id)){snprintf(r.output,sizeof(r.output),"Invalid lab.");return r;}
 char dir[512];if(!tempDir(dir,sizeof(dir))){snprintf(r.output,sizeof(r.output),"Could not create a temporary directory.");return r;}
 char src[640],exe[640],log[640],expectedFile[256],expected[LAB_OUTPUT_CAP];
 snprintf(src,sizeof(src),"%s%cstudent.%s",dir,SEP,isCpp(id)?"cpp":"c");snprintf(exe,sizeof(exe),"%s%cprogram%s",dir,SEP,
#ifdef _WIN32
 ".exe"
#else
 ""
#endif
 );snprintf(log,sizeof(log),"%s%coutput.txt",dir,SEP);
 if(!writeFile(src,code)){snprintf(r.output,sizeof(r.output),"Could not save source for compilation.");goto done;}
 const char *compiler=isCpp(id)?"g++":"gcc";
#ifdef __APPLE__
 compiler=isCpp(id)?"clang++":"clang";
#elif defined(_WIN32)
 /* MSYS2 UCRT64 installation typically puts GCC in this directory. */
 const char *configured=isCpp(id)?"C:\\msys64\\ucrt64\\bin\\g++.exe":"C:\\msys64\\ucrt64\\bin\\gcc.exe";
 if(GetFileAttributesA(configured)!=INVALID_FILE_ATTRIBUTES)compiler=configured;
#endif
 const char *args[]={compiler,isCpp(id)?"-std=c++17":"-std=c11","-O0","-Wall","-Wextra",src,"-o",exe,NULL};
 int timed=0;int exitcode=process(compiler,args,log,12000,&timed);readFile(log,r.output,sizeof(r.output));
 if(timed){r.timedOut=1;append(r.output,sizeof(r.output),"\nCompiler timed out (12 seconds).\n");goto done;}
 if(exitcode!=0){if(!r.output[0])snprintf(r.output,sizeof(r.output),"Compilation failed. Ensure GCC/G++ (or Clang) is installed and on PATH.\n");goto done;}
 r.compiled=1;const char *runArgs[]={exe,NULL};timed=0;exitcode=process(exe,runArgs,log,3000,&timed);readFile(log,r.output,sizeof(r.output));
 if(timed){r.timedOut=1;append(r.output,sizeof(r.output),"\nProgram timed out (3 seconds). Check for an infinite loop.\n");goto done;}
 if(exitcode!=0){char tmp[100];snprintf(tmp,sizeof(tmp),"\nProgram ended with exit code %d.\n",exitcode);append(r.output,sizeof(r.output),tmp);goto done;}
 r.ran=1;
 if(check){pathFor(expectedFile,sizeof(expectedFile),"lab_templates",id,"expected");readFile(expectedFile,expected,sizeof(expected));if(!expected[0]){append(r.output,sizeof(r.output),"\nNo automated reference test is available for this lab.\n");goto done;}
 r.supported=1;char got[LAB_OUTPUT_CAP];snprintf(got,sizeof(got),"%s",r.output);trim(got);trim(expected);r.passed=strcmp(got,expected)==0;
 append(r.output,sizeof(r.output),r.passed?"\n\nPASS - Output matches the reference tests.\n":"\n\nNOT YET - Output differs from the reference tests.\n");
 if(!r.passed){append(r.output,sizeof(r.output),"Expected:\n");append(r.output,sizeof(r.output),expected);append(r.output,sizeof(r.output),"\n");}
 }
 else append(r.output,sizeof(r.output),"\n[Run finished successfully]\n");
 done:
 remove(src);remove(exe);remove(log);
#ifdef _WIN32
 RemoveDirectoryA(dir);
#else
 rmdir(dir);
#endif
 return r;
}
