#include "CommsLink.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <esp_system.h>

namespace {
float computePressureKpa(float depthMeters) {
  if (depthMeters < 0.0f) {
    depthMeters = 0.0f;
  }
  return depthMeters * FloatConfig::PRESSURE_KPA_PER_METER;
}
}  // namespace

bool CommsLink::begin() {
  fileSystemReady_ = LittleFS.begin(true);
  if (!fileSystemReady_) {
    Serial.println("[FLASH] LittleFS init failed. Queue persistence disabled.");
  }

  singlePacketUrl_ = String(FloatConfig::SERVER_BASE_URL) + "/api/float/packet";
  batchPacketUrl_ = String(FloatConfig::SERVER_BASE_URL) + "/api/float/packets";
  healthUrl_ = String(FloatConfig::SERVER_BASE_URL) + "/health";

  randomSeed(esp_random());
  bootNonce_ = (uint32_t)random(100000, 999999);

  if (fileSystemReady_) {
    loadQueueFromFlash();
  }

  connectWiFiIfNeeded(millis());
  return fileSystemReady_;
}

void CommsLink::update(uint32_t nowMs) {
  printNetworkStatusIfChanged();
  connectWiFiIfNeeded(nowMs);

  if (!healthChecked_ && WiFi.status() == WL_CONNECTED) {
    checkServerHealth();
    healthChecked_ = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    healthChecked_ = false;
  }

  flushRetryQueueIfDue(nowMs);
  maybePersistQueuePeriodic(nowMs);
}

bool CommsLink::sendSample(float depthMeters, const MissionPacketContext& context, uint32_t nowMs) {
  FloatPacket packet = buildPacket(depthMeters, context, nowMs);

  if (WiFi.status() == WL_CONNECTED) {
    bool ok = sendSinglePacket(packet);
    if (ok) {
      anyTransmitSuccess_ = true;
      resetRetryBackoffSuccess();
      return true;
    }
  }

  enqueuePacket(packet);
  persistQueueToFlash();
  setRetryBackoffFailure(nowMs);

  Serial.printf("[QUEUE] Enqueued seq=%lu, size=%u\n", (unsigned long)packet.seq, (unsigned int)queueCount_);
  return false;
}

bool CommsLink::hasSuccessfulTransmit() const {
  return anyTransmitSuccess_;
}

size_t CommsLink::queuedCount() const {
  return queueCount_;
}

size_t CommsLink::queueIndex(size_t offsetFromHead) const {
  return (queueHead_ + offsetFromHead) % FloatConfig::MAX_QUEUE_SIZE;
}

bool CommsLink::queueIsFull() const {
  return queueCount_ >= FloatConfig::MAX_QUEUE_SIZE;
}

bool CommsLink::queueIsEmpty() const {
  return queueCount_ == 0;
}

bool CommsLink::queueContainsPacketId(const char* packetId) const {
  for (size_t i = 0; i < queueCount_; i++) {
    const FloatPacket& packet = packetQueue_[queueIndex(i)];
    if (strcmp(packet.packetId, packetId) == 0) {
      return true;
    }
  }
  return false;
}

bool CommsLink::enqueuePacket(const FloatPacket& packet) {
  if (queueContainsPacketId(packet.packetId)) {
    return true;
  }

  if (queueIsFull()) {
    Serial.println("[QUEUE] Full. Dropping oldest packet.");
    queueHead_ = (queueHead_ + 1) % FloatConfig::MAX_QUEUE_SIZE;
    queueCount_--;
  }

  size_t tail = queueIndex(queueCount_);
  packetQueue_[tail] = packet;
  queueCount_++;
  return true;
}

bool CommsLink::dequeuePackets(size_t n) {
  if (n > queueCount_) {
    return false;
  }

  queueHead_ = (queueHead_ + n) % FloatConfig::MAX_QUEUE_SIZE;
  queueCount_ -= n;
  return true;
}

