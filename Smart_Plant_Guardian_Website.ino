#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// SMART PLANT GUARDIAN
// ESP32 + HC-SR04 + DHT22 + Servo + Buzzer + LED + OLED
// ============================================================

// ---------------- PIN DEFINITIONS ----------------
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

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);
Servo myServo;

// ---------------- WIFI ----------------
const char* ssid = "ZTE_2.4G_SA7sNx";
const char* password = "12345678";

WebServer server(80);

// ---------------- SENSOR / TIMER ----------------
unsigned long prevSensorRead = 0;
unsigned long prevBlink = 0;
unsigned long prevServo = 0;
unsigned long btnPressStartTime = 0;
unsigned long testStartTime = 0;
unsigned long prevSerialPrint = 0;

float temperature = 25.0;
float humidity = 50.0;
long distance = 999;

bool alarmEnabled = true;
bool isTestMode = false;
int oledMode = 0;

bool lastBtnState = HIGH;
bool isBtnPressed = false;
bool ledState = LOW;

int servoPos = 0;
int servoDir = 5;
int currentBuzzerFreq = -1;

// ============================================================
// WEB DASHBOARD
// ============================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="theme-color" content="#18352a">
<title>Smart Plant Guardian</title>

<style>
:root{
  --bg:#f3f7f5;
  --card:#ffffff;
  --nav:#18352a;
  --nav2:#244b3b;
  --text:#17231e;
  --muted:#75837d;
  --line:#e3ebe6;
  --green:#20a463;
  --green-dark:#16784a;
  --green-soft:#eaf8f0;
  --orange:#ee922d;
  --orange-soft:#fff4e5;
  --red:#df5050;
  --red-soft:#fdecec;
  --blue:#398ed4;
  --blue-soft:#eaf4fc;
  --shadow:0 12px 32px rgba(25,55,43,.075);
}

*{box-sizing:border-box}

html{scroll-behavior:smooth}

body{
  margin:0;
  background:var(--bg);
  color:var(--text);
  font-family:Inter,"Segoe UI",Arial,sans-serif;
}

button{
  font-family:inherit;
}

.app{
  min-height:100vh;
}

/* ================= SIDEBAR ================= */

