# 🌱 Smart Plant Guardian

**Dibuat oleh:** Safina Rahmatus Sa'diyah. (E43251922) - Prodi: TRK

Sistem monitoring dan perlindungan tanaman pintar berbasis ESP32. Proyek ini memantau kondisi lingkungan (suhu dan kelembaban) serta mendeteksi pergerakan hama di sekitar tanaman. Dilengkapi dengan antarmuka web, alarm peringatan, pengusir hama mekanis, dan layar OLED.

---

## 📄 Catatan Versi Kode (.ino)

Proyek ini memiliki dua varian file kode program:

* 📄 **`Smart Plant Guardian.ino`**  
  Merupakan kode program utama yang berjalan secara *offline* (tanpa antarmuka web). Mengontrol seluruh sensor dan aktuator secara lokal melalui ESP32 dan layar OLED.
* 🌐 **`Smart Plant Guardian Website.ino`**  
  Merupakan kode program lengkap yang sudah mencakup *source code* Web Server (HTML, CSS, & JavaScript). Gunakan file ini agar sistem dapat terhubung ke jaringan WiFi dan menampilkan *dashboard* pemantauan interaktif di *website*.

---

## ✨ Fitur Utama

* 📊 **Real-time Web Dashboard**: Memantau data suhu, kelembaban, dan jarak secara langsung melalui perangkat *mobile* atau laptop.
* 🐛 **Sistem Pengusir Hama Otomatis**: Motor servo bergerak, LED berkedip, dan *buzzer* berbunyi ketika hama terdeteksi pada jarak kurang dari 15 cm.
* 🌡️ **Peringatan Suhu Ekstrem**: Peringatan visual dan suara jika suhu lingkungan berada di luar batas optimal (di bawah 22°C atau di atas 30°C).
* 📺 **Layar OLED Terintegrasi**: Menampilkan status langsung di perangkat tanpa harus membuka web.
* 🎛️ **Mode Kontrol Fleksibel**: Menyediakan fitur *Mute Alarm* dan *Test Mode* yang dapat dikendalikan baik dari *dashboard* web maupun tombol fisik.

---

## 🛠️ Komponen Perangkat Keras

* **Microcontroller**: ESP32
* **Sensor Suhu & Kelembaban**: DHT11
* **Sensor Jarak**: HC-SR04 (Ultrasonic)
* **Aktuator**: Motor Servo (SG90/MG996R)
* **Output**: Buzzer (Aktif/Pasif) & LED
* **Display**: OLED 0.96" I2C
* **Input**: Push Button

---

## 📌 Konfigurasi Pin (Pin Mapping)

| Komponen | Pin ESP32 | Keterangan |
| :--- | :---: | :--- |
| **OLED (I2C)** | 21 (SDA), 22 (SCL) | Menampilkan status lokal |
| **Ultrasonic** | 5 (TRIG), 18 (ECHO) | Sensor deteksi hama |
| **DHT11** | 3 | Sensor suhu & kelembaban |
| **Servo** | 13 | Penggerak mekanis pengusir hama |
| **Buzzer** | 25 | Output alarm audio |
| **LED** | 27 | Output alarm visual |
| **Push Button** | 32 | Kontrol Mute/Test/Ganti Layar (Pull-up) |

---

## 📚 Kebutuhan Library (Dependencies)

Pastikan kamu telah menginstal *library* berikut di Arduino IDE sebelum melakukan *compile*:
* `Adafruit GFX Library`
* `Adafruit SSD1306`
* `DHT sensor library` (oleh Adafruit)
* `ESP32Servo`

---

## 🚀 Cara Penggunaan

1. Buka file **`Smart Plant Guardian Website.ino`** di Arduino IDE jika ingin terhubung ke web.
2. Sesuaikan kredensial WiFi pada variabel berikut:
   ```cpp
   const char* ssid = "NAMA_WIFI_KAMU";       
   const char* password = "PASSWORD_WIFI_KAMU";
