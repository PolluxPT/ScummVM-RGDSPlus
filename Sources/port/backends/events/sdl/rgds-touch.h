#ifndef RGDS_TOUCH_H
#define RGDS_TOUCH_H
#include "backends/platform/sdl/sdl-sys.h"
#include "backends/events/sdl/rgds-touch-state.h"
#include "backends/events/sdl/rgds-calibration.h"
static const Uint32 RGDS_DIRECT_MOUSE_ID = 0x52474453;

class RgdsTouchReader {
public:
	RgdsTouchReader();
	~RgdsTouchReader();
	bool active() const { return _fd >= 0; }
	bool recent() const { return _lastTouchTick && SDL_GetTicks() - _lastTouchTick < 200; }
	void pump(SDL_Window *window);
private:
	int _fd, _minX, _maxX, _minY, _maxY, _logs, _rawLogs;
	bool _pumpLogged;
	bool _swap, _invertX, _invertY;
	bool _calibrated;
	RgdsCalibration _calibration;
	Uint32 _lastTouchTick;
	RgdsTouchState _state;
};
#endif
