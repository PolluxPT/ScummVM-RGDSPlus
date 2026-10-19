#include "backends/events/sdl/rgds-volume.cpp"
#include <cassert>
#include <climits>
#include <initializer_list>
static void fileLevel(int n) { FILE *f=fopen(volumePath,"w"); assert(f); fprintf(f,"%d\n",n); fclose(f); }
int main() {
 const int16_t values[]={INT16_MIN,-1000,-1,0,1,1000,INT16_MAX};
 for(int percent: {0,10,20,50,90,100}) {
  gain.store(percent); unsigned char bytes[sizeof(values)+1]; memcpy(bytes,values,sizeof(values));bytes[sizeof(values)]=0xA7;
  rgdsApplyVolume(bytes,sizeof(bytes));
  for(unsigned i=0;i<sizeof(values)/sizeof(values[0]);++i){ int16_t out;memcpy(&out,bytes+i*2,2);assert(out==int32_t(values[i])*percent/100); }
  assert(bytes[sizeof(values)]==0xA7);
 }
 char file[]="/tmp/rgds-volume-XXXXXX";int tmp=mkstemp(file);assert(tmp>=0);close(tmp);volumePath=file;enabled=true;
 for(int base: {0,5,10}) for(int key: {KEY_VOLUMEDOWN,KEY_VOLUMEUP}) {
  char events[]="/tmp/rgds-volume-events-XXXXXX";int fd=mkstemp(events);assert(fd>=0);
  input_event e={};e.type=EV_KEY;e.code=key;e.value=1;assert(write(fd,&e,sizeof(e))==sizeof(e));lseek(fd,0,SEEK_SET);
  keyCount=1;keyFds[0]=fd;fileLevel(base);level=base;lastRead=0;rgdsVolumePoll();
  int expected=base+(key==KEY_VOLUMEUP?1:-1);if(expected<0)expected=0;if(expected>10)expected=10;
  assert(level==expected&&gain.load()==expected*10);int saved=-1;assert(readLevel(saved)&&saved==expected);
  close(fd);unlink(events);
 }
 keyCount=0;fileLevel(3);lastRead=0;rgdsVolumePoll();assert(gain.load()==30);
 fileLevel(11);int invalid;assert(!readLevel(invalid));unlink(file);
 puts("ARM volume tests passed: silence, PCM gain, signed extremes, odd byte, keys, boundaries, firmware save, external change, invalid input.");
}
