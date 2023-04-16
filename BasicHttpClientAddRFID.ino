
/*
Set wifi SSID and Password line 28
Set WebAddr_Submit link line 22
*/

#include <M5StickCPlus.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MFRC522_I2C.h"


MFRC522 mfrc522(0x28);
WiFiMulti wifiMulti;
HTTPClient http;
const int buttonPin = 2;
const String nodeID = "M5CP0001";
const String WebAddr_Submit = "http://bjnorgnija.xyz/complete_chore.php";

void setup() {
  M5.begin();
  pinMode(M5_BUTTON_HOME, INPUT);
  M5.Lcd.setRotation(3);
  wifiMulti.addAP("SSID", "PASSWORD");
  Wire.begin();
  mfrc522.PCD_Init();
  ShowReaderDetails();
}

void loop() {
  M5.Lcd.print("\nConnecting Wifi...\n");
  if ((wifiMulti.run() == WL_CONNECTED)) {
    lcd_wash();
    M5.Lcd.printf("Wifi SSID: %s\n", WiFi.SSID());
    M5.Lcd.printf("RSSI: %d\n", WiFi.RSSI());
    //M5.Lcd.printf("IP Address: %s\n", WiFi.localIP());
    delay(100);

    //Running the HTTP Connection from here on a button press

    String uid = RFIDRead();
    HTTPPostConnect(uid);

  } else {
    lcd_wash();
    M5.Lcd.fillScreen(TFT_RED);
    //Delay for wifi connect
    M5.Lcd.print("Connection failed, attempting in 3");
    delay(1000);
    M5.Lcd.print(".");
    delay(1000);
    M5.Lcd.print(".");
    delay(1000);

    //RFIDSerialDump();
  }
}

void lcd_wash() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(1, 0);
}

void Btn_wait() {
  while (1) {
    if (digitalRead(M5_BUTTON_HOME) == LOW) {
      while (digitalRead(M5_BUTTON_HOME) == LOW)
        ;
      break;
    }
    delay(100);
  }
}

void RFIDSerialDump() {

  lcd_wash();
  M5.Lcd.print("Awaiting RFIDSerialDump");

  while (1) {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(2000);
    } else {
      break;
    }
  }

  String uid = "";

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    // Serial.print(mfrc522.uid.uidByte[i], HEX);
    // M5.Lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    // M5.Lcd.print(mfrc522.uid.uidByte[i], HEX);
    uid += mfrc522.uid.uidByte[i];
  }
  M5.Lcd.println("\n");
  M5.Lcd.println(uid);

  M5.Lcd.print("\nOutput done\n");
  Btn_wait();



  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); //Both of these timeout after getting the id. Not sure where to go from here.
  //mfrc522.PICC_DumpMifareUltralightToSerial();
  //  04
  //  2F
  //  48
  //  7C
  //  B6
  //  2A
  //  81
  // Page  0  1  2  3
  // MIFARE_Read() failed: Timeout in communication.
}


String RFIDRead() {
  M5.Lcd.fillScreen(TFT_YELLOW);
  M5.Lcd.println("Awaiting Tag");
  while (1) {

    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(1000);
    } else {
      break;
    }
  }

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    M5.Lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    M5.Lcd.print(mfrc522.uid.uidByte[i], HEX);
    uid += mfrc522.uid.uidByte[i];
  }

  M5.Lcd.print("\n");
  M5.Lcd.print("Data read, click for HTTP");
  delay(4000);

  return uid;
}

void HTTPConnect(String uid) {
  M5.Lcd.fillScreen(TFT_GREEN);
  M5.Lcd.setCursor(1, 0);
  if ((wifiMulti.run() == WL_CONNECTED)) {  //double check the wifi connection, reconnect if needed

    M5.Lcd.print("[HTTP] begin...\n");

    /*
          Should this generate a unique timestamp and sent it to the server? Or use a key and send that to the server for TS gen? tag uid?
        */
    //http.begin(WebAddr_Submit);

    http.begin("http:addr:port/submit.php?nodeid=");
    M5.Lcd.print("[HTTP] GET...\n");
    int httpCode = http.GET();  // start connection and send HTTP header.
    if (httpCode > 0) {
      M5.Lcd.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();  //payload is the actual html that the ws serves
        Serial.println(payload);
      }
    } else {
      M5.Lcd.fillScreen(TFT_RED);
      M5.Lcd.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    M5.Lcd.fillScreen(TFT_RED);
    M5.Lcd.print("connect failed");
  }
  delay(5000);
  lcd_wash();  // clear the screen.  清除屏幕
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(
      F("WARNING: Communication failure, is the MFRC522 properly "
        "connected?"));
  }
}

void HTTPPostConnect(String uid) {
  M5.Lcd.fillScreen(TFT_GREEN);
  M5.Lcd.setCursor(1, 0);
  if ((wifiMulti.run() == WL_CONNECTED)) {  //double check the wifi connection, reconnect if needed

    M5.Lcd.print("[HTTP] begin...\n");

    /*
          Should this generate a unique timestamp and sent it to the server? Or use a key and send that to the server for TS gen? tag uid?
        */
    //http.begin("http://example.com/index.html");
    http.begin(WebAddr_Submit);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    //data1 will be scannerid, data2 will be choreid
    String httpRequestData = "data1=" + nodeID + "&data2=" + uid;
    M5.Lcd.print("[HTTP] POST...\n");
    int httpCode = http.POST(httpRequestData);  // start connection and send HTTP header.
    if (httpCode > 0) {
      M5.Lcd.printf("[HTTP] POST... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        M5.Lcd.print("Success");
        delay(1000);
      }
    } else {
      M5.Lcd.fillScreen(TFT_RED);
      M5.Lcd.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      delay(4000);
    }
    http.end();
  } else {
    M5.Lcd.fillScreen(TFT_RED);
    M5.Lcd.print("Connection Failed");
  }
  delay(5000);
  lcd_wash();  // clear the screen.  清除屏幕
}
