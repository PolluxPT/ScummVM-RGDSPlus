/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _GNU_SOURCE
#include "sdl-api.h"
static SDL_Window *upper;static SDL_Renderer *renderer;
static int ready,failed,busy,presented,applied,upper_saved,lower_saved;
static uint32_t apply_after;
static const char *upper_path,*lower_path;
static int read_level(const char *path,int *value){
 FILE*f=fopen(path,"r");if(!f)return 0;int ok=fscanf(f,"%d",value)==1;fclose(f);
 return ok&&*value>=0&&*value<=255;
}
static int write_level(const char *path,int value){
 FILE*f=fopen(path,"w");if(!f)return 0;int ok=fprintf(f,"%d\n",value)>0;
 return fclose(f)==0&&ok;
}
static int known_path(const char *path){
 return path&&(!strcmp(path,"/sys/class/backlight/backlight/brightness")||!strcmp(path,"/sys/class/backlight/backlight1/brightness"));
}
static void restore_levels(void){
 if(applied){write_level(upper_path,upper_saved);write_level(lower_path,lower_saved);applied=0;}
}
static void apply_levels(void){
 if(!presented||applied||!upper||failed||(int32_t)(pSDL_GetTicks()-apply_after)<0)return;
 if(!read_level(upper_path,&upper_saved)||!read_level(lower_path,&lower_saved)){
  fprintf(stderr,"RGDS lower: brightness unreadable; keeping lighting unchanged\n");failed=1;return;
 }
 applied=1;
 int lit=lower_saved?lower_saved:70;
 // Stock hardware may share an enable line: reassert the lower panel last.
 if(!write_level(upper_path,0)||!write_level(lower_path,lit)){
  fprintf(stderr,"RGDS lower: lighting write failed; restoring both panels\n");restore_levels();failed=1;return;
 }
 int top=-1,bottom=-1;
 if(!read_level(upper_path,&top)||!read_level(lower_path,&bottom)||bottom!=lit){
  fprintf(stderr,"RGDS lower: lower brightness verification failed; restoring both panels\n");restore_levels();failed=1;return;
 }
 fprintf(stderr,"RGDS lower: after first frame upper=%s:%d lower=%s:%d (requested %d)\n",upper_path,top,lower_path,bottom,lit);
 const char*nodes[]={"/sys/class/backlight/backlight/actual_brightness","/sys/class/backlight/backlight1/actual_brightness","/sys/class/backlight/backlight/bl_power","/sys/class/backlight/backlight1/bl_power","/sys/class/anbernic_misc/lcdoff","/sys/class/anbernic_misc/lcdswap"};
 for(unsigned i=0;i<sizeof(nodes)/sizeof(nodes[0]);i++){int v;if(read_level(nodes[i],&v))fprintf(stderr,"RGDS lower: readback %s=%d\n",nodes[i],v);}

}
static int initialize(void){
 if(ready)return 1;if(failed)return 0;
 upper_path=getenv("SCUMMVM_RGDS_UPPER_BACKLIGHT");lower_path=getenv("SCUMMVM_RGDS_LOWER_BACKLIGHT");
 if(!known_path(upper_path)||!known_path(lower_path)||!strcmp(upper_path,lower_path)){failed=1;return 0;}
 ready=api_load(RTLD_NEXT,1);failed=!ready;return ready;
}
static int prepare(SDL_Window *source){
 if(upper)return 1;
 if(pSDL_GetNumVideoDisplays()<2){failed=1;return 0;}
 upper=pSDL_CreateWindow("ScummVM - superior preto",0x2fff0000,0x2fff0000,1024,768,4|16|0x10000);
 if(upper){pSDL_SetWindowFullscreen(upper,FULLSCREEN_DESKTOP);pSDL_ShowWindow(upper);renderer=pSDL_CreateRenderer(upper,-1,1);}
 if(!upper||!renderer||pSDL_GetWindowDisplayIndex(upper)!=0){
  fprintf(stderr,"RGDS lower: upper surface unavailable; keeping lighting unchanged\n");
  if(renderer)pSDL_DestroyRenderer(renderer);if(upper)pSDL_DestroyWindow(upper);renderer=NULL;upper=NULL;failed=1;pSDL_RaiseWindow(source);return 0;
 }
 pSDL_SetRenderDrawColor(renderer,0,0,0,255);pSDL_RenderClear(renderer);pSDL_RenderPresent(renderer);
 pSDL_RaiseWindow(source);
 fprintf(stderr,"RGDS lower: black upper window display=0; lower lighting unchanged until first frame\n");
 return 1;
}
static void frame_submitted(void){
 if(!presented){presented=1;apply_after=pSDL_GetTicks()+250;fprintf(stderr,"RGDS lower: lower frame submitted; lighting change deferred 250 ms\n");}
 apply_levels();
}
void SDL_RenderPresent(SDL_Renderer *source){
 if(!initialize()){void(*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");if(real)real(source);return;}
 if(busy||failed){pSDL_RenderPresent(source);return;}
 SDL_Window*window=pSDL_RenderGetWindow(source);
 if(!window||pSDL_GetWindowDisplayIndex(window)!=1){pSDL_RenderPresent(source);return;}
 busy=1;int ok=prepare(window);pSDL_RenderPresent(source);if(ok)frame_submitted();busy=0;
}
void SDL_GL_SwapWindow(SDL_Window *window){
 if(!initialize()){void(*real)(SDL_Window*)=dlsym(RTLD_NEXT,"SDL_GL_SwapWindow");if(real)real(window);return;}
 if(busy||failed||pSDL_GetWindowDisplayIndex(window)!=1){pSDL_GL_SwapWindow(window);return;}
 busy=1;void*context=pSDL_GL_GetCurrentContext();int ok=prepare(window);pSDL_GL_MakeCurrent(window,context);
 pSDL_GL_SwapWindow(window);if(ok)frame_submitted();busy=0;
}
int SDL_PollEvent(SDL_Event *event){
 if(!initialize()){int(*real)(SDL_Event*)=dlsym(RTLD_NEXT,"SDL_PollEvent");return real?real(event):0;}
 apply_levels();int result;
 while((result=pSDL_PollEvent(event))){
  if(!upper||!event)return result;uint32_t id;memcpy(&id,event->bytes+8,4);
  if(id!=pSDL_GetWindowID(upper)||!(event->type==0x200||(event->type>=0x300&&event->type<=0x303)||(event->type>=0x400&&event->type<=0x403)))return result;
 }
 return result;
}
void SDL_Quit(void){
 if(!initialize()){void(*real)(void)=dlsym(RTLD_NEXT,"SDL_Quit");if(real)real();return;}
 busy=1;restore_levels();if(renderer)pSDL_DestroyRenderer(renderer);if(upper)pSDL_DestroyWindow(upper);
 upper=NULL;renderer=NULL;ready=failed=busy=presented=applied=0;pSDL_Quit();
}
