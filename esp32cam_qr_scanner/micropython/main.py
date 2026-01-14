# main.py for ESP32-CAM (MicroPython)
# Author: Gusti Permana Putra
# Date: 2026-01-14

# IMPORTANT: Standard MicroPython firmware for ESP32 does not include a QR code
# decoding library. This code provides the structure for capturing an image and
# sending data, but the `scan_qr_code_from_image` function is a placeholder.
# To make this work, you would need to:
# 1. Find/build a custom MicroPython firmware with a QR library included.
# 2. Or, send the captured image to an online API for QR decoding.

import camera
import time
import network
import urequests
import ujson

# --- Configuration ---
# WiFi Credentials - Ganti dengan kredensial Anda
WIFI_SSID = "Uban"
WIFI_PASS = "cox12379"

# Django Server URL - Ganti dengan IP address server Django Anda
# Pastikan server Django Anda berjalan di network yang sama.
# Gunakan `ip a` atau `ifconfig` di Linux/macOS, atau `ipconfig` di Windows
# untuk menemukan IP address komputer Anda.
SERVER_URL = "http://192.168.68.140:8000/api/attendance/"

# --- Pin Definition for AI-THINKER ESP32-CAM ---
PIN_PWDN = -1
PIN_RESET = -1
PIN_XCLK = 0
PIN_SIOD = 21
PIN_SIOC = 22
PIN_D7 = 36
PIN_D6 = 37
PIN_D5 = 38
PIN_D4 = 39
PIN_D3 = 35
PIN_D2 = 14
PIN_D1 = 13
PIN_D0 = 34
PIN_VSYNC = 25
PIN_HREF = 23
PIN_PCLK = 26

def connect_wifi():
    """Menyambungkan ESP32 ke jaringan WiFi."""
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        print(f"Connecting to network: {WIFI_SSID}...")
        sta_if.active(True)
        sta_if.connect(WIFI_SSID, WIFI_PASS)
        # Tunggu sampai koneksi berhasil
        for _ in range(10): # Timeout 10 detik
            if sta_if.isconnected():
                break
            time.sleep(1)
    
    if sta_if.isconnected():
        print("Network connected! IP info:", sta_if.ifconfig())
        return True
    else:
        print("Failed to connect to WiFi.")
        return False

def init_camera():
    """Menginisialisasi modul kamera."""
    print("Initializing camera...")
    try:
        camera.init(0, format=camera.JPEG, fb_location=camera.PSRAM,
                    xclk_freq=camera.XCLK_10MHz,
                    d0=PIN_D0, d1=PIN_D1, d2=PIN_D2, d3=PIN_D3, d4=PIN_D4, 
                    d5=PIN_D5, d6=PIN_D6, d7=PIN_D7,
                    href=PIN_HREF, vsync=PIN_VSYNC, pwdn=PIN_PWDN, 
                    reset=PIN_RESET, sioc=PIN_SIOC, siod=PIN_SIOD, 
                    xclk=PIN_XCLK, pclk=PIN_PCLK)
        
        # Pengaturan kamera
        camera.framesize(camera.FRAME_VGA)  # 640x480 - Resolusi yang baik untuk QR code
        camera.quality(12) # 0-63, semakin rendah semakin baik kualitasnya
        camera.speffect(camera.EFFECT_NONE)
        camera.whitebalance(camera.WB_AUTO)
        camera.saturation(0)
        camera.brightness(0)
        camera.contrast(0)

        print("Camera initialized successfully.")
        return True
    except Exception as e:
        print(f"Error initializing camera: {e}")
        # De-initialize on failure
        camera.deinit()
        return False

def scan_qr_code_from_image(img_buffer):
    """
    Placeholder untuk memindai QR code dari buffer gambar.
    Fungsi ini perlu diganti dengan implementasi nyata.
    """
    print("--- QR SCANNING PLACEHOLDER ---")
    # Logika di sini akan diganti. Contoh: mengirim `img_buffer` ke API.
    # Untuk tujuan demonstrasi, kita akan mengembalikan ID statis.
    time.sleep(1)
    # Simulasi token QR. Di sistem nyata, ini adalah string acak dari QR code.
    qr_token = "TOKEN-SIMULASI-123"
    print(f"Simulated QR Code found: {qr_token}")
    return qr_token

def send_attendance_to_server(qr_token):
    """Mengirim data absensi ke server Django."""
    headers = {'Content-Type': 'application/json'}
    # Backend Django (views.py) mengharapkan key 'qr_token', bukan 'employee_id'
    payload = {'qr_token': qr_token}
    
    print(f"Sending attendance token '{qr_token}' to {SERVER_URL}...")
    
    try:
        response = urequests.post(SERVER_URL, data=ujson.dumps(payload), headers=headers)
        
        print(f"Server response Status: {response.status_code}")
        print("Server response Body:", response.text)
        
        response.close()
        
        if response.status_code == 201 or response.status_code == 200:
            print("Attendance marked successfully on server.")
            return True
        else:
            print("Server returned an error.")
            return False

    except Exception as e:
        print(f"Failed to send request: {e}")
        return False

def main_loop():
    """Loop utama program."""
    print("\n--- ESP32-CAM QR Code Attendance ---")
    print("System ready. Point a QR code at the camera.")
    
    while True:
        try:
            print("\nCapturing image...")
            # Ambil gambar dari kamera
            img = camera.capture()
            
            if img:
                print("Image captured. Scanning for QR code...")
                # Pindai QR code dari gambar
                qr_token = scan_qr_code_from_image(img)
                
                if qr_token:
                    # Kirim data jika QR code ditemukan
                    send_attendance_to_server(qr_token)
                    print("Waiting for 5 seconds before next scan...")
                    time.sleep(5) # Jeda agar tidak spam server
                else:
                    print("No QR code found. Retrying in 2 seconds...")
                    time.sleep(2)
            else:
                print("Failed to capture image. Retrying in 2 seconds...")
                time.sleep(2)

        except Exception as e:
            print(f"An error occurred in the main loop: {e}")
            # Jika terjadi error, coba restart kamera
            init_camera() 
            time.sleep(5)


if __name__ == "__main__":
    # Inisialisasi sistem
    if connect_wifi():
        if init_camera():
            # Jalankan loop utama
            main_loop()
    
    print("System setup failed. Halting.")
