#ifndef OLED_DISPLAY
#define OLED_DISPLAY

#include "types.h"
#include "SSD1306.h"
#include <TinyGPS++.h>

enum txOledPages {
    STARTUP,
    SETTINGS,
    OLED_PAGE_NONE,
    OLED_PAGE_STATS,
    OLED_PAGE_SPEED,
    OLED_PAGE_ALTITUDE,
    OLED_PAGE_RELATIVE_ALTITUDE
};

#define OLED_COL_COUNT 64
#define OLED_DISPLAY_PAGE_COUNT 6

extern TinyGPSPlus gps;
extern double Long;
extern double Lat;
extern GpsDataState_t gpsState;
extern uint8_t screen;

const uint8_t pageSequence[OLED_DISPLAY_PAGE_COUNT] = {
    STARTUP,
    SETTINGS,
    OLED_PAGE_STATS,
    OLED_PAGE_SPEED,
    OLED_PAGE_ALTITUDE,
    OLED_PAGE_RELATIVE_ALTITUDE
};

class OledDisplay {
    public:
        OledDisplay(SSD1306 *display);
        void init();
        void loop();
        void page(uint8_t screen);
        void nextPage();
    private:
        SSD1306 *_display;
        void renderStartupScreen();
        void renderPageStats();
        void renderPageSpeed();
        void renderPageAltitude();
        void renderPageRelativeAltitude();
        void renderHeader(String title);
        uint8_t _page = OLED_PAGE_NONE;
        uint8_t _mainPageSequenceIndex = 0;
        bool _forceDisplay = false;
};


#endif
