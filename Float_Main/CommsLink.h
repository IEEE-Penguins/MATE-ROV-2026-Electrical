#ifndef COMMS_LINK_H
#define COMMS_LINK_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "FloatConfig.h"
#include "FloatTypes.h"

class CommsLink {
  public:
    bool begin();
    void update(uint32_t nowMs);

    bool sendSample(float depthMeters, const MissionPacketContext& context, uint32_t nowMs);

    bool hasSuccessfulTransmit() const;
    size_t queuedCount() const;

  private:
    struct FloatPacket {
      char companyId[16];
      char timestamp[32];
      float pressureKpa;
      float depthMeters;
      uint32_t seq;
      char packetId[64];
      uint8_t profileIndex;
      char phase[24];
      bool inDeepRange;
      bool inShallowRange;
      bool profilePenalty;
      bool recoveryReady;
      uint8_t successfulProfiles;
      uint8_t completedProfiles;
      uint8_t holdSampleCount;
    };

    FloatPacket packetQueue_[FloatConfig::MAX_QUEUE_SIZE];
    size_t queueHead_ = 0;
    size_t queueCount_ = 0;

    uint32_t sequenceCounter_ = 1;
    uint32_t bootNonce_ = 0;

    uint32_t lastWiFiAttemptAt_ = 0;
    uint32_t lastPersistAt_ = 0;
    uint32_t nextRetryAt_ = 0;
    uint32_t retryDelayMs_ = FloatConfig::RETRY_BACKOFF_START_MS;

    bool healthChecked_ = false;
    bool anyTransmitSuccess_ = false;
    bool fileSystemReady_ = false;

    int lastWiFiStatus_ = -1;

    String singlePacketUrl_;
    String batchPacketUrl_;
    String healthUrl_;

    size_t queueIndex(size_t offsetFromHead) const;
    bool queueIsFull() const;
    bool queueIsEmpty() const;
    bool queueContainsPacketId(const char* packetId) const;
    bool enqueuePacket(const FloatPacket& packet);
    bool dequeuePackets(size_t n);

    String missionTimestamp(uint32_t nowMs) const;
    FloatPacket buildPacket(float depthMeters, const MissionPacketContext& context, uint32_t nowMs);

    void packetToJson(JsonObject obj, const FloatPacket& packet) const;
    bool jsonToPacket(JsonObject obj, FloatPacket& out) const;

    bool persistQueueToFlash();
    bool loadQueueFromFlash();

    void setRetryBackoffFailure(uint32_t nowMs);
    void resetRetryBackoffSuccess();

    bool is2xx(int code) const;
    bool postJson(const String& url, const String& jsonBody, int& httpCode, String& responseBody);

    bool sendSinglePacket(const FloatPacket& packet);
    bool sendBatchFromQueue(size_t batchSize);

    void connectWiFiIfNeeded(uint32_t nowMs);
    void printNetworkStatusIfChanged();
    bool checkServerHealth();

    void flushRetryQueueIfDue(uint32_t nowMs);
    void maybePersistQueuePeriodic(uint32_t nowMs);
};

#endif
