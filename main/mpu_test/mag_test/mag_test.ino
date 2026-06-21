#include <ArduinoJson.h>
#include "mag.h"

MPU6050Kalman imu;

static void publishTelemetry(bool commandAlive)
{
  StaticJsonDocument<512> out;

  out["type"] = "sensors";

  JsonObject data = out["data"].to<JsonObject>();

  /*
    IMU format:
    - acc   -> linear acceleration [x, y, z] in m/s^2
    - gyro  -> angular velocity [x, y, z] in rad/s
    - angle -> orientation [roll, pitch, yaw] in radians

    Your current code only has pitch and yaw getters,
    so roll, acc, and gyro are filled with zeros.
  */

  JsonObject imuObj = data["imu"].to<JsonObject>();

  JsonArray acc = imuObj["acc"].to<JsonArray>();
  acc.add(0.0);
  acc.add(0.0);
  acc.add(0.0);

  JsonArray gyro = imuObj["gyro"].to<JsonArray>();
  gyro.add(0.0);
  gyro.add(0.0);
  gyro.add(0.0);

  JsonArray angle = imuObj["angle"].to<JsonArray>();
  angle.add(0.0);                         // roll
  angle.add(imu.getPitch() * DEG_TO_RAD); // pitch
  angle.add(imu.getYaw() * DEG_TO_RAD);   // yaw

  /*
    Other sensors.
    Filled with zeros until real sensors are connected.
  */

  data["depth"] = 0.0;
  data["pressure"] = 0.0;
  data["temperature"] = 0.0;
  data["voltage"] = 0.0;
  data["current"] = 0.0;

  JsonObject leakObj = data["leak"].to<JsonObject>();
  leakObj["front"] = 0;
  leakObj["back"] = 0;

  JsonObject magObj = data["mag"].to<JsonObject>();
  magObj["x"] = 0.0;
  magObj["y"] = 0.0;
  magObj["z"] = 0.0;

  data["command_alive"] = commandAlive ? 1 : 0;

  serializeJson(out, Serial);
  Serial.println();
}

void setup()
{
  Serial.begin(460800);
  imu.begin();
}

void loop()
{
  imu.update();

  publishTelemetry(true);

  delay(20); // ~50 Hz telemetry
}