String CommsLink::missionTimestamp(uint32_t nowMs) const {
  uint32_t totalSeconds = nowMs / 1000;
  uint32_t hours = totalSeconds / 3600;
  uint32_t minutes = (totalSeconds % 3600) / 60;
  uint32_t seconds = totalSeconds % 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%lu:%02lu:%02lu", (unsigned long)hours, (unsigned long)minutes,
           (unsigned long)seconds);
  return String(buf);
}

CommsLink::FloatPacket CommsLink::buildPacket(float depthMeters, const MissionPacketContext& context,
                                               uint32_t nowMs) {
  FloatPacket packet;
  if (depthMeters < 0.0f) {
    depthMeters = 0.0f;
  }

  strncpy(packet.companyId, FloatConfig::COMPANY_ID, sizeof(packet.companyId) - 1);
  packet.companyId[sizeof(packet.companyId) - 1] = '\0';

  String ts = missionTimestamp(nowMs);
  strncpy(packet.timestamp, ts.c_str(), sizeof(packet.timestamp) - 1);
  packet.timestamp[sizeof(packet.timestamp) - 1] = '\0';

  packet.depthMeters = depthMeters;
  packet.pressureKpa = computePressureKpa(depthMeters);
  packet.seq = sequenceCounter_++;

  snprintf(packet.packetId, sizeof(packet.packetId), "%s-%lu-%lu", packet.companyId,
           (unsigned long)bootNonce_, (unsigned long)packet.seq);

  packet.profileIndex = context.profileIndex;
  strncpy(packet.phase, context.phaseName, sizeof(packet.phase) - 1);
  packet.phase[sizeof(packet.phase) - 1] = '\0';
  packet.inDeepRange = context.inDeepRange;
  packet.inShallowRange = context.inShallowRange;
  packet.profilePenalty = context.profilePenalty;
  packet.recoveryReady = context.recoveryReady;
  packet.successfulProfiles = context.successfulProfiles;
  packet.completedProfiles = context.completedProfiles;
  packet.holdSampleCount = context.holdSampleCount;

  return packet;
}

void CommsLink::packetToJson(JsonObject obj, const FloatPacket& packet) const {
  obj["companyId"] = packet.companyId;
  obj["timestamp"] = packet.timestamp;
  obj["pressureKpa"] = packet.pressureKpa;
  obj["depthMeters"] = packet.depthMeters;
  obj["packetId"] = packet.packetId;
  obj["seq"] = packet.seq;

  obj["profileIndex"] = packet.profileIndex;
  obj["phase"] = packet.phase;
  obj["inDeepRange"] = packet.inDeepRange;
  obj["inShallowRange"] = packet.inShallowRange;
  obj["profilePenalty"] = packet.profilePenalty;
  obj["recoveryReady"] = packet.recoveryReady;
  obj["successfulProfiles"] = packet.successfulProfiles;
  obj["completedProfiles"] = packet.completedProfiles;
  obj["holdSampleCount"] = packet.holdSampleCount;
}

