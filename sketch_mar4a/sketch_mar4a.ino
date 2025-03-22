#include "esp_camera.h"
#include <WiFi.h>

// Thông tin WiFi
const char* ssid = "WIFI SINH VIEN";
const char* password = "";

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

WiFiServer server(80);
bool recording = false;

const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Control</title>
  <style>
    .alert { color: red; font-weight: bold; display: none; }
    img { max-width: 100%; }
  </style>
</head>
<body>
  <h1>ESP32-CAM Stream</h1>
  <img src="/stream" id="video">
  <div><img id="photo" src="" style="display:none;"></div>
  <div>
    <button onclick="capturePhoto()">Chụp ảnh</button>
    <button onclick="startRecording()">Bắt đầu quay</button>
    <button onclick="stopRecording()">Dừng quay</button>
  </div>
  <div id="alert" class="alert">Người được phát hiện!</div>
  <script>
    function capturePhoto() {
      fetch('/capture').then(response => response.blob()).then(blob => {
        document.getElementById('photo').src = URL.createObjectURL(blob);
        document.getElementById('photo').style.display = 'block';
      });
    }
    function startRecording() { fetch('/start_recording'); }
    function stopRecording() { fetch('/stop_recording'); }
    const img = document.getElementById('video');
    img.onload = function() {
      fetch('/check_person').then(response => response.text()).then(data => {
        document.getElementById('alert').style.display = data === 'detected' ? 'block' : 'none';
      });
    };
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("GET / ") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println();
      client.print(htmlContent);
    }
    else if (request.indexOf("GET /stream") >= 0) {
      streamVideo(client);
    }
    else if (request.indexOf("GET /capture") >= 0) {
      capturePhoto(client);
    }
    else if (request.indexOf("GET /start_recording") >= 0) {
      recording = true;
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.print("Recording started");
    }
    else if (request.indexOf("GET /stop_recording") >= 0) {
      recording = false;
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.print("Recording stopped");
    }
    else if (request.indexOf("GET /check_person") >= 0) {
      checkPerson(client);
    }
    client.stop();
  }
}

void streamVideo(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();
  
  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) continue;

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();
    client.write(fb->buf, fb->len);
    client.println();
    
    esp_camera_fb_return(fb);
    delay(100);
  }
}

void capturePhoto(WiFiClient client) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();
    client.write(fb->buf, fb->len); // Gửi dữ liệu ảnh JPEG
    Serial.println("Photo captured and sent");
    esp_camera_fb_return(fb);
  } else {
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-type:text/plain");
    client.println();
    client.print("Failed to capture photo");
  }
}

void checkPerson(WiFiClient client) {
  bool personDetected = random(0, 2); // Giả lập
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.print(personDetected ? "detected" : "none");
}