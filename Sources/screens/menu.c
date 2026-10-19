/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _GNU_SOURCE
#include "sdl-api.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <math.h>
static char directory[4000],modepath[4096],toppath[4096],languagepath[4096];
static const char *lamp[2]={"/sys/class/backlight/backlight/brightness","/sys/class/backlight/backlight1/brightness"};
static int english,language_error;
static int selected,stage,probe_saved=-1,probe_index=0,probe_active;
static uint32_t probe_until,axis_until;
static SDL_Window *window;static SDL_Renderer *renderer;static void *font,*smallfont;
static int (*ttfInit)(void);static void*(*ttfOpen)(const char*,int);static SDL_Surface*(*ttfRender)(void*,const char*,SDL_Color);static void(*ttfClose)(void*);static void(*ttfQuit)(void);
static int read_number(const char *path,int *out){FILE*f=fopen(path,"r");if(!f)return 0;int v=-1;int ok=fscanf(f,"%d",&v)==1;fclose(f);if(!ok||v<0||v>255)return 0;*out=v;return 1;}
static int write_number(const char *path,int value){FILE*f=fopen(path,"w");if(!f)return 0;int ok=fprintf(f,"%d\n",value)>0;return fclose(f)==0&&ok;}
static void restore_probe(void){if(probe_active){write_number(lamp[probe_index],probe_saved);probe_active=0;}}
static void interrupted(int sig){restore_probe();_exit(128+sig);}
static int save_text(const char *path,const char *text){char temp[4096];snprintf(temp,sizeof(temp),"%s.new",path);FILE*f=fopen(temp,"w");if(!f)return 0;int ok=fprintf(f,"%s\n",text)>0;if(fclose(f)!=0)ok=0;if(ok)ok=rename(temp,path)==0;return ok;}
static void label(const char *text,int y,void *face,SDL_Color color){SDL_Surface*s=ttfRender(face,text,color);if(!s)return;SDL_Texture*t=pSDL_CreateTextureFromSurface(renderer,s);pSDL_FreeSurface(s);if(!t)return;int w,h;pSDL_QueryTexture(t,NULL,NULL,&w,&h);SDL_Rect dst={(1024-w)/2,y,w,h};pSDL_RenderCopy(renderer,t,NULL,&dst);pSDL_DestroyTexture(t);}
static void box(int index,const char *text,const char *sub){int y=260+index*140;SDL_Rect rect={90,y,844,114};
 pSDL_SetRenderDrawColor(renderer,selected==index?31:30,selected==index?117:41,selected==index?172:59,255);pSDL_RenderFillRect(renderer,&rect);
 label(text,y+17,font,(SDL_Color){245,249,255,255});if(sub)label(sub,y+64,smallfont,(SDL_Color){207,226,244,255});}
