#ifndef MS5540C_H
#define MS5540C_H

#include "driver/ledc.h"

#define MOSI_PIN 23 // DIN  -- blue-white
#define MISO_PIN 19 // DOUT -- orange
#define SCK_PIN 18  // SCLK -- orange-white
#define MCLK 17      // MCLK -- blue



#include <stdint.h>

typedef unsigned char byte;


/* Since you can access Timer/Counter1 output match A output
 * only through digital pin 9 (on Arduino Uno R3, see it's pinout
 * here: https://www.circuito.io/blog/arduino-uno-pinout), 'mclk' input
 * of the ms5540c sensor should be attached to 9th digital pin.
 * For different boards check it's pinouts to find out which pin is
 * linked with OC1A AVR MCU output.
 */

namespace lib_int {

  // const int8_t MCLK = 9;


  const int8_t CONV_DUR = 35;

  const byte RST_SEQ[3] = {
      0x15,
      0x55,
      0x40
  };

  const int16_t WORDS_ACQ[8] = {
      0x1D, 0x50, // 1st word
      0x1D, 0x60, // 2nd word
      0x1D, 0x90, // 3rd word
      0x1D, 0xA0  // 4th word
  };

  enum MeasurementType {
      Temperature,
      Pressure
  };

  const byte TMP_MSR[2] = {
      0x0F,
      0x20
  };

  const byte PRS_MSR[2] = {
      0x0F,
      0x40
  };
} // namespace internal

enum SecondOrderCompensation {
    SOC = true,
    NO_SOC = false
};

namespace conv {
    float mbarTommHg(float mbar);
    float mbarToAtm(float mbar);
    float mbarToPascal(float mbar);
} // namespace conv

class ms5540c {
  public:
      ms5540c() = default;

      void begin();
      void reset() const;
      float getTemperature(SecondOrderCompensation secondOrder = SOC) const;
      float getPressure(SecondOrderCompensation secondOrder = SOC) const;

  private:
      int16_t readWord(int8_t widx) const;
      int16_t readData(lib_int::MeasurementType t) const;
      long getTempi() const;
      long getPressurei() const;
      long calcRefAndActualTempDifference() const;

      void setupSPI() const;

      long coefs[6];
};

#endif
