#define BLYNK_TEMPLATE_ID "TMPL3vl2D2ZdV"
#define BLYNK_TEMPLATE_NAME "Gas Detection System"
#define BLYNK_AUTH_TOKEN "wnQV8sbg0s8y2i2gB93_7C77zbOHyYtG"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Nothing";       // your WiFi name
char pass[] = "hallo9860";     // your WiFi password

#define GAS_SENSOR_PIN 34   // MQ2 analog output
#define LED_PIN 13          // LED pin

BlynkTimer timer;
bool ledState = false;      // for blinking LED in warning mode

void checkGasLevel() {
  int gasLevel = analogRead(GAS_SENSOR_PIN);
  int gasPercent = map(gasLevel, 0, 4095, 0, 100);  // ESP32 uses 12-bit ADC

  Serial.print("Gas Level (Raw): ");
  Serial.print(gasLevel);
  Serial.print(" | Gas Percent: ");
  Serial.print(gasPercent);
  Serial.println("%");

  // Send data to Blynk
  Blynk.virtualWrite(V1, gasLevel);
  Blynk.virtualWrite(V2, gasPercent);

  if (gasLevel > 800) {
    Serial.println("Extreme Danger: LED ON");
    Blynk.logEvent("gas_alert", "🚨 Gas level critically high!");
    digitalWrite(LED_PIN, HIGH);
  } 
  else if (gasLevel > 600) {
    Serial.println("Danger: LED ON");
    Blynk.logEvent("gas_alert", "⚠️ Gas level high!");
    digitalWrite(LED_PIN, HIGH);
  } 
  else if (gasLevel > 100) {
    Serial.println("Warning: LED BLINKING");
    Blynk.logEvent("gas_alert", "⚠️ Gas level rising!");
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  } 
  else {
    Serial.println("Safe: LED OFF");
    digitalWrite(LED_PIN, LOW);
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🔧 Gas Detection System starting...");

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // replaces config + connect

  if (Blynk.connected()) {
    Serial.println("✅ Blynk connected!");
  } else {
    Serial.println("❌ Blynk not connected!");
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // initially off

  timer.setInterval(3000L, checkGasLevel); // check every 3 sec
}

void loop() {
  Blynk.run();
  timer.run();
}
