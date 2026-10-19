/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _GNU_SOURCE
#include "sdl-api.h"
static SDL_Window *upper;static SDL_Renderer *renderer;static SDL_Texture *texture;
static unsigned char *pixels,*flipped;static int width,height,ready,busy,failed;static uint32_t last_frame;
static void cleanup(void){
 if(texture)pSDL_DestroyTexture(texture);if(renderer)pSDL_DestroyRenderer(renderer);if(upper)pSDL_DestroyWindow(upper);
 free(pixels);free(flipped);upper=NULL;renderer=NULL;texture=NULL;pixels=NULL;flipped=NULL;width=height=0;
}
static int initialize(void){if(ready)return 1;if(failed)return 0;ready=api_load(RTLD_NEXT,1);failed=!ready;return ready;}
static int prepare(SDL_Window *source,int w,int h){
 if(w<1||h<1||w>4096||h>4096)return 0;
 if(!upper){
  if(pSDL_GetNumVideoDisplays()<2){fprintf(stderr,"RGDS mirror: second display unavailable\n");failed=1;return 0;}
  upper=pSDL_CreateWindow("ScummVM - ecrã superior",0x2fff0000,0x2fff0000,1024,768,4|16|0x10000);
  if(upper){pSDL_SetWindowFullscreen(upper,FULLSCREEN_DESKTOP);pSDL_ShowWindow(upper);renderer=pSDL_CreateRenderer(upper,-1,1);}
  pSDL_RaiseWindow(source);
  if(!upper||!renderer){fprintf(stderr,"RGDS mirror: initialization failed: %s\n",pSDL_GetError());failed=1;cleanup();return 0;}
  fprintf(stderr,"RGDS mirror: upper window on display %d\n",pSDL_GetWindowDisplayIndex(upper));
  if(pSDL_GetWindowDisplayIndex(upper)!=0){failed=1;cleanup();pSDL_RaiseWindow(source);return 0;}
 }
 if(w!=width||h!=height){
  if(texture)pSDL_DestroyTexture(texture);texture=NULL;
  free(pixels);free(flipped);pixels=malloc((size_t)w*h*4);flipped=malloc((size_t)w*h*4);
  texture=pSDL_CreateTexture(renderer,RGBA32,1,w,h);width=w;height=h;
  if(!pixels||!flipped||!texture){failed=1;cleanup();return 0;}
 }
 return 1;
}
static void show_copy(const void *data){
 if(pSDL_UpdateTexture(texture,NULL,data,width*4)<0)return;
 pSDL_SetRenderDrawColor(renderer,0,0,0,255);pSDL_RenderClear(renderer);
 int w,h;pSDL_GetRendererOutputSize(renderer,&w,&h);double factor=(double)w/width;
 if((double)h/height<factor)factor=(double)h/height;
 SDL_Rect dst={(w-(int)(width*factor))/2,(h-(int)(height*factor))/2,(int)(width*factor),(int)(height*factor)};
 pSDL_RenderCopy(renderer,texture,NULL,&dst);pSDL_RenderPresent(renderer);
}
void SDL_RenderPresent(SDL_Renderer *source){
 if(!initialize()){void (*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");if(real)real(source);return;}
 if(busy||failed){pSDL_RenderPresent(source);return;}
 SDL_Window *window=pSDL_RenderGetWindow(source);uint32_t now=pSDL_GetTicks();
 if(!window||pSDL_GetWindowDisplayIndex(window)!=1||now-last_frame<50){pSDL_RenderPresent(source);return;}
 busy=1;int w,h;pSDL_GetRendererOutputSize(source,&w,&h);
 if(prepare(window,w,h)&&pSDL_RenderReadPixels(source,NULL,RGBA32,pixels,w*4)==0){show_copy(pixels);last_frame=now;}
 busy=0;pSDL_RenderPresent(source);
}
void SDL_GL_SwapWindow(SDL_Window *window){
 if(!initialize()){void (*real)(SDL_Window*)=dlsym(RTLD_NEXT,"SDL_GL_SwapWindow");if(real)real(window);return;}
 if(busy||failed||pSDL_GetWindowDisplayIndex(window)!=1||pSDL_GetTicks()-last_frame<50){pSDL_GL_SwapWindow(window);return;}
 busy=1;void *context=pSDL_GL_GetCurrentContext();int w,h;pSDL_GL_GetDrawableSize(window,&w,&h);
 if(prepare(window,w,h)){
  pSDL_GL_MakeCurrent(window,context);
  void (*readpixels)(int,int,int,int,unsigned,unsigned,void*)=pSDL_GL_GetProcAddress("glReadPixels");
  void (*getint)(unsigned,int*)=pSDL_GL_GetProcAddress("glGetIntegerv");
  void (*bindfb)(unsigned,unsigned)=pSDL_GL_GetProcAddress("glBindFramebuffer");
  unsigned (*geterror)(void)=pSDL_GL_GetProcAddress("glGetError");
  if(readpixels&&getint&&bindfb&&geterror){
   int fb=0;getint(0x8ca6,&fb);if(fb)bindfb(0x8d40,0);
   readpixels(0,0,w,h,0x1908,0x1401,pixels);unsigned error=geterror();if(fb)bindfb(0x8d40,fb);
   if(!error){for(int y=0;y<h;y++)memcpy(flipped+(size_t)y*w*4,pixels+(size_t)(h-1-y)*w*4,(size_t)w*4);show_copy(flipped);last_frame=pSDL_GetTicks();}
   else if(!failed){fprintf(stderr,"RGDS mirror: GL capture error=%u\n",error);failed=1;}
  }
 }
 pSDL_GL_MakeCurrent(window,context);busy=0;pSDL_GL_SwapWindow(window);
}
int SDL_PollEvent(SDL_Event *event){
 if(!initialize()){int (*real)(SDL_Event*)=dlsym(RTLD_NEXT,"SDL_PollEvent");return real?real(event):0;}int result;
 while((result=pSDL_PollEvent(event))){
  if(!upper||!event)return result;
  uint32_t id=0;memcpy(&id,event->bytes+8,4);
  if(id!=pSDL_GetWindowID(upper)||!(event->type==0x200||(event->type>=0x300&&event->type<=0x303)||(event->type>=0x400&&event->type<=0x403)))return result;
 }
 return result;
}
void SDL_Quit(void){
 if(!initialize()){void(*real)(void)=dlsym(RTLD_NEXT,"SDL_Quit");if(real)real();return;}busy=1;cleanup();pSDL_Quit();ready=failed=busy=0;
}
