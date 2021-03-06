#include "oled_display.h"
#include "Arduino.h"

OledDisplay::OledDisplay(SSD1306 *display) {
  _display = display;
}

void OledDisplay::init() {
  _display->init();
}

void OledDisplay::loop() {
  page(pageSequence[_mainPageSequenceIndex]);
}

void OledDisplay::nextPage() {
  _mainPageSequenceIndex++;
  if (_mainPageSequenceIndex == OLED_DISPLAY_PAGE_COUNT) {
    _mainPageSequenceIndex = 0;
  }
}

void OledDisplay::page(uint8_t page) {

  static uint32_t lastUpdate = 0;

  //Do not allow for OLED to be updated too often
  if (lastUpdate > 0 && millis() - lastUpdate < 200 && _forceDisplay == false) {
    return;
  }

  _forceDisplay = false;

  switch (page) {

    case STARTUP:
      renderStartupScreen();
      break;
    case OLED_PAGE_STATS:
      renderPageStats();
      break;
    case OLED_PAGE_SPEED:
      renderPageSpeed();
      break;
    case OLED_PAGE_ALTITUDE:
      renderPageAltitude();
      break;
    case OLED_PAGE_RELATIVE_ALTITUDE:
      renderPageRelativeAltitude();
      break;
  }
  _page = page;

  lastUpdate = millis();
}

void OledDisplay::renderStartupScreen() {
  _display->clear();
}

void OledDisplay::renderPageStats() {
  _display->clear();
  _display->setFont(ArialMT_Plain_10);

  _display->drawString(0, 0, "Sat:");
  _display->drawString(26, 0, String(gps.satellites.value()));

  _display->drawString(0, 12, "Alt:");
  _display->drawString(26, 12, String(gps.altitude.meters(), 1));
  if (gpsState.altMax > -999999) {
    _display->drawString(64, 12, String(gpsState.altMax, 1));
  } else {
    _display->drawString(64, 12, "-");
  }

  _display->drawString(0, 24, "Spd:");
  _display->drawString(26, 24, String(gps.speed.mps(), 1));
  _display->drawString(64, 24, String(gpsState.spdMax, 1));

  _display->drawString(0, 36, "Lon:");

  _display->drawString(26, 36, String(Long, 6));

  _display->drawString(0, 44, "Lat:");
  _display->drawString(26, 44, String(Lat, 6));

  _display->display();
}

void OledDisplay::renderHeader(String title) {
  _display->setFont(ArialMT_Plain_10);
  _display->drawString(0, 0, title);

  _display->drawString(90, 0, String(gps.satellites.value()) + " sats");
}

void OledDisplay::renderPageSpeed() {
  _display->clear();

  renderHeader("Speed");

  _display->setFont(ArialMT_Plain_24);

  _display->drawString(0, 20, String(gps.speed.kmph(), 1) + " km/h");

  _display->setFont(ArialMT_Plain_10);
  _display->drawString(0, 54, "Max speed " + String(gpsState.spdMax, 1) + " km/h");

  _display->display();
}

void OledDisplay::renderPageAltitude() {
  _display->clear();

  renderHeader("Abs. Altitude");

  _display->setFont(ArialMT_Plain_24);
  _display->drawString(0, 20, String(gps.altitude.meters(), 1) + " m");

  _display->setFont(ArialMT_Plain_10);

  if (gpsState.altMin < 999999) {
    _display->drawString(0, 54, "Min " + String(gpsState.altMin, 1) + " m");
  } else {
    _display->drawString(0, 54, "Min -");
  }

  if (gpsState.altMax > -999999) {
    _display->drawString(64, 54, "Max " + String(gpsState.altMax, 1) + " m");
  } else {
    _display->drawString(64, 54, "Max -");
  }

  _display->display();
}

void OledDisplay::renderPageRelativeAltitude() {
  _display->clear();

  renderHeader("Rel. Altitude");

  _display->setFont(ArialMT_Plain_24);
  _display->drawString(0, 20, String(gps.altitude.meters() - gpsState.originAlt, 1) + " m");

  _display->setFont(ArialMT_Plain_10);

  if (gpsState.altMin < 999999) {
    _display->drawString(0, 54, "Min " + String(gpsState.altMin - gpsState.originAlt, 1) + " m");
  } else {
    _display->drawString(0, 54, "Min -");
  }

  if (gpsState.altMax > -999999) {
    _display->drawString(64, 54, "Max " + String(gpsState.altMax - gpsState.originAlt, 1) + " m");
  } else {
    _display->drawString(64, 54, "Max -");
  }

  _display->display();
}
