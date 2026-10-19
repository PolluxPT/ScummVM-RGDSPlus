#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "backends/events/sdl/rgds-volume.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <time.h>
#include <cstdint>

static std::atomic<int> gain(100);
static bool initialized = false, enabled = false;
static int keyFds[32], keyCount = 0, level = 2;
static unsigned long lastRead = 0, lastRepeat = 0;
static const char *volumePath = "/sys/class/anbernic_misc/openbor_volume";
static unsigned long milliseconds() {
    timespec ts = {}; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL;
}
static bool readLevel(int &out) {
    FILE *f = fopen(volumePath, "r");
    if (!f) return false;
    int value = -1; bool ok = fscanf(f, "%d", &value) == 1;
    fclose(f);
    if (!ok || value < 0 || value > 10) return false;
    out = value; return true;
}
static bool hasKey(const unsigned long *bits, int key) {
    return bits[key / (8 * sizeof(unsigned long))] & (1UL << (key % (8 * sizeof(unsigned long))));
}
void rgdsVolumeInit() {
    if (initialized) return;
    initialized = true;
    // Restrict device-specific behavior to this launcher's explicit environment.
    const char *flag = getenv("SCUMMVM_RGDS_STOCK_VOLUME");
    enabled = flag && strcmp(flag, "1") == 0;
    if (!enabled) return;
    bool valid = readLevel(level);
    gain.store(level * 10);
    fprintf(stderr, "RGDS volume: Stock level=%d/10, source=%s\n", level, valid ? volumePath : "unavailable; fallback 20 percent");
    for (int i = 0; i < 32; ++i) {
        char path[64]; snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) continue;
        unsigned long bits[(KEY_MAX + 8 * sizeof(unsigned long)) / (8 * sizeof(unsigned long))] = {};
        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits) < 0 ||
            (!hasKey(bits, KEY_VOLUMEUP) && !hasKey(bits, KEY_VOLUMEDOWN))) { close(fd); continue; }
        keyFds[keyCount++] = fd;
        fprintf(stderr, "RGDS volume keys: %s\n", path);
    }
}
void rgdsVolumePoll() {
    if (!enabled) return;
    unsigned long now = milliseconds();
    if (now - lastRead >= 200) {
        int current;
        if (readLevel(current) && current != level) {
            level = current; gain.store(level * 10);
            fprintf(stderr, "RGDS volume: external level=%d/10\n", level);
        }
        lastRead = now;
    }
    for (int i = 0; i < keyCount; ++i) {
        input_event event;
        for (int n = 0; n < 64 && read(keyFds[i], &event, sizeof(event)) == sizeof(event); ++n) {
            if (event.type != EV_KEY || (event.code != KEY_VOLUMEUP && event.code != KEY_VOLUMEDOWN) ||
                (event.value != 1 && event.value != 2)) continue;
            if (event.value == 2 && now - lastRepeat < 180) continue;
            lastRepeat = now;
            level += event.code == KEY_VOLUMEUP ? 1 : -1;
            if (level < 0) level = 0;
            if (level > 10) level = 10;
            gain.store(level * 10);
            FILE *f = fopen(volumePath, "w");
            bool saved = false;
            if (f) { bool wrote = fprintf(f, "%d\n", level) > 0; saved = fclose(f) == 0 && wrote; }
            fprintf(stderr, "RGDS volume: button level=%d/10, firmware saved=%d\n", level, saved);
        }
    }
}
void rgdsApplyVolume(unsigned char *samples, int length) {
    if (!enabled) return;
    int percent = gain.load();
    // Same quadratic curve as Mario Kart Wii: 0.35 * (Stock level / 10)^2.
    int coefficient = percent * percent * 35;
    // SDL output is signed native-endian 16-bit PCM. Never access files here.
    for (int i = 0; i + 1 < length; i += 2) {
        int16_t value; memcpy(&value, samples + i, sizeof(value));
        value = int64_t(value) * coefficient / 1000000;
        memcpy(samples + i, &value, sizeof(value));
    }
}
