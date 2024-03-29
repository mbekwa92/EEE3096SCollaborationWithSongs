/*
 * BinClock.c
 * Jarrod Olivier
 * Modified for EEE3095S/3096S by Keegan Crankshaw
 * August 2019
 * 
 * <STUDNUM_1> <STUDNUM_2>
 * Date
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance

int HH,MM,SS;

void initGPIO(void){
	/* 
	 * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
	 * You can also use "gpio readall" in the command line to get the pins
	 * Note: wiringPi does not use GPIO or board pin numbers (unless specifically set to that mode)
	 */
	printf("Setting up\n");
	wiringPiSetup(); //This is the default mode. If you want to change pinouts, be aware
	
	RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC
	
	//Set up the LEDS
	for(int i=0; i < sizeof(LEDS)/sizeof(LEDS[0]); i++){
	    pinMode(LEDS[i], OUTPUT);
	}
	
	//Set Up the Seconds LED for PWM
	//Write your logic here
	pinMode(SECS,PWM_OUTPUT);
	
	
	printf("LEDS done\n");
	
	//Set up the Buttons
	for(int j=0; j < sizeof(BTNS)/sizeof(BTNS[0]); j++){
		pinMode(BTNS[j], INPUT);
		pullUpDnControl(BTNS[j], PUD_UP);
	}
	
	//Attach interrupts to Buttons
	//Write your logic here
	wiringPiISR(BTNS[0],INT_EDGE_FALLING,hourInc);
	wiringPiISR(BTNS[1],INT_EDGE_FALLING,minInc);
		
	printf("BTNS done\n");
	printf("Setup done\n");
}


/*
 * The main function
 * This function is called, and calls all relevant functions we've written
 */
int main(void){
	initGPIO();

	//Set random time (3:04PM)
	//You can comment this file out later
	wiringPiI2CWriteReg8(RTC, HOUR, 0x13+TIMEZONE);
	wiringPiI2CWriteReg8(RTC, MIN, 0x4);
	wiringPiI2CWriteReg8(RTC, SEC, 0x00);
	
	// Repeat this until we shut down
	for (;;){
		//Fetch the time from the RTC
		
		//Write your logic here
		
		//Function calls to toggle LEDs
		//Write your logic here
		lightHours(3);
		lightHours(10);
		lightHours(12);
		lightHours(15);
		lightHours(8);
		lightHours(2);
		
		// Print out the time we have stored on our RTC
		printf("The current time is: %x:%x:%x\n", hours, mins, secs);
		break;
		
		//using a delay to make our program "less CPU hungry"
		delay(1000); //milliseconds
	}
	return 0;
}

/*
 * Change the hour format to 12 hours
 */
int hFormat(int hours){
	/*formats to 12h*/
	if (hours >= 24){
		hours = 0;
	}
	else if (hours > 12){
		hours -= 12;
	}
	return (int)hours;
}

/*
 * Turns on corresponding LED's for hours
 */
void lightHours(int units){
	
	//check for validity of units
	if(units > 15 || units < 0)
 		return;
 		
 	
 	printf("%d = ",units);	
 	// convert units to binary
 	// 	
 	int k;
 	int binary[4];	
 	for(int bitno = 3;bitno >= 0 ;--bitno){
 		
 	    //shift logical right		
 	    k = units >> bitno;
 	    
 	    //remainder 1 ? 0
 	    if(k&1){
 	    	printf("1");
 	    	binary[3-bitno] = 1;
	    }
 	    else{
 	    	printf("0");
 	    	binary[3-bitno] = 0;
 	    }    	
 	}
 		
	
	// output to appropriate first 4 
	// LEDS the corresponding bit number
	for(int i = 0;i < 4 ;++i){
	
	  digitalWrite(LEDS[i],binary[i]);
		
	
	}
	printf("\n");
		
}


/* Method called immediately a falling edge is 
 * detected on the hours button (pin 5)
 */
void hourBtnPressed(void){
  printf("%s\n","Hour button is Pressed.");	
		

}



/* Interrupt handler for handling a falling edge signal
 * on the minute button pin (pin 30)
 */
 
void minuteBtnPressed(void){
  printf("%s\n","Minute Button is Pressed.");

}


/*
 * Turn on the Minute LEDs
 */
