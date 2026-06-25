#include <M5Stack.h>

namespace {
constexpr uint32_t kStatusIntervalMs = 1000;

uint32_t lastStatusMs = 0;
uint32_t counter = 0;
uint16_t backgroundColor = BLACK;

void drawHeader() {
  M5.Lcd.fillScreen(backgroundColor);
  M5.Lcd.setTextColor(WHITE, backgroundColor);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("M5Stack Basic Check");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 42);
  M5.Lcd.println("A: count + beep");
  M5.Lcd.println("B: change screen color");
  M5.Lcd.println("C: reset counter");

  M5.Lcd.drawFastHLine(0, 84, 320, DARKGREY);
}

void drawStatus() {
  const uint32_t uptimeSec = millis() / 1000;

  M5.Lcd.fillRect(0, 95, 320, 105, backgroundColor);
  M5.Lcd.setTextColor(WHITE, backgroundColor);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 100);
  M5.Lcd.printf("Counter: %lu\n", static_cast<unsigned long>(counter));
  M5.Lcd.printf("Uptime : %lu sec\n", static_cast<unsigned long>(uptimeSec));
  M5.Lcd.printf("Millis : %lu\n", static_cast<unsigned long>(millis()));

  Serial.printf("counter=%lu uptime=%lu sec millis=%lu\n",
                static_cast<unsigned long>(counter),
                static_cast<unsigned long>(uptimeSec),
                static_cast<unsigned long>(millis()));
}

void drawFooter() {
  M5.Lcd.fillRect(0, 210, 320, 30, DARKGREY);
  M5.Lcd.setTextColor(WHITE, DARKGREY);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(34, 218);
  M5.Lcd.print("A");
  M5.Lcd.setCursor(154, 218);
  M5.Lcd.print("B");
  M5.Lcd.setCursor(274, 218);
  M5.Lcd.print("C");
}

void cycleBackgroundColor() {
  if (backgroundColor == BLACK) {
    backgroundColor = NAVY;
  } else if (backgroundColor == NAVY) {
    backgroundColor = MAROON;
  } else if (backgroundColor == MAROON) {
    backgroundColor = DARKGREEN;
  } else {
    backgroundColor = BLACK;
  }

  drawHeader();
  drawStatus();
  drawFooter();
}
}  // namespace

void setup() {
  M5.begin();
  Serial.begin(115200);

  drawHeader();
  drawStatus();
  drawFooter();

  M5.Speaker.tone(880, 120);
  Serial.println("M5Stack basic check started");
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    counter++;
    M5.Speaker.tone(880, 80);
    drawStatus();
  }

  if (M5.BtnB.wasPressed()) {
    M5.Speaker.tone(660, 80);
    cycleBackgroundColor();
  }

  if (M5.BtnC.wasPressed()) {
    counter = 0;
    M5.Speaker.tone(440, 80);
    drawStatus();
  }

  const uint32_t now = millis();
  if (now - lastStatusMs >= kStatusIntervalMs) {
    lastStatusMs = now;
    drawStatus();
  }
}
