#include "servo.h"

// ======================================================
// ServoBase
// ======================================================
ServoBase::ServoBase(uint8_t pin, int minPulseUs, int maxPulseUs)
    : pin(pin),
      attached(false),
      minPulseUs(minPulseUs),
      maxPulseUs(maxPulseUs),
      frequencyHz(ServoConfig::SERVO_FREQUENCY_HZ),
      lastPulseUs((minPulseUs + maxPulseUs) / 2)
{
}

void ServoBase::begin()
{
    attach();
}

void ServoBase::attach()
{
    if (attached)
    {
        return;
    }

    servo.setPeriodHertz(frequencyHz);
    servo.attach(pin, minPulseUs, maxPulseUs);
    attached = true;
}

void ServoBase::detach()
{
    if (!attached)
    {
        return;
    }

    servo.detach();
    attached = false;
}

bool ServoBase::isAttached() const
{
    return attached;
}

uint8_t ServoBase::getPin() const
{
    return pin;
}

void ServoBase::setFrequencyHz(int frequencyHz)
{
    this->frequencyHz = frequencyHz;

    if (attached)
    {
        servo.setPeriodHertz(frequencyHz);
    }
}

int ServoBase::getFrequencyHz() const
{
    return frequencyHz;
}

int ServoBase::getLastPulseUs() const
{
    return lastPulseUs;
}

void ServoBase::ensureAttached()
{
    if (!attached)
    {
        attach();
    }
}

void ServoBase::writePulseUs(int pulseUs)
{
    pulseUs = constrain(pulseUs, minPulseUs, maxPulseUs);
    ensureAttached();
    servo.writeMicroseconds(pulseUs);
    lastPulseUs = pulseUs;
}

// ======================================================
// PositionalServo
// ======================================================
PositionalServo::PositionalServo(uint8_t pin,
                                 float minAngleDeg,
                                 float maxAngleDeg,
                                 float homeAngleDeg,
                                 int minPulseUs,
                                 int maxPulseUs)
    : ServoBase(pin, minPulseUs, maxPulseUs),
      minAngleDeg(minAngleDeg),
      maxAngleDeg(maxAngleDeg),
      homeAngleDeg(homeAngleDeg),
      currentAngleDeg(homeAngleDeg)
{
    if (this->minAngleDeg > this->maxAngleDeg)
    {
        float temp = this->minAngleDeg;
        this->minAngleDeg = this->maxAngleDeg;
        this->maxAngleDeg = temp;
    }

    this->homeAngleDeg = constrain(this->homeAngleDeg, this->minAngleDeg, this->maxAngleDeg);
    this->currentAngleDeg = this->homeAngleDeg;
}

void PositionalServo::begin()
{
    ServoBase::begin();
    moveHome();
}

void PositionalServo::setAngle(float angleDeg)
{
    angleDeg = constrain(angleDeg, minAngleDeg, maxAngleDeg);
    currentAngleDeg = angleDeg;

    ensureAttached();
    servo.write(static_cast<int>(currentAngleDeg));
}

void PositionalServo::stepBy(float deltaDeg)
{
    setAngle(currentAngleDeg + deltaDeg);
}

void PositionalServo::moveHome()
{
    setAngle(homeAngleDeg);
}

float PositionalServo::getAngle() const
{
    return currentAngleDeg;
}

float PositionalServo::getMinAngle() const
{
    return minAngleDeg;
}

float PositionalServo::getMaxAngle() const
{
    return maxAngleDeg;
}

float PositionalServo::getHomeAngle() const
{
    return homeAngleDeg;
}

// ======================================================
// ContinuousServo
// ======================================================
ContinuousServo::ContinuousServo(uint8_t pin,
                                 int minPulseUs,
                                 int neutralPulseUs,
                                 int maxPulseUs)
    : ServoBase(pin, minPulseUs, maxPulseUs),
      neutralPulseUs(neutralPulseUs),
      currentSpeed(0.0f)
{
    this->neutralPulseUs = constrain(this->neutralPulseUs, this->minPulseUs, this->maxPulseUs);
}

void ContinuousServo::begin()
{
    ServoBase::begin();
    stop();
}

void ContinuousServo::setSpeed(float speed)
{
    speed = constrain(speed, -1.0f, 1.0f);
    currentSpeed = speed;

    int pulseUs = neutralPulseUs;

    if (speed > 0.0f)
    {
        pulseUs = neutralPulseUs +
                  static_cast<int>((maxPulseUs - neutralPulseUs) * speed);
    }
    else if (speed < 0.0f)
    {
        pulseUs = neutralPulseUs -
                  static_cast<int>((neutralPulseUs - minPulseUs) * (-speed));
    }

    writePulseUs(pulseUs);
}

void ContinuousServo::stop()
{
    currentSpeed = 0.0f;
    writePulseUs(neutralPulseUs);
}

float ContinuousServo::getSpeed() const
{
    return currentSpeed;
}

int ContinuousServo::getNeutralPulseUs() const
{
    return neutralPulseUs;
}

int ContinuousServo::getMinPulseUs() const
{
    return minPulseUs;
}

int ContinuousServo::getMaxPulseUs() const
{
    return maxPulseUs;
}
