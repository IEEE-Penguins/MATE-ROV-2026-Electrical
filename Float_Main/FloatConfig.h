#ifndef FLOAT_CONFIG_H
#define FLOAT_CONFIG_H

#include <Arduino.h>

namespace FloatConfig {

static const char* WIFI_SSID = "Abbas";
static const char* WIFI_PASS = "4112004hamdy";
static const char* COMPANY_ID = "PN09";
static const char* SERVER_BASE_URL = "http://10.81.35.231:4000";

constexpr uint8_t PIN_STEPPER_DIR = 5;
constexpr uint8_t PIN_STEPPER_STEP = 18;
constexpr uint8_t PIN_DEPTH_DOUT = 14;
constexpr uint8_t PIN_DEPTH_SCK = 21;

constexpr float STEPPER_STEPS_PER_SECOND = 320.0f;
constexpr bool STEPPER_DIR_FOR_DESCEND = true;

constexpr uint32_t SAMPLE_INTERVAL_MS = 5000;
constexpr uint32_t CONTROL_INTERVAL_MS = 40;
constexpr uint32_t STEPPER_STEP_INTERVAL_MS = 30;
constexpr uint32_t STATUS_PRINT_INTERVAL_MS = 1000;

constexpr uint32_t HOLD_DURATION_MS = 30000;
constexpr uint8_t REQUIRED_HOLD_SAMPLES = 7;

constexpr uint32_t MISSION_DURATION_LIMIT_MS = 15UL * 60UL * 1000UL;
constexpr uint8_t TARGET_SUCCESSFUL_PROFILES = 2;
constexpr uint8_t MAX_PROFILE_ATTEMPTS = 6;

constexpr float DEEP_MIN_M = 2.27f;
constexpr float DEEP_MAX_M = 2.83f;

constexpr float SENSOR_TO_TOP_OFFSET_M = 0.85f;
constexpr float TOP_SHALLOW_MIN_M = 0.07f;
constexpr float TOP_SHALLOW_MAX_M = 0.73f;

constexpr float SHALLOW_SENSOR_MIN_M = TOP_SHALLOW_MIN_M + SENSOR_TO_TOP_OFFSET_M;
constexpr float SHALLOW_SENSOR_MAX_M = TOP_SHALLOW_MAX_M + SENSOR_TO_TOP_OFFSET_M;

constexpr float SURFACE_CONTACT_SENSOR_DEPTH_M = SENSOR_TO_TOP_OFFSET_M;
constexpr float RANGE_MARGIN_M = 0.03f;
constexpr float CENTER_TRIM_MARGIN_M = 0.05f;
constexpr float MAX_VALID_SENSOR_DEPTH_M = 8.0f;

constexpr float PRESSURE_KPA_PER_METER = 9.81f;
constexpr float DEPTH_CALIBRATION_FACTOR = 0.007f;
constexpr uint8_t DEPTH_CALIBRATE_SAMPLES = 20;
constexpr uint8_t DEPTH_READ_SAMPLES = 5;

constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 10000;
constexpr uint32_t PERSIST_INTERVAL_MS = 10000;
constexpr uint32_t HTTP_TIMEOUT_MS = 5000;
constexpr uint32_t RETRY_BACKOFF_START_MS = 1000;
constexpr uint32_t RETRY_BACKOFF_MAX_MS = 30000;

constexpr size_t MAX_QUEUE_SIZE = 120;
constexpr size_t MAX_BATCH_SEND = 20;
static const char* QUEUE_FILE = "/float_queue.json";
constexpr size_t QUEUE_DOC_CAPACITY = 49152;
constexpr size_t BATCH_DOC_CAPACITY = 16384;

}  // namespace FloatConfig

#endif
