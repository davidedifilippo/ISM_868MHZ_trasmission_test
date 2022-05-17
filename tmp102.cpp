#include <tmp102.h>

bool tmp102::begin(uint8_t deviceAddress, TwoWire &wirePort){

_address = deviceAddress; //If provided, store the I2C address from user
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  _i2cPort->beginTransmission(_address);

  uint8_t error = _i2cPort->endTransmission();

  if(error == 0)
	return true; //Device online!
  else 
  return false; //Device not attached?

}


float tmp102::getTemp()
{
  byte MSB;
  byte LSB;
  int dato_12bit; 
  float temperatura;
 
  
  Wire.beginTransmission(_address); //Say hi to the sensor. 
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_address, 2);
  Wire.endTransmission();
 
 
  MSB = Wire.read(); 

  LSB = Wire.read(); 
  
  
    dato_12bit = MSB<<4; 
 
    dato_12bit = dato_12bit | (LSB >> 4);    

    temperatura = dato_12bit*0.0625;
   
  return temperatura;
 
}