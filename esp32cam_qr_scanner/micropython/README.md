# Panduan Menjalankan MicroPython pada ESP32-CAM

Dokumen ini menjelaskan cara melakukan setup MicroPython pada board ESP32-CAM (model AI-Thinker) dan menjalankan script `main.py` untuk aplikasi absensi QR code.

**PENTING: Keterbatasan MicroPython**
Firmware standar MicroPython untuk ESP32 **tidak memiliki pustaka bawaan untuk mendekode QR code**. Script `main.py` yang disediakan hanya sebuah kerangka. Fungsi `scan_qr_code_from_image()` adalah *placeholder* yang menyimulasikan penemuan QR code. Untuk implementasi nyata, Anda perlu:
1.  Mencari atau membangun firmware MicroPython kustom yang menyertakan pustaka QR code (misalnya, `zbar` atau `quirc`). Ini adalah proses yang kompleks.
2.  Mengubah script untuk mengirim gambar yang ditangkap ke sebuah API endpoint online yang bisa mendekode QR code, lalu menerima hasilnya.

Opsi C++ yang disediakan di folder lain adalah pendekatan yang lebih langsung untuk pemindaian QR code di perangkat.

## Langkah-langkah Setup

### 1. Prasyarat Hardware
- ESP32-CAM (AI-Thinker).
- USB-to-TTL Serial Converter (seperti FTDI FT232RL atau CP2102).
- Kabel jumper.
- Komputer dengan Python terpasang.

### 2. Koneksi Kabel untuk Flashing
Hubungkan ESP32-CAM ke USB-to-TTL converter Anda sebagai berikut:

| ESP32-CAM | USB-to-TTL | Catatan                               |
|:----------|:-----------|:--------------------------------------|
| `GND`     | `GND`      | Ground                                |
| `VCC`     | `5V`       | Power supply (gunakan 5V)             |
| `U0R`     | `TX`       | Menerima data dari converter          |
| `U0T`     | `RX`       | Mengirim data ke converter            |
| `GND`     | `GPIO0`    | **PENTING: Hubungkan ke GND untuk mode flashing** |

### 3. Instal `esptool`
`esptool` adalah utilitas untuk berkomunikasi dengan bootloader ESP32. Instal melalui `pip`:
```bash
pip install esptool
```

### 4. Download Firmware MicroPython
Download firmware MicroPython terbaru untuk ESP32 dari situs resminya:
[https://micropython.org/download/ESP32_GENERIC/](https://micropython.org/download/ESP32_GENERIC/)

Pilih file `.bin` yang stabil.

### 5. Hapus Flash (Opsional, tapi Direkomendasikan)
Hubungkan converter ke USB komputer. Temukan port serial yang digunakan (misalnya `COM3` di Windows, `/dev/ttyUSB0` di Linux). Hapus memori flash untuk memastikan instalasi bersih:
```bash
# Ganti /dev/ttyUSB0 dengan port Anda
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
```

### 6. Flash Firmware MicroPython
Flash firmware yang sudah di-download:
```bash
# Ganti /dev/ttyUSB0 dengan port Anda dan .bin dengan nama file firmware
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 NAMA_FILE_FIRMWARE.bin
```

Setelah selesai, **lepaskan jumper antara `GPIO0` dan `GND`**, lalu tekan tombol `RST` pada board ESP32-CAM untuk me-reboot ke mode normal.

### 7. Instal `rshell` atau `ampy`
Untuk meng-upload file `main.py` ke ESP32, Anda memerlukan alat seperti `rshell` (direkomendasikan) atau `ampy`.
```bash
pip install rshell
```

### 8. Konfigurasi dan Upload Script
1.  **Edit `main.py`**: Buka file `main.py` dan ubah nilai `WIFI_SSID`, `WIFI_PASS`, dan `SERVER_URL` sesuai dengan konfigurasi jaringan dan server Anda.
2.  **Gunakan `rshell` untuk upload**:
    - Buka terminal dan jalankan `rshell`:
      ```bash
      # Ganti /dev/ttyUSB0 dengan port Anda
      rshell --buffer-size=512 -p /dev/ttyUSB0
      ```
    - Anda akan masuk ke shell `rshell`.
    - Navigasi ke direktori `micropython` di komputer lokal Anda menggunakan `cd`.
    - Salin `main.py` ke board (yang di-mount sebagai `/pyboard`):
      ```
      > cp main.py /pyboard/main.py
      ```
    - (Opsional) Masuk ke REPL untuk melihat output:
      ```
      > repl
      ```
      Tekan `Ctrl+X` untuk keluar dari REPL.
3.  **Reboot ESP32**: Tekan tombol `RST` pada board. Script `main.py` akan berjalan secara otomatis saat boot. Anda bisa memonitor outputnya melalui `rshell` atau terminal serial lainnya.
