#include <Arduino.h>
#include <Wire.h>

class tmp102{

public:

bool begin(uint8_t deviceAddress = 0x48, TwoWire &wirePort = Wire);
float getTemp();

private:

		TwoWire *_i2cPort; //The generic connection to user's chosen I2C hardware
		//Variables
		int _address; // Address of Temperature sensor (0x48,0x49,0x4A,0x4B)

};