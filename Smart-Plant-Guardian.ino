#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

// --- PIN DEFINITIONS ---
#define SDA_PIN       21
#define SCL_PIN       22
#define TRIG_PIN      5
#define ECHO_PIN      18
#define DHT_PIN       4
#define DHT_TYPE      DHT22 
#define SERVO_PIN     13
#define BUZZER_PIN    25
#define LED_PIN       27 
#define BUTTON_PIN    32

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);
Servo myServo;

// --- WIFI CREDENTIALS ---
const char* ssid = "ZTE_2.4G_SA7sNx";       
const char* password = "12345678"; 

// Membuat objek WebServer di port 80
WebServer server(80);

// --- VARIABEL & TIMER ---
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
int currentBuzzerFreq = -1; // Menyimpan status frekuensi buzzer saat ini

// ==========================================
// KODE HTML, CSS, & JS UNTUK TAMPILAN WEBSITE
// ==========================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Plant Guardian</title>
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    <style>
        :root { --bg: #f4f7f6; --primary: #2c3e50; --card: #ffffff; --text: #333; --green: #27ae60; --yellow: #f39c12; --red: #e74c3c; }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: var(--bg); margin: 0; padding: 0; color: var(--text); }
        .header { background: var(--primary); color: white; padding: 20px; text-align: center; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        .header h1 { margin: 0; font-size: 24px; letter-spacing: 1px; }
        .header p { margin: 5px 0 0 0; font-size: 14px; opacity: 0.8; }
        .container { max-width: 1000px; margin: 30px auto; padding: 0 20px; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; }
        .card { background: var(--card); border-radius: 12px; padding: 25px; box-shadow: 0 5px 15px rgba(0,0,0,0.05); text-align: center; transition: transform 0.3s; }
        .card:hover { transform: translateY(-5px); }
        .card i { font-size: 40px; margin-bottom: 15px; color: var(--primary); }
        .card h3 { margin: 0 0 10px 0; font-size: 18px; color: #7f8c8d; }
        .value { font-size: 36px; font-weight: bold; margin: 0; }
        .status-badge { display: inline-block; padding: 8px 15px; border-radius: 20px; font-weight: bold; font-size: 14px; margin-top: 15px; color: white; transition: background 0.3s;}
        .safe { background: var(--green); } .warning { background: var(--yellow); color: white; } .danger { background: var(--red); }
        .controls { margin-top: 30px; display: flex; justify-content: center; gap: 15px; flex-wrap: wrap; }
        button { background: var(--primary); color: white; border: none; padding: 12px 25px; border-radius: 8px; font-size: 16px; cursor: pointer; font-weight: bold; transition: all 0.3s; }
        button:hover { background: #1a252f; }
        button.active { background: var(--red); }
        .footer { text-align: center; margin-top: 40px; color: #95a5a6; font-size: 14px; padding-bottom: 20px;}

        /* CSS NOTIFIKASI */
        #toast {
            visibility: hidden; min-width: 250px; background-color: #333; color: #fff; text-align: center; 
            border-radius: 8px; padding: 16px; position: fixed; z-index: 1; left: 50%; bottom: 30px; 
            font-size: 17px; transform: translateX(-50%); box-shadow: 0px 4px 10px rgba(0,0,0,0.3);
        }
        #toast.show { visibility: visible; animation: fadein 0.5s, fadeout 0.5s 4.5s; }
        #toast.danger-toast { background-color: var(--red); font-weight: bold; }
        #toast.warning-toast { background-color: var(--yellow); font-weight: bold; color: #333;}
        
        @keyframes fadein { from {bottom: 0; opacity: 0;} to {bottom: 30px; opacity: 1;} }
        @keyframes fadeout { from {bottom: 30px; opacity: 1;} to {bottom: 0; opacity: 0;} }
    </style>
</head>
<body>
    <div class="header">
        <h1><i class="fas fa-leaf"></i> Smart Plant Guardian</h1>
        <p>Real-time Monitoring Dashboard</p>
    </div>
    
    <div class="container">
        <div class="grid">
            <!-- Jarak Card -->
            <div class="card">
                <i class="fas fa-bug"></i>
                <h3>Deteksi Hama (Jarak)</h3>
                <p class="value"><span id="distVal">--</span> <span style="font-size: 20px;">cm</span></p>
                <div id="distStatus" class="status-badge safe">Memuat...</div>
            </div>
            <!-- Suhu Card -->
            <div class="card">
                <i class="fas fa-temperature-high" style="color: #e67e22;"></i>
                <h3>Suhu Lingkungan</h3>
                <p class="value"><span id="tempVal">--</span> <span style="font-size: 20px;">°C</span></p>
                <div id="tempStatus" class="status-badge safe">Memuat...</div>
            </div>
            <!-- Kelembaban Card -->
            <div class="card">
                <i class="fas fa-tint" style="color: #3498db;"></i>
                <h3>Kelembaban</h3>
                <p class="value"><span id="humVal">--</span> <span style="font-size: 20px;">%</span></p>
            </div>
        </div>

        <div class="controls">
            <button type="button" id="btnMute" onclick="toggleMute()"><i class="fas fa-volume-mute"></i> Toggle Mute Alarm</button>
            <button type="button" id="btnTest" onclick="triggerTest()"><i class="fas fa-tools"></i> Trigger Test Mode</button>
        </div>
    </div>
    
    <div class="footer">
        Dibuat oleh Safina Rahmatus Sa'diyah. (E43251922) - Prodi: TRK
    </div>

    <!-- Container Notifikasi -->
    <div id="toast">Notifikasi!</div>

    <script>
        let lastPestState = "safe";
        let lastTempState = "safe";

        function showToast(message, type) {
            let toast = document.getElementById("toast");
            toast.innerText = message;
            toast.className = "show " + type;
            setTimeout(function(){ toast.className = toast.className.replace("show", ""); }, 5000);
        }

        setInterval(fetchData, 1000);

        function fetchData() {
            fetch('/data')
            .then(response => response.json())
            .then(data => {
                document.getElementById('distVal').innerText = data.dist > 990 ? "Out" : data.dist;
                document.getElementById('tempVal').innerText = data.temp.toFixed(1);
                document.getElementById('humVal').innerText = data.hum.toFixed(1);
                
                // Status Jarak
                let distBadge = document.getElementById('distStatus');
                let currentPestState = "safe";
                
                if (data.dist < 15) {
                    currentPestState = "danger";
                    distBadge.className = 'status-badge danger'; distBadge.innerText = 'BAHAYA! (Usir)';
                } else if (data.dist <= 40) {
                    currentPestState = "warning";
                    distBadge.className = 'status-badge warning'; distBadge.innerText = 'WASPADA (Ada Hama)';
                } else {
                    currentPestState = "safe";
                    distBadge.className = 'status-badge safe'; distBadge.innerText = 'AMAN';
                }

                if (currentPestState === "danger" && lastPestState !== "danger") {
                    showToast("⚠️ AWAS! Ada Hama mendekat! Sistem pengusir aktif!", "danger-toast");
                } else if (currentPestState === "warning" && lastPestState === "safe") {
                    showToast("👀 Waspada, terdeteksi pergerakan di dekat tanaman.", "warning-toast");
                }
                lastPestState = currentPestState;

                // Status Suhu
                let tempBadge = document.getElementById('tempStatus');
                let currentTempState = (data.temp < 22 || data.temp > 30) ? "danger" : "safe";
                
                if (currentTempState === "danger") {
                    tempBadge.className = 'status-badge danger'; tempBadge.innerText = 'SUHU EKSTREM!';
                } else {
                    tempBadge.className = 'status-badge safe'; tempBadge.innerText = 'OPTIMAL';
                }

                if (currentTempState === "danger" && lastTempState !== "danger") {
                    showToast("🌡️ PERINGATAN: Suhu Lingkungan Ekstrem!", "danger-toast");
                }
                lastTempState = currentTempState;

                // Visual Tombol Mute
                let btnMute = document.getElementById('btnMute');
                if(data.muted) {
                    btnMute.classList.add('active');
                    btnMute.innerHTML = '<i class="fas fa-volume-up"></i> Alarm Bisu (Aktif)';
                } else {
                    btnMute.classList.remove('active');
                    btnMute.innerHTML = '<i class="fas fa-volume-mute"></i> Mute Alarm';
                }

                // Visual Tombol Test
                let btnTest = document.getElementById('btnTest');
                if(data.testMode) {
                    btnTest.classList.add('active');
                    btnTest.innerHTML = '<i class="fas fa-tools"></i> Mode Test (Aktif)';
                } else {
                    btnTest.classList.remove('active');
                    btnTest.innerHTML = '<i class="fas fa-tools"></i> Trigger Test Mode';
                }
            })
            .catch(err => console.error("Error Fetch Data:", err));
        }

        function toggleMute() { 
            fetch('/toggleMute')
            .then(() => fetchData())
            .catch(err => console.error("Error Mute:", err)); 
        }
        
        function triggerTest() { 
            fetch('/triggerTest')
            .then(() => fetchData())
            .catch(err => console.error("Error Test:", err)); 
        }
        
        fetchData();
    </script>
</body>
</html>
)rawliteral";
// ==========================================


// Deklarasi fungsi
long readUltrasonic();
void handleButton(unsigned long currentMillis);
void runMainLogic(unsigned long currentMillis);
void runTestMode(unsigned long currentMillis);
void updateOLED();
void setBuzzer(int freq);

// --- FUNGSI HELPER BUZZER (Perbaikan Utama) ---
void setBuzzer(int freq) {
  if (isMuted) freq = -1; // Jika Muted, matikan buzzer

  if (freq != currentBuzzerFreq) {
    currentBuzzerFreq = freq;
    if (freq > 0) {
      tone(BUZZER_PIN, freq);
      digitalWrite(BUZZER_PIN, HIGH); // Jalur alternatif untuk Active Buzzer
    } else {
      noTone(BUZZER_PIN);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

// --- FUNGSI ROUTING WEB SERVER ---
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleData() {
  char jsonBuffer[250];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"temp\":%.1f, \"hum\":%.1f, \"dist\":%ld, \"muted\":%s, \"testMode\":%s}", 
           temperature, humidity, distance, 
           isMuted ? "true" : "false", 
           isTestMode ? "true" : "false");
           
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonBuffer);
}

void handleToggleMute() {
  isMuted = !isMuted;
  if (isMuted) {
    muteStartTime = millis();
    setBuzzer(-1);
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK");
}

void handleTriggerTest() {
  isTestMode = !isTestMode;
  if (isTestMode) {
    testStartTime = millis();
  } else {
    setBuzzer(-1);
    myServo.write(0);
    digitalWrite(LED_PIN, LOW);
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK");
}

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

  // INISIALISASI I2C & OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED gagal diinisialisasi!"));
    for(;;);
  }
  
  // LAYAR AWAL & KONEKSI WIFI
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Menghubungkan WiFi.."));
  display.println(ssid);
  display.display();

  Serial.print(F("Menghubungkan ke "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(F("."));
    display.print(F("."));
    display.display();
    attempt++;
  }
  Serial.println();

  display.clearDisplay();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("WiFi Terhubung!"));
    Serial.print(F("Alamat IP Web: "));
    Serial.println(WiFi.localIP());
    
    display.setCursor(0, 10);
    display.println(F("WiFi Terhubung!"));
    display.print(F("IP: "));
    display.println(WiFi.localIP());
  } else {
    Serial.println(F("Gagal Konek WiFi. Jalan Offline."));
    display.setCursor(0, 10);
    display.println(F("Offline Mode"));
  }
  display.display();
  delay(3000);

  // ROUTING WEB SERVER
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/toggleMute", handleToggleMute);
  server.on("/triggerTest", handleTriggerTest);
  server.begin();
  Serial.println(F("Web Server Aktif."));

  // Tampilan Wajah Tersenyum
  display.clearDisplay();
  display.drawCircle(20, 32, 18, WHITE);
  display.fillCircle(14, 25, 2, WHITE);
  display.fillCircle(26, 25, 2, WHITE);
  display.drawLine(14, 37, 17, 40, WHITE);
  display.drawLine(17, 40, 23, 40, WHITE);
  display.drawLine(23, 40, 26, 37, WHITE);
  display.setCursor(45, 8);  display.println(F("Haiii!"));
  display.setCursor(45, 20); display.println(F("Safina R. S."));
  display.setCursor(45, 34); display.println(F("e43251922"));
  display.setCursor(45, 46); display.println(F("Prodi: TRK"));
  display.display();
  delay(3000);
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. TANGANI REQUEST DARI WEBSITE
  server.handleClient();

  // 2. MANAJEMEN TOMBOL FISIK
  handleButton(currentMillis);

  // 3. CEK TIMER MUTE (60 Detik)
  if (isMuted && (currentMillis - muteStartTime >= 60000)) {
    isMuted = false;
  }

  // 4. PEMBACAAN SENSOR (Setiap 200ms)
  if (currentMillis - prevSensorRead >= 200) {
    prevSensorRead = currentMillis;
    distance = readUltrasonic();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;
  }

  // 5. LOGIKA SISTEM
  if (isTestMode) {
    runTestMode(currentMillis);
  } else {
    runMainLogic(currentMillis);
  }

  // 6. UPDATE TAMPILAN OLED
  updateOLED();

  // 7. PRINT SERIAL
  if (currentMillis - prevSerialPrint >= 2000) {
    prevSerialPrint = currentMillis;
    Serial.print(F("Jarak: ")); Serial.print(distance);
    Serial.print(F("cm | Suhu: ")); Serial.print(temperature);
    Serial.print(F("C | WiFi IP: "));
    if(WiFi.status() == WL_CONNECTED) Serial.println(WiFi.localIP());
    else Serial.println(F("Disconnected"));
  }
}

// --- FUNGSI ULTRASONIK ---
long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 50000); 
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// --- LOGIKA UTAMA ---
void runMainLogic(unsigned long currentMillis) {
  bool isTempExtreme = (temperature < 22.0 || temperature > 30.0);
  bool isPestDanger = (distance < 15);
  bool isPestWarning = (distance >= 15 && distance <= 40);

  // LOGIKA SERVO
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

  // LOGIKA LED
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

  // LOGIKA BUZZER
  if (isPestDanger) {
    if (ledState) setBuzzer(2000);
    else setBuzzer(-1);
  } 
  else if (isTempExtreme) {
    setBuzzer(1000);
  } 
  else {
    setBuzzer(-1);
  }
}

// --- FUNGSI MANAJEMEN TOMBOL FISIK ---
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
      isTestMode = !isTestMode;
      if (isTestMode) testStartTime = currentMillis;
      else {
        setBuzzer(-1);
        myServo.write(0);
        digitalWrite(LED_PIN, LOW);
      }
    } else if (pressDuration >= 1000) {
      isMuted = !isMuted;
      if (isMuted) {
        muteStartTime = currentMillis;
        setBuzzer(-1);
      }
    } else {
      oledMode = (oledMode == 0) ? 1 : 0;
    }
  }
  lastBtnState = reading;
}

