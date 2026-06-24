#include <Arduino.h>
#include "stepper.h"

// =====================================================
// Stepper module serial test sketch
// =====================================================
// Hardware defaults taken from the previous gripper setup.
// Change these if you wire STEP/DIR to different ESP32 pins.
static constexpr uint8_t STEPPER_STEP_PIN = 33;
static constexpr uint8_t STEPPER_DIR_PIN  = 25;

static constexpr int32_t TEST_MIN_POSITION = 0;
static constexpr int32_t TEST_MAX_POSITION = 5000;

static constexpr unsigned long STATUS_INTERVAL_MS = 500;
static constexpr size_t LINE_BUFFER_SIZE = 96;

RovStepper testStepper;

char lineBuffer[LINE_BUFFER_SIZE];
size_t lineLength = 0;
unsigned long lastStatusPrintMs = 0;

// -----------------------------------------------------
// Helpers
// -----------------------------------------------------
void printHelp()
{
    Serial.println();
    Serial.println(F("================ Stepper Serial Test ================"));
    Serial.println(F("Direct normalized input:"));
    Serial.println(F("  -1        Move toward min limit at fixed max speed"));
    Serial.println(F("  -0.3      Same direction, still fixed max speed"));
    Serial.println(F("   0        Stop"));
    Serial.println(F("   0.8      Move toward max limit at fixed max speed"));
    Serial.println();
    Serial.println(F("Commands:"));
    Serial.println(F("  n <value>       Set normalized command [-1..1]"));
    Serial.println(F("  p <0..100>      Move to percentage position"));
    Serial.println(F("  o               Open / move to 100%"));
    Serial.println(F("  c               Close / move to 0%"));
    Serial.println(F("  x               Normal stop"));
    Serial.println(F("  e               Emergency stop"));
    Serial.println(F("  z               Set current position to 0 and save"));
    Serial.println(F("  pos <steps>     Set current position and save"));
    Serial.println(F("  lim <min> <max> Set software limits"));
    Serial.println(F("  a <steps_s2>    Set acceleration"));
    Serial.println(F("  on              Enable outputs"));
    Serial.println(F("  off             Disable outputs"));
    Serial.println(F("  s               Print status"));
    Serial.println(F("  ?               Print this help"));
    Serial.println(F("====================================================="));
    Serial.println();
}

void printStatus()
{
    Serial.printf(
        "[status] ready=%d running=%d pos=%ld min=%ld max=%ld cmd=%.3f dir=%d\n",
        testStepper.isReady() ? 1 : 0,
        testStepper.isRunning() ? 1 : 0,
        static_cast<long>(testStepper.getCurrentPosition()),
        static_cast<long>(testStepper.getMinPosition()),
        static_cast<long>(testStepper.getMaxPosition()),
        testStepper.getLastCommand(),
        static_cast<int>(testStepper.getLastDirection())
    );
}

bool parseFloatStrict(const String& text, float& value)
{
    String s = text;
    s.trim();

    if (s.length() == 0)
    {
        return false;
    }

    char* endPtr = nullptr;
    value = strtof(s.c_str(), &endPtr);

    if (endPtr == s.c_str())
    {
        return false;
    }

    while (*endPtr != '\0')
    {
        if (!isspace(static_cast<unsigned char>(*endPtr)))
        {
            return false;
        }
        ++endPtr;
    }

    return true;
}

bool parseLongStrict(const String& text, long& value)
{
    String s = text;
    s.trim();

    if (s.length() == 0)
    {
        return false;
    }

    char* endPtr = nullptr;
    value = strtol(s.c_str(), &endPtr, 10);

    if (endPtr == s.c_str())
    {
        return false;
    }

    while (*endPtr != '\0')
    {
        if (!isspace(static_cast<unsigned char>(*endPtr)))
        {
            return false;
        }
        ++endPtr;
    }

    return true;
}

String getToken(const String& input, int tokenIndex)
{
    int start = 0;
    int currentToken = 0;

    while (start < input.length())
    {
        while (start < input.length() && isspace(static_cast<unsigned char>(input[start])))
        {
            ++start;
        }

        if (start >= input.length())
        {
            break;
        }

        int end = start;
        while (end < input.length() && !isspace(static_cast<unsigned char>(input[end])))
        {
            ++end;
        }

        if (currentToken == tokenIndex)
        {
            return input.substring(start, end);
        }

        start = end + 1;
        ++currentToken;
    }

    return String();
}

void handleNormalizedCommand(float command)
{
    testStepper.setNormalized(command);
    Serial.printf("[cmd] normalized=%.3f -> fixed-speed direction command sent\n", command);
    printStatus();
}

