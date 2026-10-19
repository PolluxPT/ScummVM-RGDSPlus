#define main app_main
#include "../menu.c"
#undef main
#include <assert.h>
static uint32_t ticks(void){return 1000;}
int main(void){char tmp[]="/tmp/rgds-menu-XXXXXX";assert(mkdtemp(tmp));char a[512],b[512];snprintf(a,sizeof(a),"%s/brightness",tmp);snprintf(b,sizeof(b),"%s/other",tmp);lamp[0]=a;lamp[1]=b;write_number(a,70);write_number(b,51);snprintf(toppath,sizeof(toppath),"%s/top",tmp);snprintf(modepath,sizeof(modepath),"%s/mode",tmp);pSDL_GetTicks=ticks;
 stage=0;selected=0;assert(activate()==-1);int n;assert(read_number(a,&n)&&n==0);assert(stage==1&&probe_active);restore_probe();assert(read_number(a,&n)&&n==70);
 stage=2;selected=1;assert(activate()==-1);assert(read_number(toppath,&n)&&n==1);assert(stage==3);selected=1;assert(activate()==0);FILE*f=fopen(modepath,"r");char value[16];assert(fscanf(f,"%15s",value)==1);fclose(f);assert(!strcmp(value,"mirror"));
 stage=2;selected=2;assert(activate()==2);assert(read_number(toppath,&n)&&n==1);
 unlink(a);unlink(b);unlink(toppath);unlink(modepath);rmdir(tmp);puts("Menu probe/save/cancel tests PASS");}
