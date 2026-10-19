#define main screen_app_main
#include "../menu.c"
#undef main
#include <assert.h>
int main(void){void*sdl=dlopen("libSDL2-2.0.so.0",RTLD_NOW);void*ttf=dlopen("libSDL2_ttf-2.0.so.0",RTLD_NOW);assert(sdl&&ttf&&api_load(sdl,0));
 *(void**)(&ttfInit)=dlsym(ttf,"TTF_Init");*(void**)(&ttfOpen)=dlsym(ttf,"TTF_OpenFont");*(void**)(&ttfRender)=dlsym(ttf,"TTF_RenderUTF8_Blended");assert(pSDL_Init(0x20)==0&&ttfInit()==0);font=ttfOpen("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",36);smallfont=ttfOpen("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",24);assert(font&&smallfont);window=pSDL_CreateWindow("Test",0,0,1024,768,4);renderer=pSDL_CreateRenderer(window,-1,1);assert(renderer);stage=3;
 for(int lang=0;lang<2;lang++){english=lang;selected=2;draw();unsigned char*p=malloc(1024*768*4);assert(pSDL_RenderReadPixels(renderer,NULL,RGBA32,p,4096)==0);char name[128];snprintf(name,sizeof(name),"tests/menu-%s.ppm",lang?"en":"pt");FILE*f=fopen(name,"wb");fprintf(f,"P6\n1024 768\n255\n");for(int i=0;i<1024*768;i++)fwrite(p+4*i,1,3,f);fclose(f);free(p);}
 pSDL_Quit();puts("PT/EN menu rendering PASS");}
