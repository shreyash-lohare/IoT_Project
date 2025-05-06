#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

#define PIR_PIN 13
bool motionDetected = false;
unsigned long lastMotionTime = 0;
unsigned long motionCooldown = 10000; // 10s cooldown

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char* ssid = "Nothing";
const char* password = "hallo9860";

WebServer server(80);
camera_fb_t * lastPhoto = NULL;

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Smart Home - Motion Detection</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background-color: #f0f0f0;
          margin: 0;
          padding: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
        }
        .container {
          background-color: #fff;
          border-radius: 10px;
          box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
          padding: 20px;
          text-align: center;
          width: 80%;
          max-width: 600px;
        }
        h1 {
          color: #333;
          font-size: 2rem;
        }
        .status {
          font-size: 1.2rem;
          color: #666;
        }
        .status.active {
          color: green;
        }
        .status.inactive {
          color: red;
        }
        .img-container {
          margin-top: 20px;
        }
        .img-container img {
          width: 100%;
          height: auto;
          border-radius: 10px;
          box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Smart Home - Motion Detection</h1>
        <p class="status" id="status">Waiting for motion...</p>
        <div class="img-container">
          <img id="motionImage" src="" alt="Motion Image">
        </div>
      </div>

      <script>
        function updateImage() {
          const imageElement = document.getElementById("motionImage");
          const statusElement = document.getElementById("status");
          
          // Fetch the latest image from the server
          fetch('/photo')
            .then(response => response.blob())
            .then(blob => {
              const url = URL.createObjectURL(blob);
              imageElement.src = url;
              statusElement.textContent = "Motion detected!";
              statusElement.classList.remove("inactive");
              statusElement.classList.add("active");
            })
            .catch(error => {
              statusElement.textContent = "No motion detected. Waiting...";
              statusElement.classList.remove("active");
              statusElement.classList.add("inactive");
            });
        }

        // Update the image initially
        updateImage();

        // Poll for updates every 1 seconds
        setInterval(updateImage, 1000);

      </script>
    </body>
    </html>
  )rawliteral");
}


void handlePhoto() {
  if (lastPhoto) {
    server.send_P(200, "image/jpeg", (char*)lastPhoto->buf, lastPhoto->len);
  } else {
    server.send(200, "text/plain", "No photo taken yet.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  Serial.println("Booting...");

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

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Access camera at: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/photo", handlePhoto);
  server.begin();
}

void loop() {
  server.handleClient();

  if (digitalRead(PIR_PIN) == HIGH && millis() - lastMotionTime > motionCooldown) {
    Serial.println("📸 Motion detected, capturing...");

    // Free previous photo if it exists
    if (lastPhoto != NULL) {
      esp_camera_fb_return(lastPhoto);
      lastPhoto = NULL;
    }

    lastPhoto = esp_camera_fb_get();
    if (!lastPhoto) {
      Serial.println("❌ Failed to capture image.");
      return;
    }

    Serial.printf("✅ Photo captured (%zu bytes)\n", lastPhoto->len);
    lastMotionTime = millis();
  }

  delay(100);
}