void handleLine(String input)
{
    input.trim();

    if (input.length() == 0)
    {
        return;
    }

    // Direct mode: user can type only -1..1, for example: -0.5, 0, 1
    float directCommand = 0.0f;
    if (parseFloatStrict(input, directCommand))
    {
        handleNormalizedCommand(directCommand);
        return;
    }

    String command = getToken(input, 0);
    command.toLowerCase();

    if (command == "?" || command == "h" || command == "help")
    {
        printHelp();
        return;
    }

    if (command == "s" || command == "status")
    {
        printStatus();
        return;
    }

    if (command == "n")
    {
        float value = 0.0f;
        if (!parseFloatStrict(getToken(input, 1), value))
        {
            Serial.println(F("[error] usage: n <value>, example: n -1"));
            return;
        }

        handleNormalizedCommand(value);
        return;
    }

    if (command == "p")
    {
        long percent = 0;
        if (!parseLongStrict(getToken(input, 1), percent) || percent < 0 || percent > 100)
        {
            Serial.println(F("[error] usage: p <0..100>, example: p 50"));
            return;
        }

        testStepper.moveToPercent(static_cast<uint8_t>(percent));
        Serial.printf("[cmd] moveToPercent=%ld\n", percent);
        printStatus();
        return;
    }

    if (command == "o" || command == "open")
    {
        testStepper.open();
        Serial.println(F("[cmd] open -> move to max limit"));
        printStatus();
        return;
    }

    if (command == "c" || command == "close")
    {
        testStepper.close();
        Serial.println(F("[cmd] close -> move to min limit"));
        printStatus();
        return;
    }

    if (command == "x" || command == "stop")
    {
        testStepper.stop();
        Serial.println(F("[cmd] stop"));
        printStatus();
        return;
    }

    if (command == "e" || command == "estop")
    {
        testStepper.emergencyStop();
        Serial.println(F("[cmd] emergency stop"));
        printStatus();
        return;
    }

    if (command == "z" || command == "zero")
    {
        testStepper.setCurrentPosition(0, true);
        Serial.println(F("[cmd] current position set to 0 and saved"));
        printStatus();
        return;
    }

    if (command == "pos")
    {
        long position = 0;
        if (!parseLongStrict(getToken(input, 1), position))
        {
            Serial.println(F("[error] usage: pos <steps>, example: pos 2500"));
            return;
        }

        testStepper.setCurrentPosition(static_cast<int32_t>(position), true);
        Serial.printf("[cmd] current position set to %ld and saved\n", position);
        printStatus();
        return;
    }

    if (command == "lim" || command == "limits")
    {
        long minPos = 0;
        long maxPos = 0;
        if (!parseLongStrict(getToken(input, 1), minPos) ||
            !parseLongStrict(getToken(input, 2), maxPos))
        {
            Serial.println(F("[error] usage: lim <min> <max>, example: lim 0 5000"));
            return;
        }

        testStepper.setSoftwareLimits(static_cast<int32_t>(minPos), static_cast<int32_t>(maxPos));
        Serial.printf("[cmd] software limits set to [%ld, %ld]\n", minPos, maxPos);
        printStatus();
        return;
    }

    if (command == "a" || command == "accel")
    {
        long accel = 0;
        if (!parseLongStrict(getToken(input, 1), accel) || accel <= 0)
        {
            Serial.println(F("[error] usage: a <positive steps/s^2>, example: a 500"));
            return;
        }

        testStepper.setAcceleration(static_cast<uint32_t>(accel));
        Serial.printf("[cmd] acceleration set to %ld steps/s^2\n", accel);
        printStatus();
        return;
    }

    if (command == "on")
    {
        testStepper.powerOn();
        Serial.println(F("[cmd] outputs enabled"));
        printStatus();
        return;
    }

    if (command == "off")
    {
        testStepper.powerOff();
        Serial.println(F("[cmd] outputs disabled"));
        printStatus();
        return;
    }

    Serial.print(F("[error] unknown command: "));
    Serial.println(input);
    Serial.println(F("        send '?' for help"));
}

void readSerialLines()
{
    while (Serial.available() > 0)
    {
        const char c = static_cast<char>(Serial.read());

        if (c == '\r')
        {
            continue;
        }

        if (c == '\n')
        {
            lineBuffer[lineLength] = '\0';
            handleLine(String(lineBuffer));
            lineLength = 0;
            return;
        }

        if (lineLength < LINE_BUFFER_SIZE - 1)
        {
            lineBuffer[lineLength++] = c;
        }
        else
        {
            lineLength = 0;
            Serial.println(F("[error] serial line too long; buffer cleared"));
        }
    }
}

// -----------------------------------------------------
// Arduino entry points
// -----------------------------------------------------
void setup()
{
    Serial.begin(115200);
    delay(1000);

    printHelp();

    const bool ok = testStepper.begin(STEPPER_STEP_PIN, STEPPER_DIR_PIN, "stepper_test");
    testStepper.setSoftwareLimits(TEST_MIN_POSITION, TEST_MAX_POSITION);

    testStepper.setOnTargetReached([](int32_t finalPosition) {
        Serial.printf("[event] target reached, finalPosition=%ld\n", static_cast<long>(finalPosition));
    });

    if (ok)
    {
        Serial.println(F("[setup] RovStepper initialized"));
    }
    else
    {
        Serial.println(F("[setup] ERROR: RovStepper failed to initialize"));
    }

    printStatus();
}

void loop()
{
    testStepper.update();
    readSerialLines();

    const unsigned long now = millis();
    if (now - lastStatusPrintMs >= STATUS_INTERVAL_MS)
    {
        lastStatusPrintMs = now;
        printStatus();
    }
}
