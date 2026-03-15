
#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <micro_ros_arduino.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

#include "mpu.h"
#include "depth.h"

/*
  MAIN VERSION (NO THRUSTERS)

  This is exactly the same structure as the working debug version,
  but the THRUSTER MODULE is completely removed.

  Features:
  - Subscribes to /rov/command
  - Uses lights[0] to control built‑in LED (GPIO 2)
  - Publishes sensor data on /rov/sensors
  - MPU + Depth sensor active
  - No thrusters
  - No servos
  - No debug topics
*/

#define LIGHT_PIN 14

namespace Config
{
    static constexpr char NODE_NAME[] = "esp32_rov_main_node";
    static constexpr char COMMAND_TOPIC[] = "/rov/command";
    static constexpr char SENSORS_TOPIC[] = "/rov/sensors";

    static constexpr uint32_t COMMAND_TIMEOUT_MS = 1000;
    static constexpr uint32_t SENSOR_PUBLISH_PERIOD_MS = 100;
    static constexpr uint32_t EXECUTOR_SPIN_TIMEOUT_MS = 10;

    static constexpr uint8_t I2C_SDA_PIN = 21;
    static constexpr uint8_t I2C_SCL_PIN = 22;
    static constexpr uint32_t I2C_FREQUENCY_HZ = 400000;

    static constexpr size_t COMMAND_BUFFER_SIZE = 256;
    static constexpr size_t SENSOR_BUFFER_SIZE = 384;
}

namespace Ros
{
    rcl_allocator_t allocator;
    rclc_support_t support;
    rcl_node_t node;
    rcl_subscription_t commandSubscriber;
    rcl_publisher_t sensorsPublisher;
    rclc_executor_t executor;

    std_msgs__msg__String commandMsg;
    std_msgs__msg__String sensorsMsg;
}

namespace State
{
    uint8_t lightCmd = 0;

    uint32_t lastCommandMs = 0;
    uint32_t lastPublishMs = 0;
    bool rosReady = false;

    bool mpuReady = false;
    bool depthReady = false;
}

#define RCCHECK(fn) { rcl_ret_t rc = (fn); if (rc != RCL_RET_OK) errorLoop(); }
#define RCSOFTCHECK(fn) { rcl_ret_t rc = (fn); (void)rc; }

MPU6050 imu(Wire);
DepthSensor depthSensor(Wire);

// ------------------------------------------------------------

static void errorLoop()
{
    while (true)
    {
        delay(100);
    }
}

static void allocateRosString(std_msgs__msg__String& msg, size_t capacity)
{
    msg.data.data = static_cast<char*>(malloc(capacity));
    msg.data.size = 0;
    msg.data.capacity = capacity;

    if (msg.data.data == nullptr)
    {
        errorLoop();
    }

    msg.data.data[0] = '\0';
}

static void initializeLight()
{
    pinMode(LIGHT_PIN, OUTPUT);
    digitalWrite(LIGHT_PIN, LOW);
}

static void applyLight()
{
    digitalWrite(LIGHT_PIN, State::lightCmd ? HIGH : LOW);
}

static void initializeSensors()
{
    Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
    Wire.setClock(Config::I2C_FREQUENCY_HZ);

    State::mpuReady = imu.begin();
    if (State::mpuReady)
    {
        imu.calibrateGyro();
    }

    State::depthReady = depthSensor.begin();
}

// ------------------------------------------------------------

static bool parseCommandMessage(const char* json)
{
    if (json == nullptr)
    {
        return false;
    }

    StaticJsonDocument<Config::COMMAND_BUFFER_SIZE> doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err)
    {
        return false;
    }

    JsonArray lightArray = doc["lights"].as<JsonArray>();

    if (!lightArray.isNull() && lightArray.size() > 0)
    {
        State::lightCmd = lightArray[0].as<int>() ? 1 : 0;
    }
    else
    {
        State::lightCmd = 0;
    }

    return true;
}

// ------------------------------------------------------------

static void commandCallback(const void* msgin)
{
    const std_msgs__msg__String* incoming = static_cast<const std_msgs__msg__String*>(msgin);

    if (incoming == nullptr || incoming->data.data == nullptr)
    {
        return;
    }

    if (!parseCommandMessage(incoming->data.data))
    {
        return;
    }

    State::lastCommandMs = millis();
    applyLight();
}