.sidebar{
  position:fixed;
  left:0;
  top:0;
  bottom:0;
  width:245px;
  padding:22px 15px;
  background:linear-gradient(180deg,var(--nav),#10251c);
  color:#fff;
  z-index:50;
}

.brand{
  display:flex;
  align-items:center;
  gap:11px;
  padding:5px 9px 25px;
}

.brand-icon{
  width:43px;
  height:43px;
  display:grid;
  place-items:center;
  border-radius:13px;
  background:var(--yellow);
  font-size:23px;
}

.brand-name{
  font-size:16px;
  font-weight:800;
}

.brand-sub{
  color:#9fb4aa;
  font-size:11px;
  margin-top:3px;
}

.nav-label{
  color:#728d81;
  text-transform:uppercase;
  letter-spacing:1.4px;
  font-size:10px;
  margin:18px 10px 8px;
}

.nav a{
  display:flex;
  align-items:center;
  gap:12px;
  padding:11px 12px;
  margin:3px 0;
  border-radius:10px;
  color:#cbd9d3;
  text-decoration:none;
  font-size:13px;
  transition:.2s;
}

.nav a:hover,
.nav a.active{
  color:#fff;
  background:var(--nav2);
}

.nav-icon{
  width:21px;
  text-align:center;
  font-size:16px;
}

.side-status{
  position:absolute;
  left:15px;
  right:15px;
  bottom:17px;
  padding:13px;
  border-radius:13px;
  border:1px solid #315244;
  background:#1b382d;
  font-size:12px;
}

.side-status small{
  display:block;
  color:#91a79d;
  margin-top:7px;
}

.dot{
  display:inline-block;
  width:8px;
  height:8px;
  border-radius:50%;
  background:#41d889;
  margin-right:6px;
  box-shadow:0 0 0 4px rgba(65,216,137,.08);
}

/* ================= MAIN ================= */

.main{
  margin-left:245px;
  padding:26px 31px 40px;
  max-width:1500px;
}

.topbar{
  display:flex;
  align-items:center;
  justify-content:space-between;
  gap:15px;
  margin-bottom:22px;
}

.eyebrow{
  color:var(--green);
  text-transform:uppercase;
  letter-spacing:1.4px;
  font-size:11px;
  font-weight:800;
}

h1{
  margin:5px 0 3px;
  font-size:28px;
}

.subtitle{
  color:var(--muted);
  font-size:13px;
}

.top-actions{
  display:flex;
  align-items:center;
  gap:9px;
}

.live{
  padding:10px 13px;
  border:1px solid var(--line);
  background:#fff;
  border-radius:10px;
  box-shadow:var(--shadow);
  font-size:12px;
}

.btn{
  border:0;
  background:var(--nav);
  color:#fff;
  padding:10px 14px;
  border-radius:10px;
  cursor:pointer;
  font-size:12px;
  font-weight:800;
  transition:.2s;
}

.btn:hover{
  transform:translateY(-1px);
  filter:brightness(1.08);
}

.btn:active{
  transform:translateY(0);
}

.mobile-menu{
  display:none;
  border:0;
  background:#fff;
  border-radius:9px;
  padding:9px 12px;
  cursor:pointer;
}

/* ================= GENERAL CARDS ================= */

.grid{
  display:grid;
  gap:17px;
}

.card{
  background:var(--card);
  border:1px solid var(--line);
  border-radius:17px;
  box-shadow:var(--shadow);
}

.summary{
  grid-template-columns:1.3fr repeat(3,1fr);
}

.health-card{
  min-height:174px;
  padding:20px;
  position:relative;
  overflow:hidden;
  color:#fff;
  background:linear-gradient(135deg,#1c4032,#2b654b);
}

.health-card:before{
  content:"";
  position:absolute;
  width:190px;
  height:190px;
  right:-70px;
  top:-90px;
  border-radius:50%;
  background:rgba(255,255,255,.055);
}

.health-label{
  color:#b9d9ca;
  text-transform:uppercase;
  letter-spacing:1px;
  font-size:10px;
  font-weight:700;
}

.health-row{
  display:flex;
  align-items:center;
  justify-content:space-between;
  margin-top:15px;
}

.health-score{
  font-size:43px;
  line-height:1;
  font-weight:900;
}

.health-score span{
  font-size:16px;
  color:#b9d9ca;
}

.health-status{
  margin-top:7px;
  font-weight:800;
  font-size:13px;
}

.health-note{
  margin-top:14px;
  color:#d3e6dd;
  font-size:10px;
  line-height:1.45;
  max-width:330px;
}

.ring{
  width:76px;
  height:76px;
  border-radius:50%;
  display:grid;
  place-items:center;
  background:conic-gradient(#53dd90 0 86%,rgba(255,255,255,.12) 86% 100%);
}

.ring:after{
  content:"86%";
  width:58px;
  height:58px;
  border-radius:50%;
  background:#214b39;
  display:grid;
  place-items:center;
  font-size:14px;
  font-weight:900;
}

.metric{
  padding:18px;
  min-height:174px;
}

.metric-top{
  display:flex;
  align-items:flex-start;
  justify-content:space-between;
  gap:8px;
}

.metric-icon{
  width:41px;
  height:41px;
  border-radius:12px;
  display:grid;
  place-items:center;
  font-size:19px;
}

.green-icon{background:var(--green-soft);color:var(--green-dark)}
.orange-icon{background:var(--orange-soft);color:var(--orange)}
.blue-icon{background:var(--blue-soft);color:var(--blue)}
.red-icon{background:var(--red-soft);color:var(--red)}

.metric-name{
  color:var(--muted);
  font-size:12px;
  margin-top:14px;
}

.metric-value{
  font-size:29px;
  font-weight:900;
  margin-top:4px;
}

.metric-value small{
  color:var(--muted);
  font-size:13px;
  font-weight:700;
}

.metric-foot{
  color:var(--muted);
  font-size:10px;
  margin-top:7px;
}

.badge{
  display:inline-flex;
  align-items:center;
  padding:5px 8px;
  border-radius:20px;
  font-size:9px;
  font-weight:900;
  letter-spacing:.3px;
}

.badge-ok{
  background:var(--green-soft);
  color:var(--green-dark);
}

.badge-warning{
  background:var(--orange-soft);
  color:#bd7013;
}

.badge-danger{
  background:var(--red-soft);
  color:#bd3d3d;
}

.badge-blue{
  background:var(--blue-soft);
  color:#2675b6;
}

/* ================= SECTION ================= */

.section-title{
  display:flex;
  justify-content:space-between;
  align-items:center;
  gap:15px;
  margin:26px 0 11px;
}

.section-title h2{
  margin:0;
  font-size:16px;
}

.section-title span{
  color:var(--muted);
  font-size:10px;
}

/* ================= EXTRA SENSORS ================= */

.sensor-grid{
  grid-template-columns:repeat(3,1fr);
}

.sensor-card{
  padding:17px;
}

.sensor-card .metric-value{
  font-size:24px;
}

/* ================= CHART ================= */

.main-grid{
  grid-template-columns:2fr 1fr;
}

.chart-card,
.alert-card{
  padding:19px;
}

.chart-head{
  display:flex;
  align-items:flex-start;
  justify-content:space-between;
  gap:10px;
}

.chart-head h3{
  margin:0;
  font-size:15px;
}

.chart-head p{
  margin:5px 0 0;
  color:var(--muted);
  font-size:10px;
}

.tabs{
  display:flex;
  gap:5px;
  flex-wrap:wrap;
}

.tab{
  border:1px solid var(--line);
  background:#fff;
  color:var(--muted);
  border-radius:8px;
  padding:6px 8px;
  cursor:pointer;
  font-size:9px;
  font-weight:700;
}

.tab.active{
  background:var(--green-soft);
  color:var(--green-dark);
  border-color:#c5e7d4;
}

#chart{
  display:block;
  width:100%;
  height:255px;
  margin-top:15px;
}

/* ================= ALERT ================= */

.alert-list{
  display:flex;
  flex-direction:column;
  gap:9px;
  margin-top:15px;
}

.alert{
  display:flex;
  align-items:flex-start;
  gap:10px;
  padding:11px;
  border-radius:11px;
  border:1px solid var(--line);
  background:#f9fbfa;
}

.alert-icon{
  width:32px;
  height:32px;
  flex:none;
  display:grid;
  place-items:center;
  border-radius:9px;
  font-size:14px;
}

.alert-icon.ok{background:var(--green-soft);color:var(--green-dark)}
.alert-icon.warning{background:var(--orange-soft);color:#bd7013}
.alert-icon.danger{background:var(--red-soft);color:#bd3d3d}
.alert-icon.info{background:var(--blue-soft);color:#2675b6}

.alert strong{
  font-size:11px;
}

.alert p{
  margin:4px 0 0;
  color:var(--muted);
  font-size:9px;
  line-height:1.4;
}

/* ================= DEVICE ================= */

.device-grid{
  grid-template-columns:repeat(3,1fr);
}

.device-card{
  padding:15px;
  display:flex;
  align-items:center;
  gap:12px;
}

.device-icon{
  width:40px;
  height:40px;
  border-radius:11px;
  display:grid;
  place-items:center;
  background:#f1f6f3;
  font-size:18px;
}

.device-card strong{
  display:block;
  font-size:12px;
}

.device-card small{
  display:block;
  color:var(--muted);
  margin-top:4px;
  font-size:10px;
}

/* ================= CONTROL ================= */

.control-card{
  padding:18px;
  display:flex;
  align-items:center;
  justify-content:space-between;
  gap:20px;
}

.control-left{
  display:flex;
  align-items:center;
  gap:13px;
}

.control-icon{
  width:45px;
  height:45px;
  display:grid;
  place-items:center;
  border-radius:12px;
  background:var(--nav);
  color:#fff;
  font-size:19px;
}

.control-left strong{
  font-size:13px;
}

.control-left small{
  display:block;
  color:var(--muted);
  margin-top:4px;
  font-size:10px;
}

.control-buttons{
  display:flex;
  gap:8px;
  flex-wrap:wrap;
}

.control-btn{
  border:0;
  border-radius:9px;
  padding:10px 13px;
  cursor:pointer;
  font-size:11px;
  font-weight:800;
}

.alarm-on{
  color:#fff;
  background:var(--green);
}

.alarm-off{
  color:#fff;
  background:#7e8985;
}

.test-btn{
  color:#fff;
  background:var(--nav);
}

/* ================= LOG ================= */

.log-card{
  padding:17px;
  overflow:auto;
}

.log-table{
  width:100%;
  border-collapse:collapse;
}

.log-table th,
.log-table td{
  padding:10px 7px;
  text-align:left;
  border-bottom:1px solid var(--line);
  font-size:10px;
  white-space:nowrap;
}

.log-table th{
  color:var(--muted);
  font-weight:700;
}

.log-table td:nth-child(2){
  white-space:normal;
}

/* ================= TOAST ================= */

#toast{
  position:fixed;
  z-index:100;
  top:22px;
  right:22px;
  width:min(390px,calc(100vw - 44px));
  padding:14px 15px;
  border-radius:13px;
  background:#20352c;
  color:#fff;
  box-shadow:0 14px 35px rgba(0,0,0,.18);
  transform:translateX(130%);
  opacity:0;
  transition:.3s ease;
  pointer-events:none;
}

#toast.show{
  transform:translateX(0);
  opacity:1;
}

.toast-title{
  font-weight:900;
  font-size:12px;
  margin-bottom:4px;
}

.toast-message{
  color:#d7e4df;
  font-size:10px;
  line-height:1.4;
}

#toast.danger{
  background:#8f3030;
}

#toast.warning{
  background:#a96816;
}

#toast.success{
  background:#176b45;
}

