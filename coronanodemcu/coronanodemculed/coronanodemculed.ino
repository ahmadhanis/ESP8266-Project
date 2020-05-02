#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#define WIFI_SSID "your wifi ssid"
#define WIFI_PASSWORD "your wifi password"
#include <ArduinoJson.h>
int pinCS = D3; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
int wait   = 100; // In milliseconds between scroll movements
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
String SITE_WIDTH =  "1000";
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  matrix.setIntensity(2);    // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);  // The first display is position upside down
  matrix.setRotation(1, 1);  // The first display is position upside down
  matrix.setRotation(2, 1);  // The first display is position upside down
  matrix.setRotation(3, 1);  
  
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //bGotIpFlag == true;


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
        String msj = "NEGARA:"+country+"-JUMLAH KES:"+cases+"-KES HARI INI:"+todaycases+"-JUMLAH KEMATIAN:"+deaths+"-KEMATIAN HARI INI:"+todayDeaths+"-PULIH:"+recovered+"-KES AKTIF:"+active+"-KRITIKAL:"+critical;
        drawScreenInfo(msj);
      }
    }
    http.end();
  }

}

void drawScreenInfo(String message)
{
    for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background OFF, reverse these to invert the display!
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait / 2);
  }
}
