#include <Wire.h>
#include <QMC5883LCompass.h>
#include "mag.h"

QMC5883LCompass compass;
float filteredHeading = 0.0;

// ↑ Increased alpha for faster responsiveness
const float alpha = 0.5;

float angleDiff(float a, float b) {
  float diff = a - b;
  while (diff > 180) diff -= 360;
  while (diff < -180) diff += 360;
  return diff;
}

void MAG::begin() {
  Wire.begin();
  compass.init();
}

float MAG::read() {
  compass.read();
  float rawHeading = compass.getAzimuth();
  float diff = angleDiff(rawHeading, filteredHeading);
  filteredHeading += alpha * diff;

  if (filteredHeading < 0) filteredHeading += 360;
  if (filteredHeading >= 360) filteredHeading -= 360;

  return filteredHeading;
}