#toast.info{
  background:#245d87;
}

/* ================= FOOTER ================= */

.footer{
  text-align:center;
  color:#8e9d96;
  font-size:10px;
  margin-top:25px;
}

/* ================= MOBILE ================= */

@media(max-width:1080px){
  .summary{
    grid-template-columns:1fr 1fr;
  }

  .health-card{
    grid-column:1/-1;
  }

  .main-grid{
    grid-template-columns:1fr;
  }
}

@media(max-width:780px){
  .sidebar{
    transform:translateX(-100%);
    transition:.25s;
  }

  .sidebar.open{
    transform:translateX(0);
  }

  .main{
    margin-left:0;
    padding:18px;
  }

  .mobile-menu{
    display:block;
  }

  .live{
    display:none;
  }

  .topbar{
    align-items:flex-start;
  }

  h1{
    font-size:22px;
  }

  .summary,
  .sensor-grid,
  .device-grid{
    grid-template-columns:1fr;
  }

  .health-card{
    grid-column:auto;
  }

  .chart-head{
    flex-direction:column;
  }

  .control-card{
    align-items:flex-start;
    flex-direction:column;
  }

  .control-buttons{
    width:100%;
  }

  .control-btn{
    flex:1;
  }

  #toast{
    top:12px;
    right:12px;
    width:calc(100vw - 24px);
  }
}
</style>
</head>

<body>

<div class="app">

<!-- =========================================================
     SIDEBAR
========================================================= -->
<aside class="sidebar" id="sidebar">

  <div class="brand">
    <div class="brand-icon">☘️</div>
    <div>
      <div class="brand-name">Smart Plant</div>
      <div class="brand-sub">by ayaa/safina</div>
    </div>
  </div>

  <div class="nav-label">Monitoring</div>

  <nav class="nav">
    <a href="#" class="active">
      <span class="nav-icon">⌂</span> Dashboard
    </a>
    <a href="#sensor">
      <span class="nav-icon">◉</span> Sensor Data
    </a>
  </nav>

  <div class="nav-label">System</div>

  <nav class="nav">
    <a href="#device">
      <span class="nav-icon">⚙</span> Perangkat
    </a>
    <a href="#activity">
      <span class="nav-icon">☷</span> Activity Log
    </a>
  </nav>

  <div class="side-status">
    <span><i class="dot"></i>ESP32 Online</span>
    <small id="sideIp">IP: menunggu...</small>
  </div>

</aside>


<!-- =========================================================
     MAIN
