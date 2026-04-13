// #include <WiFi.h>
// #include <ESPmDNS.h>
// #include <NetworkUdp.h>
// #include <ArduinoOTA.h>
// #include <WebServer.h>

// #define DIR_PIN 5
// #define STEP_PIN 18

// float stepsPerRevolution = 200.0;
// float stepAngle = 360.0 / stepsPerRevolution;

// int currentPosition = 0; // بالـ steps

// const char *ssid = "Abbas";
// const char *password = "4112004hamdy";

// WebServer server(80);

// uint32_t last_ota_time = 0;
// String receivedData = "";


// void moveToAngle(float angle) {
//   int targetSteps = angle / stepAngle;
//   int stepsToMove = targetSteps - currentPosition;

//   if (stepsToMove > 0) {
//     digitalWrite(DIR_PIN, HIGH);
//   } else {
//     digitalWrite(DIR_PIN, LOW);
//     stepsToMove = -stepsToMove;
//   }

//   for (int i = 0; i < stepsToMove; i++) {
//     digitalWrite(STEP_PIN, HIGH);
//     delayMicroseconds(2000);
//     digitalWrite(STEP_PIN, LOW);
//     delayMicroseconds(2000);
//   }

//   currentPosition = targetSteps;
// }

// void setup() {
//   pinMode(2, OUTPUT);
//   Serial.begin(115200);

//   WiFi.begin(ssid, password);
//   while (WiFi.waitForConnectResult() != WL_CONNECTED) {
//     Serial.println("Connection Failed! Rebooting...");
//     delay(5000);
//     ESP.restart();
//   }

//   // ===== OTA =====
//   ArduinoOTA.begin();

//   // ===== WEB SERVER =====
//   server.on("/", []() {
//     String html = "<h2>ESP32 Web Serial 🔥</h2>";
//     html += "<form action='/send'>";
//     html += "<input name='data'>";
//     html += "<input type='submit'>";
//     html += "</form>";
//     html += "<p>Last Data: " + receivedData + "</p>";
//     server.send(200, "text/html", html);
//   });

//   server.on("/send", []() {
//     receivedData = server.arg("data");
//     Serial.println(receivedData); // زي السيريال
//     server.sendHeader("Location", "/");
//     server.send(303);
//   });

//   server.begin();

//   Serial.println("Ready");
//   Serial.println(WiFi.localIP());

//   pinMode(STEP_PIN, OUTPUT);
//   pinMode(DIR_PIN, OUTPUT);
//   Serial.begin(115200);

//   Serial.println("Enter angle:");
// }

// void loop() {
//   ArduinoOTA.handle();
//   server.handleClient();   // ❗ مهم جدًا

//   if (Serial.available()) {
//     float angle = Serial.parseFloat(); // اقرأ الزاوية

//     Serial.print("Moving to: ");
//     Serial.println(angle);

//     moveToAngle(angle);
//   }
// }


#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

#define DIR_PIN 5
#define STEP_PIN 18

float stepsPerRevolution = 200.0;
float stepAngle = 360.0 / stepsPerRevolution;

int currentPosition = 0; // بالـ steps

const char *ssid = "Abbas";
const char *password = "4112004hamdy";

WebServer server(80);

String receivedData = "";

void moveToAngle(float angle) {
  int targetSteps = angle / stepAngle;
  int stepsToMove = targetSteps - currentPosition;

  if (stepsToMove > 0) {
    digitalWrite(DIR_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, LOW);
    stepsToMove = -stepsToMove;
  }

  for (int i = 0; i < stepsToMove; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(2000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(2000);
  }

  currentPosition = targetSteps;
}

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.begin();

  // ===== Web Page UI =====
  server.on("/", []() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Motor Control</title>

  <style>
    body {
      font-family: Arial;
      background: #0f172a;
      color: white;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }

    .card {
      background: #1e293b;
      padding: 25px;
      border-radius: 15px;
      width: 300px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.4);
      text-align: center;
    }

    h2 {
      margin-bottom: 20px;
      color: #38bdf8;
    }

    input {
      width: 100%;
      padding: 12px;
      margin: 10px 0;
      border-radius: 10px;
      border: none;
      outline: none;
      font-size: 16px;
    }

    button {
      width: 100%;
      padding: 12px;
      background: #38bdf8;
      border: none;
      border-radius: 10px;
      font-size: 16px;
      cursor: pointer;
      font-weight: bold;
    }

    button:hover {
      background: #0ea5e9;
    }

    .info {
      margin-top: 15px;
      font-size: 14px;
      color: #cbd5e1;
    }
  </style>
</head>

<body>

  <div class="card">
    <h2>⚙️ ESP32 Motor Control</h2>

    <form action="/send">
      <input type="number" name="data" placeholder="Enter angle (0 - 360)" required>
      <button type="submit">Move Motor</button>
    </form>

    <div class="info">
      Last Angle: )rawliteral" + receivedData + R"rawliteral(
    </div>
  </div>

</body>
</html>
)rawliteral";

    server.send(200, "text/html", html);
  });

  // ===== Receive angle =====
  server.on("/send", []() {
    receivedData = server.arg("data");

    float angle = receivedData.toFloat();

    Serial.print("Moving to: ");
    Serial.println(angle);

    moveToAngle(angle);

    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.begin();

  Serial.println("Ready");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  if (Serial.available()) {
    float angle = Serial.parseFloat();

    Serial.print("Moving to: ");
    Serial.println(angle);

    moveToAngle(angle);
  }
}