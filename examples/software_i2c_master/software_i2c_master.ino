#include <software_i2c.h>

#define I2C_CLOCK 5
#define I2C_DATA  6

Software_I2C i2c_bus(I2C_DATA, 
                     I2C_CLOCK, 
                     1000);

#define I2C_ADDRESS 0x54 // (pre shifted by one bit, really 0x2A)


void setup() {
  Serial.begin(9600);

}

void loop() {

  unsigned char value;
  char error;

  /* Write 0xAA to the device. */
  
  i2c_bus.start_i2c();
  i2c_bus.write(I2C_ADDRESS | I2C_WRITE);
  i2c_bus.write(0xAA);
  i2c_bus.stop_i2c();

  delay(10);

  /* Read a value back from the device. */

  i2c_bus.start_i2c();
  i2c_bus.write(I2C_ADDRESS | I2C_READ);
  value = i2c_bus.read(&error);
  i2c_bus.stop_i2c();

  Serial.println(value);

}