========================================================= -->
<main class="main">

  <!-- TOP BAR -->
  <header class="topbar">

    <div>
      <div class="eyebrow">Plant Monitoring System</div>
      <h1>Smart Plant Guardian</h1>
      <div class="subtitle">
        Sistem monitoring tanaman dan deteksi hama secara real-time.
      </div>
    </div>

    <div class="top-actions">
      <button class="mobile-menu" onclick="toggleMenu()">☰</button>

      <div class="live">
        <i class="dot"></i> Live Monitoring
      </div>

      <button class="btn" onclick="refreshData()">↻ Refresh</button>
    </div>

  </header>


  <!-- =======================================================
       SUMMARY SENSOR
  ======================================================== -->
  <section class="grid summary" id="sensor">

    <!-- OVERALL -->
    <div class="card health-card">

      <div class="health-label">Overall Plant Condition</div>

      <div class="health-row">

        <div>
          <div class="health-score">
            <span id="healthScore">86</span><span>/100</span>
          </div>

          <div class="health-status" id="healthStatus">
            Kondisi Baik
          </div>
        </div>

        <div class="ring" id="healthRing"></div>

      </div>

      <div class="health-note">
        Status dihitung dari suhu, kelembapan lingkungan,
        jarak objek, dan kondisi alarm.
      </div>

    </div>


    <!-- JARAK -->
    <div class="card metric">

      <div class="metric-top">
        <div class="metric-icon red-icon">🐛</div>
        <span id="distBadge" class="badge badge-ok">AMAN</span>
      </div>

      <div class="metric-name">Deteksi Hama / Jarak</div>

      <div class="metric-value">
        <span id="distVal">--</span>
        <small>cm</small>
      </div>

      <div class="metric-foot" id="distFoot">
        Menunggu pembacaan sensor...
      </div>

    </div>


    <!-- SUHU -->
    <div class="card metric">

      <div class="metric-top">
        <div class="metric-icon orange-icon">🌡</div>
        <span id="tempBadge" class="badge badge-ok">NORMAL</span>
      </div>

      <div class="metric-name">Suhu Lingkungan</div>

      <div class="metric-value">
        <span id="tempVal">--</span>
        <small>°C</small>
      </div>

      <div class="metric-foot">
        Batas optimal: 22–30 °C
      </div>

    </div>


    <!-- HUMIDITY -->
    <div class="card metric">

      <div class="metric-top">
        <div class="metric-icon blue-icon">💧</div>
        <span id="humBadge" class="badge badge-ok">NORMAL</span>
      </div>

      <div class="metric-name">Kelembapan Udara</div>

      <div class="metric-value">
        <span id="humVal">--</span>
        <small>%</small>
      </div>

      <div class="metric-foot">
        Pembacaan DHT11
      </div>

    </div>

  </section>

  <!-- =======================================================
       DEVICE STATUS
  ======================================================== -->
  <div class="section-title" id="device">
    <h2>Status Perangkat</h2>
    <span>ESP32 + sensor system</span>
  </div>

  <section class="grid device-grid">

    <div class="card device-card">
      <div class="device-icon">📡</div>

      <div>
        <strong>ESP32</strong>
        <small><i class="dot"></i><span id="espStatus">Online</span></small>
      </div>
    </div>


    <div class="card device-card">
      <div class="device-icon">📶</div>

      <div>
        <strong>Wi-Fi</strong>
        <small id="wifiStatus">Terhubung</small>
      </div>
    </div>


    <div class="card device-card">
      <div class="device-icon">⏱</div>

      <div>
        <strong>Monitoring Uptime</strong>
        <small id="uptime">00:00:00</small>
      </div>
    </div>

  </section>


  <!-- =======================================================
       CONTROL
  ======================================================== -->
  <div class="section-title">
    <h2>Kontrol Sistem</h2>
    <span>Perintah dikirim langsung ke ESP32</span>
  </div>

  <section class="card control-card">

    <div class="control-left">

      <div class="control-icon">🔔</div>

      <div>
        <strong>Alarm & Sistem Pengusir Hama</strong>

        <small id="alarmDescription">
          Alarm aktif. Sistem akan memberi peringatan ketika
          objek berada pada jarak bahaya.
        </small>

      </div>

    </div>

    <div class="control-buttons">

      <button
        id="alarmButton"
        class="control-btn alarm-on"
        onclick="toggleAlarm()">
        🔊 Alarm ON
      </button>

      <button
        id="testButton"
        class="control-btn test-btn"
        onclick="triggerTest()">
        🛠 Tes Alat
      </button>

    </div>

  </section>


  <!-- =======================================================
       ACTIVITY LOG
  ======================================================== -->
  <div class="section-title" id="activity">
    <h2>Activity Log</h2>
    <span>Aktivitas terbaru sistem</span>
  </div>

  <section class="card log-card">

    <table class="log-table">

      <thead>
        <tr>
          <th>Waktu</th>
          <th>Aktivitas</th>
          <th>Status</th>
        </tr>
      </thead>

      <tbody id="logBody">

        <tr>
          <td>--:--:--</td>
          <td>Dashboard dimuat</td>
          <td>
            <span class="badge badge-ok">READY</span>
          </td>
        </tr>

      </tbody>

    </table>

  </section>


  <div class="footer">
    Smart Plant Guardian · ESP32 IoT Monitoring · Prodi TRK · Safina Rahmatus Sa'diyah · E43251922
  </div>

</main>
</div>


<!-- =========================================================
     TOAST NOTIFICATION
========================================================= -->
<div id="toast">
  <div class="toast-title" id="toastTitle">Notifikasi</div>
  <div class="toast-message" id="toastMessage"></div>
</div>


<script>

// ============================================================
// DATA HISTORY
// ============================================================

const historyData = {
  temp: [],
  humidity: [],
  distance: []
};

let chartType = "temp";

let lastPestState = "safe";
let lastTempState = "safe";
let lastHumState = "safe";

let alarmEnabled = true;
let startTime = Date.now();


// ============================================================
// UTILITIES
// ============================================================

function nowTime(){

  return new Date().toLocaleTimeString("id-ID");

}


function showToast(title, message, type){

  const toast = document.getElementById("toast");

  document.getElementById("toastTitle").innerText = title;
  document.getElementById("toastMessage").innerText = message;

  toast.className = "";
  toast.classList.add("show", type);

  clearTimeout(window.toastTimer);

  window.toastTimer = setTimeout(() => {
    toast.classList.remove("show");
  }, 4500);

}


function addLog(activity, status="NORMAL", type="ok"){

  const body = document.getElementById("logBody");

  const row = document.createElement("tr");

  row.innerHTML = `
    <td>${nowTime()}</td>
    <td>${activity}</td>
    <td>
      <span class="badge ${type === "danger" ? "badge-danger" :
                           type === "warning" ? "badge-warning" :
                           "badge-ok"}">
        ${status}
      </span>
    </td>
  `;

  body.prepend(row);

  while(body.children.length > 7){
    body.lastElementChild.remove();
  }

}


// ============================================================
// HEALTH SCORE
// ============================================================

function calculateHealth(temp, hum, dist){

  let score = 100;

  if(temp < 22 || temp > 30){
    score -= 20;
  }

  if(hum < 40 || hum > 70){
    score -= 10;
  }

  if(dist < 15){
    score -= 30;
  }
  else if(dist <= 40){
    score -= 12;
  }

  if(!alarmEnabled){
    score -= 5;
  }

  return Math.max(0, Math.min(100, score));

}


function updateHealth(temp, hum, dist){

  const score = calculateHealth(temp, hum, dist);

  document.getElementById("healthScore").innerText = score;

  let status = "Kondisi Baik";

  if(score < 60){
    status = "Perlu Perhatian";
  }
  else if(score < 80){
    status = "Kondisi Waspada";
  }

  document.getElementById("healthStatus").innerText = status;

  const ring = document.getElementById("healthRing");

  ring.style.background =
    `conic-gradient(#53dd90 0 ${score}%,
                    rgba(255,255,255,.12) ${score}% 100%)`;

  ring.style.setProperty("--score", score);

}


// ============================================================
// SENSOR STATUS
// ============================================================

