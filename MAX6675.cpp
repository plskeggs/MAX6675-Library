/*
  MAX6675.cpp - Library for reading temperature from a MAX6675.
  Created by Ryan McLaughlin <ryanjmclaughlin@gmail.com>
*/

#include <Arduino.h>
#include <MAX6675.h>

MAX6675::MAX6675(int CS_pin, int SO_pin, int SCK_pin, int units, float error)
{
  pinMode(CS_pin, OUTPUT);
  pinMode(SO_pin, INPUT);
  pinMode(SCK_pin, OUTPUT);
  
  digitalWrite(CS_pin, HIGH);
  
  _CS_pin = CS_pin;
  _SO_pin = SO_pin;
  _SCK_pin = SCK_pin;
  _units = units;
  _error = error;
}

float MAX6675::read_temp(int samples)
{
  int value = 0;
  int error_tc = 0;
  float temp = 0;
	
  for (int i=samples; i>0; i--){
	
    /* 
	  Initiate a temperature conversion. According to MAX's tech notes FAQ's 
	  for the chip, Line going high initiates a conversion, which means, we 
	  need to clock the chip low to high to initiate the conversion, then wait 
	  for the conversion to be complete before trying to read the data from 
	  the chip.
	*/
    digitalWrite(_CS_pin,LOW);				 
    delay(2);
    digitalWrite(_CS_pin,HIGH);
	delay(220);
	
	/* Read the chip and return the raw temperature value */
	
	/*
	  Bring CS pin low to allow us to read the data from
	  the conversion process
	*/
	digitalWrite(_CS_pin,LOW);
	
  /* Cycle the clock for dummy bit 15 */
  digitalWrite(_SCK_pin,HIGH);
	delay(1);
  digitalWrite(_SCK_pin,LOW);

 	/* 
	  Read bits 14-3 from MAX6675 for the Temp. Loop for each bit reading 
	  the value and storing the final value in 'temp' 
	*/
  for (int j=11; j>=0; j--){
		digitalWrite(_SCK_pin,HIGH);
		value += digitalRead(_SO_pin) << j;
		digitalWrite(_SCK_pin,LOW);
  }
  
	/* Read the TC Input inp to check for TC Errors */
	digitalWrite(_SCK_pin,HIGH);
	error_tc = digitalRead(_SO_pin);
	digitalWrite(_SCK_pin,LOW);
  
	/* 
	  Read the last two bits from the chip, faliure to do so will result 
	  in erratic readings from the chip. 
	*/
	for (int j=1; j>=0; j--) {
		digitalWrite(_SCK_pin,HIGH);
		delay(1);
		digitalWrite(_SCK_pin,LOW);
	}
	// Disable Device
	digitalWrite(_CS_pin, HIGH);
  }
  value = value/samples;  // Divide the value by the number of samples to get the average
  
  /* 
     Keep in mind that the temp that was just read is on the digital scale
     from 0˚C to 1023.75˚C at a resolution of 2^12.  We now need to convert
     to an actual readable temperature (this drove me nuts until I figured 
     this out!).  Now multiply by 0.25.  I tried to avoid float math but
     it is tough to do a good conversion to ˚F.  THe final value is converted 
     to an int and returned at x10 power.
     
	  2 = temp in deg F
	  1 = temp in deg C
	  0 = raw chip value 0-4095
   */
   
  value = value + _error;						// Insert the calibration error value
  
	if(_units == 2) {
		temp = (value*0.25) * 9.0/5.0 + 32.0;
	} else if(_units == 1) {
		temp = (value*0.25);
	} else {
		temp = value;
  }
  
	/* Output negative of CS_pin if there is a TC error, otherwise return 'temp' */
  if(error_tc != 0) {
		return -_CS_pin; 
  } else { 
    return temp; 
  }
}
