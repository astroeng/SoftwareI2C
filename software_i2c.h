#ifndef SOFTWARE_I2C_H
#define SOFTWARE_I2C_H

#define I2C_HIGH 1
#define I2C_LOW  0

#define I2C_WRITE 0x00
#define I2C_READ  0x01

#define I2C_AVAILABLE               10
#define I2C_BUSY                    11

/* Errors */
#define I2C_NO_ERROR                 0
#define I2C_START_CLOCK_PIN_ERROR   -1
#define I2C_START_DATA_PIN_ERROR    -2
#define I2C_WRITE_CLOCK_PIN_TIMEOUT -3
#define I2C_WRITE_ARBITRATION_LOST  -4
#define I2C_READ_CLOCK_TIMEOUT      -5
#define I2C_STOP_CLOCK_ERROR        -6
#define I2C_STOP_DATA_ERROR         -7


class Software_I2C 
{
public:
  Software_I2C(char data_pin,
               char clock_pin,
               unsigned int clock_period);
  
  char status();
  unsigned char read(char* error, char ack = I2C_LOW);
  char write(unsigned char output);

  char start_i2c();
  char stop_i2c();

private:
  char _data_pin;
  char _clock_pin;
  unsigned int clock_delay;
  
  char _read_bit(char* error);
  char _write_bit(unsigned char);
  void _release_control();

};

#endif