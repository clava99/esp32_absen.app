/*
 * ESP32-CAM Hybrid QR Scanner & Video Streamer
 * Author: Gusti Permana Putra
 * Date: 2026-01-14
 *
 * Deskripsi:
 * Script ini menggabungkan dua fungsionalitas:
 * 1. Web server untuk video streaming (monitoring via browser).
 * 2. Pemindai QR code yang berjalan di background.
 *
 * FITUR:
 * - Buka IP address ESP32 di browser untuk melihat video stream.
 * - Halaman web juga menampilkan status QR code terakhir yang dipindai dan respons server.
 * - Pemindaian QR code berjalan terus-menerus di Core 1.
 * - Saat QR code terdeteksi, data dikirim ke server Django.
 * - TIDAK menggunakan OpenCV karena tidak praktis untuk ESP32. Menggunakan pustaka ESP32QRCodeReader yang efisien.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32QRCodeReader.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"

// --- Konfigurasi ---
const char* WIFI_SSID = "Uban";
const char* WIFI_PASS = "cox12379";
const char* SERVER_URL = "http://192.168.68.140:8000/api/attendance/";

// --- Global Variables ---
// Variabel untuk menyimpan status terakhir (untuk ditampilkan di web)
char last_qr_result[128] = "None";
char last_server_response[128] = "N/A";
// Mutex untuk melindungi akses ke variabel global dari task yang berbeda
SemaphoreHandle_t status_mutex;

// --- Objek & Handler ---
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
httpd_handle_t stream_httpd = NULL;

// --- HTML & JavaScript untuk Halaman Web ---
const char* HTML_CONTENT = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM Monitoring</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin: 0; }
        .container { padding: 10px; }
        h1 { background-color: #0078d4; color: white; padding: 10px 0; margin: 0; }
        h2 { color: #333; }
        img { max-width: 100%; height: auto; border: 2px solid #ddd; border-radius: 5px; }
        .status-box { border: 1px solid #ccc; padding: 10px; margin: 20px auto; max-width: 600px; background-color: #f9f9f9; border-radius: 5px; }
        .status-item { margin: 5px 0; }
        .status-label { font-weight: bold; }
    </style>
</head>
<body>
    <h1>ESP32-CAM QR Attendance</h1>
    <div class="container">
        <h2>Live Camera Feed</h2>
        <img src="/stream" width="640" height="480">
        <div class="status-box">
            <h2>Scan Status</h2>
            <div class="status-item"><span class="status-label">Last QR Code: </span><span id="qr_result">Loading...</span></div>
            <div class="status-item"><span class="status-label">Server Response: </span><span id="server_response">Loading...</span></div>
        </div>
    </div>
    <script>
        setInterval(function() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('qr_result').innerText = data.last_qr_result;
                    document.getElementById('server_response').innerText = data.last_server_response;
                })
                .catch(error => console.error('Error fetching status:', error));
        }, 2000); // Update setiap 2 detik
    </script>
</body>
</html>
)rawliteral";

// --- Fungsi-fungsi ---

// Fungsi untuk mengirim data absensi ke server
void sendAttendanceData(const char* qr_token) {
    if (WiFi.status() != WL_CONNECTED) {
        xSemaphoreTake(status_mutex, portMAX_DELAY);
        strcpy(last_server_response, "WiFi not connected");
        xSemaphoreGive(status_mutex);
        return;
    }

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<128> jsonDoc;
    jsonDoc["qr_token"] = qr_token;
    String jsonPayload;
    serializeJson(jsonDoc, jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);
    String response_body = http.getString();

    xSemaphoreTake(status_mutex, portMAX_DELAY);
    if (httpResponseCode > 0) {
        snprintf(last_server_response, sizeof(last_server_response), "HTTP %d: %s", httpResponseCode, response_body.c_str());
    } else {
        snprintf(last_server_response, sizeof(last_server_response), "Error: %s", http.errorToString(httpResponseCode).c_str());
    }
    xSemaphoreGive(status_mutex);
    
    http.end();
}

// Task yang berjalan di background untuk memindai QR code
void qrCodeTask(void *pvParameters) {
    struct QRCodeData qrCodeData;
    Serial.println("QR Code scanning task started on Core 1.");

    while (true) {
        if (reader.receiveQrCode(&qrCodeData, 100)) {
            if (qrCodeData.valid) {
                Serial.printf("QR Code found: %s\n", (const char *)qrCodeData.payload);
                
                xSemaphoreTake(status_mutex, portMAX_DELAY);
                strncpy(last_qr_result, (const char*)qrCodeData.payload, sizeof(last_qr_result) - 1);
                xSemaphoreGive(status_mutex);
                
                sendAttendanceData((const char *)qrCodeData.payload);
                
                vTaskDelay(5000 / portTICK_PERIOD_MS); // Tunggu setelah scan berhasil
            }
        }
        vTaskDelay(200 / portTICK_PERIOD_MS); // Jeda singkat
    }
}

// --- Web Server Handlers ---

// Handler untuk video stream
esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char part_buf[128];
    
    res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=--FRAME");
    if(res != ESP_OK){
        return res;
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        
        // Periksa apakah formatnya sudah JPEG. Jika tidak, konversi.
        // Ini penting karena QR scanner mungkin menggunakan format Grayscale.
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if(!jpeg_converted){
                esp_camera_fb_return(fb); // Kembalikan buffer asli
                Serial.println("JPEG compression failed");
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
        
        // Kirim frame jika konversi berhasil
        if(res == ESP_OK){
            size_t hlen = snprintf(part_buf, 128, "--FRAME\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
            res = httpd_resp_send_chunk(req, part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char*)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, "\r\n", 2);
        }
        
        // Jika buffer baru dialokasikan untuk konversi JPEG, bebaskan.
        if(fb->format != PIXFORMAT_JPEG && _jpg_buf != NULL){
            free(_jpg_buf);
        }
        
        // Kembalikan frame buffer asli
        esp_camera_fb_return(fb);
        
        if(res != ESP_OK){
            break;
        }
        vTaskDelay(66 / portTICK_PERIOD_MS); // ~15 fps
    }
    return res;
}

// Handler untuk halaman utama (HTML)
esp_err_t root_handler(httpd_req_t *req){
    return httpd_resp_send(req, HTML_CONTENT, HTTPD_RESP_USE_STRLEN);
}

// Handler untuk endpoint /status (JSON)
esp_err_t status_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "application/json");
    StaticJsonDocument<256> doc;
    
    xSemaphoreTake(status_mutex, portMAX_DELAY);
    doc["last_qr_result"] = last_qr_result;
    doc["last_server_response"] = last_server_response;
    xSemaphoreGive(status_mutex);

    String output;
    serializeJson(doc, output);
    return httpd_resp_send(req, output.c_str(), HTTPD_RESP_USE_STRLEN);
}

// Fungsi untuk memulai web server
void startCameraServer(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t root_uri = { .uri = "/", .method = HTTP_GET, .handler = root_handler };
    httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler };
    httpd_uri_t status_uri = { .uri = "/status", .method = HTTP_GET, .handler = status_handler };

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &root_uri);
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        httpd_register_uri_handler(stream_httpd, &status_uri);
    }
}

// --- Setup & Loop ---

void setup() {
    Serial.begin(115200);
    Serial.println("\nESP32-CAM Hybrid QR Scanner & Streamer");

    // Buat mutex
    status_mutex = xSemaphoreCreateMutex();

    // Inisialisasi kamera (dikelola oleh pustaka QR reader)
    reader.setup();
    Serial.println("Camera setup complete.");

    // Hubungkan ke WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: http://");
    Serial.println(WiFi.localIP());

    // Mulai server kamera (di Core 0)
    startCameraServer();
    Serial.println("HTTP server started on port 80.");

    // Mulai QR reader di Core 1
    reader.beginOnCore(1);
    
    // Buat task untuk memindai QR code di Core 1
    xTaskCreatePinnedToCore(
        qrCodeTask, "qrCodeTask", 4096, NULL, 1, NULL, 1
    );

    Serial.println("System ready. Open browser to monitor.");
}

void loop() {
    // Biarkan loop utama kosong.
    vTaskDelay(portMAX_DELAY);
}