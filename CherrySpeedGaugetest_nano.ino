

// 
// GPS car speedometer, with TM1637 display, 5 speed leds , arduino nano and GY-GPS6MU2 GPS module. 1 pot dimmer 
// 

// --------- HARDWARE ---------
// Dimmer center pin connected to pin A3
// GPS TX connected to pin RX
// DATA display pin connected to pin D3
// CLK display pin connected to pin D4 
// Led 1 connected to pin D5(PWM)through 200 ohms resistor
// Led 2 connected to pin D6(PWM)through 200 ohms resistor
// Led 3 connected to pin D9(PWM)through 200 ohms resistor
// Led 4 connected to pin D10(PWM)through 200 ohms resistor
// Led 5 connected to pin D11(PWM)through 200 ohms resistor

#include <TinyGPS++.h>
#include <TM1637.h>

// Instantiate display
const int CLK = 4;
const int DIO = 3;
TM1637 tm1637(CLK, DIO);

// Instantiate the TinyGPS++ object
TinyGPSPlus gps;
static const uint32_t GPSBaud = 9600;

#define LedPin_G1 5
#define LedPin_G2 6
#define LedPin_G3 9
#define LedPin_Y4 10
#define LedPin_R5 11
#define LedPinTest 8
#define DimmerPin A3 

int              ledpins[5];
unsigned char    led_status[5];
float            led_speed_threshold[5];
unsigned char    enable_speed_auto_rampup = 0;
int              dim_command;
int              dim_command_prev;


uint8_t          speed=0;
uint32_t         satellites;
float            latitude,longitude,speed_gps,time_gps,speed_gps_test;
int              year , month , date, hour , minute , second;
String           date_str , time_str , lat_str , lng_str;

void setup()
{
    Serial.begin(GPSBaud);

    pinMode(LedPin_G1, OUTPUT);
    pinMode(LedPin_G2, OUTPUT);
    pinMode(LedPin_G3, OUTPUT);
    pinMode(LedPin_Y4, OUTPUT);
    pinMode(LedPin_R5, OUTPUT);
    pinMode(DimmerPin, INPUT);
    pinMode(LedPinTest, OUTPUT);

    ledpins[0] = LedPin_G1; 
    ledpins[1] = LedPin_G2;
    ledpins[2] = LedPin_G3;
    ledpins[3] = LedPin_Y4;
    ledpins[4] = LedPin_R5;
 
    led_speed_threshold[0] = 39;   
    led_speed_threshold[1] = 63;   
    led_speed_threshold[2] = 83;   
    led_speed_threshold[3] = 99;   
    led_speed_threshold[4] = 118;   

// Initialize display, make it illuminate 128 bright
    tm1637.init();
    tm1637.set(BRIGHTEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;  
}

void loop()
{
    unsigned char i_led = 0;

    if ( enable_speed_auto_rampup )  // Autorampump test for debug purposes
      { 
       speed_gps_test += 1.0;
       if (speed_gps_test > 130.0) 
        {
        speed_gps_test = 0.0;
        }
      delay(100);
      displayInfo();
      }
    else   // Normal mode
      { 
       while ( Serial.available() > 0 ) gps.encode(Serial.read());
      
       if(gps.speed.age() < 3000.0f)       
         {
          if ( gps.speed.isUpdated() )
            displayInfo();
         }
       else
         { 
          tm1637.displayStr("nGPS");
         }

       DimmerControl();

       UpdateLedStatus();
             
      }      
}

// This sketch displays information every time a new sentence is correctly encoded.

   
//      if (gps.date.isValid())
//      {
//        date_str = "";
//        date = gps.date.day();
//        month = gps.date.month();
//        year = gps.date.year();
//      }
 

void displayInfo()
{
  if ( enable_speed_auto_rampup )
    {
     speed_gps = speed_gps_test;
     tm1637.displayNum(speed_gps);
    }
  else 
    {
//     if(!(gps.speed.isValid()))
//       { 
//        tm1637.displayStr("nGPS");
//       }
//     else 
//       {
//      speed_gps = gps.speed.kmph();

      DecodeGPSinfo();
           
      if ( speed_gps < 10 ) // display time under 10 KM-h
        {
         tm1637.displayNum(time_gps); 
         tm1637.point(POINT_ON);
        } 
      else
        {
         tm1637.displayNum(speed_gps); 
        } 
     }    
   } 

void DimmerControl()
{
 float dimmer_input;
 
 dim_command_prev = dim_command;
 dimmer_input = (analogRead(DimmerPin))/ 1024.0f;
 dim_command = (int) (dimmer_input * 10);
 dim_command = (int) (dim_command * 22);
 tm1637.set((int)((dim_command / 225.0f )*7.0f)) ;
}

void UpdateLedStatus()
{
 int i_led = 0;
 unsigned char led_status_prev[5];
 
 for ( i_led = 0; i_led < 5; i_led++ )
   {
    led_status_prev[i_led] = led_status[i_led];

    led_status[i_led] = speed_gps > led_speed_threshold[i_led];

    if ( dim_command != dim_command_prev || led_status_prev[i_led] != led_status[i_led] )
      {
       if ( led_status[i_led] )
         {
          analogWrite(ledpins[i_led], dim_command);
         }
      else
        {
         analogWrite(ledpins[i_led], 0);
        } 
     }
   }
}

void DecodeGPSinfo()
  {
   int gps_hour;
   speed_gps = gps.time.second();

// GPS hour is in UTC, put to EST (-4 hrs)
   gps_hour = gps.time.hour() - 4;  
   if ( gps_hour < 0 ) gps_hour = gps_hour + 12;

   time_gps = (int)(gps_hour * 100.0f + gps.time.minute());
  }
