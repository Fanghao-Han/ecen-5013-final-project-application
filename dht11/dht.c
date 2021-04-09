#include <wiringPi.h>
#include <lcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef DEBUG
	//#define printf(...)
#endif

#define LCD_RS  25               //Register select pin
#define LCD_E   24               //Enable Pin
#define LCD_D4  23               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  21               //Data pin 6
#define LCD_D7  14               //Data pin 7
#define MAXTIMINGS 85
#define DHTPIN      7
#define OUTPUTPIN   29

#define DEBUG_PIN_OUT_ENABLE 1
#if DEBUG_PIN_OUT_ENABLE
#define DEBUG_PIN     0
#define DEBUG_PIN_LOW()    digitalWrite( DEBUG_PIN, LOW )
#define DEBUG_PIN_HI()     digitalWrite( DEBUG_PIN, HIGH )
#endif//DEBUG_PIN_OUT_ENABLE
uint32_t l;
#define WAIT_DHTPIN_HI()   l =50000; while (digitalRead(DHTPIN) != HIGH && (l-- > 0))//wait hi
#define WAIT_DHTPIN_LOW()  l = 50000; while (digitalRead(DHTPIN) != LOW && (l-- > 0))//wait low

int lcd;
int fan_status = 0;

void read_dht11_dat()
{
   uint8_t i;
      uint32_t t1,t2;
      int dht11_dat[5] = { 0 };
      
      int dht11_bittime[40] = { 0 };
#if DEBUG_PIN_OUT_ENABLE
      pinMode(DEBUG_PIN, OUTPUT);
      DEBUG_PIN_LOW();
#endif//DEBUG_PIN_OUT_ENABLE


//------
//start handshake
   pinMode( DHTPIN, OUTPUT );
   digitalWrite( DHTPIN, LOW );
   delay( 18 );
#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE

   digitalWrite( DHTPIN, HIGH );
   delayMicroseconds( 40 );
   /* prepare to read the pin */
   pinMode( DHTPIN, INPUT );


#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_LOW();
#endif//DEBUG_PIN_OUT_ENABLE
      WAIT_DHTPIN_HI();
//end handshake
//--------



#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE

      WAIT_DHTPIN_LOW();
      for (i = 0; i < 40; i++)
      {
             WAIT_DHTPIN_HI();
#if DEBUG_PIN_OUT_ENABLE
             DEBUG_PIN_LOW();
#endif//DEBUG_PIN_OUT_ENABLE

			
             t1 = micros();
             WAIT_DHTPIN_LOW();
             
             t2 = micros();
#if DEBUG_PIN_OUT_ENABLE
             DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE
             dht11_bittime[i] = t2 - t1;
      }
#if DEBUG_PIN_OUT_ENABLE
      DEBUG_PIN_HI();
      delay(1);
      DEBUG_PIN_LOW();
      delay(1);
      DEBUG_PIN_HI();
#endif//DEBUG_PIN_OUT_ENABLE
     
      
      for (int i = 0; i < sizeof(dht11_bittime) / sizeof(int); i++)
      {
             dht11_dat[i / 8] <<= 1;

             if (dht11_bittime[i] > 60)
             {
                    dht11_dat[i / 8] |= 1;
             }
      }
      
      if (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) )
	{
       
        if((dht11_dat[2]>23) && fan_status==0){
            digitalWrite(OUTPUTPIN,HIGH);
            fan_status = 1;
            printf("turn on\n");
        }
        else if((dht11_dat[2]<=23) && fan_status==1){     
            digitalWrite(OUTPUTPIN,LOW);
            fan_status = 0;
            printf("turn off\n");
        }
        
        printf( "Temperature = %d.%d C \n",
            dht11_dat[2], dht11_dat[3] );
  
        lcdPosition(lcd, 0, 0);      
        if(fan_status)
            lcdPrintf(lcd, "Fan Status: ON\n");
        else
            lcdPrintf(lcd, "Fan Status: OFF\n");

        lcdPosition(lcd, 0, 1);
        lcdPrintf(lcd, "Temp: %d.0 C", dht11_dat[2]);
 
	}else  {
		printf( "Data not good, skip\n" );
	}
}


int main( void )
{
   printf( "Raspberry Pi wiringPi DHT11 Temperature test program\n" );
   //wiringPiSetup
    
    wiringPiSetup();
    pinMode (OUTPUTPIN, OUTPUT);
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
    
    fan_status = 0;
    digitalWrite(OUTPUTPIN,LOW);
    printf("init: turn off\n");
    while (1)
    {
        read_dht11_dat();
        delay(1000); 
    }
   return(0);
}
