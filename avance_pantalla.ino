#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_DC 12
#define TFT_CS 13
#define TFT_MOSI 14
#define TFT_CLK 15
#define TFT_RST 0
#define TFT_MISO -1
#define BUTTON_PIN 2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.fillRoundRect(30, 50, 60, 30, 5, ST7735_YELLOW);
  tft.setCursor(40, 60);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(1);
  tft.println("BOTON");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(40, 60);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.println("1");
    Serial.println("1");
    delay(500);
  }
}
