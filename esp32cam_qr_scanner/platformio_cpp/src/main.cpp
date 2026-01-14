/*
 * ESP32-CAM QR Code Attendance Scanner for PlatformIO
 * Author: Gusti Permana Putra
 * Date: 2026-01-14
 * 
 * Deskripsi:
 * Script ini mengubah ESP32-CAM menjadi pemindai absensi berbasis QR code.
 * 1. Terhubung ke WiFi.
 * 2. Menginisialisasi kamera (model AI-Thinker).
 * 3. Menangkap gambar secara periodik.
 * 4. Menggunakan pustaka 'quirc' untuk mendeteksi dan mendekode QR code dari gambar.
 * 5. Jika QR code terdeteksi, mengirimkan datanya (diasumsikan ID Karyawan)
 *    ke server backend Django melalui HTTP POST request.
 * 6. Memberikan umpan balik visual melalui LED flash internal.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "quirc.h"
#include <ArduinoJson.h>

// --- Konfigurasi ---
// Ganti dengan kredensial WiFi Anda
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// Ganti dengan IP address server Django Anda
// Pastikan server Django berjalan di jaringan yang sama
const char* SERVER_URL = "http://192.168.1.10:8000/api/attendance/";

// Pin untuk LED Flash (GPIO 4)
#define FLASH_LED_PIN 4

// --- Pin Kamera (didefinisikan di platformio.ini) ---
#include "camera_pins.h"

// Pointer global untuk pustaka QR code
struct quirc* qr = NULL;

// Fungsi untuk menyalakan/mematikan flash
void setFlash(bool on) {
    digitalWrite(FLASH_LED_PIN, on ? HIGH : LOW);
}

// Inisialisasi WiFi
void initWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// Inisialisasi Kamera
void initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE; // Grayscale lebih cepat untuk QR code
    config.frame_size = FRAMESIZE_VGA;       // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        ESP.restart();
    } else {
        Serial.println("Camera initialized successfully.");
    }
}

// Inisialisasi QR code decoder
void initQR() {
    qr = quirc_new();
    if (!qr) {
        Serial.println("Failed to allocate quirc struct");
        ESP.restart();
    }
}

// Kirim data absensi ke server
void sendAttendance(String employeeId) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(SERVER_URL);
        http.addHeader("Content-Type", "application/json");

        // Buat JSON payload
        StaticJsonDocument<128> doc;
        doc["employee_id"] = employeeId;
        String payload;
        serializeJson(doc, payload);

        Serial.printf("Sending payload: %s\n", payload.c_str());

        int httpCode = http.POST(payload);

        if (httpCode > 0) {
            String response = http.getString();
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);
            Serial.printf("[HTTP] response: %s\n", response.c_str());
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                setFlash(true); // Nyalakan flash sebagai tanda sukses
                delay(500);
                setFlash(false);
            }
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
            // Beri sinyal error (kedip cepat)
            for (int i=0; i<3; i++) {
                setFlash(true); delay(100); setFlash(false); delay(100);
            }
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected. Cannot send data.");
    }
}

// Fungsi utama untuk memindai QR code
void scanQRCode() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    // Resize buffer quirc jika perlu
    if (quirc_resize(qr, fb->width, fb->height) < 0) {
        Serial.println("Failed to resize quirc");
        esp_camera_fb_return(fb);
        return;
    }

    // Dapatkan pointer ke buffer gambar dan mulai proses
    uint8_t* qr_image = quirc_begin(qr, NULL, NULL);
    memcpy(qr_image, fb->buf, fb->len);
    quirc_end(qr);
    
    int count = quirc_count(qr);
    if (count > 0) {
        Serial.printf("Found %d QR codes\n", count);
        struct quirc_code code;
        struct quirc_data data;

        // Ambil data dari QR code pertama yang ditemukan
        quirc_extract(qr, 0, &code);
        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (err) {
            Serial.printf("QR decode failed: %s\n", quirc_strerror(err));
        } else {
            String qrData = (const char*)data.payload;
            Serial.printf("QR Data: %s\n", qrData.c_str());
            sendAttendance(qrData);
            delay(3000); // Jeda 3 detik setelah berhasil mengirim
        }
    }

    esp_camera_fb_return(fb); // Penting! Kembalikan buffer frame
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nESP32-CAM QR Attendance Scanner");

    pinMode(FLASH_LED_PIN, OUTPUT);
    setFlash(false);

    initWiFi();
    initCamera();
    initQR();
}

void loop() {
    scanQRCode();
    delay(200); // Jeda singkat antar-scan
}
