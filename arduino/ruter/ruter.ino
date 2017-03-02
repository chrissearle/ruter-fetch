#include <Arduino.h>

#include <SPI.h>
#include <U8g2lib.h>

#include <ESP8266WiFi.h>
#include <HttpClient.h>


#define CLK_PIN D5
#define DIN_PIN D7
#define CE_PIN D1
#define DC_PIN D6
#define RST_PIN D2
#define LIGHT_PIN D0

const char* ssid = "**********";
const char* password = "**********";

U8G2_PCD8544_84X48_1_4W_SW_SPI u8g2(U8G2_R0, CLK_PIN, DIN_PIN, CE_PIN, DC_PIN, RST_PIN);

u8g2_uint_t rowHeight = 12;

const char *kHostname = "**********";
const char *kPath = "**********";
const int kNetworkTimeout = 30*1000;
const int kNetworkDelay = 1000;

#define ROWS 16
#define MAX_LINE 16

char *lines[ROWS][MAX_LINE] = { '\0' };

void setup(void) {
  Serial.begin(115200);
  

  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected");
  
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);  
  
  u8g2.begin();
}

void updateLines(const char *data) {
    char* pch = NULL;
    
    int line = 0;

    char *c = strdup(data);

    pch = strtok((char *)c, "\r\n");

    while (pch != NULL) {
        if (strlen(pch) > 8 && line < ROWS) {
            strncpy( (char*)lines[line], pch, MAX_LINE - 1);
            lines[line][MAX_LINE - 1]='\0';
            
            line += 1;
        }

        pch = strtok(NULL, "\r\n");
    }
    
    free(c);
}

void fetchData() {
  int err = 0;

  String body = "";
  
  WiFiClient client;
  HttpClient http(client);

  err = http.get(kHostname, kPath);

  if (err >= 0) {
    err = http.responseStatusCode();
    
    if (err >= 200 && err < 300) {
      err = http.skipResponseHeaders();

      if (err >= 0) {
        int bodyLen = http.contentLength();

        unsigned long timeoutStart = millis();
        char c;

        while (
          (http.connected() || http.available()) &&
          ((millis() - timeoutStart) < kNetworkTimeout)
          ) {

          if (http.available()) {
            c = http.read();

            body = body + c;

            bodyLen--;

            timeoutStart = millis();
          } else  {
            delay(kNetworkDelay);
          }
        }

        updateLines(body.c_str());
      } else {
        Serial.print("Header skip failed");
        Serial.print(err);
      }
    } else {
      Serial.print("Fetch failed");
      Serial.print(err);
    }
  } else {
    Serial.print("Connect failed");
    Serial.print(err);
  }

  http.stop();
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

void drawPage(int start, int count) {
  drawTitle(lines[start][0]);
  for(int i = 1; i < count; i++) {
    drawTextInRow(i,lines[start + i][0]);
  }
}

void showPage(int offset) {
  u8g2.firstPage();
  
  do {
    u8g2.drawRFrame(0, rowHeight, 84, rowHeight * 3, 3);
    drawLineAtHeight(rowHeight * 2);
    drawLineAtHeight(rowHeight * 3);

    drawPage(offset, 4);

  } while ( u8g2.nextPage() );

}

void loop(void) {
  fetchData();
  showPage(0);
  delay(5000);
  showPage(4);
  delay(5000);
}
