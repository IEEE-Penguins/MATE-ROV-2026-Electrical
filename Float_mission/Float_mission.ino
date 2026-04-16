#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

// ===== User Configuration =====
static const char* WIFI_SSID = "Abbas";
static const char* WIFI_PASS = "4112004hamdy";
static const char* COMPANY_ID = "PN09";

// Example: "http://192.168.1.50:4000"
static const char* SERVER_BASE_URL = "http://10.81.35.231:4000";

// Timing and retry behavior
static const uint32_t SAMPLE_INTERVAL_MS = 5000;      // Mission requirement: at least every 5s
static const uint32_t WIFI_RETRY_INTERVAL_MS = 10000; // Wi-Fi reconnect attempt interval
static const uint32_t PERSIST_INTERVAL_MS = 10000;    // Queue save interval
static const uint32_t HTTP_TIMEOUT_MS = 5000;
static const uint32_t RETRY_BACKOFF_START_MS = 1000;
static const uint32_t RETRY_BACKOFF_MAX_MS = 30000;

// Queue behavior
static const size_t MAX_QUEUE_SIZE = 120;
static const size_t MAX_BATCH_SEND = 20;
static const char* QUEUE_FILE = "/float_queue.json";

struct FloatPacket {
  char companyId[16];
  char timestamp[32];
  float pressureKpa;
  float depthMeters;
  uint32_t seq;
  char packetId[48];
};

static FloatPacket packetQueue[MAX_QUEUE_SIZE];
static size_t queueHead = 0;
static size_t queueCount = 0;

static uint32_t sequenceCounter = 1;
static uint32_t bootNonce = 0;

static uint32_t lastSampleAt = 0;
static uint32_t lastWiFiAttemptAt = 0;
static uint32_t lastPersistAt = 0;
static uint32_t nextRetryAt = 0;
static uint32_t retryDelayMs = RETRY_BACKOFF_START_MS;

static String singlePacketUrl;
static String batchPacketUrl;
static String healthUrl;

size_t queueIndex(size_t offsetFromHead) {
  return (queueHead + offsetFromHead) % MAX_QUEUE_SIZE;
}

bool queueIsFull() {
  return queueCount >= MAX_QUEUE_SIZE;
}

bool queueIsEmpty() {
  return queueCount == 0;
}

FloatPacket* queueFront() {
  if (queueIsEmpty()) return nullptr;
  return &packetQueue[queueHead];
}

bool queueContainsPacketId(const char* packetId) {
  for (size_t i = 0; i < queueCount; i++) {
    const FloatPacket& p = packetQueue[queueIndex(i)];
    if (strcmp(p.packetId, packetId) == 0) {
      return true;
    }
  }
  return false;
}

bool enqueuePacket(const FloatPacket& packet) {
  if (queueContainsPacketId(packet.packetId)) {
    return true;
  }

  if (queueIsFull()) {
    Serial.println("[QUEUE] Full. Dropping oldest packet.");
    queueHead = (queueHead + 1) % MAX_QUEUE_SIZE;
    queueCount--;
  }

  size_t tail = queueIndex(queueCount);
  packetQueue[tail] = packet;
  queueCount++;
  return true;
}

bool dequeuePackets(size_t n) {
  if (n > queueCount) return false;
  queueHead = (queueHead + n) % MAX_QUEUE_SIZE;
  queueCount -= n;
  return true;
}

float computePressureKpa(float depthMeters) {
  return depthMeters * 9.8f;
}

String missionTimestamp() {
  uint32_t totalSeconds = millis() / 1000;
  uint32_t hours = totalSeconds / 3600;
  uint32_t minutes = (totalSeconds % 3600) / 60;
  uint32_t seconds = totalSeconds % 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%lu:%02lu:%02lu", (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds);
  return String(buf);
}

void packetToJson(JsonObject obj, const FloatPacket& p) {
  obj["companyId"] = p.companyId;
  obj["timestamp"] = p.timestamp;
  obj["pressureKpa"] = p.pressureKpa;
  obj["depthMeters"] = p.depthMeters;
  obj["packetId"] = p.packetId;
  obj["seq"] = p.seq;
}

