// Wireless RFID Door Lock Using NodeMCU
// Created by LUIS SANTOS & RICARDO VEIGA
// 7th of June, 2017

//#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RST_PIN 20 // RST-PIN for RC522 - RFID - SPI - Module GPIO15
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Module GPIO2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
//SSD1306 display(0x3c, 10, 9); //4-15
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String wifissid = "UUMWiFi_Guest";
String wifipass = "";
double temp = 0;
String content = "";
String request;
const int MLX_addr = 0x5A;

MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(115200);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  mlx.begin();
  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Wire.begin();
  Wire.beginTransmission(MLX_addr);
  Wire.write(0x5B);
  Wire.write(0);
  Wire.endTransmission(true);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2);
  display.clearDisplay();
  drawScreen("WELCOME TO UUM", "RFID based temperature", "recording system", "Please wait...", "Booting...");
  delay(3000);
  drawScreen("WELCOME TO UUM", "SETTING UP", "Connecting to ", wifissid, "Booting...");
  if (testWifi()) {
    drawScreen("WELCOME TO UUM", "SETTING UP", "Connected to ", wifissid, "Booting...");
    delay(2000);
  } else {
    drawScreen("WELCOME TO UUM", "SETTING UP", "Connection Failed ", wifissid, "Restarting...");
    delay(5000);
    ESP.restart();
  }
  delay(2000);
  drawScreen("WELCOME TO UUM", "SYSTEM READY", "Connected to", wifissid, "Ready...");
  delay(3000);
}

boolean testWifi() {
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid.c_str(), wifipass.c_str());
  int c = 0;
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(WiFi.status());
      Serial.println(WiFi.localIP());
      delay(100);
      return true;
    }
    Serial.print(".");
    delay(400);
    c++;
  }
  Serial.println("Connection time out...");
  return false;
}

void drawScreen(String a, String b, String c, String d, String e)
{
  display.display();
  delay(10);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(15,5);
  display.print(a);
  display.setCursor(0,20);
  display.print(b);
  display.setCursor(0,30);
  display.print(c);
  display.setCursor(0,40);
  display.print(d);
  display.setCursor(0,50);
  display.print(e);
  display.display();
}

void readTemp() {
  Serial.print("Object = ");
  //Serial.print(mlx.readAmbientTempC());
  //Serial.print("*C\tObject = ");
  temp = mlx.readObjectTempC() + 0.98;
  while (temp < 34){
    temp = mlx.readObjectTempC() + 0.98;
    drawScreen("WELCOME TO SOC", "TAKING TEMPERATURE", "CLOSER TO SENSOR", String(temp) + " celcius", "Reading...");
    delay(100);
  }
  Serial.print(temp);
  Serial.println("*C");
  Serial.println();
  delay(1000);
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void loop() {
  // Look for new cards
  drawScreen("WELCOME TO UUM", "PRESENT YOUR CARD", "", "", "Waiting...");
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  ////-------------------------------------------------RFID----------------------------------------------


  // Shows the card ID on the serial console
  content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  content.toUpperCase();
  Serial.println("Card read:" + content);
  drawScreen("WELCOME TO UUM", "REMOVE YOUR CARD ", "CARD ID IS "+content, "Prepare to take your ", "Temperature");
  delay(5000);
  readTemp();
  drawScreen("WELCOME TO SOC", "", "YOUR TEMPERATURE", String(temp) + " celcius", "Completed...");
  delay(3000);
  drawScreen("WELCOME TO SOC", "", "UPLOADING DATA...", "", "Please wait...");
  uploadData();
  drawScreen("WELCOME TO SOC", "", "UPLOADING DATA...", "Completed", "Please wait...");
  delay(3000);
}

void readCard() {
  byte sector         = 1;
  byte blockAddr      = 4;
  byte dataBlock[]    = {
    0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
    0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
    0x09, 0x0a, 0xff, 0x0b, //  9, 10, 255, 11,
    0x0c, 0x0d, 0x0e, 0x0f  // 12, 13, 14, 15
  };
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Read data from the block
  Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();
  Serial.println();
  Serial.println(F("Authenticating again using key B..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
}

void uploadData() {
  request = "";
  request = "http://slumberjer.com/covid/insert_record.php?id=" + String(content);
  request += "&temp=" + String(temp);
  Serial.println(request);
  HTTPClient http;
  http.begin(request);
  int httpCode = http.GET();
  String result = "";
  Serial.println(httpCode);
  if (httpCode > 0) {
    result = http.getString();
    Serial.println(result);
    Serial.println("Upload completed");
  } else {
    Serial.println("Upload failed");
  }
  http.end();

}
