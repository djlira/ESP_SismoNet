#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <HTTPUpdate.h>

// Configuración de Wi-Fi
const char* ssid = "INFINITUM1B9E_2.4";
const char* password = "LDg2XDDMnR";

// URLs de GitHub
const char* baseURL = "https://raw.githubusercontent.com/djlira/ESP_SismoNet/main/firmware/";
String versionURL = String(baseURL) + "version.txt";
String firmwareURL = String(baseURL) + "build/esp32.esp32.lilygo_t_display/firmware.ino.bin";

// Versión actual del firmware
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
  // Tu código principal aquí...
}

void checkForUpdates() {
  Serial.println("Verificando actualizaciones...");

  // Descargar la versión disponible en GitHub
  String latestVersion = downloadVersion();
  Serial.println("Versión actual: " + currentVersion);
  Serial.println("Versión disponible: " + latestVersion);

  // Comparar versiones
  if (latestVersion != currentVersion && latestVersion != "") {
    Serial.println("Nueva versión disponible. Actualizando...");
    updateFirmware();
  } else {
    Serial.println("El firmware está actualizado.");
  }
}

String downloadVersion() {
  WiFiClientSecure client;
  client.setInsecure(); // Desactiva la verificación SSL

  HTTPClient http;
  String url = versionURL + "?nocache=" + String(millis()); // Evitar caché
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  String version = "";

  if (httpCode == HTTP_CODE_OK) {
    version = http.getString();
    version.trim(); // Limpiar espacios y saltos de línea
  } else {
    Serial.println("Error al descargar version.txt. Código: " + String(httpCode));
  }

  http.end();
  return version;
}

void updateFirmware() {
  Serial.println("Descargando firmware...");

  WiFiClientSecure client;
  client.setInsecure(); // Desactiva la verificación SSL

  String url = firmwareURL + "?nocache=" + String(millis()); // Evitar caché

  t_httpUpdate_return ret = UpdateFirmware(client, url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Error en la actualización: %s\n", httpUpdate.getLastErrorString().c_str());

      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No hay actualizaciones disponibles.");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("Actualización exitosa. Reiniciando...");
      ESP.restart();
      break;
  }
}

t_httpUpdate_return UpdateFirmware(WiFiClientSecure &client, String url) {
  HTTPClient http;
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.println("Iniciando actualización...");
  return httpUpdate.update(client, url);
}