// --- MODE TEST ---
void runTestMode(unsigned long currentMillis) {
  digitalWrite(LED_PIN, HIGH);
  setBuzzer(1500);

  if (currentMillis - prevServo >= 10) {
    prevServo = currentMillis;
    servoPos += servoDir;
    if (servoPos <= 0 || servoPos >= 180) servoDir = -servoDir;
    myServo.write(servoPos);
  }

  // Mati otomatis setelah 3 detik
  if (currentMillis - testStartTime >= 3000) {
    isTestMode = false;
    setBuzzer(-1);
    myServo.write(0);
    digitalWrite(LED_PIN, LOW);
  }
}

// --- UPDATE TAMPILAN OLED ---
void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (isTestMode) {
    display.setCursor(15, 25);
    display.print(F("--- TEST MODE ---"));
  } 
  else if (oledMode == 0) {
    display.setCursor(8, 0); display.print(F("PENJAGAAN SERANGGA"));
    display.drawLine(0, 10, 128, 10, WHITE);
    display.setCursor(0, 18); display.print(F("Jarak : "));
    if (distance >= 999) display.print(F("Out of Range"));
    else { display.print(distance); display.print(F(" cm")); }

    display.setCursor(0, 34);
    if (distance < 15) { display.print(F("Status: BAHAYA!")); display.setCursor(0, 48); display.print(F("Usir Serangga!")); } 
    else if (distance <= 40) { display.print(F("Status: WASPADA")); display.setCursor(0, 48); display.print(F("Ada Serangga")); } 
    else { display.print(F("Status: AMAN")); display.setCursor(0, 48); display.print(F("Tanaman Aman")); }
  } 
  else {
    display.setCursor(10, 0); display.print(F("MONITOR LINGKUNGAN"));
    display.drawLine(0, 10, 128, 10, WHITE);
    display.setCursor(0, 18); display.print(F("Suhu   : ")); display.print(temperature, 1); display.print(F(" C"));
    display.setCursor(0, 30); display.print(F("Lembab : ")); display.print(humidity, 1); display.print(F(" %"));
    display.setCursor(0, 48);
    if (temperature < 22.0 || temperature > 30.0) display.print(F("Ket    : SUHU EKSTREM!"));
    else display.print(F("Ket    : Suhu Optimal"));
  }

  if (isMuted) { display.setCursor(90, 54); display.print(F("[MUTE]")); }
  display.display();
}