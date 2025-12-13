#include <mag.h>

MAG mag;

void setup() {
  Wire.begin();
  mag.begin();
  Serial.begin(115200);
}

void loop() {

  delay(50);
  MAGValues values = mag.read();
  
  Serial.print("x⁰: "); Serial.print(values.angle_x); Serial.print(", ");
  Serial.print("y⁰: "); Serial.print(values.angle_y); Serial.print(", ");
  Serial.print("z⁰: "); Serial.println(values.angle_z);

  delay(50); 

}