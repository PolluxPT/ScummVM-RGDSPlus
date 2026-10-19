#ifndef BASE_RGDS_CALIBRATION_H
#define BASE_RGDS_CALIBRATION_H
class OSystem;
bool rgdsTouchCalibrationAvailable();
bool requestRgdsRecalibration();
int runRgdsCalibrationIfRequested(OSystem &system);
#endif