bool CommsLink::jsonToPacket(JsonObject obj, FloatPacket& out) const {
  if (!obj["depthMeters"].is<float>() && !obj["depthMeters"].is<int>()) {
    return false;
  }

  const char* company = obj["companyId"] | FloatConfig::COMPANY_ID;
  const char* ts = obj["timestamp"] | "0:00:00";
  float depth = obj["depthMeters"].as<float>();
  float pressure = obj["pressureKpa"].is<float>() || obj["pressureKpa"].is<int>()
                       ? obj["pressureKpa"].as<float>()
                       : computePressureKpa(depth);
  uint32_t seq = obj["seq"] | 0;
  const char* packetId = obj["packetId"] | "";

  uint8_t profileIndex = obj["profileIndex"] | 1;
  const char* phase = obj["phase"] | "Unknown";
  bool inDeepRange = obj["inDeepRange"] | false;
  bool inShallowRange = obj["inShallowRange"] | false;
  bool profilePenalty = obj["profilePenalty"] | false;
  bool recoveryReady = obj["recoveryReady"] | false;
  uint8_t successfulProfiles = obj["successfulProfiles"] | 0;
  uint8_t completedProfiles = obj["completedProfiles"] | 0;
  uint8_t holdSampleCount = obj["holdSampleCount"] | 0;

  strncpy(out.companyId, company, sizeof(out.companyId) - 1);
  out.companyId[sizeof(out.companyId) - 1] = '\0';

  strncpy(out.timestamp, ts, sizeof(out.timestamp) - 1);
  out.timestamp[sizeof(out.timestamp) - 1] = '\0';

  out.depthMeters = depth;
  out.pressureKpa = pressure;
  out.seq = seq;

  strncpy(out.packetId, packetId, sizeof(out.packetId) - 1);
  out.packetId[sizeof(out.packetId) - 1] = '\0';

  out.profileIndex = profileIndex;
  strncpy(out.phase, phase, sizeof(out.phase) - 1);
  out.phase[sizeof(out.phase) - 1] = '\0';

  out.inDeepRange = inDeepRange;
  out.inShallowRange = inShallowRange;
  out.profilePenalty = profilePenalty;
  out.recoveryReady = recoveryReady;
  out.successfulProfiles = successfulProfiles;
  out.completedProfiles = completedProfiles;
  out.holdSampleCount = holdSampleCount;

  return true;
}

bool CommsLink::persistQueueToFlash() {
  if (!fileSystemReady_) {
    return false;
  }

  DynamicJsonDocument doc(FloatConfig::QUEUE_DOC_CAPACITY);
  JsonArray arr = doc.to<JsonArray>();

  for (size_t i = 0; i < queueCount_; i++) {
    JsonObject obj = arr.add<JsonObject>();
    packetToJson(obj, packetQueue_[queueIndex(i)]);
  }

  File file = LittleFS.open(FloatConfig::QUEUE_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("[FLASH] Failed opening queue file for write.");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("[FLASH] Failed serializing queue.");
    file.close();
    return false;
  }

  file.close();
  lastPersistAt_ = millis();
  return true;
}

bool CommsLink::loadQueueFromFlash() {
  if (!fileSystemReady_) {
    return false;
  }

  if (!LittleFS.exists(FloatConfig::QUEUE_FILE)) {
    return true;
  }

  File file = LittleFS.open(FloatConfig::QUEUE_FILE, FILE_READ);
  if (!file) {
    Serial.println("[FLASH] Failed opening queue file for read.");
    return false;
  }

  DynamicJsonDocument doc(FloatConfig::QUEUE_DOC_CAPACITY);
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    Serial.printf("[FLASH] Failed parsing queue file: %s\n", err.c_str());
    return false;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("[FLASH] Queue file format invalid.");
    return false;
  }

  queueHead_ = 0;
  queueCount_ = 0;

  uint32_t maxSeq = 0;
  JsonArray arr = doc.as<JsonArray>();
  for (JsonVariant v : arr) {
    if (!v.is<JsonObject>()) {
      continue;
    }

    FloatPacket packet;
    if (!jsonToPacket(v.as<JsonObject>(), packet)) {
      continue;
    }

    if (packet.seq > maxSeq) {
      maxSeq = packet.seq;
    }

    enqueuePacket(packet);
  }

  if (maxSeq >= sequenceCounter_) {
    sequenceCounter_ = maxSeq + 1;
  }

  Serial.printf("[FLASH] Restored %u queued packet(s).\n", (unsigned int)queueCount_);
  return true;
}

void CommsLink::setRetryBackoffFailure(uint32_t nowMs) {
  nextRetryAt_ = nowMs + retryDelayMs_;

  uint32_t nextDelay = retryDelayMs_ * 2U;
  if (nextDelay > FloatConfig::RETRY_BACKOFF_MAX_MS) {
    nextDelay = FloatConfig::RETRY_BACKOFF_MAX_MS;
  }
  retryDelayMs_ = nextDelay;
}

void CommsLink::resetRetryBackoffSuccess() {
  retryDelayMs_ = FloatConfig::RETRY_BACKOFF_START_MS;
  nextRetryAt_ = 0;
}

