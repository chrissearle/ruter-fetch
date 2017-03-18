#include <Arduino.h>

#include <SPI.h>
#include <U8g2lib.h>

#include <ESP8266WiFi.h>
#include <HttpClient.h>

#include <ArduinoJson.h>

#define CLK_PIN D5
#define DIN_PIN D7
#define CE_PIN D1
#define DC_PIN D6
#define RST_PIN D2
#define LIGHT_PIN D0
#define SENSOR_PIN D4

const char* ssid = "SSID";
const char* password = "WIFIPASSWORD";

const char *kHostname = "HOST";
const char *kPath = "/RUTER_PATH.json";

U8G2_PCD8544_84X48_1_4W_SW_SPI u8g2(U8G2_R0, CLK_PIN, DIN_PIN, CE_PIN, DC_PIN, RST_PIN);

u8g2_uint_t rowHeight = 12;

const int kNetworkTimeout = 30*1000;
const int kNetworkDelay = 1000;

const unsigned long pollIntervalFetchMillis = 60L * 1000;
const unsigned long pollIntervalScreenMillis = 5L * 1000;

unsigned long lastUpdateFetchMillis = -pollIntervalFetchMillis;
unsigned long lastUpdateScreenMillis = -pollIntervalScreenMillis;

bool showUp = true;
int screenToShow = 0;
const int maxScreen = 2;

#define TITLELENGTH 15

struct Slot {
  char line[2];
  char destination[16];
  char departure[6];
};

struct Direction {
  char title[TITLELENGTH];
  struct Slot slots[6];
};


Direction up;
Direction down;

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
  pinMode(SENSOR_PIN, INPUT);

  memset(&up, '\0', sizeof(Direction));
  memset(&down, '\0', sizeof(Direction));

  strcpy(up.title, "Fetching");
  strcpy(down.title, "Fetching");
  
  u8g2.begin();
}

void extractDirection(struct Direction *direction, JsonObject *data) {
  JsonObject& root = *data;
  strcpy(direction->title, root["title"]);

  for (int i = 0; i < 6; i++) {
    // memset and then strncpy? 
    strcpy(direction->slots[i].line, root["times"][i]["line"]);
    strcpy(direction->slots[i].destination, root["times"][i]["dest"]);
    strcpy(direction->slots[i].departure, root["times"][i]["time"]);
  }
}

bool readData(struct Direction *up, struct Direction *down, const char *json) {
  char *jsonCopy = strdup(json);
  
  DynamicJsonBuffer jsonBuffer;
  
  JsonObject& root = jsonBuffer.parseObject(jsonCopy);

  if (!root.success())
  {
    free(jsonCopy);

    return false;
  } else {
    JsonObject& upData = root["up"];
    JsonObject& downData = root["down"];
    
    extractDirection(up, &upData);
    extractDirection(down, &downData);
  }

  free(jsonCopy);
  
  return true;
}

void dumpDirection(struct Direction *direction) {
    Serial.println(direction->title);

    for (int i = 0; i < 6; i++) {
      Serial.print(direction->slots[i].line);
      Serial.print(" ");
      Serial.print(direction->slots[i].destination);
      Serial.print(" ");
      Serial.print(direction->slots[i].departure);
      Serial.println();
    } 
}

void updateData(const char *json) {
   Direction newUp;
   Direction newDown;

   if (readData(&newUp, &newDown, json)) {
    dumpDirection(&newUp);
    dumpDirection(&newDown);

    memcpy(&up, &newUp, sizeof(Direction));
    memcpy(&down, &newDown, sizeof(Direction));
  } else {
    Serial.println("Failed to parse");
  }
}

void fetchData() {
  Serial.println("Fetching");
  
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

        Serial.println("Body fetched");
        
        updateData(body.c_str());
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

void drawTextInRow(int row, Slot *slot) {  
  u8g2.setFont(u8g2_font_5x8_tr);
  int position = ((row + 2) * rowHeight) - 3;

  char dest[10] = { '\0' };

  strncpy(dest, slot->destination, 9);
  
  u8g2.drawStr(3,position,slot->line);
  u8g2.drawStr(10,position,dest);
  u8g2.drawStr(57,position,slot->departure);
}

void drawPage(Direction *direction, int page, int rowCount) {
  if (page == 0) {
    drawTitle(direction->title);
  } else {
    char title[TITLELENGTH];
    
    strcpy(title, direction->title);
    strcat(title, " 2");

    drawTitle(title);
  }

  for(int i = 0; i < rowCount; i++) {
    drawTextInRow(i, &direction->slots[(page * rowCount) + i]);
  }
}

void showPage(Direction *direction, int page, int rowCount) {
  u8g2.firstPage();
  
  do {
    u8g2.drawRFrame(0, rowHeight, 84, rowHeight * 3, 3);
    drawLineAtHeight(rowHeight * 2);
    drawLineAtHeight(rowHeight * 3);

    drawPage(direction, page, rowCount);

  } while ( u8g2.nextPage() );
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdateFetchMillis >= pollIntervalFetchMillis) {
    lastUpdateFetchMillis = currentMillis;

    fetchData();
  }
  
  if (currentMillis - lastUpdateScreenMillis >= pollIntervalScreenMillis) {
    lastUpdateScreenMillis = currentMillis;

    if (showUp) {
      showPage(&up, screenToShow, 3);
    } else {
      showPage(&down, screenToShow, 3);
    }

    screenToShow = screenToShow + 1;
    if (screenToShow >= maxScreen) {
      screenToShow = 0;
      showUp = !showUp;
    }
  }

  int val = digitalRead(SENSOR_PIN);
  if (val == HIGH) {
    digitalWrite(LIGHT_PIN, LOW);
  } else {
    digitalWrite(LIGHT_PIN, HIGH);
  }
}

