#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RST_PIN 22
#define SS_PIN 21

MFRC522 mfrc522(SS_PIN, RST_PIN);

const char* ssid = "DR";
const char* password = "123123123";
const char* serverName = "http://192.168.43.222:3000/rfid";

struct UserInfo {
  String rfid;
  String name;
  String npm;
};

// Data pengguna
UserInfo users[] = {
  { "2365A234", "Hoppus", "140910200002" },
  { "23BFB334", "Delonge", "140910200004" },
  { "53424D34", "Mark", "140910200001" },
  { "E3863336", "Tom", "140910200003" }
};

// Fungsi untuk mendapatkan informasi pengguna berdasarkan RFID tag
UserInfo getUserInfo(String rfid) {
  for (UserInfo user : users) {
    if (user.rfid == rfid) {
      return user;
    }
  }
  return { "Unknown", "Unknown", "000000000" }; // Default jika RFID tidak ditemukan
}

void setup() {
  Serial.begin(115200);
  SPI.begin();          // Menginisialisasi SPI bus
  mfrc522.PCD_Init();   // Menginisialisasi MFRC522

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  String rfidTag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    rfidTag += String(mfrc522.uid.uidByte[i], HEX);
  }
  rfidTag.toUpperCase();
  Serial.println("RFID Tag: " + rfidTag);

  // Dapatkan informasi pengguna berdasarkan RFID tag
  UserInfo user = getUserInfo(rfidTag);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    String jsonPayload = "{\"rfid\":\"" + rfidTag + "\", \"name\":\"" + user.name + "\", \"npm\":\"" + user.npm + "\"}";

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(1000); // Adjust delay as needed
}