bool CommsLink::is2xx(int code) const {
  return code >= 200 && code < 300;
}

bool CommsLink::postJson(const String& url, const String& jsonBody, int& httpCode, String& responseBody) {
  HTTPClient http;
  http.setTimeout(FloatConfig::HTTP_TIMEOUT_MS);

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

bool CommsLink::sendSinglePacket(const FloatPacket& packet) {
  StaticJsonDocument<640> doc;
  packetToJson(doc.to<JsonObject>(), packet);

  String body;
  serializeJson(doc, body);

  int code = -1;
  String response;
  bool ok = postJson(singlePacketUrl_, body, code, response);

  if (ok) {
    Serial.printf("[SEND] Single packet OK (%d) seq=%lu\n", code, (unsigned long)packet.seq);
  } else {
    Serial.printf("[SEND] Single packet FAIL (%d): %s\n", code, response.c_str());
  }

  return ok;
}

bool CommsLink::sendBatchFromQueue(size_t batchSize) {
  if (queueCount_ == 0) {
    return true;
  }

  if (batchSize > queueCount_) {
    batchSize = queueCount_;
  }

  DynamicJsonDocument doc(FloatConfig::BATCH_DOC_CAPACITY);
  JsonArray arr = doc.to<JsonArray>();

  for (size_t i = 0; i < batchSize; i++) {
    JsonObject obj = arr.add<JsonObject>();
    packetToJson(obj, packetQueue_[queueIndex(i)]);
  }

  String body;
  serializeJson(doc, body);

  int code = -1;
  String response;
  bool ok = postJson(batchPacketUrl_, body, code, response);

  if (!ok) {
    Serial.printf("[SEND] Batch FAIL (%d): %s\n", code, response.c_str());
    return false;
  }

  Serial.printf("[SEND] Batch OK (%d), sent=%u\n", code, (unsigned int)batchSize);
  dequeuePackets(batchSize);
  persistQueueToFlash();
  return true;
}

void CommsLink::connectWiFiIfNeeded(uint32_t nowMs) {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  if ((uint32_t)(nowMs - lastWiFiAttemptAt_) < FloatConfig::WIFI_RETRY_INTERVAL_MS) {
    return;
  }

  lastWiFiAttemptAt_ = nowMs;
  Serial.printf("[WIFI] Connecting to %s ...\n", FloatConfig::WIFI_SSID);
  WiFi.begin(FloatConfig::WIFI_SSID, FloatConfig::WIFI_PASS);
}

void CommsLink::printNetworkStatusIfChanged() {
  int status = WiFi.status();
  if (status == lastWiFiStatus_) {
    return;
  }

  lastWiFiStatus_ = status;
  if (status == WL_CONNECTED) {
    Serial.printf("[WIFI] Connected. IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("[WIFI] Status changed: %d\n", status);
  }
}

bool CommsLink::checkServerHealth() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  HTTPClient http;
  http.setTimeout(FloatConfig::HTTP_TIMEOUT_MS);
  if (!http.begin(healthUrl_)) {
    return false;
  }

  int code = http.GET();
  String response = http.getString();
  http.end();

  bool ok = is2xx(code);
  Serial.printf("[HEALTH] %s (%d): %s\n", ok ? "OK" : "FAIL", code, response.c_str());
  return ok;
}

void CommsLink::flushRetryQueueIfDue(uint32_t nowMs) {
  if (queueIsEmpty()) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (nextRetryAt_ != 0 && nowMs < nextRetryAt_) {
    return;
  }

  bool ok = sendBatchFromQueue(FloatConfig::MAX_BATCH_SEND);
  if (ok) {
    anyTransmitSuccess_ = true;
    resetRetryBackoffSuccess();
  } else {
    setRetryBackoffFailure(nowMs);
  }
}

void CommsLink::maybePersistQueuePeriodic(uint32_t nowMs) {
  if ((uint32_t)(nowMs - lastPersistAt_) < FloatConfig::PERSIST_INTERVAL_MS) {
    return;
  }
  persistQueueToFlash();
}
