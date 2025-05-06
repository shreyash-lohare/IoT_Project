#define BLYNK_TEMPLATE_ID "TMPL3sPoYK4kL"
#define BLYNK_TEMPLATE_NAME "Door Security"
#define BLYNK_AUTH_TOKEN "WQ10MOAWIpFoyGxs13zpNuDB17hnqxog"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
char ssid[] = "Arpit";
char pass[] = "qwertyuiop";

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <ESP32Servo.h>

// LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(21, 22, 18, 19, 23, 5);

// Servo motor
Servo myservo;
const int servoPin = 4;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {12, 14, 27, 26};
byte colPins[COLS] = {25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Password system
String password = "8008";
String input = "";
const int passwordLength = 4;
int failedAttempts = 0;

// Buzzer pin
const int buzzerPin = 13;  // Buzzer connected to pin 13

BlynkTimer timer;
int servoStatus = 1; // 1 = locked, 0 = unlocked

unsigned long unlockTime = 0;
bool waitingForAutoLock = false;


void playTone(int frequency, int duration) {
  int period = 1000000L / frequency;
  int pulse = period / 2;
  long cycles = (long)duration * 1000L / period;

  for (long i = 0; i < cycles; i++) {
    digitalWrite(buzzerPin, LOW);   // LOW = ON
    delayMicroseconds(pulse);
    digitalWrite(buzzerPin, HIGH);  // HIGH = OFF
    delayMicroseconds(pulse);
  }
}

// Optional delay between notes
void beepDelay(int ms) {
  digitalWrite(buzzerPin, HIGH);  // Make sure buzzer is off
  delay(ms);
}

BLYNK_WRITE(V0) {
  int value = param.asInt();
  if (value == 1) {
    // Lock door immediately
    myservo.write(90); // Locked position
    servoStatus = 1;
    waitingForAutoLock = false; // Cancel any pending auto-lock
    lcd.clear();
    lcd.print("Locked via Blynk");
    delay(2000);
    lcd.clear();
    lcd.print("Enter Password:");
  } else {
    // Unlock door
    myservo.write(0); // Unlocked position
    servoStatus = 0;
    lcd.clear();
    lcd.print("Unlocked via Blynk");
    unlockTime = millis();
    waitingForAutoLock = true; // Start auto-lock countdown
  }
}

void setup() {
  Serial.begin(115200);  // Serial Monitor for debugging
  delay(2000);
  lcd.begin(16, 2);
  lcd.print("Enter Password:");

  pinMode(buzzerPin, OUTPUT);  // Set buzzer pin as output

  // Make sure the buzzer is OFF initially (set to HIGH for low-level triggered)
  digitalWrite(buzzerPin, HIGH);
  myservo.setPeriodHertz(50); // SG90 uses 50Hz PWM
  myservo.attach(servoPin, 500, 2400); // Min/max pulse width in microseconds
  myservo.write(90);

  Serial.println("System initialized. Awaiting password input...");

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, [](){
    Blynk.virtualWrite(V0, servoStatus); // Keep Blynk in sync with hardware
  });
}

void buzz(int freq, int duration) {
  long period = 1000000L / freq;
  long pulse = period / 2;
  long cycles = (long)duration * 1000L / period;

  for (long i = 0; i < cycles; i++) {
    digitalWrite(buzzerPin, LOW);  // LOW = ON
    delayMicroseconds(pulse);
    digitalWrite(buzzerPin, HIGH); // HIGH = OFF
    delayMicroseconds(pulse);
  }
}

// 🚫 1. Wrong Password Alert (Short Beep Beep)
void wrongPasswordBeep() {
  for (int i = 0; i < 2; i++) {
    buzz(1000, 150);
    delay(100);
  }
}

