

#include <software_i2c.h>
#include <Arduino.h>

#define DRIVE_PIN_LOW(x) pinMode(x,OUTPUT)
#define FLOAT_PIN_HIGH(x) pinMode(x,INPUT)
#define READ_PIN(x) digitalRead(x)

#define CLOCK_STRETCH_TIMEOUT clock_delay * 10
#define CLOCK_STRETCH_DELAY() delayMicroseconds(1)

#define CLOCK_DELAY() delayMicroseconds(clock_delay) 

// Test macro uncomment the 'x' to run the code in test mode.
#define TEST(x) //x


Software_I2C::Software_I2C(char data_pin,
                           char clock_pin,
                           unsigned int clock_period)
{
  _data_pin   = data_pin;
  _clock_pin  = clock_pin;
  clock_delay = clock_period >> 1;

  _release_control();
  
  /* Set the outputs to low so that when the mode is changed to output
     the value on the wire will be low.
   */
  digitalWrite(_data_pin, I2C_LOW);
  digitalWrite(_clock_pin, I2C_LOW);
  
}

char Software_I2C::status()
{
  int stretch_timer = 0;
  
  _release_control();
  
  while (READ_PIN(_clock_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      return I2C_BUSY;
    }
  }
  
  stretch_timer = 0;
  
  while (READ_PIN(_data_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      return I2C_BUSY;
    }
  }
  
  return I2C_AVAILABLE;
}

/* Read a byte of information from a slave device and reply with the 
   appropriate acknowledgement.
 */
unsigned char Software_I2C::read(char* error, char ack)
{
  unsigned char input = 0;
  char input_bit = 0;
  int bit_looper;
  
  /* Loop through sending the bits of the byte. */

  TEST(Serial.print("r"));
  
  for (bit_looper = 0; bit_looper < 8; bit_looper++)
  {

    input_bit = _read_bit(error);
    
    if (*error != I2C_NO_ERROR)
    {
      _release_control();
      return 0;
    }

    input = input << 1;
    input = input | (input_bit & 0x1);

  }
  
  /* Send the Acknowledge */
  
  *error = _write_bit(ack);
  
  TEST(Serial.println());
  
  return input;
  
}

/* Read a bit of information from a slave device. The slave device will
   output the new data on the rising edge of the clock pulse. The slave
   device will hold the clock signal low until it is ready to output the
   data.
 */

char Software_I2C::_read_bit(char* error)
{
  char bit = 0;
  int stretch_timer = 0;
  *error = I2C_NO_ERROR;
  
  /* Give up control of the data pin. */

  FLOAT_PIN_HIGH(_data_pin);

  CLOCK_DELAY();
  
  /* Give up control of the clock pin, this allows for the clock stretch
     check. It also causes the rising edge of the clock signal.
   */

  FLOAT_PIN_HIGH(_clock_pin);

  while (READ_PIN(_clock_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      *error = I2C_READ_CLOCK_TIMEOUT;
      _release_control();
      return 0;
    }
  }

  /* Since the clock is high the slave must have released it and the data
     is now valid. Read it and move on.
   */

  bit = READ_PIN(_data_pin);
  
  if (bit > 0) 
  {
    TEST(Serial.print("rb1"));
  }
  else 
  {
    TEST(Serial.print("rb0"));
  }

  CLOCK_DELAY();
  
  /* Reassume control of the clock pin and return the result from the remote
     device.
   */

  DRIVE_PIN_LOW(_clock_pin);
  
  return bit;

}


/* Output a byte of information.
 */

char Software_I2C::write(unsigned char output)
{
  char error;
  int bit_looper;
  char write_status;
  
  TEST(Serial.print("w"));
  for (bit_looper = 0; bit_looper < 8; bit_looper++)
  {
    write_status = _write_bit(output & 0x80);
    if (write_status == 0)
    {
      output = output << 1;
    }
    else
    {
      _release_control();
      return write_status;
    }
  }
  
  
  FLOAT_PIN_HIGH(_data_pin);
  
  TEST(Serial.println());
  
  return _read_bit(&error);
}


/* Output a single bit of information.
 */

char Software_I2C::_write_bit(unsigned char bit)
{
  int stretch_timer = 0;
  
  if (bit > 0)
  {
    FLOAT_PIN_HIGH(_data_pin);
    
    TEST(Serial.print("wb1"));
  }
  else 
  {
    DRIVE_PIN_LOW(_data_pin);
    
    TEST(Serial.print("wb0"));
  }

  CLOCK_DELAY();
  
  /* Give up control of the clock pin, this allows for the clock stretch
     check. It will also cause the rising edge of the clock signal.
   */
  
  FLOAT_PIN_HIGH(_clock_pin);
  
  while (READ_PIN(_clock_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    TEST(Serial.print("S"));
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      _release_control();
      TEST(Serial.println("Strech1"));
      return I2C_WRITE_CLOCK_PIN_TIMEOUT;
    }
  }
  
  /* Check to make sure bit and the data line actually match. This 
     ensures that the bus is being driven by this device and not
     someone else. This check can only be performed if this device
     was trying to output a HIGH (1).
   */
  
  if ((bit == 1) && (READ_PIN(_data_pin) == 0))
  {
    _release_control();
    TEST(Serial.println("Arbitration_Lost"));
    return I2C_WRITE_ARBITRATION_LOST;
  }
  
  CLOCK_DELAY();
  
  DRIVE_PIN_LOW(_clock_pin);
  
  return I2C_NO_ERROR;
  
}


/* Allow the clock pin to be high and if it is take the data pin low.
   This will signal to the slave devices that something is about to 
   start.
 */

char Software_I2C::start_i2c()
{
  int stretch_timer = 0;

  _release_control();
  
  while (READ_PIN(_clock_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      return I2C_START_CLOCK_PIN_ERROR;
    }
  }
  
  stretch_timer = 0;
  
  while (READ_PIN(_data_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      return I2C_START_DATA_PIN_ERROR;
    }
  }
  
  DRIVE_PIN_LOW(_data_pin);
  CLOCK_DELAY();
  DRIVE_PIN_LOW(_clock_pin);
  
  return I2C_NO_ERROR;

}


/* Release both the data pin and the clock pin. This allows another device
   to then control the bus.
 */

char Software_I2C::stop_i2c()
{
  int stretch_timer = 0;
  
  FLOAT_PIN_HIGH(_clock_pin);
  
  while (READ_PIN(_clock_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      return I2C_STOP_CLOCK_ERROR;
    }
  }
  
  CLOCK_DELAY();
  FLOAT_PIN_HIGH(_data_pin);
  
  stretch_timer = 0;
  
  while (READ_PIN(_data_pin) == I2C_LOW)
  {
    CLOCK_STRETCH_DELAY();
    stretch_timer++;
    
    if (stretch_timer > CLOCK_STRETCH_TIMEOUT)
    {
      return I2C_STOP_DATA_ERROR;
    }
  }
  
  return I2C_NO_ERROR;
}

void Software_I2C::_release_control()
{
  FLOAT_PIN_HIGH(_clock_pin);
  FLOAT_PIN_HIGH(_data_pin);
}