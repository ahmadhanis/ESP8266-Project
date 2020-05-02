#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define WIFI_SSID "your wifi ssid"
#define WIFI_PASSWORD "your wifi password"
#include <ArduinoJson.h>

SSD1306  display(0x3c, 4, 5);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  display.init();
  display.clear();

  drawScreenInfo("WiFi", "Connecting", "...", "","","","","");

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //bGotIpFlag == true;

  drawScreenInfo("WiFi", "Connected", WiFi.localIP().toString(), "","","","","");

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  checkCorona("Malaysia");
  delay(10000);
  checkCorona("World");
  delay(10000);
}

void checkCorona(String country) {
  Serial.println("Check  Corona for " + country);
  String payload;
  HTTPClient http;  //Object of class HTTPClient
  if (WiFi.status() == WL_CONNECTED) {
    String urlcheck = "http://coronavirus-19-api.herokuapp.com/countries/" + country;
    Serial.println(urlcheck);
    http.begin(urlcheck);
    int httpCode = http.GET();
    Serial.println(httpCode);
    payload = http.getString();
      Serial.println(payload);
    //Check the returning code
    if (httpCode > 0) {
      // Get the request response payload
      
      int httpCode = http.GET();
      if (httpCode > 0) {
        const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
        DynamicJsonBuffer jsonBuffer(bufferSize);
        JsonObject& root = jsonBuffer.parseObject(http.getString());
        const char* cases = root["cases"];
        const char* todaycases = root["todayCases"];
        const char* deaths = root["deaths"];
        const char* todayDeaths = root["todayDeaths"];
        const char* recovered = root["recovered"];
        const char* active = root["active"];
        const char* critical = root["critical"];
        Serial.println(cases);
        Serial.println(todaycases);
        Serial.println(deaths);
        Serial.println(todayDeaths);
        Serial.println(recovered);
        Serial.println(active);
        Serial.println(critical);
        drawScreenInfo(country,cases,todaycases,deaths,todayDeaths,recovered,active,critical);
      }
    }
    http.end();
  }

}

void drawScreenInfo(String a, String b, String c, String d,String e,String f,String g,String h)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, a);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 10, "T. Case: " + b +"/new: "+c);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, "T.D: " + d+"/new: "+e);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 30, "Rec: " + f + "/Act: "+g);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 40, "Critical :" + h);
  display.display();
}
