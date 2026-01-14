# Panduan Menjalankan Proyek C++ dengan Arduino IDE

Dokumen ini menjelaskan cara melakukan kompilasi dan upload kode absensi QR code ke ESP32-CAM menggunakan Arduino IDE. Ini adalah metode yang baik untuk pemula atau untuk proyek yang lebih sederhana.

## 1. Prasyarat
- **Arduino IDE**: Versi 1.8.19 atau lebih baru, atau Arduino IDE 2.0+. [Download di sini](https://www.arduino.cc/en/software).
- **Hardware**: ESP32-CAM (AI-Thinker) dan USB-to-TTL converter.

## 2. Setup Awal Arduino IDE

### Langkah 2.1: Tambahkan URL Board ESP32
- Buka Arduino IDE.
- Buka `File > Preferences`.
- Di field "Additional Boards Manager URLs", tambahkan URL berikut:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- Klik "OK".

### Langkah 2.2: Instal Paket Board ESP32
- Buka `Tools > Board > Boards Manager...`.
- Cari "esp32".
- Pilih "esp32 by Espressif Systems" dan klik "Install". Tunggu hingga proses instalasi selesai.

### Langkah 2.3: Instal Pustaka yang Dibutuhkan
- Buka `Sketch > Include Library > Manage Libraries...`.
- Cari dan instal pustaka berikut satu per satu:
  1.  **"ArduinoJson"** oleh Benoit Blanchon.
  2.  **"quirc"** oleh dlbeer (atau varian lain yang mungkin muncul dari pencarian). Pastikan pustaka tersebut adalah wrapper untuk C/C++ library `quirc`. Jika tidak ditemukan, Anda mungkin perlu menginstalnya secara manual (lihat catatan di bawah).

**Catatan tentang Pustaka `quirc`:**
Pustaka `quirc` mungkin tidak selalu tersedia di Library Manager. Jika tidak ada, Anda bisa menambahkannya secara manual:
1.  Download source code-nya dari [GitHub: `dlbeer/quirc`](https://github.com/dlbeer/quirc).
2.  Unzip file tersebut.
3.  Rename folder yang diekstrak menjadi `quirc`.
4.  Copy folder `quirc` tersebut ke dalam direktori `libraries` di dalam folder sketchbook Arduino Anda (biasanya di `Documents/Arduino/libraries`).

## 3. Langkah-langkah Setup dan Upload

### 1. Buka Sketch
- Buka file `arduino_ide_cpp/arduino_ide_cpp.ino` dengan Arduino IDE.

### 2. Konfigurasi Kode
- Di dalam kode, ubah nilai konstanta berikut sesuai dengan pengaturan Anda:
  ```cpp
  const char* WIFI_SSID = "YOUR_WIFI_SSID";
  const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
  const char* SERVER_URL = "http://192.168.1.10:8000/api/attendance/";
  ```

### 3. Konfigurasi Board
- Buka menu `Tools`.
- Atur konfigurasi berikut:
  - **Board**: "ESP32 Wrover Module" (atau yang sejenis)
  - **Sub-menu Board**: "AI Thinker ESP32-CAM"
  - **Partition Scheme**: "Huge APP (3MB No OTA/1MB SPIFFS)"
  - **Port**: Pilih port serial dari USB-to-TTL converter Anda (misal, `COM3` atau `/dev/ttyUSB0`).

### 4. Siapkan ESP32-CAM untuk Flashing
- Hubungkan `GPIO0` ke `GND` pada ESP32-CAM untuk masuk ke mode flashing.
- Pastikan koneksi lainnya (5V, GND, TX, RX) sudah benar.
- Tekan tombol `RST` pada board sesaat sebelum memulai upload jika diperlukan.

### 5. Kompilasi dan Upload
- Klik tombol **Verify** (ikon centang) untuk mengkompilasi sketch.
- Klik tombol **Upload** (ikon panah) untuk meng-flash kode ke ESP32-CAM.

### 6. Monitor
- Setelah upload berhasil, **LEPASKAN KABEL JUMPER ANTARA `GPIO0` dan `GND`**.
- Tekan tombol `RST` pada board.
- Buka `Tools > Serial Monitor`.
- Atur baud rate ke **115200**.
- Anda akan melihat output dari ESP32-CAM, termasuk status koneksi dan data QR code yang terdeteksi.
