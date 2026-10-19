#ifndef RGDS_VOLUME_H
#define RGDS_VOLUME_H
void rgdsVolumeInit();
void rgdsVolumePoll();
void rgdsApplyVolume(unsigned char *samples, int length);
#endif
