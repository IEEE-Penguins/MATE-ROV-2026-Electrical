#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "Abbas";
const char* password = "4112004hamdy";

WebSocketsServer webSocket = WebSocketsServer(81);
WiFiServer server(80);

unsigned long lastTime = 0;
int counter = 0;
uint32_t last_ota_time = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
  webSocket.begin();

  // OTA
  ArduinoOTA
    .onStart([]() {
      String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
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

void loop() {
  ArduinoOTA.handle();   // مهم للـ OTA
  webSocket.loop();      // مهم للـ WebSocket

  // إرسال كل 100ms
  if (millis() - lastTime >= 1) {
    lastTime = millis();
    counter++;

    String message = "Hello Count = " + String(counter);
    webSocket.broadcastTXT(message);
  }

  WiFiClient client = server.available();
  if (client) {

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Live Monitor</title>

  <style>
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #0f2027, #203a43, #2c5364);
      color: white;
      text-align: center;
    }

    .container {
      margin-top: 50px;
    }

    h1 {
      font-size: 28px;
      opacity: 0.8;
    }

    .card {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(10px);
      border-radius: 20px;
      padding: 30px;
      width: 300px;
      margin: 30px auto;
      box-shadow: 0 8px 30px rgba(0,0,0,0.3);
    }

    .value {
      font-size: 40px;
      font-weight: bold;
      margin-top: 10px;
      transition: 0.2s;
    }

    .dot {
      height: 10px;
      width: 10px;
      background-color: #00ffcc;
      border-radius: 50%;
      display: inline-block;
      margin-right: 8px;
      animation: blink 1s infinite;
    }

    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.3; }
      100% { opacity: 1; }
    }
  </style>
</head>

<body>

  <div class="container">
    <h1>ESP32 Live Data</h1>

    <div class="card">
      <div><span class="dot"></span> Live Stream</div>
      <div id="data" class="value">Waiting...</div>
    </div>
  </div>

  <script>
    var ws = new WebSocket("ws://" + location.hostname + ":81/");

    ws.onmessage = function(event) {
      var el = document.getElementById("data");
      el.innerHTML = event.data;

      el.style.transform = "scale(1.1)";
      setTimeout(() => {
        el.style.transform = "scale(1)";
      }, 100);
    };
  </script>

</body>
</html>
)rawliteral");

    client.stop();
  }
}