function updateSensorUI(data){

  // ---------------- JARAK ----------------

  const distanceValue = data.dist > 990 ? "Out" : data.dist;

  document.getElementById("distVal").innerText = distanceValue;

  const distBadge = document.getElementById("distBadge");

  let pestState = "safe";

  if(data.dist < 15){

    pestState = "danger";

    distBadge.className = "badge badge-danger";
    distBadge.innerText = "BAHAYA";

    document.getElementById("distFoot").innerText =
      "Objek sangat dekat! Sistem pengusir aktif.";

  }
  else if(data.dist <= 40){

    pestState = "warning";

    distBadge.className = "badge badge-warning";
    distBadge.innerText = "WASPADA";

    document.getElementById("distFoot").innerText =
      "Ada objek pada area pemantauan.";

  }
  else{

    pestState = "safe";

    distBadge.className = "badge badge-ok";
    distBadge.innerText = "AMAN";

    document.getElementById("distFoot").innerText =
      "Tidak ada objek berbahaya.";

  }


  // ---------------- SUHU ----------------

  document.getElementById("tempVal").innerText =
    Number(data.temp).toFixed(1);

  const tempBadge = document.getElementById("tempBadge");

  let tempState = "safe";

  if(data.temp < 22 || data.temp > 30){

    tempState = "danger";

    tempBadge.className = "badge badge-danger";
    tempBadge.innerText = "EKSTREM";

  }
  else{

    tempState = "safe";

    tempBadge.className = "badge badge-ok";
    tempBadge.innerText = "NORMAL";

  }


  // ---------------- KELEMBAPAN ----------------

  document.getElementById("humVal").innerText =
    Number(data.hum).toFixed(1);

  const humBadge = document.getElementById("humBadge");

  let humState = "safe";

  if(data.hum < 40 || data.hum > 70){

    humState = "warning";

    humBadge.className = "badge badge-warning";
    humBadge.innerText = "WASPADA";

  }
  else{

    humState = "safe";

    humBadge.className = "badge badge-ok";
    humBadge.innerText = "NORMAL";

  }


  // ========================================================
  // REAL-TIME NOTIFICATIONS
  // ========================================================

  if(pestState === "danger" && lastPestState !== "danger"){

    showToast(
      "⚠️ BAHAYA HAMA",
      "Objek terdeteksi sangat dekat dengan tanaman. Sistem pengusir aktif.",
      "danger"
    );

    addLog(
      "Hama / objek berada pada jarak bahaya",
      "DANGER",
      "danger"
    );

  }
  else if(pestState === "warning" && lastPestState === "safe"){

    showToast(
      "👀 PERINGATAN",
      "Terdeteksi pergerakan di area dekat tanaman.",
      "warning"
    );

    addLog(
      "Objek terdeteksi pada area pemantauan",
      "WARNING",
      "warning"
    );

  }


  if(tempState === "danger" && lastTempState !== "danger"){

    showToast(
      "🌡️ PERINGATAN SUHU",
      "Suhu lingkungan berada di luar batas optimal.",
      "danger"
    );

    addLog(
      "Suhu lingkungan berada di luar batas optimal",
      "DANGER",
      "danger"
    );

  }


  if(humState === "warning" && lastHumState === "safe"){

    showToast(
      "💧 PERINGATAN KELEMBAPAN",
      "Kelembapan udara berada di luar rentang monitoring.",
      "warning"
    );

    addLog(
      "Kelembapan udara perlu diperhatikan",
      "WARNING",
      "warning"
    );

  }


  lastPestState = pestState;
  lastTempState = tempState;
  lastHumState = humState;

  // ---------------- HISTORY ----------------

  historyData.temp.push(Number(data.temp));
  historyData.humidity.push(Number(data.hum));
  historyData.distance.push(
    data.dist > 990 ? 100 : Number(data.dist)
  );

  if(historyData.temp.length > 25) historyData.temp.shift();
  if(historyData.humidity.length > 25) historyData.humidity.shift();
  if(historyData.distance.length > 25) historyData.distance.shift();

  updateHealth(
    Number(data.temp),
    Number(data.hum),
    Number(data.dist)
  );

  updateControls(data);

  document.getElementById("lastUpdate").innerText =
    "Terakhir diperbarui: " + nowTime();

}


// ============================================================
// CONTROL UI
// ============================================================

function updateControls(data){

  alarmEnabled = !data.muted;

  const button = document.getElementById("alarmButton");

  if(alarmEnabled){

    button.className = "control-btn alarm-on";
    button.innerText = "🔊 Alarm ON";

    document.getElementById("alarmDescription").innerText =
      "Alarm aktif. Sistem akan memberi peringatan ketika objek berada pada jarak bahaya.";

  }
  else{

    button.className = "control-btn alarm-off";
    button.innerText = "🔇 Alarm OFF";

    document.getElementById("alarmDescription").innerText =
      "Alarm dimatikan. Sensor tetap memantau kondisi tanaman.";

  }


  const testButton = document.getElementById("testButton");

  if(data.testMode){

    testButton.innerText = "🛠 Tes Berjalan...";
    testButton.style.background = "#df5050";

  }
  else{

    testButton.innerText = "🛠 Tes Alat";
    testButton.style.background = "";

  }

}


// ============================================================
// FETCH DATA
// ============================================================

function fetchData(){

  fetch("/data")

  .then(response => {

    if(!response.ok){
      throw new Error("HTTP " + response.status);
    }

    return response.json();

  })

  .then(data => {

    document.getElementById("espStatus").innerText = "Online";
    document.getElementById("wifiStatus").innerText =
      "Terhubung";

    if(data.ip){
      document.getElementById("sideIp").innerText =
        "IP: " + data.ip;
    }

    updateSensorUI(data);

  })

  .catch(error => {

    console.error(error);

    document.getElementById("espStatus").innerText =
      "Tidak terhubung";

    document.getElementById("wifiStatus").innerText =
      "Koneksi bermasalah";

  });

}


// ============================================================
// ALARM BUTTON
// ============================================================

function toggleAlarm(){

  fetch("/toggleMute")

  .then(() => {

    fetchData();

    setTimeout(() => {

      if(alarmEnabled){

        showToast(
          "🔊 Alarm Aktif",
          "Alarm dan sistem peringatan kembali aktif.",
          "success"
        );

        addLog("Alarm diaktifkan", "ON", "ok");

      }
      else{

        showToast(
          "🔇 Alarm Dimatikan",
          "Sensor tetap berjalan, tetapi buzzer dinonaktifkan.",
          "info"
        );

        addLog("Alarm dimatikan", "OFF", "warning");

      }

    }, 150);

  })

  .catch(error => {

    showToast(
      "❌ Gagal",
      "Tidak dapat mengubah status alarm.",
      "danger"
    );

    console.error(error);

  });

}


// ============================================================
// TEST ALAT
// ============================================================