bool jsonToPacket(JsonObject obj, FloatPacket& out) {
  if (!obj["depthMeters"].is<float>() && !obj["depthMeters"].is<int>()) {
    return false;
  }

  const char* company = obj["companyId"] | COMPANY_ID;
  const char* ts = obj["timestamp"] | "0:00:00";
  float depth = obj["depthMeters"].as<float>();
  float pressure = obj["pressureKpa"].is<float>() || obj["pressureKpa"].is<int>()
                       ? obj["pressureKpa"].as<float>()
                       : computePressureKpa(depth);
  uint32_t seq = obj["seq"] | 0;
  const char* packetId = obj["packetId"] | "";

  strncpy(out.companyId, company, sizeof(out.companyId) - 1);
  out.companyId[sizeof(out.companyId) - 1] = '\0';

  strncpy(out.timestamp, ts, sizeof(out.timestamp) - 1);
  out.timestamp[sizeof(out.timestamp) - 1] = '\0';

  out.depthMeters = depth;
  out.pressureKpa = pressure;
  out.seq = seq;

  strncpy(out.packetId, packetId, sizeof(out.packetId) - 1);
  out.packetId[sizeof(out.packetId) - 1] = '\0';

  return true;
}

bool persistQueueToFlash() {
  DynamicJsonDocument doc(20480);
  JsonArray arr = doc.to<JsonArray>();

  for (size_t i = 0; i < queueCount; i++) {
    JsonObject obj = arr.add<JsonObject>();
    packetToJson(obj, packetQueue[queueIndex(i)]);
  }

  File f = LittleFS.open(QUEUE_FILE, FILE_WRITE);
  if (!f) {
    Serial.println("[FLASH] Failed opening queue file for write.");
    return false;
  }

  if (serializeJson(doc, f) == 0) {
    Serial.println("[FLASH] Failed serializing queue.");
    f.close();
    return false;
  }

  f.close();
  lastPersistAt = millis();
  return true;
}

bool loadQueueFromFlash() {
  if (!LittleFS.exists(QUEUE_FILE)) {
    return true;
  }

  File f = LittleFS.open(QUEUE_FILE, FILE_READ);
  if (!f) {
    Serial.println("[FLASH] Failed opening queue file for read.");
    return false;
  }

  DynamicJsonDocument doc(20480);
  DeserializationError err = deserializeJson(doc, f);
  f.close();

  if (err) {
    Serial.printf("[FLASH] Failed parsing queue file: %s\n", err.c_str());
    return false;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("[FLASH] Queue file format invalid.");
    return false;
  }

  queueHead = 0;
  queueCount = 0;

  JsonArray arr = doc.as<JsonArray>();
  for (JsonVariant v : arr) {
    if (!v.is<JsonObject>()) continue;
    FloatPacket p;
    if (!jsonToPacket(v.as<JsonObject>(), p)) continue;
    enqueuePacket(p);
  }

  Serial.printf("[FLASH] Restored %u queued packet(s).\n", (unsigned int)queueCount);
  return true;
}

void setRetryBackoffFailure() {
  nextRetryAt = millis() + retryDelayMs;
  retryDelayMs = min(retryDelayMs * 2, RETRY_BACKOFF_MAX_MS);
}

void resetRetryBackoffSuccess() {
  retryDelayMs = RETRY_BACKOFF_START_MS;
  nextRetryAt = 0;
}

bool is2xx(int code) {
  return code >= 200 && code < 300;
}

