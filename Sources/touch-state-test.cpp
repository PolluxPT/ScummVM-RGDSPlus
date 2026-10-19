#include "backends/events/sdl/rgds-touch-state.h"
#include <cassert>
#include <cstdio>

static bool send(RgdsTouchState &s, RgdsTouchFrame &f, int type, int code, int value = 0) {
    input_event e = {}; e.type = type; e.code = code; e.value = value;
    return s.consume(e, f);
}
int main() {
    RgdsTouchState s; RgdsTouchFrame f;
    send(s,f,EV_ABS,ABS_MT_TRACKING_ID,42);
    send(s,f,EV_ABS,ABS_MT_POSITION_X,100);
    send(s,f,EV_ABS,ABS_MT_POSITION_Y,200);
    assert(send(s,f,EV_SYN,SYN_REPORT) && f.down && f.changed && f.x==100 && f.y==200);
    send(s,f,EV_ABS,ABS_MT_POSITION_X,300);
    assert(send(s,f,EV_SYN,SYN_REPORT) && f.down && !f.changed && f.moved && f.x==300);
    send(s,f,EV_ABS,ABS_MT_SLOT,1);
    send(s,f,EV_ABS,ABS_MT_TRACKING_ID,43);
    send(s,f,EV_ABS,ABS_MT_POSITION_X,900);
    assert(!send(s,f,EV_SYN,SYN_REPORT));
    send(s,f,EV_ABS,ABS_MT_SLOT,0);
    send(s,f,EV_ABS,ABS_MT_TRACKING_ID,-1);
    send(s,f,EV_KEY,BTN_TOUCH,1); // another finger cannot keep primary pressed
    assert(send(s,f,EV_SYN,SYN_REPORT) && !f.down && f.changed && f.x==300);
    send(s,f,EV_ABS,ABS_MT_TRACKING_ID,44);
    assert(send(s,f,EV_SYN,SYN_REPORT) && f.down && f.changed);
    send(s,f,EV_SYN,SYN_DROPPED);
    send(s,f,EV_ABS,ABS_MT_POSITION_X,900);
    assert(send(s,f,EV_SYN,SYN_REPORT) && !f.down && f.changed && f.x==300);
    RgdsTouchState legacy(false);
    send(legacy,f,EV_KEY,BTN_TOUCH,1);
    send(legacy,f,EV_ABS,ABS_X,20);
    send(legacy,f,EV_ABS,ABS_Y,30);
    assert(send(legacy,f,EV_SYN,SYN_REPORT) && f.down && f.x==20 && f.y==30);
    send(legacy,f,EV_KEY,BTN_TOUCH,0);
    assert(send(legacy,f,EV_SYN,SYN_REPORT) && !f.down && f.changed);
    // Some vendor drivers advertise tracking IDs but emit only BTN_TOUCH.
    RgdsTouchState advertisedTracking(true);
    send(advertisedTracking,f,EV_KEY,BTN_TOUCH,1);
    send(advertisedTracking,f,EV_ABS,ABS_MT_POSITION_X,512);
    send(advertisedTracking,f,EV_ABS,ABS_MT_POSITION_Y,384);
    assert(send(advertisedTracking,f,EV_SYN,SYN_REPORT) && f.down && f.changed && f.x==512);
    send(advertisedTracking,f,EV_KEY,BTN_TOUCH,0);
    assert(send(advertisedTracking,f,EV_SYN,SYN_REPORT) && !f.down && f.changed);
    assert(RgdsTouchState::scale(0,0,1023,640)==0);
    assert(RgdsTouchState::scale(1023,0,1023,640)==639);
    assert(RgdsTouchState::scale(-100,0,1023,640)==0);
    assert(RgdsTouchState::scale(3000,0,1023,640)==639);
    assert(RgdsTouchState::scale(20,10,30,101)==50);
    assert(RgdsTouchState::scale(0,0,0,640)==0);
    puts("ARM touch tests passed: press, motion, release, second finger, dropped sync, legacy events and coordinate scaling.");
}
