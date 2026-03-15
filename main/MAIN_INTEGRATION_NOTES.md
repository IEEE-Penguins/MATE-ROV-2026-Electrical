# main.ino integration notes

## Purpose
This `main.ino` turns the ESP32 into the ROV micro-ROS node that sits between the Raspberry Pi ROS 2 system and the low-level hardware modules.

It is designed around the already-confirmed project contract:
- subscribe to `/rov/command` as `std_msgs/String`
- publish to `/rov/sensors` as `std_msgs/String`
- use JSON payloads on both topics
- apply a 1 second command timeout failsafe

## Implemented command contract
Incoming command JSON is expected in this shape:

```json
{
  "esc":    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
  "servo":  [0.0, 0.0, 0.0, 0.0],
  "lights": [0, 0]
}
```

### Meaning
- `esc[6]`: normalized thruster commands in the range `-1.0` to `1.0`
- `servo[4]`: reserved for future use; the code already supports them structurally
- `lights[2]`: digital commands, `0` or `1`

## Implemented sensor contract
The sketch publishes sensor JSON in this shape:

```json
{
  "depth": 0.00,
  "depth_status": "OK",
  "mpu": {
    "acc": [0.0, 0.0, 0.0],
    "gyro": [0.0, 0.0, 0.0],
    "angle": [0.0, 0.0, 0.0],
    "temp_in": 0.0,
    "ready": true
  },
  "command_alive": true
}
```

### Units
To stay aligned with the working GUI/ROS test path:
- `mpu.acc` is published in **m/s^2**
- `mpu.gyro` is published in **rad/s**
- `mpu.angle` is published in **rad**
- `depth` is published in **meters**

## Safety behavior
If no valid `/rov/command` message is received for 1 second:
- all thrusters are stopped
- lights blink until commands resume
- servos move to home position

## Important hardware notes

### 1. Built-in LED conflict
The old MPU-only micro-ROS sketch used `LED_BUILTIN = 2`.

That is **not safe** in the integrated sketch because `thruster.h` already assigns:
- `THRUSTER_3_PIN = 2` fileciteturn2file5

So the integrated `main.ino` disables the status LED by default.

### 2. Thruster hardware mapping
The integrated code uses the thruster module's existing pinout and normalized mapping logic from `thruster.h/.cpp` fileciteturn2file5 fileciteturn2file3

### 3. MPU behavior
The integrated code preserves the already-working MPU setup pattern and uses the module's output conversions:
- accelerometer is exposed in g by the module and converted to m/s^2 in `main.ino`
- gyroscope is exposed in deg/s and converted to rad/s
- roll, pitch, yaw are exposed in degrees and converted to radians

This matches the current MPU module implementation fileciteturn2file4 fileciteturn2file0

### 4. Depth behavior
Depth is included from day one using the ADS1115-based `DepthSensor` module, with status strings propagated into the sensor JSON payload fileciteturn2file1 fileciteturn2file7

### 5. Servo support
The code is structured for 4 servos, but the servo feature is disabled by default until final hardware pin assignments are confirmed.

To enable them later:
- set `ROVConfig::ENABLE_SERVOS = true`
- replace `ROVConfig::SERVO_PINS[]` with the final pin map
- replace `ROVConfig::SERVO_HOME_DEG[]` with the final home positions

The current implementation uses positional servo objects as placeholders because the final mechanical role of each servo has not yet been fixed.

### 6. Lights
The code supports two lights because the GUI command contract has `lights[2]`.

Current pin mapping in `main.ino`:
- light 1 = GPIO 25
- light 2 = GPIO 26

GPIO 25 comes from the older config header as `EXTERNAL_LIGHTS` fileciteturn2file6
GPIO 26 is a placeholder and should be updated if your real hardware uses a different second light pin.

## What to update first before field testing
1. Confirm the second light pin.
2. Confirm the final servo pin map.
3. Confirm whether any thrusters must be inverted in the `ThrusterConfig` objects.
4. Verify that the ESP32 board package has these libraries installed:
   - `micro_ros_arduino`
   - `ArduinoJson`
   - `ESP32Servo`
   - `Adafruit ADS1X15`
5. Run dry tests with thrusters disconnected or props removed.

## Output files
- `main.ino`: integrated and structured firmware entrypoint
- `MAIN_INTEGRATION_NOTES.md`: high-level explanation and bring-up notes
