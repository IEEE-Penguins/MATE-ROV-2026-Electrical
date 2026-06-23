/*
  Arduino .ino test for integrated MPU6050 + magnetometer yaw telemetry format.

  Purpose:
    - Verify the existing ROS serial topic/telemetry JSON format is unchanged.
    - Do NOT publish any `mag` or `mag_ready` fields.
    - Put magnetometer yaw only in the existing `mpu.angle[2]` slot.
    - Fill all other sensor values with zero for this test frame.

  Expected JSON shape:

    {
      "type": "sensors",
      "data": {
        "depth": 0,
        "depth_status": "OK",
        "mpu": {
          "acc": [0, 0, 0],
          "gyro": [0, 0, 0],
          "angle": [0, 0, yaw_rad],
          "temp_in": 0,
          "ready": true
        },
        "command_alive": false
      }
    }

  Requirements:
    - Use the integrated mpu.h/mpu.cpp that includes magnetometer internals.
    - ArduinoJson library must be installed.

  Notes:
    - This is a logic-level .ino test. It injects fake magnetometer vectors.
    - It does not need the physical MPU/QMC sensors connected.
*/

#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

// Test-only access to internal magnetometer members/helpers.
// Do NOT use this trick in production code.
#define private public
#include "mpu.h"
#undef private

static constexpr float EPS_DEG = 0.001f;
static constexpr float EPS_RAD = 0.00001f;
static constexpr size_t TEST_TELEMETRY_JSON_CAPACITY = 320;
static constexpr size_t TEST_TELEMETRY_BUFFER_SIZE = 256;

MPU6050 imu(Wire);

uint16_t testsRun = 0;
uint16_t testsFailed = 0;

void pass(const char* name)
{
    testsRun++;
    Serial.print("[PASS] ");
    Serial.println(name);
}

void fail(const char* name, const char* reason)
{
    testsRun++;
    testsFailed++;
    Serial.print("[FAIL] ");
    Serial.print(name);
    Serial.print(" -> ");
    Serial.println(reason);
}

bool nearFloat(float actual, float expected, float eps)
{
    return fabs(actual - expected) <= eps;
}

void expectNear(const char* name, float actual, float expected, float eps)
{
    if (nearFloat(actual, expected, eps))
    {
        pass(name);
    }
    else
    {
        Serial.print("       expected: ");
        Serial.print(expected, 8);
        Serial.print(" actual: ");
        Serial.println(actual, 8);
        fail(name, "value outside tolerance");
    }
}

void expectTrue(const char* name, bool condition)
{
    if (condition)
    {
        pass(name);
    }
    else
    {
        fail(name, "condition is false");
    }
}

void expectFalse(const char* name, bool condition)
{
    if (!condition)
    {
        pass(name);
    }
    else
    {
        fail(name, "condition is true");
    }
}

void expectString(const char* name, const char* actual, const char* expected)
{
    if (actual != nullptr && strcmp(actual, expected) == 0)
    {
        pass(name);
    }
    else
    {
        fail(name, "string mismatch");
    }
}

void setFlatMagVector(MPU6050& mpu, float magX, float magY, float magZ)
{
    mpu._roll = 0.0f;
    mpu._pitch = 0.0f;
    mpu._yaw = 0.0f;

    mpu._accX = 0.0f;
    mpu._accY = 0.0f;
    mpu._accZ = 0.0f;

    mpu._gyroX = 0.0f;
    mpu._gyroY = 0.0f;
    mpu._gyroZ = 0.0f;

    mpu._temp = 0.0f;

    mpu._magX = magX;
    mpu._magY = magY;
    mpu._magZ = magZ;
}

size_t buildTopicFormatTelemetry(MPU6050& mpu, char* outBuffer, size_t outSize)
{
    StaticJsonDocument<TEST_TELEMETRY_JSON_CAPACITY> out;

    out["type"] = "sensors";
    JsonObject data = out["data"].to<JsonObject>();

    data["depth"] = 0.0f;
    data["depth_status"] = "OK";

    JsonObject mpuObj = data["mpu"].to<JsonObject>();

    JsonArray acc = mpuObj["acc"].to<JsonArray>();
    acc.add(0.0f);
    acc.add(0.0f);
    acc.add(0.0f);

    JsonArray gyro = mpuObj["gyro"].to<JsonArray>();
    gyro.add(0.0f);
    gyro.add(0.0f);
    gyro.add(0.0f);

    JsonArray angle = mpuObj["angle"].to<JsonArray>();
    angle.add(0.0f);                    // roll = zero in this test frame
    angle.add(0.0f);                    // pitch = zero in this test frame
    angle.add(mpu.yaw() * DEG_TO_RAD);  // yaw only, same existing field

    mpuObj["temp_in"] = 0.0f;
    mpuObj["ready"] = true;

    data["command_alive"] = false;

    return serializeJson(out, outBuffer, outSize);
}

