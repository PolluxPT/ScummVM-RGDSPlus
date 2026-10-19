// RG DS Plus evdev input reducer. No device grab or calibration writes.
#ifndef RGDS_TOUCH_STATE_H
#define RGDS_TOUCH_STATE_H
#include <linux/input.h>
#include <stdint.h>

struct RgdsTouchFrame {
	int x, y;
	bool down, changed, moved;
};

class RgdsTouchState {
public:
	RgdsTouchState(bool tracking = true) : _tracking(tracking), _slot(0), _x(0), _y(0),
		_down(false), _previousDown(false), _dirty(false), _dropped(false), _seenTracking(false) {}
	bool consume(const input_event &ev, RgdsTouchFrame &frame) {
		if (ev.type == EV_SYN && ev.code == SYN_DROPPED) {
			_dropped = true;
			return false;
		}
		if (_dropped) {
			if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
				_dropped = false;
				_down = false;
				_dirty = false;
				frame = {_x, _y, false, _previousDown, false};
				_previousDown = false;
				return frame.changed;
			}
			return false;
		}
		if (ev.type == EV_ABS) {
			if (ev.code == ABS_MT_SLOT) _slot = ev.value;
			else if (_slot == 0) {
				if (_tracking && ev.code == ABS_MT_TRACKING_ID) {
					_seenTracking = true; _down = ev.value >= 0;
				}
				else if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X) {
					_x = ev.value; _dirty = true;
				} else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y) {
					_y = ev.value; _dirty = true;
				}
			}
		} else if ((!_tracking || !_seenTracking) && ev.type == EV_KEY && ev.code == BTN_TOUCH) {
			_down = ev.value != 0;
		} else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
			frame = {_x, _y, _down, _down != _previousDown, _dirty && _down};
			_previousDown = _down;
			_dirty = false;
			return frame.changed || frame.moved;
		}
		return false;
	}
	static int scale(int value, int minimum, int maximum, int size) {
		if (maximum <= minimum || size <= 1) return 0;
		if (value < minimum) value = minimum;
		if (value > maximum) value = maximum;
		return int((int64_t(value) - minimum) * (size - 1) / (int64_t(maximum) - minimum));
	}
private:
	bool _tracking;
	int _slot, _x, _y;
	bool _down, _previousDown, _dirty, _dropped, _seenTracking;
};
#endif