static const char *tr(const char *pt,const char *en){return english?en:pt;}
static int choice_count(void){return stage==1?0:stage==0?2:3;}
static int hit_button(int x,int y){
 if(x<90||x>934)return -1;
 for(int i=0;i<choice_count();i++)if(y>=260+i*140&&y<=374+i*140)return i;
 return -1;
}
static void load_language(void){
 english=0;FILE*f=fopen(languagepath,"r");if(!f)return;
 char code[16]={0};if(fscanf(f,"%15s",code)==1)english=!strcmp(code,"en");fclose(f);
}
static void toggle_language(void){
 int next=!english;
 if(save_text(languagepath,next?"en":"pt")){english=next;language_error=0;fprintf(stderr,"RGDS screen menu: language=%s\n",english?"en":"pt");}
 else language_error=1;
}
static void draw(void){pSDL_SetRenderDrawColor(renderer,15,23,42,255);pSDL_RenderClear(renderer);
 label(tr("ScummVM · Ecrãs","ScummVM · Displays"),65,font,(SDL_Color){248,250,252,255});
 if(stage==0){label(tr("Primeiro vamos identificar o ecrã superior","First identify the upper screen"),155,smallfont,(SDL_Color){203,213,225,255});box(0,tr("Iniciar teste","Start test"),tr("Um painel apaga durante 3 segundos e volta a acender","One panel turns off for 3 seconds, then lights up again"));box(1,tr("Cancelar","Cancel"),NULL);}
 else if(stage==1){label(tr("Observe qual ecrã se apagou","Watch which screen turns off"),310,font,(SDL_Color){248,250,252,255});}
 else if(stage==2){label(tr("Qual ecrã se apagou durante o teste?","Which screen turned off during the test?"),155,smallfont,(SDL_Color){203,213,225,255});box(0,tr("O superior","The upper screen"),NULL);box(1,tr("O inferior","The lower screen"),NULL);box(2,tr("Nenhum / não sei","Neither / not sure"),NULL);}
 else{
  label(tr("Escolha como quer jogar","Choose your display mode"),155,smallfont,(SDL_Color){203,213,225,255});
  box(0,tr("Só ecrã inferior","Lower screen only"),tr("Apaga a iluminação do superior para poupar bateria","Upper backlight off to save battery"));
  box(1,tr("Duplicar nos dois ecrãs","Mirror both screens"),tr("A mesma imagem; o toque mantém-se no inferior","Same image on both; touch stays on the lower screen"));
  box(2,english?"Language / Idioma: English (EN)":"Idioma / Language: Português (PT)","PT / EN · Toque para mudar / Tap to change");
 }
 if(language_error)label(tr("Não foi possível guardar o idioma no cartão.","Could not save the language to the card."),666,smallfont,(SDL_Color){255,180,160,255});
 label(tr("Stylus ou D-pad + A · B cancela","Stylus or D-pad + A · B cancels"),705,smallfont,(SDL_Color){148,163,184,255});pSDL_RenderPresent(renderer);
}
static int activate(void){
 if(stage==0){if(selected==1)return 2;if(!read_number(lamp[0],&probe_saved)||!write_number(lamp[0],0)){fprintf(stderr,"RGDS screens: could not test backlight\n");return 1;}probe_active=1;probe_until=pSDL_GetTicks()+3000;stage=1;selected=0;}
 else if(stage==2){if(selected==2)return 2;probe_index=selected==0?0:1;char value[4];snprintf(value,sizeof(value),"%d",probe_index);if(!save_text(toppath,value))return 1;fprintf(stderr,"RGDS screens: upper backlight identified as %s\n",lamp[probe_index]);stage=3;selected=0;}
 else if(stage==3){if(selected==2){toggle_language();return -1;}if(!save_text(modepath,selected==0?"off":"mirror"))return 1;fprintf(stderr,"RGDS screens: chosen mode=%s\n",selected==0?"off":"mirror");return 0;}
 return -1;
}
int main(int argc,char**argv){
 if(argc!=2||strlen(argv[1])>=sizeof(directory))return 1;
 snprintf(directory,sizeof(directory),"%s",argv[1]);snprintf(modepath,sizeof(modepath),"%s/config/screen-mode.txt",directory);snprintf(toppath,sizeof(toppath),"%s/config/top-backlight.txt",directory);
 snprintf(languagepath,sizeof(languagepath),"%s/config/startup-language.txt",directory);load_language();
 fprintf(stderr,"RGDS screen menu: startup language=%s\n",english?"en":"pt");
 int mapped=-1;if(read_number(toppath,&mapped)&&mapped<=1)stage=3;else stage=0;
 FILE*mf=fopen(modepath,"r");if(mf){char m[16]={0};int got=fscanf(mf,"%15s",m);if(got!=1)m[0]=0;fclose(mf);selected=strcmp(m,"mirror")==0?1:0;}
 signal(SIGTERM,interrupted);signal(SIGINT,interrupted);signal(SIGHUP,interrupted);atexit(restore_probe);
 void *sdl=dlopen("libSDL2-2.0.so.0",RTLD_NOW|RTLD_LOCAL);void *ttf=dlopen("libSDL2_ttf-2.0.so.0",RTLD_NOW|RTLD_LOCAL);
 if(!sdl||!ttf||!api_load(sdl,0)){fprintf(stderr,"RGDS screen menu: library load failed %s\n",dlerror());return 1;}
#define T(name,sym) *(void**)(&name)=dlsym(ttf,sym);if(!name)return 1
 T(ttfInit,"TTF_Init");T(ttfOpen,"TTF_OpenFont");T(ttfRender,"TTF_RenderUTF8_Blended");T(ttfClose,"TTF_CloseFont");T(ttfQuit,"TTF_Quit");
 if(pSDL_Init(0x20|0x200|0x4000)<0||ttfInit()<0)return 1;
 const char *fontpath=getenv("RGDS_SCREEN_FONT");if(!fontpath)fontpath="/mnt/vendor/bin/default.ttf";
 font=ttfOpen(fontpath,36);smallfont=ttfOpen(fontpath,24);if(!font||!smallfont){fprintf(stderr,"RGDS screens: font unavailable\n");return 1;}
 int display=pSDL_GetNumVideoDisplays()>1?1:0;
 window=pSDL_CreateWindow("ScummVM - Ecrãs",0x2fff0000|display,0x2fff0000|display,1024,768,4|16);
 if(!window)return 1;
 pSDL_SetWindowFullscreen(window,FULLSCREEN_DESKTOP);pSDL_ShowWindow(window);pSDL_RaiseWindow(window);renderer=pSDL_CreateRenderer(window,-1,1);
 if(!renderer)return 1;
 pSDL_JoystickOpen(0);
 fprintf(stderr,"RGDS screen menu: display=%d\n",pSDL_GetWindowDisplayIndex(window));
 int fd=open("/dev/input/by-path/platform-fe5e0000.i2c-event",O_RDONLY|O_NONBLOCK|O_CLOEXEC);if(fd<0)fd=open("/dev/input/event1",O_RDONLY|O_NONBLOCK|O_CLOEXEC);
 double coef[6]={1,0,0,0,1,0};char path[4096];snprintf(path,sizeof(path),"%s/config/touch-calibration.txt",directory);FILE*cf=fopen(path,"r");if(cf){char magic[64];double c[6];if(fscanf(cf,"%63s %lf %lf %lf %lf %lf %lf",magic,c,c+1,c+2,c+3,c+4,c+5)==7&&!strcmp(magic,"RGDS_TOUCH_CALIBRATION_V1")){int valid=1;for(int i=0;i<6;i++)if(!isfinite(c[i]))valid=0;if(valid)memcpy(coef,c,sizeof(coef));}fclose(cf);}
 int result=-1,rawx=0,rawy=0,down=0,previous=0,slot=0;uint32_t began=pSDL_GetTicks();
 while(result<0){
  if(pSDL_GetTicks()-began>120000){result=2;break;}
  if(stage==1&& (int32_t)(pSDL_GetTicks()-probe_until)>=0){restore_probe();stage=2;selected=0;}
  int click=-1;
  if(fd>=0){struct input_event ie;for(int n=0;n<128&&read(fd,&ie,sizeof(ie))==sizeof(ie);n++){
   if(ie.type==EV_ABS){if(ie.code==ABS_MT_SLOT)slot=ie.value;else if(slot==0){if(ie.code==ABS_MT_POSITION_X)rawx=ie.value;else if(ie.code==ABS_MT_POSITION_Y)rawy=ie.value;else if(ie.code==ABS_MT_TRACKING_ID)down=ie.value>=0;}}
   else if(ie.type==EV_SYN&&ie.code==SYN_REPORT){if(previous&&!down){double u=rawx/1024.,v=rawy/768.;int x=(coef[0]*u+coef[1]*v+coef[2])*1023,y=(coef[3]*u+coef[4]*v+coef[5])*767;click=hit_button(x,y);}previous=down;}
  }}
  SDL_Event ev;while(pSDL_PollEvent(&ev)){
   int confirm=0,cancel=0,delta=0;
   if(ev.type==0x100)cancel=1;
   if(ev.type==0x300){int sym;memcpy(&sym,ev.bytes+20,4);if(sym==13)confirm=1;else if(sym==27)cancel=1;else if(sym==1073741906)delta=-1;else if(sym==1073741905)delta=1;}
   if(ev.type==0x603||ev.type==0x651){if(ev.bytes[12]==0)confirm=1;else if(ev.bytes[12]==1)cancel=1;else if(ev.type==0x651&&ev.bytes[12]==11)delta=-1;else if(ev.type==0x651&&ev.bytes[12]==12)delta=1;}
   if(ev.type==0x600&&ev.bytes[12]==1&&pSDL_GetTicks()>=axis_until){int16_t axis;memcpy(&axis,ev.bytes+16,2);if(axis < -16000)delta=-1;else if(axis > 16000)delta=1;if(delta)axis_until=pSDL_GetTicks()+250;}
   if(ev.type==0x602){if(ev.bytes[13]&1)delta=-1;else if(ev.bytes[13]&4)delta=1;}
   if(fd<0&&ev.type==0x402){int x,y;memcpy(&x,ev.bytes+20,4);memcpy(&y,ev.bytes+24,4);click=hit_button(x,y);}
   if(cancel){result=2;break;}
   if(stage!=1){int count=choice_count();if(delta)selected=(selected+delta+count)%count;if(confirm)result=activate();}
  }
  if(result<0&&click>=0&&stage!=1){selected=click;result=activate();}
  draw();pSDL_Delay(16);
 }
 restore_probe();if(fd>=0)close(fd);ttfClose(font);ttfClose(smallfont);ttfQuit();pSDL_DestroyRenderer(renderer);pSDL_DestroyWindow(window);pSDL_Quit();return result;
}
