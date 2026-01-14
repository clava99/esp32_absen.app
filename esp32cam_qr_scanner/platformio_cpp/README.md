# Panduan Menjalankan Proyek C++ dengan PlatformIO

Dokumen ini menjelaskan cara melakukan kompilasi dan upload kode absensi QR code ke ESP32-CAM menggunakan [PlatformIO](https://platformio.org/).

PlatformIO adalah toolkit profesional untuk pengembangan IoT yang terintegrasi baik dengan editor kode seperti Visual Studio Code. Pendekatan ini direkomendasikan karena manajemen *dependency* (pustaka) yang otomatis dan proses build yang andal.

## 1. Prasyarat
- **Visual Studio Code**: Editor kode gratis yang populer. [Download di sini](https://code.visualstudio.com/).
- **Ekstensi PlatformIO IDE**: Instal dari dalam VS Code. Buka panel ekstensi (`Ctrl+Shift+X`) dan cari "PlatformIO IDE".
- **Hardware**: Sama seperti setup MicroPython (ESP32-CAM dan USB-to-TTL converter).

## 2. Struktur Proyek
Proyek ini sudah dikonfigurasi untuk PlatformIO:
- `platformio.ini`: File konfigurasi utama. Di sinilah board, framework, dan pustaka didefinisikan. PlatformIO akan secara otomatis mengunduh `quirc` (untuk QR code) dan `ArduinoJson`.
- `src/main.cpp`: Kode sumber utama aplikasi.
- `lib/`: Direktori ini akan dibuat oleh PlatformIO untuk menyimpan pustaka yang diunduh.
- `.pio/`: Direktori ini berisi file build dan firmware yang sudah dikompilasi.

## 3. Langkah-langkah Setup dan Upload

### 1. Buka Proyek di VS Code
- Buka Visual Studio Code.
- Klik menu `File > Open Folder...` dan pilih direktori `esp32cam_qr_scanner`.
- VS Code, melalui ekstensi PlatformIO, akan otomatis mendeteksi ini sebagai proyek PlatformIO.

### 2. Konfigurasi Kode
- Buka file `src/main.cpp`.
- Ubah nilai konstanta berikut sesuai dengan pengaturan Anda:
  ```cpp
  const char* WIFI_SSID = "YOUR_WIFI_SSID";
  const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
  const char* SERVER_URL = "http://192.168.1.10:8000/api/attendance/";
  ```

### 3. Siapkan ESP32-CAM untuk Flashing
- **Putuskan semua sumber daya** dari ESP32-CAM.
- Hubungkan ESP32-CAM ke USB-to-TTL converter Anda:
  | ESP32-CAM | USB-to-TTL |
  |:----------|:-----------|
  | `GND`     | `GND`      |
  | `VCC`     | `5V`       |
  | `U0R`     | `TX`       |
  | `U0T`     | `RX`       |
  | `GND`     | `GPIO0`    | **(Hubungkan untuk mode flashing)** |
- Hubungkan converter ke port USB komputer Anda.

### 4. Kompilasi dan Upload
- Di bagian bawah jendela VS Code, Anda akan melihat toolbar PlatformIO.
- Klik ikon **centang (✓)** atau `PlatformIO: Build` dari Command Palette (`Ctrl+Shift+P`) untuk mengkompilasi proyek. Ini akan mengunduh pustaka yang diperlukan pada kali pertama.
- Setelah build berhasil, klik ikon **panah (→)** atau `PlatformIO: Upload` untuk memulai proses flashing ke ESP32-CAM. PlatformIO akan otomatis mendeteksi port serial. Jika gagal, Anda bisa menentukan port secara manual di `platformio.ini` dengan menambahkan baris `upload_port = /dev/ttyUSB0` (ganti dengan port Anda).

### 5. Monitor dan Debug
- Setelah upload selesai, **LEPASKAN KABEL JUMPER ANTARA `GPIO0` dan `GND`**.
- Tekan tombol `RST` pada board ESP32-CAM untuk menjalankan program.
- Klik ikon **colokan (🔌)** atau `PlatformIO: Serial Monitor` pada toolbar untuk melihat output `Serial.println()` dari ESP32-CAM. Ini akan menampilkan status koneksi WiFi, inisialisasi kamera, dan data QR code yang terdeteksi.

Jika semua berjalan lancar, ESP32-CAM akan mulai memindai QR code dan mengirim data ke server Anda setiap kali kode yang valid terdeteksi. LED flash akan menyala sesaat sebagai indikator pengiriman berhasil.