bool postJson(const String& url, const String& jsonBody, int& httpCode, String& responseBody) {
  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  if (!http.begin(url)) {
    httpCode = -1;
    responseBody = "http.begin failed";
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  httpCode = http.POST(jsonBody);
  responseBody = http.getString();
  http.end();

  return is2xx(httpCode);
}

bool sendSinglePacket(const FloatPacket& p) {
  StaticJsonDocument<384> doc;
  packetToJson(doc.to<JsonObject>(), p);

  String body;
  serializeJson(doc, body);

  int code;
  String response;
  bool ok = postJson(singlePacketUrl, body, code, response);

  if (ok) {
    Serial.printf("[SEND] Single packet OK (%d) seq=%lu\n", code, (unsigned long)p.seq);
  } else {
    Serial.printf("[SEND] Single packet FAIL (%d): %s\n", code, response.c_str());
  }

  return ok;
}

bool sendBatchFromQueue(size_t batchSize) {
  if (queueCount == 0) return true;
  batchSize = min(batchSize, queueCount);

  DynamicJsonDocument doc(12288);
  JsonArray arr = doc.to<JsonArray>();

  for (size_t i = 0; i < batchSize; i++) {
    JsonObject obj = arr.add<JsonObject>();
    packetToJson(obj, packetQueue[queueIndex(i)]);
  }

  String body;
  serializeJson(doc, body);

  int code;
  String response;
  bool ok = postJson(batchPacketUrl, body, code, response);

  if (!ok) {
    Serial.printf("[SEND] Batch FAIL (%d): %s\n", code, response.c_str());
    return false;
  }

  Serial.printf("[SEND] Batch OK (%d), sent=%u\n", code, (unsigned int)batchSize);
  dequeuePackets(batchSize);
  persistQueueToFlash();
  return true;
}

void connectWiFiIfNeeded() {
  if (WiFi.status() == WL_CONNECTED) return;

  uint32_t now = millis();
  if (now - lastWiFiAttemptAt < WIFI_RETRY_INTERVAL_MS) return;

  lastWiFiAttemptAt = now;
  Serial.printf("[WIFI] Connecting to %s ...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void printNetworkStatusIfChanged() {
  static wl_status_t lastStatus = WL_IDLE_STATUS;
  wl_status_t status = WiFi.status();
  if (status == lastStatus) return;
  lastStatus = status;

  if (status == WL_CONNECTED) {
    Serial.printf("[WIFI] Connected. IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("[WIFI] Status changed: %d\n", (int)status);
  }
}

bool checkServerHealth() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  if (!http.begin(healthUrl)) return false;
  int code = http.GET();
  String response = http.getString();
  http.end();

  bool ok = is2xx(code);
  Serial.printf("[HEALTH] %s (%d): %s\n", ok ? "OK" : "FAIL", code, response.c_str());
  return ok;
}

float readDepthMeters() {
  // Replace this placeholder with your real depth sensor read.
  // This simulation produces a smooth wave between ~1.5 and ~3.0m.
  float t = millis() / 1000.0f;
  float depth = 2.25f + 0.75f * sinf(t * 0.2f);
  if (depth < 0.0f) depth = 0.0f;
  return depth;
}

FloatPacket buildLivePacket() {
  FloatPacket p;
  float depth = readDepthMeters();

  strncpy(p.companyId, COMPANY_ID, sizeof(p.companyId) - 1);
  p.companyId[sizeof(p.companyId) - 1] = '\0';

  String ts = missionTimestamp();
  strncpy(p.timestamp, ts.c_str(), sizeof(p.timestamp) - 1);
  p.timestamp[sizeof(p.timestamp) - 1] = '\0';

  p.depthMeters = depth;
  p.pressureKpa = computePressureKpa(depth);
  p.seq = sequenceCounter++;

  snprintf(p.packetId, sizeof(p.packetId), "%s-%lu-%lu", p.companyId, (unsigned long)bootNonce, (unsigned long)p.seq);

  return p;
}

void maybePersistQueuePeriodic() {
  if (millis() - lastPersistAt < PERSIST_INTERVAL_MS) return;
  persistQueueToFlash();
}

void flushRetryQueueIfDue() {
  if (queueIsEmpty()) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (nextRetryAt != 0 && millis() < nextRetryAt) return;

  bool ok = sendBatchFromQueue(MAX_BATCH_SEND);
  if (ok) {
    resetRetryBackoffSuccess();
  } else {
    setRetryBackoffFailure();
  }
}

void processLiveSampleTick() {
  uint32_t now = millis();
  if (now - lastSampleAt < SAMPLE_INTERVAL_MS) return;
  lastSampleAt = now;

  if (!queueIsEmpty()) {
    flushRetryQueueIfDue();
  }

  FloatPacket packet = buildLivePacket();

  if (WiFi.status() == WL_CONNECTED) {
    bool ok = sendSinglePacket(packet);
    if (ok) {
      resetRetryBackoffSuccess();
      return;
    }
  }

  enqueuePacket(packet);
  persistQueueToFlash();
  setRetryBackoffFailure();
  Serial.printf("[QUEUE] Enqueued seq=%lu, size=%u\n", (unsigned long)packet.seq, (unsigned int)queueCount);
}

void setup() {
  Serial.begin(115200);
  delay(400);

  Serial.println();
  Serial.println("=== Float Mission ESP32 Sender ===");

  if (!LittleFS.begin(true)) {
    Serial.println("[FLASH] LittleFS init failed.");
  }

  singlePacketUrl = String(SERVER_BASE_URL) + "/api/float/packet";
  batchPacketUrl = String(SERVER_BASE_URL) + "/api/float/packets";
  healthUrl = String(SERVER_BASE_URL) + "/health";

  randomSeed(esp_random());
  bootNonce = (uint32_t)random(100000, 999999);

  loadQueueFromFlash();
  connectWiFiIfNeeded();
}

void loop() {
  printNetworkStatusIfChanged();
  connectWiFiIfNeeded();

  static bool healthChecked = false;
  if (!healthChecked && WiFi.status() == WL_CONNECTED) {
    checkServerHealth();
    healthChecked = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    healthChecked = false;
  }

  flushRetryQueueIfDue();
  processLiveSampleTick();
  maybePersistQueuePeriodic();

  delay(20);
}