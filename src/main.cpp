#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "img_converters.h"
#include "quirc.h"

// --- KONFIGURASI WIFI ---
const char* ssid = "Uban";
const char* password = "cox12379";

// --- KONFIGURASI SERVER ---
String serverUrl = "http://192.168.68.103:8000/attendance/api/scan/"; 

// --- CAMERA PINS (AI THINKER) ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WebServer server(80);

// --- GLOBAL VARIABLES ---
struct QRCodeResult {
    String payload;
    bool valid;
};
unsigned long lastScanTime = 0;
const unsigned long scanInterval = 200; // Scan every 200ms
bool flashState = false;

// --- FUNCTION DECLARATIONS ---
void startCamera();
void handleStream();
void handleIndex();

// --- QR DECODING ---
QRCodeResult decodeQR(camera_fb_t *fb) {
    QRCodeResult result = {"", false};
    
    struct quirc *q = quirc_new();
    if (q == NULL) return result;

    if (quirc_resize(q, fb->width, fb->height) < 0) {
        quirc_destroy(q);
        return result;
    }

    uint8_t *image = quirc_begin(q, NULL, NULL);
    memcpy(image, fb->buf, fb->len);
    quirc_end(q);

    int count = quirc_count(q);
    if (count > 0) {
        struct quirc_code code;
        struct quirc_data data;
        quirc_extract(q, 0, &code);
        quirc_decode_error_t err = quirc_decode(&code, &data);

        if (!err) {
             result.valid = true;
             result.payload = (const char *)data.payload;
        }
    }

    quirc_destroy(q);
    return result;
}

// --- HTTP REQUEST ---
void sendQRToServer(String token) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        Serial.println("Sending QR: " + token);
        
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        
        String jsonPayload = "{\"qr_token\": \"" + token + "\"}";
        int httpResponseCode = http.POST(jsonPayload);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response: " + response);
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);
    
    // Connect WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");

    startCamera();

    // Setup Web Server
    server.on("/", HTTP_GET, handleIndex);
    server.on("/stream", HTTP_GET, handleStream);
    server.begin();
}

void loop() {
    server.handleClient();
}

// --- CAMERA INIT ---
void startCamera() {
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
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE; // MUST BE GRAYSCALE FOR QR!
    config.frame_size = FRAMESIZE_QVGA; // 320x240 for performance
    config.jpeg_quality = 12;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
}

// --- STREAM HANDLER --
void handleStream() {
    WiFiClient client = server.client();
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);

    while (client.connected()) {
        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            continue;
        }

        // --- STREAMING (MJPEG) ---
        // Note: For streaming we need JPEG, but for QR we need Bitmap/Grayscale.
        // If we configure PIXFORMAT_GRAYSCALE, we cannot stream valid JPEGs directly without encoding.
        // esp_camera can capture JPEG, but quirc needs raw brightness values.
        // It is expensive to decode JPEG to RAW on ESP32.
        // BETTER APPROACH: Capture in GRAYSCALE.
        // To stream grayscale, we must wrap it in a pseudo-image header or send raw? 
        // Browsers expect JPEG in mjpeg stream usually.
        // Changing strategy: Capture JPEG. Use library to decode JPEG? No, quirc needs raw.
        // Dual mode is hard on single core.
        // Re-evaluating: Capture PIXFORMAT_GRAYSCALE.
        // To stream: We can't stream raw bytes as MJPEG.
        // We have to capture PIXFORMAT_JPEG for streaming.
        // And we cannot decode QR from JPEG easily.
        // SOLUTION: Switch formats? No, slow.
        // SOLUTION: Use PIXFORMAT_GRAYSCALE and use 'fmt2jpg' helper to convert to JPEG for the stream.
        
        uint8_t * jpg_buf = NULL;
        size_t jpg_len = 0;
        bool jpeg_converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, fb->format, 31, &jpg_buf, &jpg_len);
        
        if (jpeg_converted) {
             // Send JPEG chunk
            char part_buf[64];
            snprintf(part_buf, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", jpg_len);
            client.write(part_buf);
            client.write(jpg_buf, jpg_len);
            client.write("\r\n--frame\r\n");
            free(jpg_buf);
        }

        // --- QR SCANNING ---
        // We have the raw grayscale framebuffer 'fb' here!
        if (millis() - lastScanTime > scanInterval) {
            lastScanTime = millis();
            QRCodeResult qr = decodeQR(fb);
            if (qr.valid) {
                Serial.println("QR FOUND: " + qr.payload);
                sendQRToServer(qr.payload);
                delay(1000); // Debounce
            }
        }

        esp_camera_fb_return(fb);
    }
}

void handleIndex() {
  String html = "<html><body><h1>ESP32 CAM Live</h1><img src='/stream' style='width:320px;'></body></html>";
  server.send(200, "text/html", html);
}