#include "thruster.h"
#include <math.h>

/*
    ============================================================
    ThrusterConfig
    ============================================================
*/
ThrusterConfig::ThrusterConfig(
    uint8_t thrusterPin,
    int16_t minPulse,
    int16_t neutralPulse,
    int16_t maxPulse,
    uint8_t pwmFrequency,
    float normalizedDeadband,
    bool isInverted
)
    : pin(thrusterPin),
      minUs(minPulse),
      neutralUs(neutralPulse),
      maxUs(maxPulse),
      pwmHz(pwmFrequency),
      deadband(normalizedDeadband),
      inverted(isInverted)
{
}

/*
    ============================================================
    ESCChannel
    ============================================================
*/
ESCChannel::ESCChannel(
    uint8_t pin,
    int16_t minUs,
    int16_t neutralUs,
    int16_t maxUs,
    uint8_t pwmHz
)
    : pin_(pin),
      minUs_(minUs),
      neutralUs_(neutralUs),
      maxUs_(maxUs),
      pwmHz_(pwmHz),
      lastPulse_(neutralUs),
      begun_(false),
      armed_(false)
{
}

bool ESCChannel::begin()
{
    driver_.setPeriodHertz(pwmHz_);
    driver_.attach(pin_, minUs_, maxUs_);

    begun_ = driver_.attached();
    if (begun_)
    {
        stop();
    }

    return begun_;
}

void ESCChannel::arm(uint16_t armDelayMs)
{
    if (!begun_)
    {
        return;
    }

    stop();
    delay(armDelayMs);
    armed_ = true;
}

void ESCChannel::writeMicroseconds(int16_t pulseUs)
{
    if (!begun_)
    {
        return;
    }

    pulseUs = clampPulse(pulseUs);
    driver_.writeMicroseconds(pulseUs);
    lastPulse_ = pulseUs;
}

void ESCChannel::stop()
{
    if (!begun_)
    {
        return;
    }

    driver_.writeMicroseconds(neutralUs_);
    lastPulse_ = neutralUs_;
}

bool ESCChannel::isBegun() const
{
    return begun_;
}

bool ESCChannel::isArmed() const
{
    return armed_;
}

uint8_t ESCChannel::pin() const
{
    return pin_;
}

int16_t ESCChannel::minPulse() const
{
    return minUs_;
}

int16_t ESCChannel::neutralPulse() const
{
    return neutralUs_;
}

int16_t ESCChannel::maxPulse() const
{
    return maxUs_;
}

int16_t ESCChannel::lastPulse() const
{
    return lastPulse_;
}

int16_t ESCChannel::clampPulse(int16_t pulseUs) const
{
    if (pulseUs < minUs_)
    {
        return minUs_;
    }

    if (pulseUs > maxUs_)
    {
        return maxUs_;
    }

    return pulseUs;
}

/*
    ============================================================
    Thruster
    ============================================================
*/
Thruster::Thruster(const ThrusterConfig& config)
    : config_(config),
      esc_(config.pin, config.minUs, config.neutralUs, config.maxUs, config.pwmHz),
      lastNormalized_(0.0f)
{
}

bool Thruster::begin()
{
    return esc_.begin();
}

void Thruster::arm(uint16_t armDelayMs)
{
    esc_.arm(armDelayMs);
}

void Thruster::setNormalized(float command)
{
    command = clampNormalized(command);

    if (config_.inverted)
    {
        command = -command;
    }

    if (fabs(command) < config_.deadband)
    {
        command = 0.0f;
    }

    const int16_t pulseUs = mapNormalizedToPulse(command);
    esc_.writeMicroseconds(pulseUs);
    lastNormalized_ = command;
}

void Thruster::setMicroseconds(int16_t pulseUs)
{
    esc_.writeMicroseconds(pulseUs);
}

void Thruster::stop()
{
    esc_.stop();
    lastNormalized_ = 0.0f;
}

float Thruster::lastNormalized() const
{
    return lastNormalized_;
}

int16_t Thruster::lastPulse() const
{
    return esc_.lastPulse();
}

bool Thruster::isBegun() const
{
    return esc_.isBegun();
}

bool Thruster::isArmed() const
{
    return esc_.isArmed();
}

uint8_t Thruster::pin() const
{
    return esc_.pin();
}

float Thruster::clampNormalized(float x)
{
    if (x < -1.0f)
    {
        return -1.0f;
    }

    if (x > 1.0f)
    {
        return 1.0f;
    }

    return x;
}

int16_t Thruster::mapNormalizedToPulse(float command) const
{
    if (command == 0.0f)
    {
        return config_.neutralUs;
    }

    if (command > 0.0f)
    {
        const float span = static_cast<float>(config_.maxUs - config_.neutralUs);
        return static_cast<int16_t>(config_.neutralUs + command * span);
    }

    const float span = static_cast<float>(config_.neutralUs - config_.minUs);
    return static_cast<int16_t>(config_.neutralUs + command * span);
}

/*
    ============================================================
    Thrusters
    ============================================================
*/
Thrusters::Thrusters(Thruster* thrusters[], uint8_t count)
    : count_(count)
{
    if (count_ > THRUSTER_COUNT)
    {
        count_ = THRUSTER_COUNT;
    }

    for (uint8_t i = 0; i < count_; ++i)
    {
        thrusters_[i] = thrusters[i];
    }

    for (uint8_t i = count_; i < THRUSTER_COUNT; ++i)
    {
        thrusters_[i] = nullptr;
    }
}

void Thrusters::beginAll()
{
    for (uint8_t i = 0; i < count_; ++i)
    {
        if (thrusters_[i] != nullptr)
        {
            thrusters_[i]->begin();
        }
    }
}

void Thrusters::armAll(uint16_t armDelayMs)
{
    for (uint8_t i = 0; i < count_; ++i)
    {
        if (thrusters_[i] != nullptr)
        {
            thrusters_[i]->arm(0);
        }
    }

    delay(armDelayMs);
}

void Thrusters::stopAll()
{
    for (uint8_t i = 0; i < count_; ++i)
    {
        if (thrusters_[i] != nullptr)
        {
            thrusters_[i]->stop();
        }
    }
}

void Thrusters::setAll(const float commands[], uint8_t count)
{
    const uint8_t limit = (count < count_) ? count : count_;

    for (uint8_t i = 0; i < limit; ++i)
    {
        if (thrusters_[i] != nullptr)
        {
            thrusters_[i]->setNormalized(commands[i]);
        }
    }
}

void Thrusters::set(uint8_t index, float command)
{
    if (!validIndex(index))
    {
        return;
    }

    thrusters_[index]->setNormalized(command);
}

void Thrusters::stop(uint8_t index)
{
    if (!validIndex(index))
    {
        return;
    }

    thrusters_[index]->stop();
}

Thruster* Thrusters::get(uint8_t index)
{
    if (!validIndex(index))
    {
        return nullptr;
    }

    return thrusters_[index];
}

const Thruster* Thrusters::get(uint8_t index) const
{
    if (!validIndex(index))
    {
        return nullptr;
    }

    return thrusters_[index];
}

uint8_t Thrusters::count() const
{
    return count_;
}

bool Thrusters::validIndex(uint8_t index) const
{
    return (index < count_) && (thrusters_[index] != nullptr);
}