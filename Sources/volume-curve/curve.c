/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdint.h>
extern int rgds_gain __attribute__((visibility("hidden")));
extern unsigned char rgds_enabled __attribute__((visibility("hidden")));
void rgds_curve(unsigned char *samples,int length) {
    if(!rgds_enabled)return;
    int percent=__atomic_load_n(&rgds_gain,__ATOMIC_ACQUIRE);
    int coefficient=percent*percent*35;
    for(int i=0;i+1<length;i+=2){
        int16_t sample;__builtin_memcpy(&sample,samples+i,2);
        sample=(int64_t)sample*coefficient/1000000;
        __builtin_memcpy(samples+i,&sample,2);
    }
}