// ------------------------------------------------------------

static void publishSensors()
{
    const uint32_t now = millis();

    if ((now - State::lastPublishMs) < Config::SENSOR_PUBLISH_PERIOD_MS)
    {
        return;
    }

    State::lastPublishMs = now;

    if (State::mpuReady)
        imu.update();

    if (State::depthReady)
        depthSensor.update();

    StaticJsonDocument<Config::SENSOR_BUFFER_SIZE> doc;

    doc["depth"] = State::depthReady ? depthSensor.depthMeters() : -1.0f;
    doc["depth_status"] = State::depthReady ? DepthSensor::statusToString(depthSensor.status()) : "NOT_READY";

    JsonObject mpu = doc.createNestedObject("mpu");
    JsonArray acc = mpu.createNestedArray("acc");
    JsonArray gyro = mpu.createNestedArray("gyro");
    JsonArray angle = mpu.createNestedArray("angle");

    if (State::mpuReady)
    {
        acc.add(imu.accX() * 9.81f);
        acc.add(imu.accY() * 9.81f);
        acc.add(imu.accZ() * 9.81f);

        gyro.add(imu.gyroX() * DEG_TO_RAD);
        gyro.add(imu.gyroY() * DEG_TO_RAD);
        gyro.add(imu.gyroZ() * DEG_TO_RAD);

        angle.add(imu.roll() * DEG_TO_RAD);
        angle.add(imu.pitch() * DEG_TO_RAD);
        angle.add(imu.yaw() * DEG_TO_RAD);

        mpu["temp_in"] = imu.temperature();
        mpu["ready"] = true;
    }
    else
    {
        acc.add(0); acc.add(0); acc.add(0);
        gyro.add(0); gyro.add(0); gyro.add(0);
        angle.add(0); angle.add(0); angle.add(0);

        mpu["temp_in"] = 0;
        mpu["ready"] = false;
    }

    doc["light"] = State::lightCmd ? 1 : 0;

    size_t written = serializeJson(doc, Ros::sensorsMsg.data.data, Ros::sensorsMsg.data.capacity);

    if (written > 0 && written < Ros::sensorsMsg.data.capacity)
    {
        Ros::sensorsMsg.data.size = written;
        RCSOFTCHECK(rcl_publish(&Ros::sensorsPublisher, &Ros::sensorsMsg, NULL));
    }
}

// ------------------------------------------------------------

static void initializeMicroRos()
{
    set_microros_transports();
    delay(2000);

    Ros::allocator = rcl_get_default_allocator();

    RCCHECK(rclc_support_init(&Ros::support, 0, NULL, &Ros::allocator));
    RCCHECK(rclc_node_init_default(&Ros::node, Config::NODE_NAME, "", &Ros::support));

    RCCHECK(rclc_subscription_init_default(
        &Ros::commandSubscriber,
        &Ros::node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        Config::COMMAND_TOPIC));

    RCCHECK(rclc_publisher_init_default(
        &Ros::sensorsPublisher,
        &Ros::node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        Config::SENSORS_TOPIC));

    allocateRosString(Ros::commandMsg, Config::COMMAND_BUFFER_SIZE);
    allocateRosString(Ros::sensorsMsg, Config::SENSOR_BUFFER_SIZE);

    RCCHECK(rclc_executor_init(&Ros::executor, &Ros::support.context, 1, &Ros::allocator));
    RCCHECK(rclc_executor_add_subscription(
        &Ros::executor,
        &Ros::commandSubscriber,
        &Ros::commandMsg,
        &commandCallback,
        ON_NEW_DATA));

    State::rosReady = true;
    State::lastCommandMs = millis();
}

// ------------------------------------------------------------

void setup()
{
    initializeLight();
    initializeSensors();
    initializeMicroRos();
}

void loop()
{
    if (State::rosReady)
    {
        RCSOFTCHECK(rclc_executor_spin_some(
            &Ros::executor,
            RCL_MS_TO_NS(Config::EXECUTOR_SPIN_TIMEOUT_MS)));
    }

    publishSensors();
    delay(5);
}