void lightMins(int units){
	
	//max number that can be represented with 6 bits	
	if(units > 63 || units < 0)
		return;
		
	int rem = 0;
	
	int minutesBits[6];   //minutesBits.. an array to represent the states corresponding to an LED
	
	for(int bitpos = 5; bitpos >=0 ;--bitpos){
	
	 rem = units>>bitpos;
	 
	   if(rem&1){
	     //rem&1 == 1, 
	     // set MSB to 1.. then MSB-1,MSB-2... LSB
	   
	     minutesBits[5-bitpos] = 1;
	 
	   }else{
	     //rem&1 == 0
	   
	   
	     minutesBits[5-bitpos] = 0;
	   }
	
	
	}
	
	//start writing from 4th LED+ ...
	// the value represented by minuteBits[ithposition]
		
	for(int i =0 ;i < 6;++i){
		
	  digitalWrite(LEDS[4+i],minutesBits[i]);
	
	}
	

}

/*
 * PWM on the Seconds LED
 * The LED should have 60 brightness levels
 * The LED should be "off" at 0 seconds, and fully bright at 59 seconds
 */
void secPWM(int units){
	
	for(int i =0; i < 60;++i){
	
	   if(i == 60){
	   
	   	//0 seconds -- LED OFF
	   	digitalWrite(SECS,LOW);
	   
	   }else{
	     
	     pwmWrite(SECS,i); //output analog voltage
	     delay(1000); //delay for 1 second
	   }
	   
	}
	
}

/*
 * hexCompensation
 * This function may not be necessary if you use bit-shifting rather than decimal checking for writing out time values
 */
int hexCompensation(int units){
	/*Convert HEX or BCD value to DEC where 0x45 == 0d45 
	  This was created as the lighXXX functions which determine what GPIO pin to set HIGH/LOW
	  perform operations which work in base10 and not base16 (incorrect logic) 
	*/
	int unitsU = units%0x10;

	if (units >= 0x50){
		units = 50 + unitsU;
	}
	else if (units >= 0x40){
		units = 40 + unitsU;
	}
	else if (units >= 0x30){
		units = 30 + unitsU;
	}
	else if (units >= 0x20){
		units = 20 + unitsU;
	}
	else if (units >= 0x10){
		units = 10 + unitsU;
	}
	return units;
}


/*
 * decCompensation
 * This function "undoes" hexCompensation in order to write the correct base 16 value through I2C
 */
int decCompensation(int units){
	int unitsU = units%10;

	if (units >= 50){
		units = 0x50 + unitsU;
	}
	else if (units >= 40){
		units = 0x40 + unitsU;
	}
	else if (units >= 30){
		units = 0x30 + unitsU;
	}
	else if (units >= 20){
		units = 0x20 + unitsU;
	}
	else if (units >= 10){
		units = 0x10 + unitsU;
	}
	return units;
}


/*
 * hourInc
 * Fetch the hour value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 23 hours in a day
 * Software Debouncing should be used
 */
void hourInc(void){
	//Debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 1 triggered, %x\n", hours);
		
		//Fetch RTC Time
		hours = getHours();
		
		//Increase hours by 1 ensuring not to overflow
		hours++;
		hours = hFormat(hours);
		
		hours = decCompensation(hours);
		
		//Write hours back to the RTC
		wiringPiI2CWriteReg8(RTC,HOUR,hours);
		
	}
	lastInterruptTime = interruptTime;
}

/* 
 * minInc
 * Fetch the minute value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 60 minutes in an hour
 * Software Debouncing should be used
 */
void minInc(void){
	
	//debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 2 triggered, %x\n", mins);
		
		//Fetch RTC time
		mins = getMins();
		
		
		//Increase minutes by 1, ensuring not to overflow
		if(mins == 59)
			mins = 0;
		mins++;
		
		mins = decCompensation(mins);
			
		//Write minutes back to the RTC
		wiringPiI2CWriteReg8(RTC,MIN,mins);
		
	}
	lastInterruptTime = interruptTime;
}

//This interrupt will fetch current time from another script and write it to the clock registers
//This functions will toggle a flag that is checked in main
void toggleTime(void){
	
	//debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		HH = getHours();
		MM = getMins();
		SS = getSecs();

		HH = hFormat(HH);
		HH = decCompensation(HH);
		wiringPiI2CWriteReg8(RTC, HOUR, HH);

		MM = decCompensation(MM);
		wiringPiI2CWriteReg8(RTC, MIN, MM);

		SS = decCompensation(SS);
		wiringPiI2CWriteReg8(RTC, SEC, 0b10000000+SS);

	}
	lastInterruptTime = interruptTime;
}
