#include <Arduino.h>
#include "Stepper.h"

Stepper motor;

String inputString = "";
bool inputComplete = false;

void setup() {
    Serial.begin(115200);

    motor.begin(5, 18);   // DIR = 5, STEP = 18

    Serial.println("Enter speed in steps/sec:");
    
    // wait for speed input
    while (!Serial.available());
    float speed = Serial.parseFloat();

    motor.setSpeed(speed);

    Serial.print("Speed set to: ");
    Serial.println(speed);

    Serial.println("Now enter angles:");
}

void loop() {
    if (Serial.available()) {
        float angle = Serial.parseFloat();

        Serial.print("Moving to angle: ");
        Serial.println(angle);

        motor.moveToAngle(angle);

        Serial.print("Current position: ");
        Serial.println(motor.getCurrentPosition());

        Serial.println("Enter next angle:");
    }
}

// #include <Arduino.h>
// #include "Stepper.h"
// #include <WiFi.h>
// #include <WebServer.h>
// #include <ArduinoOTA.h>

// // ===== Stepper =====
// Stepper motor;

// // ===== WiFi =====
// const char* ssid = "Abbas";
// const char* password = "4112004hamdy";

// // ===== Server =====
// WebServer server(80);

// // ===== Variables =====
// float speed = 200;
// float currentAngle = 0;

// // ===== HTML =====
// String webpage = R"rawliteral(
// <!DOCTYPE html>
// <html>
// <head>
//     <title>Stepper Control</title>
//     <meta name="viewport" content="width=device-width, initial-scale=1">
//     <style>
//         body { font-family: Arial; text-align: center; background:#111; color:#fff; }
//         input, button { padding: 12px; margin: 10px; font-size: 18px; border-radius:8px; border:none; }
//         input { width: 200px; }
//         button { background:#00adb5; color:white; cursor:pointer; }
//         button:hover { background:#008c92; }
//         .card { background:#222; padding:20px; border-radius:10px; display:inline-block; }
//     </style>
// </head>
// <body>

//     <div class="card">
//         <h2>Stepper Control</h2>

//         <form action="/set">
//             <input type="number" step="any" name="speed" placeholder="Speed (steps/sec)" required>
//             <br>
//             <input type="number" step="any" name="angle" placeholder="Angle (deg)" required>
//             <br>
//             <button type="submit">Move</button>
//         </form>

//         <p>Current Speed: %SPEED%</p>
//         <p>Current Angle: %ANGLE%</p>
//     </div>

// </body>
// </html>
// )rawliteral";

// // ===== Render Page =====
// String processor() {
//     String page = webpage;
//     page.replace("%SPEED%", String(speed));
//     page.replace("%ANGLE%", String(currentAngle));
//     return page;
// }

// // ===== Root =====
// void handleRoot() {
//     server.send(200, "text/html", processor());
// }

// // ===== Handle Input =====
// void handleSet() {
//     if (server.hasArg("speed") && server.hasArg("angle")) {

//         speed = server.arg("speed").toFloat();
//         float angle = server.arg("angle").toFloat();

//         motor.setSpeed(speed);

//         Serial.print("Speed: ");
//         Serial.println(speed);

//         Serial.print("Move to: ");
//         Serial.println(angle);

//         motor.moveToAngle(angle);

//         currentAngle = angle;
//     }

//     server.send(200, "text/html", processor());
// }

// void setup() {
//     Serial.begin(115200);

//     motor.begin(5, 18);

//     // WiFi
//     WiFi.begin(ssid, password);
//     Serial.print("Connecting...");
//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }

//     Serial.println("\nConnected!");
//     Serial.println(WiFi.localIP());

//     // Server
//     server.on("/", handleRoot);
//     server.on("/set", handleSet);
//     server.begin();

//     // OTA
//     ArduinoOTA.begin();
// }

// void loop() {
//     ArduinoOTA.handle();
//     server.handleClient();
// }
