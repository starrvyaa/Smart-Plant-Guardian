#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define SDA_PIN       21  
#define SCL_PIN       22  
#define TRIG_PIN      5
#define ECHO_PIN      18
#define DHT_PIN       4
#define DHT_TYPE      DHT11 
#define SERVO_PIN     13
#define BUZZER_PIN    25
#define LED_PIN       27 
#define BUTTON_PIN    32

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);
Servo myServo;

unsigned long prevSensorRead = 0;
unsigned long prevBlink = 0;
unsigned long prevServo = 0;
unsigned long muteStartTime = 0;
unsigned long btnPressStartTime = 0;
unsigned long testStartTime = 0;
unsigned long prevSerialPrint = 0;

float temperature = 25.0; 
float humidity = 50.0;
long distance = 0;

bool isMuted = false;
bool isTestMode = false;
int oledMode = 0; 

bool lastBtnState = HIGH;
bool isBtnPressed = false;
bool ledState = LOW; 

int servoPos = 0;
int servoDir = 5;

long readUltrasonic();
void handleButton(unsigned long currentMillis);
void runMainLogic(unsigned long currentMillis);
void runTestMode(unsigned long currentMillis);
void updateOLED();

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW); 
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  
  Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED gagal diinisialisasi! Cek perkabelan atau alamat I2C."));
    for(;;);
  }
  
   display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.drawCircle(20, 32, 18, WHITE); 
  display.fillCircle(14, 25, 2, WHITE);  
  display.fillCircle(26, 25, 2, WHITE);  
  display.drawLine(14, 37, 17, 40, WHITE);
  display.drawLine(17, 40, 23, 40, WHITE);
  display.drawLine(23, 40, 26, 37, WHITE);

  display.setCursor(45, 8);
  display.println(F("Haiii!"));
  display.setCursor(45, 20);
  display.println(F("Safina R. S."));
  display.setCursor(45, 34);
  display.println(F("e43251922"));
  display.setCursor(45, 46);
  display.println(F("Prodi: TRK"));
  
  display.display();

  Serial.println(F(""));
  Serial.println(F("============================="));
  Serial.println(F("Haiii!"));
  Serial.println(F("Nama : Safina R. S."));
  Serial.println(F("NIM  : e43251922"));
  Serial.println(F("Prodi: TRK"));
  Serial.println(F("============================="));
  Serial.println(F("Sistem Memulai...\n"));

  delay(3000); 

  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();

  handleButton(currentMillis);

  if (isMuted && (currentMillis - muteStartTime >= 60000)) {
    isMuted = false;
  }

    if (currentMillis - prevSensorRead >= 200) {
    prevSensorRead = currentMillis;
    distance = readUltrasonic();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;
  }

    if (isTestMode) {
    runTestMode(currentMillis);
  } else {
    runMainLogic(currentMillis);
  }

   updateOLED();

    if (currentMillis - prevSerialPrint >= 1000) {
    prevSerialPrint = currentMillis;
    
    Serial.println(F("===================================="));
    
    if (oledMode == 0) {
      Serial.print(F("MODE   : PENJAGAAN SERANGGA "));
    } else {
      Serial.print(F("MODE   : MONITOR LINGKUNGAN "));
    }
    if (isMuted) Serial.print(F("[MUTE]"));
    if (isTestMode) Serial.print(F("[TEST MODE]"));
    Serial.println();

    Serial.print(F("Jarak  : "));
    if (distance >= 999) {
      Serial.println(F("Out of Range"));
    } else {
      Serial.print(distance);
      Serial.println(F(" cm"));
    }
    
    Serial.print(F("Status : "));
    if (distance < 15) Serial.println(F("BAHAYA! (Usir Serangga)"));
    else if (distance <= 40) Serial.println(F("WASPADA (Ada Serangga)"));
    else Serial.println(F("AMAN (Tanaman Aman)"));

    Serial.print(F("Suhu   : "));
    Serial.print(temperature, 1);
    Serial.print(F(" C ("));
    if (temperature < 22.0 || temperature > 30.0) Serial.print(F("EKSTREM!"));
    else Serial.print(F("Optimal"));
    Serial.println(F(")"));

    Serial.print(F("Lembab : "));
    Serial.print(humidity, 1);
    Serial.println(F(" %"));
    Serial.println(F("====================================\n"));
  }
}

long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 50000); 
  
  if (duration == 0) {
    return 999;
  }
  
  return duration * 0.034 / 2;
}

