#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
//aaaaaaaaaaaaa
// Configuración de Wi-Fi
const char* ssid = "INFINITUM1B9E_2.4";
const char* password = "LDg2XDDMnR";

// URLs para descargar la versión y el firmware
const char* versionURL = "https://raw.githubusercontent.com/djlira/ESP_SismoNet/main/firmware/version.txt";

const char* firmwareURL = "https://raw.githubusercontent.com/djlira/ESP_SismoNet/main/firmware/build/esp32.esp32.lilygo_t_display/firmware.bin";

// Versión actual del firmware (debe coincidir con el valor en el ESP32)
const String currentVersion = "1.0.1";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  checkForUpdates();
}

void loop() {
  // Código principal...
}

void checkForUpdates() {
  Serial.println("Verificando actualizaciones...");

  String latestVersion = downloadVersion();
  Serial.println("Versión actual: " + currentVersion);
  Serial.println("Versión disponible: " + latestVersion);

  if (latestVersion != currentVersion && latestVersion != "") {
    Serial.println("Nueva versión disponible. Actualizando...");
    updateFirmware();
  } else {
    Serial.println("El firmware está actualizado.");
  }
}

String downloadVersion() {
  WiFiClientSecure client;
  client.setInsecure();  // Deshabilita verificación SSL/TLS

  HTTPClient http;
  http.begin(client, versionURL);

  int httpCode = http.GET();
  String version = "";

  if (httpCode == HTTP_CODE_OK) {
    version = http.getString();
    version.trim(); // Eliminar espacios en blanco y saltos de línea
  } else {
    Serial.println("Error al descargar version.txt");
  }

  http.end();
  return version;
}

void updateFirmware() {
  Serial.println("Descargando firmware...");
  
  WiFiClientSecure client;
  client.setInsecure();  // Deshabilita verificación SSL/TLS

  HTTPClient http;
  http.begin(client, firmwareURL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      WiFiClient* stream = http.getStreamPtr();
      size_t written = Update.writeStream(*stream);

      if (written == contentLength) {
        Serial.println("Actualización exitosa. Reiniciando...");
        Update.end();
        ESP.restart();
      } else {
        Serial.println("Error: No se escribió el firmware completamente.");
      }
    } else {
      Serial.println("Error: No se pudo iniciar la actualización.");
    }
  } else {
    Serial.println("Error al descargar el firmware.");
  }

  http.end();
}
