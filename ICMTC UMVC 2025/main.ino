#include <config.h>
#include <esc.h>
#include <servo.h>
#include <mpu.h>
#include <mag.h>
#include <depth.h>
#include <ArduinoJson.h>

//> Initialize objects
ESCChannel M1_2 = ESCChannel(ESC_1_2);
ESCChannel M3 = ESCChannel(ESC_3, true);
ESCChannel M4 = ESCChannel(ESC_4);
ESCChannel M5 = ESCChannel(ESC_5);
ESCChannel M6 = ESCChannel(ESC_6);

ServoDriver servo1 = ServoDriver(17, 0);
ServoDriver servo2 = ServoDriver(5, 0, true);
ServoDriver servo3 = ServoDriver(19, 0);

MPU mpu;
MAG mag;
DepthSensor depthSensor;

//> ROV's state
float thrusters_speeds[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
int servos_directions[4] = {0, 0, 0, 0};
int lights_states[2] = {0, 0};

bool led_state = true;

void setup() {
  Serial.begin(115200); 

  M1_2.begin();
  M3.begin();
  M4.begin();
  M5.begin();
  M6.begin();

  servo1.begin();
  servo2.begin(); 
  servo3.begin();

  mpu.begin();
  mag.begin();
  depthSensor.begin();

  pinMode(EXTERNAL_LIGHTS, OUTPUT);
  digitalWrite(EXTERNAL_LIGHTS, 0);

  delay(3000);
}

void loop() {

  //> Handle incoming control data
  if (Serial.available()) {
    String node2esp = Serial.readStringUntil('\n');
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, node2esp);

    if (!error) {
      JsonArray esc = doc["esc"];
      JsonArray servo = doc["servo"];
      JsonArray lights = doc["lights"];

      for (int i = 0; i < 5; i++) thrusters_speeds[i] = esc[i];
      for (int i = 0; i < 4; i++) servos_directions[i] = servo[i];
      for (int i = 0; i < 2; i++) lights_states[i] = lights[i];     

      M1_2.drive(thrusters_speeds[0]);
      M3.drive(thrusters_speeds[1]);
      M4.drive(thrusters_speeds[2]);
      M5.drive(-thrusters_speeds[3]);
      M6.drive(thrusters_speeds[4]);

      servo1.drive(servos_directions[0]);
      servo2.drive(servos_directions[1]);
      servo3.drive(servos_directions[2]);

      if(lights_states[0] == 1) {
        led_state = !led_state;
        delay(100);
      }

      digitalWrite(EXTERNAL_LIGHTS, led_state);
    }
  }

  // //> Read sensors
  MPUValues mpu_values = mpu.read();
  float yaw = mag.read(); // simple magnetometer-based yaw
  float depth_sensor_value = depthSensor.read();
  // float depth_sensor_value = 0;

  //> Send telemetry
  StaticJsonDocument<256> doc;
  JsonObject mpu_json = doc.createNestedObject("mpu");
  JsonArray acc = mpu_json.createNestedArray("acc");
  JsonArray gyro = mpu_json.createNestedArray("gyro");
  JsonArray angle = mpu_json.createNestedArray("angle");
  JsonArray mag_json = doc.createNestedArray("mag");

  acc.add(mpu_values.acc_x);
  acc.add(mpu_values.acc_y);
  acc.add(mpu_values.acc_z);

  gyro.add(mpu_values.gyro_x);
  gyro.add(mpu_values.gyro_y);
  gyro.add(mpu_values.gyro_z);

  angle.add(mpu_values.angle_x);  // roll
  angle.add(mpu_values.angle_y);  // pitch
  angle.add(yaw);                 // yaw from MAG

  doc["depth"] = depth_sensor_value;

  String output;
  serializeJson(doc, output);
  Serial.println(output);

}
