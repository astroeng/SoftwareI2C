#ifndef SOFTWARE_I2C_H
#define SOFTWARE_I2C_H

#define AVAILABLE               10
#define BUSY                    11

/* Errors */
#define NO_ERROR                 0
#define START_CLOCK_PIN_ERROR   -1
#define START_DATA_PIN_ERROR    -2
#define WRITE_CLOCK_PIN_TIMEOUT -3
#define WRITE_ARBITRATION_LOST  -4
#define READ_CLOCK_TIMEOUT      -5
#define STOP_CLOCK_ERROR        -6
#define STOP_DATA_ERROR         -7


class Software_I2C 
{
public:
  Software_I2C(unsigned char address,
               char data_pin,
               char clock_pin,
               unsigned int clock_period);
  
  char status();
  unsigned char read(char* error);
  char write(unsigned char output);
  
  char start_i2c();
  char stop_i2c();

private:
  unsigned char address;
  char _data_pin;
  char _clock_pin;
  unsigned int clock_delay;
  
  char _read_bit(char* error);
  char _write_bit(unsigned char);
  void _release_control();

};

#endif