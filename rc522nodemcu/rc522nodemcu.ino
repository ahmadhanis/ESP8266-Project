// Wireless RFID Door Lock Using NodeMCU
// Created by LUIS SANTOS & RICARDO VEIGA
// 7th of June, 2017

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define RST_PIN 20 // RST-PIN for RC522 - RFID - SPI - Module GPIO15
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Module GPIO2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
SSD1306 display(0x3c, 0, 10); //4-15
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
String wifissid = "UUMWiFi_Guest";
String wifipass = "";
double temp = 0;
String content = "";
String request;
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
  //display.init();
  //display.flipScreenVertically();
  //drawScreen("WELCOME TO SOC", "CHECK", "Card-...", "", "Booting...");

  //drawScreen("WELCOME TO SOC", "CHECK", "Card-Success", "", "Booting...");
  delay(2000);
  //drawScreen("WELCOME TO SOC", "CHECK", "WiFi-...", "", "Booting...");
  if (testWifi()) {
    //drawScreen("WELCOME TO SOC", "CH", "WiFi-Success", wifissid, "Booting...");
  } else {
    //drawScreen("WELCOME TO SOC", "CHECK", "WiFi-Failed", "", "Booting...");
    delay(3000);
    ESP.restart();
  }
  //drawScreen("WELCOME TO SOC", "SYSTEM READY", "", wifissid, "Completed...");
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

//void drawScreen(String a, String b, String c, String d, String e)
//{
//  delay(10);
//  display.clear();
//  display.setTextAlignment(TEXT_ALIGN_LEFT);
//  display.setFont(ArialMT_Plain_10);
//  display.drawString(5, 0, a);
//  display.drawString(5, 10, b);
//  display.drawString(5, 20, c);
//  display.drawString(5, 30, d);
//  display.drawString(5, 40, e);
//  display.display();
//}

void readTemp() {
  Serial.print("Object = ");
  //Serial.print(mlx.readAmbientTempC());
  //Serial.print("*C\tObject = ");
  temp = mlx.readObjectTempC();
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
  //  drawScreen("WELCOME TO SOC", "", "TOUCH YOUR CARD", "", "Waiting...");
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
  //readCard();
  //drawScreen("WELCOME TO SOC", "", "CARD ID", content, "Reading...");
  delay(3000);
  //drawScreen("WELCOME TO SOC", "", "TAKING TEMPERATURE", content, "Reading...");
  delay(3000);
  readTemp();
  //drawScreen("WELCOME TO SOC", "", "YOUR TEMPERATURE", String(temp) + " celcius", "Completed...");
  delay(3000);
  //drawScreen("WELCOME TO SOC", "", "UPLOADING DATA...", "", "Please wait...");
  uploadData();
  //drawScreen("WELCOME TO SOC", "", "UPLOADING DATA...", "Completed", "Please wait...");
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