function triggerTest(){

  showToast(
    "🛠 TEST ALAT",
    "Buzzer, LED, dan servo akan diuji selama beberapa detik.",
    "info"
  );

  addLog(
    "Test alat dijalankan",
    "TEST",
    "ok"
  );

  fetch("/triggerTest")

  .then(() => {

    fetchData();

  })

  .catch(error => {

    showToast(
      "❌ Test gagal",
      "Perintah test tidak dapat dikirim ke ESP32.",
      "danger"
    );

    console.error(error);

  });

}


// ============================================================
// REFRESH
// ============================================================

function refreshData(){

  showToast(
    "↻ Refresh",
    "Data sensor sedang diperbarui.",
    "info"
  );

  fetchData();

}


// ============================================================
// CHART
// ============================================================

function setChart(type, element){

  chartType = type;

  document.querySelectorAll(".tab").forEach(button => {
    button.classList.remove("active");
  });

  element.classList.add("active");

  drawChart();

}


function drawChart(){

  const canvas = document.getElementById("chart");
  const ctx = canvas.getContext("2d");

  const rect = canvas.getBoundingClientRect();

  const dpr = window.devicePixelRatio || 1;

  canvas.width = rect.width * dpr;
  canvas.height = 255 * dpr;

  ctx.setTransform(dpr,0,0,dpr,0,0);

  const width = rect.width;
  const height = 255;

  ctx.clearRect(0,0,width,height);

  let values = historyData[chartType];

  if(values.length < 2){

    ctx.fillStyle = "#87958e";
    ctx.font = "12px Segoe UI";

    ctx.fillText(
      "Menunggu data sensor...",
      width / 2 - 70,
      height / 2
    );

    return;

  }

  const padding = {
    left:42,
    right:14,
    top:15,
    bottom:30
  };

  const minValue = Math.min(...values);
  const maxValue = Math.max(...values);

  const extra = Math.max((maxValue - minValue) * .35, 1);

  const min = minValue - extra;
  const max = maxValue + extra;

  const chartWidth =
    width - padding.left - padding.right;

  const chartHeight =
    height - padding.top - padding.bottom;


  // GRID

  ctx.font = "10px Segoe UI";
  ctx.fillStyle = "#93a099";
  ctx.strokeStyle = "#e7edea";
  ctx.lineWidth = 1;

  for(let i=0;i<5;i++){

    const y =
      padding.top +
      chartHeight * i / 4;

    ctx.beginPath();
    ctx.moveTo(padding.left,y);
    ctx.lineTo(width-padding.right,y);
    ctx.stroke();

    const value =
      max - (max-min)*i/4;

    ctx.fillText(
      value.toFixed(chartType === "humidity" ? 0 : 1),
      4,
      y+4
    );

  }


  // LINE

  const points = values.map((value,index) => {

    const x =
      padding.left +
      chartWidth *
      index /
      (values.length-1);

    const y =
      padding.top +
      (max-value) /
      (max-min) *
      chartHeight;

    return {x,y};

  });


  ctx.beginPath();

  points.forEach((point,index) => {

    if(index === 0){
      ctx.moveTo(point.x,point.y);
    }
    else{
      ctx.lineTo(point.x,point.y);
    }

  });

  ctx.strokeStyle = "#20a463";
  ctx.lineWidth = 3;
  ctx.stroke();


  // AREA

  ctx.beginPath();

  points.forEach((point,index) => {

    if(index === 0){
      ctx.moveTo(point.x,point.y);
    }
    else{
      ctx.lineTo(point.x,point.y);
    }

  });

  ctx.lineTo(
    points[points.length-1].x,
    height-padding.bottom
  );

  ctx.lineTo(
    points[0].x,
    height-padding.bottom
  );

  ctx.closePath();

  ctx.globalAlpha = .07;
  ctx.fillStyle = "#20a463";
  ctx.fill();
  ctx.globalAlpha = 1;


  // POINTS

  points.forEach(point => {

    ctx.beginPath();
    ctx.arc(point.x,point.y,3.5,0,Math.PI*2);

    ctx.fillStyle = "#fff";
    ctx.fill();

    ctx.strokeStyle = "#20a463";
    ctx.lineWidth = 2;
    ctx.stroke();

  });

}


// ============================================================
// SIDEBAR
// ============================================================

function toggleMenu(){

  document
    .getElementById("sidebar")
    .classList.toggle("open");

}


// ============================================================
// UPTIME
// ============================================================

function updateUptime(){

  const seconds =
    Math.floor((Date.now()-startTime)/1000);

  const hours =
    String(Math.floor(seconds/3600)).padStart(2,"0");

  const minutes =
    String(Math.floor((seconds%3600)/60)).padStart(2,"0");

  const secs =
    String(seconds%60).padStart(2,"0");

  document.getElementById("uptime").innerText =
    `${hours}:${minutes}:${secs}`;

}


// ============================================================
// START
// ============================================================

window.addEventListener("resize", drawChart);

setInterval(fetchData, 1000);

setInterval(updateUptime, 1000);

fetchData();

drawChart();

</script>

</body>
</html>
)rawliteral";


// ============================================================
// FUNCTION DECLARATIONS
// ============================================================
long readUltrasonic();

void handleButton(unsigned long currentMillis);

void runMainLogic(unsigned long currentMillis);

void runTestMode(unsigned long currentMillis);

void updateOLED();

void setBuzzer(int freq);


// ============================================================
// BUZZER
// ============================================================

void setBuzzer(int freq) {

  // Jika alarm dimatikan, buzzer selalu OFF.
  if (!alarmEnabled) {
    freq = -1;
  }

  // Hanya ubah output jika frekuensi berubah.
  if (freq != currentBuzzerFreq) {

    currentBuzzerFreq = freq;

    if (freq > 0) {

      tone(BUZZER_PIN, freq);

    }
    else {

      noTone(BUZZER_PIN);
      digitalWrite(BUZZER_PIN, LOW);

    }

  }
}


// ============================================================
// WEB SERVER
// ============================================================

void handleRoot() {

  server.send(
    200,
    "text/html",
    index_html
  );

}


void handleData() {

  char jsonBuffer[320];

  String ipAddress = "0.0.0.0";

  if(WiFi.status() == WL_CONNECTED){
    ipAddress = WiFi.localIP().toString();
  }

  snprintf(
    jsonBuffer,
    sizeof(jsonBuffer),

    "{\"temp\":%.1f,"
    "\"hum\":%.1f,"
    "\"dist\":%ld,"
    "\"muted\":%s,"
    "\"testMode\":%s,"
    "\"ip\":\"%s\"}",

    temperature,
    humidity,
    distance,

    alarmEnabled ? "false" : "true",

    isTestMode ? "true" : "false",

    ipAddress.c_str()
  );

  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.send(
    200,
    "application/json",
    jsonBuffer
  );

}


