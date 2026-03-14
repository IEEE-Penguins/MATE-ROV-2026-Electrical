#include "thruster.h"

/*
    Per-thruster configuration
    Set inverted = true for thrusters that are mounted opposite
*/
ThrusterConfig t1Cfg(THRUSTER_1_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t2Cfg(THRUSTER_2_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t3Cfg(THRUSTER_3_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t4Cfg(THRUSTER_4_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t5Cfg(THRUSTER_5_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t6Cfg(THRUSTER_6_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);

/*
    Single thrusters
*/
Thruster thruster1(t1Cfg);
Thruster thruster2(t2Cfg);
Thruster thruster3(t3Cfg);
Thruster thruster4(t4Cfg);
Thruster thruster5(t5Cfg);
Thruster thruster6(t6Cfg);

/*
    Bank of thrusters
*/
Thruster* thrusterPtrs[THRUSTER_COUNT] =
{
    &thruster1,
    &thruster2,
    &thruster3,
    &thruster4,
    &thruster5,
    &thruster6
};

Thrusters thrusters(thrusterPtrs, THRUSTER_COUNT);

void printThrusterState(const char* label, Thruster& thruster, uint8_t index)
{
    Serial.print(label);
    Serial.print(" | T");
    Serial.print(index + 1);
    Serial.print(" pin=");
    Serial.print(thruster.pin());
    Serial.print(" pulse=");
    Serial.print(thruster.lastPulse());
    Serial.print(" norm=");
    Serial.println(thruster.lastNormalized(), 3);
}

void testSingleThrusters()
{
    Serial.println("\n=== Single Thruster Test ===");

    for (uint8_t i = 0; i < thrusters.count(); ++i)
    {
        Thruster* t = thrusters.get(i);
        if (t == nullptr)
        {
            continue;
        }

        Serial.print("Testing thruster ");
        Serial.println(i + 1);

        t->setNormalized(0.25f);
        printThrusterState("Forward low", *t, i);
        delay(2000);

        t->stop();
        printThrusterState("Stop", *t, i);
        delay(1500);

        t->setNormalized(-0.25f);
        printThrusterState("Reverse low", *t, i);
        delay(2000);

        t->stop();
        printThrusterState("Stop", *t, i);
        delay(2000);
    }
}

void testAllThrustersTogether()
{
    Serial.println("\n=== All Thrusters Test ===");

    float forwardCmds[THRUSTER_COUNT] = {0.30f, 0.30f, 0.30f, 0.30f, 0.30f, 0.30f};
    thrusters.setAll(forwardCmds, THRUSTER_COUNT);
    Serial.println("All thrusters forward 0.30");
    delay(3000);

    thrusters.stopAll();
    Serial.println("All thrusters stop");
    delay(2000);

    float reverseCmds[THRUSTER_COUNT] = {-0.30f, -0.30f, -0.30f, -0.30f, -0.30f, -0.30f};
    thrusters.setAll(reverseCmds, THRUSTER_COUNT);
    Serial.println("All thrusters reverse -0.30");
    delay(3000);

    thrusters.stopAll();
    Serial.println("All thrusters stop");
    delay(3000);
}

void testRawMicroseconds()
{
    Serial.println("\n=== Raw Microseconds Test ===");

    thruster1.setMicroseconds(1600);
    printThrusterState("T1 raw 1600", thruster1, 0);
    delay(2500);

    thruster1.stop();
    printThrusterState("T1 stop", thruster1, 0);
    delay(1500);

    thruster1.setMicroseconds(1400);
    printThrusterState("T1 raw 1400", thruster1, 0);
    delay(2500);

    thruster1.stop();
    printThrusterState("T1 stop", thruster1, 0);
    delay(2500);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Thrusters Module Test Start ===");

    thrusters.beginAll();
    Serial.println("All thrusters begun");

    thrusters.armAll();
    Serial.println("All thrusters armed");
    Serial.println("Make sure thrusters are restrained before testing");
}

void loop()
{
    // testSingleThrusters();
    testAllThrustersTogether();
    // testRawMicroseconds();
}