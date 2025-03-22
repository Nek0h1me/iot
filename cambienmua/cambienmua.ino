#include <WiFi.h>

const char* ssid = "your_SSID";         // Thay thế bằng SSID của bạn
const char* password = "your_PASSWORD"; // Thay thế bằng mật khẩu của bạn

void setup() {
  Serial.begin(115200);
  
  // Kết nối tới Wi-Fi
  WiFi.begin(ssid, password);
  
  // Chờ kết nối
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  // In địa chỉ IP
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // In địa chỉ IP
}

void loop() {
  // Không có gì trong loop
}