// ============================================================
// TOGGLE ALARM
// ============================================================

void handleToggleMute() {

  alarmEnabled = !alarmEnabled;

  if(!alarmEnabled){

    setBuzzer(-1);

  }

  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.send(
    200,
    "text/plain",
    alarmEnabled ? "ALARM_ON" : "ALARM_OFF"
  );

}


// ============================================================
// TEST MODE
// ============================================================

void handleTriggerTest() {

  // Jika sedang test, perintah berikutnya menghentikan test.
  isTestMode = !isTestMode;

  if(isTestMode){

    testStartTime = millis();

  }
  else{

    setBuzzer(-1);

    myServo.write(0);

    digitalWrite(
      LED_PIN,
      LOW
    );

  }

  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.send(
    200,
    "text/plain",
    isTestMode ? "TEST_ON" : "TEST_OFF"
  );

}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);


  // ---------------- PIN ----------------

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);


  // ---------------- SENSOR ----------------

  dht.begin();


  // ---------------- SERVO ----------------

  myServo.attach(SERVO_PIN);

  myServo.write(0);


  // ---------------- I2C / OLED ----------------

  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

  if(!display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
  )){

    Serial.println(
      F("OLED gagal diinisialisasi!")
    );

    for(;;);

  }


  // ---------------- OLED WIFI SCREEN ----------------

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);

  display.setCursor(0,0);

  display.println(
    F("Smart Plant Guardian")
  );

  display.println();

  display.println(
    F("Menghubungkan WiFi...")
  );

  display.display();


  // ---------------- WIFI ----------------

  Serial.print(
    F("Menghubungkan ke ")
  );

  Serial.println(ssid);

  WiFi.begin(
    ssid,
    password
  );


  int attempt = 0;

  while(
    WiFi.status() != WL_CONNECTED &&
    attempt < 20
  ){

    delay(500);

    Serial.print(".");

    attempt++;

  }

  Serial.println();


  // ---------------- WIFI RESULT ----------------

  display.clearDisplay();

  display.setCursor(0,0);

  if(WiFi.status() == WL_CONNECTED){

    Serial.println(
      F("WiFi Terhubung!")
    );

    Serial.print(
      F("Alamat IP Web: ")
    );

    Serial.println(
      WiFi.localIP()
    );


    display.println(
      F("WiFi Terhubung!")
    );

    display.print(
      F("IP: ")
    );

    display.println(
      WiFi.localIP()
    );

  }
  else{

    Serial.println(
      F("Gagal Konek WiFi.")
    );

    display.println(
      F("WiFi gagal")
    );

    display.println(
      F("Mode Offline")
    );

  }

  display.display();

  delay(2500);


  // ---------------- ROUTING ----------------

  server.on(
    "/",
    handleRoot
  );

  server.on(
    "/data",
    handleData
  );

  server.on(
    "/toggleMute",
    handleToggleMute
  );

  server.on(
    "/triggerTest",
    handleTriggerTest
  );

  server.begin();

  Serial.println(
    F("Web Server Aktif.")
  );


  // ---------------- WELCOME OLED ----------------

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0,5);

  display.println(
    F("SMART PLANT")
  );

  display.println();

  display.println(
    F("GUARDIAN READY")
  );

  display.println();

  display.println(
    F("Monitoring aktif...")
  );

  display.display();

  delay(2000);

  display.clearDisplay();

}


// ============================================================
// LOOP
// ============================================================

void loop() {

  unsigned long currentMillis = millis();


  // 1. WEB SERVER

  server.handleClient();


  // 2. BUTTON

  handleButton(
    currentMillis
  );


  // 3. SENSOR

  if(
    currentMillis - prevSensorRead >= 500
  ){

    prevSensorRead = currentMillis;


    distance = readUltrasonic();


    float t =
      dht.readTemperature();

    float h =
      dht.readHumidity();


    if(!isnan(t)){
      temperature = t;
    }

    if(!isnan(h)){
      humidity = h;
    }

  }


  // 4. LOGIC

  if(isTestMode){

    runTestMode(
      currentMillis
    );

  }
  else{

    runMainLogic(
      currentMillis
    );

  }


  // 5. OLED

  updateOLED();


  // 6. SERIAL

  if(
    currentMillis - prevSerialPrint >= 2000
  ){

    prevSerialPrint = currentMillis;

    Serial.print(
      F("Jarak: ")
    );

    Serial.print(distance);

    Serial.print(
      F(" cm | Suhu: ")
    );

    Serial.print(temperature);

    Serial.print(
      F(" C | Hum: ")
    );

    Serial.print(humidity);

    Serial.print(
      F(" % | Alarm: ")
    );

    Serial.print(
      alarmEnabled ? "ON" : "OFF"
    );

    Serial.print(
      F(" | Test: ")
    );

    Serial.println(
      isTestMode ? "ON" : "OFF"
    );

  }

}


// ============================================================
// ULTRASONIC
// ============================================================

long readUltrasonic() {

  digitalWrite(
    TRIG_PIN,
    LOW
  );

  delayMicroseconds(5);

  digitalWrite(
    TRIG_PIN,
    HIGH
  );

  delayMicroseconds(10);

  digitalWrite(
    TRIG_PIN,
    LOW
  );


  long duration =
    pulseIn(
      ECHO_PIN,
      HIGH,
      50000
    );


  if(duration == 0){

    return 999;

  }


  long cm =
    duration * 0.034 / 2;


  return cm;

}


// ============================================================
// MAIN LOGIC
// ============================================================