bool buildAndParseTelemetry(MPU6050& mpu, StaticJsonDocument<TEST_TELEMETRY_JSON_CAPACITY>& parsed)
{
    char buffer[TEST_TELEMETRY_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    size_t written = buildTopicFormatTelemetry(mpu, buffer, sizeof(buffer));
    if (written == 0)
    {
        fail("build telemetry", "serializeJson wrote zero bytes");
        return false;
    }

    Serial.println("Generated topic-format telemetry:");
    Serial.println(buffer);

    DeserializationError err = deserializeJson(parsed, buffer);
    if (err)
    {
        Serial.print("JSON error: ");
        Serial.println(err.c_str());
        fail("parse telemetry", "deserializeJson failed");
        return false;
    }

    pass("parse telemetry");
    return true;
}

void testWrapAngle()
{
    expectNear("wrap 0", MPU6050::wrapAngle180(0.0f), 0.0f, EPS_DEG);
    expectNear("wrap +90", MPU6050::wrapAngle180(90.0f), 90.0f, EPS_DEG);
    expectNear("wrap -90", MPU6050::wrapAngle180(-90.0f), -90.0f, EPS_DEG);
    expectNear("wrap +190 -> -170", MPU6050::wrapAngle180(190.0f), -170.0f, EPS_DEG);
    expectNear("wrap -190 -> +170", MPU6050::wrapAngle180(-190.0f), 170.0f, EPS_DEG);
}

void testMagYawDirections()
{
    setFlatMagVector(imu, 100.0f, 0.0f, 0.0f);
    imu.computeMagYaw();
    expectNear("mag yaw north = 0 deg", imu.yaw(), 0.0f, EPS_DEG);

    setFlatMagVector(imu, 0.0f, -100.0f, 0.0f);
    imu.computeMagYaw();
    expectNear("mag yaw east = +90 deg", imu.yaw(), 90.0f, EPS_DEG);

    setFlatMagVector(imu, 0.0f, 100.0f, 0.0f);
    imu.computeMagYaw();
    expectNear("mag yaw west = -90 deg", imu.yaw(), -90.0f, EPS_DEG);

    setFlatMagVector(imu, -100.0f, 0.0f, 0.0f);
    imu.computeMagYaw();
    expectNear("mag yaw south = +/-180 deg", fabs(imu.yaw()), 180.0f, EPS_DEG);
}

void testTopicFormat()
{
    setFlatMagVector(imu, 0.0f, -100.0f, 0.0f); // east => +90 deg
    imu.computeMagYaw();

    StaticJsonDocument<TEST_TELEMETRY_JSON_CAPACITY> parsed;
    if (!buildAndParseTelemetry(imu, parsed))
    {
        return;
    }

    expectString("type = sensors", parsed["type"].as<const char*>(), "sensors");
    expectTrue("data object exists", parsed["data"].is<JsonObject>());

    JsonObject data = parsed["data"];
    JsonObject mpuObj = data["mpu"];

    expectTrue("mpu object exists", mpuObj.is<JsonObject>());
    expectTrue("acc array exists", mpuObj["acc"].is<JsonArray>());
    expectTrue("gyro array exists", mpuObj["gyro"].is<JsonArray>());
    expectTrue("angle array exists", mpuObj["angle"].is<JsonArray>());
    expectTrue("ready bool exists", mpuObj["ready"].is<bool>());
    expectFalse("no mag field", mpuObj.containsKey("mag"));
    expectFalse("no mag_ready field", mpuObj.containsKey("mag_ready"));

    expectNear("depth = 0", data["depth"].as<float>(), 0.0f, EPS_RAD);
    expectString("depth_status = OK", data["depth_status"].as<const char*>(), "OK");
    expectFalse("command_alive = false", data["command_alive"].as<bool>());

    JsonArray acc = mpuObj["acc"];
    JsonArray gyro = mpuObj["gyro"];
    JsonArray angle = mpuObj["angle"];

    expectTrue("acc size = 3", acc.size() == 3);
    expectTrue("gyro size = 3", gyro.size() == 3);
    expectTrue("angle size = 3", angle.size() == 3);

    for (uint8_t i = 0; i < 3; ++i)
    {
        char name[32];
        snprintf(name, sizeof(name), "acc[%u] = 0", i);
        expectNear(name, acc[i].as<float>(), 0.0f, EPS_RAD);

        snprintf(name, sizeof(name), "gyro[%u] = 0", i);
        expectNear(name, gyro[i].as<float>(), 0.0f, EPS_RAD);
    }

    expectNear("angle[0] roll = 0", angle[0].as<float>(), 0.0f, EPS_RAD);
    expectNear("angle[1] pitch = 0", angle[1].as<float>(), 0.0f, EPS_RAD);
    expectNear("angle[2] yaw = +90deg in rad", angle[2].as<float>(), HALF_PI, EPS_RAD);
    expectNear("temp_in = 0", mpuObj["temp_in"].as<float>(), 0.0f, EPS_RAD);
    expectTrue("ready = true", mpuObj["ready"].as<bool>());
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("=== MPU + MAG telemetry format .ino test ===");
    Serial.println("This test injects fake magnetometer vectors; no hardware sensor is required.");

    testWrapAngle();
    testMagYawDirections();
    testTopicFormat();

    Serial.println();
    Serial.print("Tests run: ");
    Serial.println(testsRun);
    Serial.print("Tests failed: ");
    Serial.println(testsFailed);

    if (testsFailed == 0)
    {
        Serial.println("RESULT: PASS");
    }
    else
    {
        Serial.println("RESULT: FAIL");
    }
}

void loop()
{
    // Test runs once in setup().
}