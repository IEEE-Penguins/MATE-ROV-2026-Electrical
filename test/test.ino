#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>
#include "Depth.h"

Depth Sensor;

// WiFi
const char *ssid = "Abbas";
const char *password = "4112004hamdy";

WiFiServer server(80);

// ================= HTML =================
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Depth Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      background: #0f172a;
      color: white;
      font-family: Arial;
      text-align: center;
    }
    .card {
      background: #1e293b;
      margin: 20px auto;
      padding: 20px;
      border-radius: 15px;
      width: 300px;
      box-shadow: 0 0 20px rgba(0,0,0,0.5);
    }
    h1 {
      color: #38bdf8;
    }
    .value {
      font-size: 28px;
      margin: 10px 0;
      color: #22c55e;
    }
  </style>
</head>
<body>

  <h1>Depth Sensor</h1>

  <div class="card">
    <p>Pressure</p>
    <div class="value" id="pressure">--</div>
  </div>

  <div class="card">
    <p>Depth</p>
    <div class="value" id="depth">--</div>
  </div>

<script>
setInterval(() => {
  fetch("/data")
    .then(res => res.json())
    .then(data => {
      document.getElementById("pressure").innerText = data.pressure + " Pa";
      document.getElementById("depth").innerText = data.depth + " cm";
    });
}, 500);
</script>

</body>
</html>
)rawliteral";

// ================= OTA =================
uint32_t last_ota_time = 0;

void setupOTA() {
  ArduinoOTA
    .onStart([]() {
      Serial.println("Start OTA");
    })
    .onEnd([]() {
      Serial.println("\nEnd OTA");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      if (millis() - last_ota_time > 500) {
        Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
        last_ota_time = millis();
      }
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]\n", error);
    });

  ArduinoOTA.begin();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // Sensor
  Sensor.begin();

  Serial.println("Calibrating...");
  delay(3000);
  Sensor.calibrate(10);
  Serial.println("Done!");

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting...");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println(" Failed! Rebooting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected!");
  Serial.println(WiFi.localIP());

  server.begin();

  setupOTA();
}

// ================= LOOP =================
void loop() {
  ArduinoOTA.handle();

  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  // ===== API =====
  if (request.indexOf("/data") != -1) {
    float pressure = Sensor.getPressure();
    float depth = Sensor.getDepthCM();

    String json = "{";
    json += "\"pressure\":" + String(pressure) + ",";
    json += "\"depth\":" + String(depth);
    json += "}";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close\n");
    client.println(json);
  }
  // ===== Web Page =====
  else {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close\n");
    client.println(webpage);
  }

  client.stop();
}