void runMainLogic(
  unsigned long currentMillis
){

  bool isTempExtreme =
    (
      temperature < 22.0 ||
      temperature > 30.0
    );


  bool isPestDanger =
    distance < 15;


  bool isPestWarning =
    distance >= 15 &&
    distance <= 40;


  // ---------------- SERVO ----------------

  if(isPestDanger){

    if(
      currentMillis - prevServo >= 15
    ){

      prevServo = currentMillis;

      servoPos += servoDir;

      if(
        servoPos <= 0 ||
        servoPos >= 180
      ){

        servoDir =
          -servoDir;

      }

      myServo.write(
        servoPos
      );

    }

  }
  else{

    myServo.write(0);

    servoPos = 0;

  }


  // ---------------- LED ----------------

  if(isPestDanger){

    if(
      currentMillis - prevBlink >= 100
    ){

      prevBlink = currentMillis;

      ledState = !ledState;

      digitalWrite(
        LED_PIN,
        ledState
      );

    }

  }
  else if(isPestWarning){

    digitalWrite(
      LED_PIN,
      HIGH
    );

    ledState = HIGH;

  }
  else{

    digitalWrite(
      LED_PIN,
      LOW
    );

    ledState = LOW;

  }


  // ---------------- BUZZER ----------------

  if(!alarmEnabled){

    setBuzzer(-1);

  }
  else if(isPestDanger){

    if(ledState){

      setBuzzer(2000);

    }
    else{

      setBuzzer(-1);

    }

  }
  else if(isTempExtreme){

    setBuzzer(1000);

  }
  else{

    setBuzzer(-1);

  }

}


// ============================================================
// PHYSICAL BUTTON
//
// < 1 detik  = ganti tampilan OLED
// 1-3 detik   = ON/OFF alarm
// >= 3 detik  = ON/OFF test mode
// ============================================================

void handleButton(
  unsigned long currentMillis
){

  bool reading =
    digitalRead(BUTTON_PIN);


  if(
    reading == LOW &&
    lastBtnState == HIGH
  ){

    btnPressStartTime =
      currentMillis;

    isBtnPressed = true;

  }


  if(
    reading == HIGH &&
    lastBtnState == LOW &&
    isBtnPressed
  ){

    unsigned long pressDuration =
      currentMillis -
      btnPressStartTime;


    isBtnPressed = false;


    // TEST MODE
    if(pressDuration >= 3000){

      isTestMode =
        !isTestMode;


      if(isTestMode){

        testStartTime =
          currentMillis;

      }
      else{

        setBuzzer(-1);

        myServo.write(0);

        digitalWrite(
          LED_PIN,
          LOW
        );

      }

    }


    // ALARM
    else if(pressDuration >= 1000){

      alarmEnabled =
        !alarmEnabled;


      if(!alarmEnabled){

        setBuzzer(-1);

      }

    }


    // OLED PAGE
    else{

      oledMode =
        (oledMode == 0)
        ? 1
        : 0;

    }

  }


  lastBtnState =
    reading;

}


// ============================================================
// TEST MODE
// ============================================================

void runTestMode(
  unsigned long currentMillis
){

  // Test LED
  digitalWrite(
    LED_PIN,
    HIGH
  );


  // Test buzzer
  //
  // Test mode memang digunakan untuk
  // memastikan buzzer bekerja walaupun
  // alarm normal sedang OFF.
  if(currentBuzzerFreq != 1500){

    currentBuzzerFreq = 1500;

    tone(
      BUZZER_PIN,
      1500
    );

  }


  // Test servo
  if(
    currentMillis - prevServo >= 10
  ){

    prevServo =
      currentMillis;

    servoPos += servoDir;

    if(
      servoPos <= 0 ||
      servoPos >= 180
    ){

      servoDir =
        -servoDir;

    }

    myServo.write(
      servoPos
    );

  }


  // Auto stop 3 detik
  if(
    currentMillis - testStartTime >= 3000
  ){

    isTestMode = false;

    setBuzzer(-1);

    myServo.write(0);

    digitalWrite(
      LED_PIN,
      LOW
    );

  }

}


// ============================================================
// OLED
// ============================================================

void updateOLED() {

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);


  // TEST
  if(isTestMode){

    display.setCursor(
      15,
      5
    );

    display.println(
      F("SMART PLANT GUARDIAN")
    );

    display.drawLine(
      0,15,128,15,WHITE
    );

    display.setCursor(
      17,
      27
    );

    display.println(
      F("TEST MODE")
    );

    display.setCursor(
      8,
      43
    );

    display.println(
      F("Buzzer + Servo + LED")
    );

  }


  // OLED PAGE 0
  else if(oledMode == 0){

    display.setCursor(
      5,
      0
    );

    display.println(
      F("DETEKSI HAMA")
    );

    display.drawLine(
      0,10,128,10,WHITE
    );


    display.setCursor(
      0,
      18
    );

    display.print(
      F("Jarak : ")
    );


    if(distance >= 999){

      display.println(
        F("Out")
      );

    }
    else{

      display.print(
        distance
      );

      display.println(
        F(" cm")
      );

    }


    display.setCursor(
      0,
      34
    );


    if(distance < 15){

      display.println(
        F("Status: BAHAYA!")
      );

      display.setCursor(
        0,
        48
      );

      display.println(
        F("Usir Hama!")

      );

    }
    else if(distance <= 40){

      display.println(
        F("Status: WASPADA")
      );

      display.setCursor(
        0,
        48
      );

      display.println(
        F("Ada Objek")
      );

    }
    else{

      display.println(
        F("Status: AMAN")
      );

      display.setCursor(
        0,
        48
      );

      display.println(
        F("Tanaman Aman")
      );

    }

  }


  // OLED PAGE 1
  else{

    display.setCursor(
      2,
      0
    );

    display.println(
      F("MONITOR LINGKUNGAN")
    );

    display.drawLine(
      0,10,128,10,WHITE
    );


    display.setCursor(
      0,
      18
    );

    display.print(
      F("Suhu   : ")
    );

    display.print(
      temperature,
      1
    );

    display.println(
      F(" C")
    );


    display.setCursor(
      0,
      31
    );

    display.print(
      F("Lembab : ")
    );

    display.print(
      humidity,
      1
    );

    display.println(
      F(" %")
    );


    display.setCursor(
      0,
      48
    );

    if(
      temperature < 22.0 ||
      temperature > 30.0
    ){

      display.println(
        F("Ket: SUHU EKSTREM!")
      );

    }
    else{

      display.println(
        F("Ket: Suhu Optimal")
      );

    }

  }


  // ALARM INDICATOR

  display.setCursor(
    88,
    54
  );

  if(!alarmEnabled){

    display.print(
      F("OFF")
    );

  }
  else if(isTestMode){

    display.print(
      F("TEST")
    );

  }
  else{

    display.print(
      F("ON")
    );

  }


  display.display();

}
