#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>

// Configuración de Wi-Fi
const char* ssid = "INFINITUM1B9E_2.4";
const char* password = "LDg2XDDMnR";

// URLs del servidor
const char* versionURL = "https://raw.githubusercontent.com/djlira/ESP_SismoNet/main/firmware/version.txt";
const char* firmwareURL = "https://raw.githubusercontent.com/djlira/ESP_SismoNet/main/firmware/build/esp32.esp32.lilygo_t_display/firmware.ino.bin";

// Versión actual del ESP32 (debe coincidir con el `version.txt`)
const String currentVersion = "1.0.1";

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando ESP32...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");

  checkForUpdates();
}

void loop() {
  // Puede ejecutarse cada cierto tiempo si deseas verificar periódicamente
}

void checkForUpdates() {
  Serial.println("\nVerificando actualizaciones...");

  String latestVersion = downloadVersion();

  Serial.println("Versión actual del ESP32: " + currentVersion);
  Serial.println("Versión obtenida del servidor: " + latestVersion);

  if (latestVersion.length() == 0) {
    Serial.println("Error: No se pudo obtener la versión.");
    return;
  }

  if (latestVersion != currentVersion) {
    Serial.println("Nueva versión detectada. Iniciando actualización...");
    updateFirmware();
  } else {
    Serial.println("El firmware ya está actualizado.");
  }
}

String downloadVersion() {
  WiFiClientSecure client;
  client.setInsecure();  // Deshabilita verificación SSL/TLS

  HTTPClient http;
  Serial.println("Descargando versión desde: " + String(versionURL));
  http.begin(client, versionURL);

  int httpCode = http.GET();
  String version = "";

  if (httpCode == HTTP_CODE_OK) {
    version = http.getString();
    version.trim();  // Eliminar espacios y saltos de línea
    Serial.println("Versión descargada: " + version);
  } else {
    Serial.println("Error al descargar version.txt. Código HTTP: " + String(httpCode));
  }

  http.end();
  return version;
}

void updateFirmware() {
  Serial.println("Descargando firmware desde: " + String(firmwareURL));

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, firmwareURL);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("Error al descargar el firmware. Código HTTP: " + String(httpCode));
    http.end();
    return;
  }

  int contentLength = http.getSize();
  if (contentLength <= 0) {
    Serial.println("Error: Tamaño del firmware inválido.");
    http.end();
    return;
  }

  Serial.println("Tamaño del firmware: " + String(contentLength) + " bytes");

  bool canBegin = Update.begin(contentLength);
  if (!canBegin) {
    Serial.println("Error: No se pudo iniciar la actualización.");
    http.end();
    return;
  }

  Serial.println("Escribiendo firmware...");
  WiFiClient* stream = http.getStreamPtr();
  size_t written = Update.writeStream(*stream);

  if (written == contentLength) {
    Serial.println("Firmware escrito correctamente. Reiniciando...");
    Update.end();
    ESP.restart();
  } else {
    Serial.println("Error: No se escribió el firmware completamente.");
    Update.end();
  }

  http.end();
}
