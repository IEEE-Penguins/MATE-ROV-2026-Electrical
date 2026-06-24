#include "stepper.h"
#include <math.h>

FastAccelStepperEngine RovStepper::engine = FastAccelStepperEngine();
bool RovStepper::engineInitialized = false;

RovStepper::RovStepper()
    : stepper(nullptr),
      nvsName("stepper"),
      minPosition(StepperConfig::DEFAULT_MIN_POSITION),
      maxPosition(StepperConfig::DEFAULT_MAX_POSITION),
      acceleration(StepperConfig::ACCELERATION),
      isInitialized(false),
      preferencesOpen(false),
      wasRunning(false),
      needsSaving(false),
      lastStopTime(0),
      lastCommand(0.0f),
      lastDirection(0),
      onTargetReached(nullptr)
{
}

void RovStepper::initEngine()
{
    if (!engineInitialized)
    {
        engine.init();
        engineInitialized = true;
    }
}

bool RovStepper::begin(uint8_t stepPin, uint8_t dirPin, const char* memoryName)
{
    initEngine();

    nvsName = (memoryName != nullptr && memoryName[0] != '\0') ? memoryName : "stepper";

    stepper = engine.stepperConnectToPin(stepPin);
    if (stepper == nullptr)
    {
        isInitialized = false;
        return false;
    }

    stepper->setDirectionPin(dirPin);
    stepper->setSpeedInHz(StepperConfig::FIXED_MAX_SPEED_HZ);
    stepper->setAcceleration(acceleration);

    preferencesOpen = preferences.begin(nvsName.c_str(), false);

    int32_t savedPosition = minPosition;
    if (preferencesOpen)
    {
        savedPosition = preferences.getInt("pos", minPosition);
    }

    stepper->setCurrentPosition(clampPosition(savedPosition));

    isInitialized = true;
    wasRunning = false;
    needsSaving = false;
    lastCommand = 0.0f;
    lastDirection = 0;

    powerOn();
    stop();

    return true;
}

void RovStepper::setSoftwareLimits(int32_t minPos, int32_t maxPos)
{
    if (minPos <= maxPos)
    {
        minPosition = minPos;
        maxPosition = maxPos;
    }
    else
    {
        minPosition = maxPos;
        maxPosition = minPos;
    }

    if (isInitialized && stepper != nullptr)
    {
        const int32_t clamped = clampPosition(stepper->getCurrentPosition());
        stepper->setCurrentPosition(clamped);
    }
}

void RovStepper::setAcceleration(uint32_t newAcceleration)
{
    if (newAcceleration == 0)
    {
        return;
    }

    acceleration = newAcceleration;

    if (isInitialized && stepper != nullptr)
    {
        stepper->setAcceleration(acceleration);
    }
}

void RovStepper::setCurrentPosition(int32_t position, bool saveNow)
{
    if (!isInitialized || stepper == nullptr)
    {
        return;
    }

    stepper->setCurrentPosition(clampPosition(position));

    if (saveNow)
    {
        savePositionToFlash();
    }
}

void RovStepper::setOnTargetReached(ActionCallback callback)
{
    onTargetReached = callback;
}

void RovStepper::setNormalized(float command)
{
    if (!isInitialized || stepper == nullptr)
    {
        return;
    }

    command = clampNormalized(command);
    lastCommand = command;

    if (fabsf(command) <= StepperConfig::COMMAND_DEADBAND)
    {
        stop();
        return;
    }

    const int8_t direction = (command > 0.0f) ? 1 : -1;
    moveTowardLimit(direction);
}

void RovStepper::joystickMove(float command)
{
    setNormalized(command);
}

void RovStepper::moveToPercent(uint8_t percent)
{
    if (!isInitialized || stepper == nullptr)
    {
        return;
    }

    if (percent > 100)
    {
        percent = 100;
    }

    const int32_t span = maxPosition - minPosition;
    const int32_t target = minPosition + static_cast<int32_t>((static_cast<int64_t>(span) * percent) / 100);

    stepper->setSpeedInHz(StepperConfig::FIXED_MAX_SPEED_HZ);
    stepper->moveTo(clampPosition(target));

    const int32_t current = stepper->getCurrentPosition();
    lastDirection = (target > current) ? 1 : ((target < current) ? -1 : 0);
    lastCommand = static_cast<float>(percent) / 100.0f;
    wasRunning = true;
}

void RovStepper::open()
{
    moveToPercent(100);
}

void RovStepper::close()
{
    moveToPercent(0);
}

void RovStepper::stop()
{
    if (!isInitialized || stepper == nullptr)
    {
        return;
    }

    stepper->stopMove();
    lastCommand = 0.0f;
    lastDirection = 0;
}

void RovStepper::emergencyStop()
{
    if (!isInitialized || stepper == nullptr)
    {
        return;
    }

    stepper->forceStop();
    lastCommand = 0.0f;
    lastDirection = 0;
    markPositionDirty();
}

void RovStepper::update()
{
    if (!isInitialized || stepper == nullptr)
    {
        return;
    }

    const bool currentlyRunning = stepper->isRunning();

    if (wasRunning && !currentlyRunning)
    {
        wasRunning = false;
        markPositionDirty();

        if (onTargetReached)
        {
            onTargetReached(stepper->getCurrentPosition());
        }
    }

    if (needsSaving && !currentlyRunning)
    {
        if (millis() - lastStopTime >= StepperConfig::POSITION_SAVE_DELAY_MS)
        {
            savePositionToFlash();
            needsSaving = false;
        }
    }
}

void RovStepper::powerOn()
{
    if (isInitialized && stepper != nullptr)
    {
        stepper->enableOutputs();
    }
}

void RovStepper::powerOff()
{
    if (isInitialized && stepper != nullptr)
    {
        emergencyStop();
        stepper->disableOutputs();
    }
}

bool RovStepper::isReady() const
{
    return isInitialized && stepper != nullptr;
}

bool RovStepper::isRunning() const
{
    return (isInitialized && stepper != nullptr) ? stepper->isRunning() : false;
}

int32_t RovStepper::getCurrentPosition() const
{
    return (isInitialized && stepper != nullptr) ? stepper->getCurrentPosition() : 0;
}

int32_t RovStepper::getMinPosition() const
{
    return minPosition;
}

int32_t RovStepper::getMaxPosition() const
{
    return maxPosition;
}

float RovStepper::getLastCommand() const
{
    return lastCommand;
}

int8_t RovStepper::getLastDirection() const
{
    return lastDirection;
}

float RovStepper::clampNormalized(float command)
{
    if (command > 1.0f)
    {
        return 1.0f;
    }

    if (command < -1.0f)
    {
        return -1.0f;
    }

    return command;
}

int32_t RovStepper::clampPosition(int32_t position) const
{
    if (position < minPosition)
    {
        return minPosition;
    }

    if (position > maxPosition)
    {
        return maxPosition;
    }

    return position;
}

void RovStepper::moveTowardLimit(int8_t direction)
{
    if (!isInitialized || stepper == nullptr || direction == 0)
    {
        return;
    }

    const int32_t target = (direction > 0) ? maxPosition : minPosition;

    stepper->setSpeedInHz(StepperConfig::FIXED_MAX_SPEED_HZ);
    stepper->moveTo(target);

    lastDirection = direction;
    wasRunning = true;
}

void RovStepper::markPositionDirty()
{
    needsSaving = true;
    lastStopTime = millis();
}

void RovStepper::savePositionToFlash()
{
    if (isInitialized && stepper != nullptr && preferencesOpen)
    {
        preferences.putInt("pos", stepper->getCurrentPosition());
    }
}
