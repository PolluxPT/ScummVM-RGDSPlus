#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "backends/events/sdl/rgds-touch.h"
#include "backends/events/sdl/rgds-volume.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

RgdsTouchReader::RgdsTouchReader() : _fd(-1), _minX(0), _maxX(1023),
	_minY(0), _maxY(767), _logs(0), _rawLogs(0), _pumpLogged(false), _swap(false), _invertX(false), _invertY(false), _calibrated(false), _lastTouchTick(0) {
	rgdsVolumeInit();
	const char *device = getenv("SCUMMVM_RGDS_TOUCH_DEVICE");
	if (!device || !*device) return;
	_fd = open(device, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (_fd < 0) {
		fprintf(stderr, "RGDS evdev open failed: %s: %s\n", device, strerror(errno));
		return;
	}
	char name[256] = {};
	ioctl(_fd, EVIOCGNAME(sizeof(name)), name);
	// Only accept the touchscreen confirmed by the device's diagnostic.
	if (!strstr(name, "gt9xx")) {
		fprintf(stderr, "RGDS evdev rejected unexpected device: %s\n", name);
		close(_fd); _fd = -1; return;
	}
	input_absinfo ax = {}, ay = {}, tracking = {};
	bool mt = ioctl(_fd, EVIOCGABS(ABS_MT_POSITION_X), &ax) == 0 &&
		ioctl(_fd, EVIOCGABS(ABS_MT_POSITION_Y), &ay) == 0;
	if (!mt && (ioctl(_fd, EVIOCGABS(ABS_X), &ax) < 0 ||
		ioctl(_fd, EVIOCGABS(ABS_Y), &ay) < 0)) {
		fprintf(stderr, "RGDS evdev missing coordinate axes\n");
		close(_fd); _fd = -1; return;
	}
	_minX = ax.minimum; _maxX = ax.maximum;
	_minY = ay.minimum; _maxY = ay.maximum;
	const char *calibrationPath = getenv("SCUMMVM_RGDS_TOUCH_CALIBRATION");
	if (calibrationPath) {
		FILE *file = fopen(calibrationPath, "r");
		if (file) {
			char magic[64] = {};
			if (fscanf(file, "%63s %lf %lf %lf %lf %lf %lf", magic,
				&_calibration.a,&_calibration.b,&_calibration.c,&_calibration.d,&_calibration.e,&_calibration.f) == 7 &&
				strcmp(magic,"RGDS_TOUCH_CALIBRATION_V1")==0 && _calibration.valid()) _calibrated=true;
			fclose(file);
			fprintf(stderr,"RGDS calibration: %s (%s)\n",_calibrated ? "loaded" : "invalid, using default",calibrationPath);
		}
	}
	_state = RgdsTouchState(mt && ioctl(_fd, EVIOCGABS(ABS_MT_TRACKING_ID), &tracking) == 0);
	_swap = getenv("SCUMMVM_RGDS_TOUCH_SWAP_XY") && atoi(getenv("SCUMMVM_RGDS_TOUCH_SWAP_XY"));
	_invertX = getenv("SCUMMVM_RGDS_TOUCH_INVERT_X") && atoi(getenv("SCUMMVM_RGDS_TOUCH_INVERT_X"));
	_invertY = getenv("SCUMMVM_RGDS_TOUCH_INVERT_Y") && atoi(getenv("SCUMMVM_RGDS_TOUCH_INVERT_Y"));
	fprintf(stderr, "RGDS evdev opened %s (%s) axes X=%d..%d Y=%d..%d; swap=%d invert=%d,%d\n",
		device, name, _minX, _maxX, _minY, _maxY, _swap, _invertX, _invertY);
}

RgdsTouchReader::~RgdsTouchReader() { if (_fd >= 0) close(_fd); }

void RgdsTouchReader::pump(SDL_Window *window) {
	rgdsVolumePoll();
	if (!_pumpLogged) {
		fprintf(stderr, "RGDS evdev pump entered: fd=%d window=%s\n", _fd, window ? "available" : "missing");
		_pumpLogged = true;
	}
	if (_fd < 0 || !window) return;
	int width = 0, height = 0;
	SDL_GetWindowSize(window, &width, &height);
	if (width <= 0 || height <= 0) return;
	input_event input;
	// Bound work per poll to keep the UI responsive while moving the stylus.
	for (int n = 0; n < 256; ++n) {
		ssize_t result = read(_fd, &input, sizeof(input));
		if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
		if (result != sizeof(input)) {
			if (result < 0 && errno == EINTR) continue;
			fprintf(stderr, "RGDS evdev read stopped: %s\n", result < 0 ? strerror(errno) : "short read");
			close(_fd); _fd = -1; break;
		}
		RgdsTouchFrame frame;
		if (_rawLogs++ < 160)
			fprintf(stderr, "RGDS input type=%u code=%u value=%d\n", input.type, input.code, input.value);
		if (!_state.consume(input, frame)) continue;
		_lastTouchTick = SDL_GetTicks();
		int x = _swap ? RgdsTouchState::scale(frame.y, _minY, _maxY, width)
			: RgdsTouchState::scale(frame.x, _minX, _maxX, width);
		int y = _swap ? RgdsTouchState::scale(frame.x, _minX, _maxX, height)
			: RgdsTouchState::scale(frame.y, _minY, _maxY, height);
		if (_invertX) x = width - 1 - x;
		if (_invertY) y = height - 1 - y;
		if (_calibrated && _maxX>_minX && _maxY>_minY) {
			double cx,cy;
			_calibration.map(double(frame.x-_minX)/(_maxX-_minX),double(frame.y-_minY)/(_maxY-_minY),cx,cy);
			x=int(std::lround(cx*(width-1)));y=int(std::lround(cy*(height-1)));
			x=x<0?0:x>=width?width-1:x;y=y<0?0:y>=height?height-1:y;
		}
		if (_logs++ < 40)
			fprintf(stderr, "RGDS touch raw=%d,%d mapped=%d,%d window=%dx%d down=%d edge=%d\n",
				frame.x, frame.y, x, y, width, height, frame.down, frame.changed);
		SDL_Event ev = {};
		if (frame.moved || frame.changed) {
			ev.type = SDL_MOUSEMOTION;
			ev.motion.windowID = SDL_GetWindowID(window);
			ev.motion.which = RGDS_DIRECT_MOUSE_ID;
			ev.motion.x = x; ev.motion.y = y;
			ev.motion.state = frame.down ? SDL_BUTTON_LMASK : 0;
			SDL_PushEvent(&ev);
		}
		if (frame.changed) {
			ev = {};
			ev.type = frame.down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
			ev.button.windowID = SDL_GetWindowID(window);
			ev.button.which = RGDS_DIRECT_MOUSE_ID;
			ev.button.button = SDL_BUTTON_LEFT;
			ev.button.state = frame.down ? SDL_PRESSED : SDL_RELEASED;
			ev.button.clicks = 1;
			ev.button.x = x; ev.button.y = y;
			SDL_PushEvent(&ev);
		}
	}
}