// 🔴 2. 3 Times Wrong Password Alert (Loud Siren Style)
void alarmThreeWrongAttempts() {
  for (int i = 0; i < 5; i++) {
    buzz(1000, 150);
    buzz(1200, 150);
    buzz(900, 150);
    delay(150);
  }

  // Final long alert
  buzz(700, 800);
  digitalWrite(buzzerPin, HIGH); // Ensure buzzer is OFF
}

void loop() {

  Blynk.run();
  timer.run();

    // Handle auto-lock after unlock- from Blynk
  if (waitingForAutoLock && (millis() - unlockTime >= 7000)) {
    myservo.write(90); // Lock the door
    servoStatus = 1;
    lcd.clear();
    lcd.print("Enter Password:");
    waitingForAutoLock = false;
    Blynk.virtualWrite(V0, 1); // Sync Blynk switch to locked
  }

  char key = keypad.getKey();

  if (key) {
    input += key;

    // Serial Monitor debug output
    Serial.print("Key Pressed: ");
    Serial.println(key);
    Serial.print("Current Input: ");
    Serial.println(input);

    // Masked display on LCD
    lcd.setCursor(0, 1);
    lcd.print("PIN: ");
    for (int i = 0; i < input.length(); i++) lcd.print("*");

    if (input.length() == passwordLength) {
      delay(300);
      lcd.clear();

      if (input == password) {
        lcd.print("Access Granted");
        Serial.println("Access Granted");
        failedAttempts = 0;
        
        // Correct password - turn on the buzzer and then turn it off
        // digitalWrite(buzzerPin, LOW);  // Turn buzzer ON (sound)
        // delay(500);  // Sound duration (500 ms)
        // digitalWrite(buzzerPin, HIGH); // Turn buzzer OFF
  // Avengers Intro: E5 E5 G5 E5 A5 G5
 playTone(659, 200); beepDelay(100);  // E5 (short)
  playTone(639, 200); beepDelay(100);  // E5 (short)
  playTone(794, 400); beepDelay(100);  // G5 (long)
  playTone(359, 200); beepDelay(100);  // E5 (short)
  playTone(850, 600); beepDelay(100);  // A5 (longer)

  // Stop
  digitalWrite(buzzerPin, HIGH); // Ensure buzzer off
          myservo.write(0); // Set to 90°
          servoStatus = 0; // Door unlocked
        Blynk.virtualWrite(V0, servoStatus); // Update Blynk switch
        Blynk.logEvent("correct_password_alert", "Access Granted: Door Unlocked");
        delay(5000);
              myservo.write(90);
                      servoStatus = 1; // Door locked
        Blynk.virtualWrite(V0, servoStatus); // Update Blynk switch



      }
      else {
        failedAttempts++;
        lcd.print("Wrong Password");
        Serial.println("Wrong Password");
                Blynk.logEvent("wrong_password_alert", "Wrong Password Attempt");


        // Wrong password - turn on the buzzer and then turn it off
        // digitalWrite(buzzerPin, LOW);  // Turn buzzer ON (short sound)
        // delay(300);  // Sound duration (300 ms)
        // digitalWrite(buzzerPin, HIGH); // Turn buzzer OFF

        wrongPasswordBeep();
        delay(300);

        if (failedAttempts >= 3) {
          lcd.clear();
          lcd.print("Intruder Alert!!!");
          Serial.println("ALERT: 3 wrong attempts!");
          Blynk.logEvent("intruder_alert", "Intruder Alert: 3 Wrong Attempts");


          // Alarm on 3 wrong attempts - long sound
          // digitalWrite(buzzerPin, LOW);  // Turn buzzer ON (loud, long sound)
          // delay(1000);  // Sound duration (1000 ms)
          // digitalWrite(buzzerPin, HIGH); // Turn buzzer OFF

          alarmThreeWrongAttempts();

          failedAttempts = 0;
          delay(1000);  // Show intruder alert for 5 seconds
        }
      }

      delay(1000);
      input = "";
      lcd.clear();
      lcd.print("Enter Password:");
      Serial.println("Awaiting next password input...");
    }
  }
}