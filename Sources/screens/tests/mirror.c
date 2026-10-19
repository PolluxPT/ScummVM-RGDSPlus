#include "../mirror.c"
#include <assert.h>
static int originals,copies,restores,swapcount,events;
static SDL_Window*w1=(SDL_Window*)1,*w2=(SDL_Window*)2;
static SDL_Renderer*r1=(SDL_Renderer*)3,*r2=(SDL_Renderer*)4;
static unsigned char capture[16];
static uint32_t ticks(void){return 1000;}
static int displays(void){return 2;}
static SDL_Window*create(const char*t,int x,int y,int w,int h,uint32_t f){return w2;}
static int fullscreen(SDL_Window*w,uint32_t f){return 0;}
static void noopwindow(SDL_Window*w){}
static void noopr(SDL_Renderer*r){}
static void noopt(SDL_Texture*t){}
static SDL_Renderer*cr(SDL_Window*w,int n,uint32_t f){return r2;}
static int display(SDL_Window*w){return w==w1?1:0;}
static SDL_Window*rw(SDL_Renderer*r){return r==r1?w1:w2;}
static int size(SDL_Renderer*r,int*w,int*h){*w=2;*h=2;return 0;}
static SDL_Texture*ct(SDL_Renderer*r,uint32_t f,int a,int w,int h){return (SDL_Texture*)5;}
static int readpx(SDL_Renderer*r,const SDL_Rect*rect,uint32_t f,void*p,int pitch){memset(p,77,16);return 0;}
static int update(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){memcpy(capture,p,16);copies++;return 0;}
static int color(SDL_Renderer*r,uint8_t a,uint8_t b,uint8_t c,uint8_t d){return 0;}
static int clear(SDL_Renderer*r){return 0;}
static int copy(SDL_Renderer*r,SDL_Texture*t,const SDL_Rect*a,const SDL_Rect*b){return 0;}
static void present(SDL_Renderer*r){if(r==r1)originals++;}
static void*context(void){return (void*)6;}
static int current(SDL_Window*w,void*c){assert(w==w1&&c==(void*)6);restores++;return 0;}
static void drawable(SDL_Window*w,int*x,int*y){*x=*y=2;}
static void glread(int x,int y,int w,int h,unsigned f,unsigned t,void*p){memset(p,11,8);memset((char*)p+8,22,8);}
static void glint(unsigned p,int*x){*x=0;}
static void glbind(unsigned a,unsigned b){}
static unsigned glerror(void){return 0;}
static void*proc(const char*n){if(!strcmp(n,"glReadPixels"))return glread;if(!strcmp(n,"glGetIntegerv"))return glint;if(!strcmp(n,"glBindFramebuffer"))return glbind;if(!strcmp(n,"glGetError"))return glerror;return NULL;}
static void swap(SDL_Window*w){assert(w==w1);swapcount++;}
static uint32_t id(SDL_Window*w){return w==w1?1:2;}
static int poll(SDL_Event*e){if(events==2)return 0;e->type=0x401;uint32_t n=events++?1:2;memcpy(e->bytes+8,&n,4);return 1;}
static void quit(void){}
int main(void){ready=1;pSDL_GetTicks=ticks;pSDL_GetNumVideoDisplays=displays;pSDL_CreateWindow=create;pSDL_SetWindowFullscreen=fullscreen;pSDL_ShowWindow=noopwindow;pSDL_RaiseWindow=noopwindow;pSDL_CreateRenderer=cr;pSDL_GetWindowDisplayIndex=display;pSDL_RenderGetWindow=rw;pSDL_GetRendererOutputSize=size;pSDL_CreateTexture=ct;pSDL_RenderReadPixels=readpx;pSDL_UpdateTexture=update;pSDL_SetRenderDrawColor=color;pSDL_RenderClear=clear;pSDL_RenderCopy=copy;pSDL_RenderPresent=present;pSDL_DestroyTexture=noopt;pSDL_DestroyRenderer=noopr;pSDL_DestroyWindow=noopwindow;pSDL_GL_GetCurrentContext=context;pSDL_GL_MakeCurrent=current;pSDL_GL_GetDrawableSize=drawable;pSDL_GL_GetProcAddress=proc;pSDL_GL_SwapWindow=swap;pSDL_GetWindowID=id;pSDL_PollEvent=poll;pSDL_Quit=quit;
 SDL_RenderPresent(r1);assert(originals==1&&copies==1&&capture[0]==77);SDL_RenderPresent(r1);assert(originals==2&&copies==1);
 last_frame=0;SDL_GL_SwapWindow(w1);assert(swapcount==1&&copies==2&&capture[0]==22&&capture[8]==11&&restores>=2);
 SDL_Event e;assert(SDL_PollEvent(&e)==1&&events==2);assert(SDL_PollEvent(&e)==0);
 failed=1;SDL_RenderPresent(r1);assert(originals==3);SDL_GL_SwapWindow(w1);assert(swapcount==2);failed=0;SDL_Quit();assert(!upper&&!pixels);puts("Mirror 2D/GL flip/context/input isolation/failure forwarding PASS");}
