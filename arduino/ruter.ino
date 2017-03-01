#include <Arduino.h>

#include <SPI.h>
#include <U8g2lib.h>

#define CLK_PIN D5
#define DIN_PIN D7
#define CE_PIN D1
#define DC_PIN D6
#define RST_PIN D2
#define LIGHT_PIN D0

U8G2_PCD8544_84X48_1_4W_SW_SPI u8g2(U8G2_R0, CLK_PIN, DIN_PIN, CE_PIN, DC_PIN, RST_PIN);

u8g2_uint_t rowHeight = 12;

void setup(void)
{
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);	
  
  u8g2.begin();
}

void drawLineAtHeight(u8g2_uint_t height) {
    u8g2.drawLine(0, height, 84, height);
}

void drawTitle(const char *title) {
    u8g2.setFont(u8g2_font_6x13_tr);
    u8g2.drawStr(3,10,title);
}

void drawTextInRow(int row, const char *text) {
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(7,((row + 1) * rowHeight) - 2,text);
}

void drawPage(char *lines[], int start, int count) {
    drawTitle(lines[start]);
    for(int i = 1; i < count; i++) {
      drawTextInRow(i,lines[start + i]);
    }
}

void loop(void)
{
  char *lines[] = {
    "Mot Sentrum",
    "4 Bergkr-19:34",
    "5 Vestli-19:39",
    "5 Ringen-19:43",
    "Fra sentrum",
    "4 Vestli-19:39",
    "5 Sognsv-19:42",
    "5 Ringen-19:50"
  };

  u8g2.firstPage();
  
  do {
    u8g2.drawRFrame(0, rowHeight, 84, rowHeight * 3, 3);
    drawLineAtHeight(rowHeight * 2);
    drawLineAtHeight(rowHeight * 3);

    drawPage(lines, 4, 4);

  } while ( u8g2.nextPage() );
}