void runMainLogic(unsigned long currentMillis) {
  bool isTempExtreme = (temperature < 22.0 || temperature > 30.0);
  bool isPestDanger = (distance < 15);
  bool isPestWarning = (distance >= 15 && distance <= 40);

  if (isPestDanger) {
    if (currentMillis - prevServo >= 15) {
      prevServo = currentMillis;
      servoPos += servoDir;
      if (servoPos <= 0 || servoPos >= 180) servoDir = -servoDir;
      myServo.write(servoPos);
    }
  } else {
    myServo.write(0); 
  }

  if (isPestDanger) {
    if (currentMillis - prevBlink >= 100) {
      prevBlink = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } 
  else if (isPestWarning) {
    digitalWrite(LED_PIN, HIGH);
    ledState = HIGH; 
  } 
  else {
    digitalWrite(LED_PIN, LOW);
    ledState = LOW;
  }

  if (isMuted) {
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    if (isPestDanger) {
      if (ledState) tone(BUZZER_PIN, 2000);
      else {
        noTone(BUZZER_PIN);
        digitalWrite(BUZZER_PIN, LOW);
      }
    } 
    else if (isTempExtreme) {
      tone(BUZZER_PIN, 1000);
    } 
    else {
      noTone(BUZZER_PIN);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

void handleButton(unsigned long currentMillis) {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading == LOW && lastBtnState == HIGH) {
    btnPressStartTime = currentMillis;
    isBtnPressed = true;
  }

  if (reading == HIGH && lastBtnState == LOW && isBtnPressed) {
    unsigned long pressDuration = currentMillis - btnPressStartTime;
    isBtnPressed = false;

    if (pressDuration >= 3000) {
      isTestMode = true;
      testStartTime = currentMillis;
    } 
    else if (pressDuration >= 1000) {
      isMuted = !isMuted;
      if (isMuted) muteStartTime = currentMillis;
    } 
    else {
      oledMode = (oledMode == 0) ? 1 : 0;
    }
  }
  lastBtnState = reading;
}

void runTestMode(unsigned long currentMillis) {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);

  if (currentMillis - prevServo >= 10) {
    prevServo = currentMillis;
    servoPos += servoDir;
    if (servoPos <= 0 || servoPos >= 180) servoDir = -servoDir;
    myServo.write(servoPos);
  }

  if (currentMillis - testStartTime >= 3000) {
    isTestMode = false;
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
    myServo.write(0);
    digitalWrite(LED_PIN, LOW);
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (isTestMode) {
    display.setCursor(15, 25);
    display.print(F("--- TEST MODE ---"));
  } 
  else if (oledMode == 0) {
    display.setCursor(8, 0);
    display.print(F("PENJAGAAN SERANGGA"));
    display.drawLine(0, 10, 128, 10, WHITE);
    
    display.setCursor(0, 18);
    display.print(F("Jarak : "));
    if (distance >= 999) display.print(F("Out of Range"));
    else {
      display.print(distance);
      display.print(F(" cm"));
    }

    display.setCursor(0, 34);
    if (distance < 15) {
      display.print(F("Status: BAHAYA!"));
      display.setCursor(0, 48);
      display.print(F("Usir Serangga!"));
    } else if (distance <= 40) {
      display.print(F("Status: WASPADA"));
      display.setCursor(0, 48);
      display.print(F("Ada Serangga"));
    } else {
      display.print(F("Status: AMAN"));
      display.setCursor(0, 48);
      display.print(F("Tanaman Aman"));
    }
  } 
  else {
    display.setCursor(10, 0);
    display.print(F("MONITOR LINGKUNGAN"));
    display.drawLine(0, 10, 128, 10, WHITE);
    
    display.setCursor(0, 18);
    display.print(F("Suhu   : "));
    display.print(temperature, 1);
    display.print(F(" C"));
    
    display.setCursor(0, 30);
    display.print(F("Lembab : "));
    display.print(humidity, 1);
    display.print(F(" %"));

    display.setCursor(0, 48);
    if (temperature < 22.0 || temperature > 30.0) {
      display.print(F("Ket    : SUHU EKSTREM!"));
    } else {
      display.print(F("Ket    : Suhu Optimal"));
    }
  }

  if (isMuted) {
    display.setCursor(90, 54);
    display.print(F("[MUTE]"));
  }

  display.display();
}