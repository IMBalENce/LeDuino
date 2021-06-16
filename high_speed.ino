/* Arduino highspeed photo controller (beta) 0.1a
 * 
 * ##### trigger by phooto sensor #####
 *
 * Benjaminxuu 2012
 *
 *
 * 1. Program itself
 *    This program is used for the photos of water droplets relased from a solenoid valve. It has three basic functions:
 *
 *    Camera trigger with a delay timer
 *    power out to laser pointer with the combination of input photocell signal
 *    signal output to relay to control solenoid valve
 *    
 * 2. Pin in use:
 *
 *
 *
 * 3. Interface design:
 *
 *    a. Setting the first droplet, interval is not displayed for the first droplet
 * 	
 *   	FLSH_N_#_ITV_SIZ
 *   	0600_3_1_____060
 * 
 *    b. Setting the second droplet, default interval is 90ms
 * 
 *   	FLSH_N_#_ITV_SIZ
 *   	0600_3_2_090_060
 * 
 *    c. (trigger delay)_(total number droplet)_(number of edit)_(droplet interval)_(droplet size)
 *
 *      * tigger delay is the daley time (in ms) after when the first droplet block the laser tripping trap.
 *      * total number of droplet defines how many droplets of water will be released from the solenoid valve.
 *      * changing the number of edit will switch the last 7 digits to display the droplet interval and droplet size for editting.
 *      * droplet interval defines the time between two water droplets, this value will be 0 and not editable for droplet #1.
 *      * droplet size is controlled by the solenoid valve opening time. (longer the larger, but need to find an optimum valve for a clean image)
 *
 *
 * 4. Instruction:
 *
 *
 * 5. Notes:
 *
 *
 *
 *
 *
 */


#include <LiquidCrystal.h>


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


int backLightSwitch = 10;   	// Pin 10 control back light on LCD
int Valve = 13;         	// Pin 13 control solenoid valve open/close
int Laser = 12;			// pin 12 output power laser pointer
int Trigger = 11;              	// pin 11 output to trigger camera
int photoSensor = 1;            // Analog pin 1 read photocell input

int soundSensor = 3;		// Analog pin 1 read microphone input

int soundVal = analogRead(soundSensor);

/*
 * Right        Value = 0
 * Up           Value = 1
 * Down         Value = 2
 * Left         Value = 3
 * Select       Value = 4
 */

int  adc_key_val[] ={ 30, 150, 360, 535, 760 };

int adc_key_in;
int key=-1;
int oldkey=-1;

int RowTop = 0;
int RowBottom = 1;
int cursorValue = 0;


//initial for ON/OFF
int backLightON = HIGH;
int backLightOFF = LOW;

int valueBottom[] = {0, 6, 0, 0, -1 , 3, -1, 1, -1, 0, 0, 0, -1 , 0, 8, 1};
long Interval[] =    {0, 91, 92, 93, 94, 95, 96, 97, 98}; 	// pause between two droplets in (ms)
long dropletSize[] = {81, 82, 83, 84, 85, 86, 87, 88, 89}; 	// open time (ms) of solenoid valve control size
long triggerDelay;
long totalNm = valueBottom[5];		                        // total number of droplets in one run
long numberofDrop;                             // value of #

unsigned long timer;



void update()
{
  numberofDrop = valueBottom[7];
  totalNm = valueBottom[5];
  valueBottom[9] = (Interval[numberofDrop-1]/100) % 10;
  valueBottom[10] = (Interval[numberofDrop-1]/10) % 10;
  valueBottom[11] = (Interval[numberofDrop-1]) % 10;
 
  valueBottom[13] = (dropletSize[numberofDrop-1]/100) % 10;
  valueBottom[14] = (dropletSize[numberofDrop-1]/10) % 10;
  valueBottom[15] = (dropletSize[numberofDrop-1]) % 10;
  
  triggerDelay = valueBottom[0] * 1000 + valueBottom[1] * 100 + valueBottom[2] * 10 + valueBottom[3];

  lcd.setCursor(0, RowBottom);
 
  int t = 0 ;
  for (t = 0; t < 16; t++)
  {
    if (valueBottom[t] <= -1)
    {
      lcd.print(" ");
    }
    else
    {
      lcd.print(valueBottom[t]);
    }
  }
}


void save()
{
  Interval[numberofDrop-1] = valueBottom[9] * 100 + valueBottom[10] * 10 + valueBottom[11];
  dropletSize[numberofDrop-1] = valueBottom[13] * 100 + valueBottom[14] * 10 + valueBottom[15];
  triggerDelay = valueBottom[0] * 1000 + valueBottom[1] * 100 + valueBottom[2] * 10 + valueBottom[3];		                        
}
  


void setup()
{

  pinMode(backLightSwitch, OUTPUT);
  pinMode(Valve, OUTPUT);
  pinMode(Laser, OUTPUT);
  pinMode(Trigger, OUTPUT);
  pinMode(photoSensor, INPUT);

//pinMode(soundSensor, INPUT);
  
  lcd.begin(16, 2);

//Serial.begin(9600);

  digitalWrite(backLightSwitch, backLightON);
  lcd.clear();
  

  lcd.setCursor(0, RowTop);
  lcd.print("FLSH N # ITV SIZ");
  
  lcd.setCursor(0, RowBottom);
  int i = 0 ;
  for (i = 0; i < 16; i++)
  {
    if (valueBottom[i] <= -1)
    {
      lcd.print(" ");
    }
    else
    {
      lcd.print(valueBottom[i]);
    }
  }
  //lcd.print("0600 3 1 000 050");
  
  triggerDelay = valueBottom[0] * 1000 + valueBottom[1] * 100 + valueBottom[2] * 10 + valueBottom[3];
  
      
  lcd.setCursor(cursorValue, RowBottom);
  lcd.cursor();
}




void loop()
{

  adc_key_in = analogRead(0);	// read the value from the sensor  

  key = get_key(adc_key_in);	// convert into key press
  if (key != oldkey) 		// if keypress is detected
  {
    delay(50);			// wait for debounce time
    adc_key_in = analogRead(0); // read the value from the sensor  
    key = get_key(adc_key_in);  // convert into key press
    if (key != oldkey)             
    {           
      oldkey = key;

      if (key >=0)     
      {
        rightkey();     	//right key
      }

      if (key == 1)     	//up key
      {
        upkey();
      }

      else if (key == 2)   	//down  key
      {
        downkey();
      }

      else if (key == 3)    	//left key
      {
        leftkey();
      }

      else if (key == 4)     	//select key
      { 
        selectkey();
        delay(500);
      }
    }
  } 
}



// Convert ADC value to key number

int get_key(unsigned int input)
{   
  int k;
  for (k = 0; k < 5; k++)
  {
    if (input < adc_key_val[k])
    {  
      return k;  
    }
  }
  if (k >= 5)
    k = -1;     // No valid key pressed
  return k;
}


/*
*/
void rightkey()
{
  if (key == 0)    //right key
  {
    cursorValue++ ;
    if (cursorValue >= 16) cursorValue = 0;
    lcd.setCursor (cursorValue, RowBottom);
  }
}

/*
*/
void leftkey()
{
  cursorValue-- ;
  if (cursorValue <= -1) cursorValue = 15;
  lcd.setCursor (cursorValue, RowBottom);
}


/*
*/
void upkey()
{
  int i;
  for (i = 0; i < 16; i++)
    {
      if (i == cursorValue)
      {
        if (i == 4 || i == 6 || i == 8 || i == 12)
        {
          lcd.print(" ");
        }
        
        else if (i >= 0 && i <= 3)
        {
          valueBottom[i]++;
          if (valueBottom[i] >=10) valueBottom[i] = 0;
          lcd.print(valueBottom[i]);
          update();
        }
        
        else if (i == 5)
        {
          valueBottom[i]++;
          if (valueBottom[i] >=10) valueBottom[i] = 9;
          valueBottom[7] = 1;
          update();
        }
          

        else if (i == 7)
        {
          valueBottom[i]++;
          if (valueBottom[i] >= valueBottom[5]) valueBottom[i] = valueBottom[5];
          update();
        }
        
        else if (i >= 9 && i <= 11 && valueBottom[7] != 1 )
        {
          valueBottom[i]++;
          if (valueBottom[i] >=10) valueBottom[i] = 0;
          save();
          update();
        }
        
        else if (i >=13 && i <= 15)
        {
          valueBottom[i]++;
          if (valueBottom[i] >= 10) valueBottom[i] = 0;
          save();
          update();
        }
      }
    }
    lcd.setCursor (cursorValue, RowBottom);
}


/*
*/
void downkey()
{
  int i;
  for (i = 0; i < 16; i++)
    {
      if (i == cursorValue)
      {
        if (i == 4 || i == 6 || i == 8 || i == 12)
        {
          lcd.print(" ");
        }
        
        else if (i >= 0 && i <= 3)
        {
          valueBottom[i]--;
          if (valueBottom[i] <= -1) valueBottom[i] = 9;
          lcd.print(valueBottom[i]);
          update();
        }
        
        else if (i == 5)
        {
          valueBottom[i]--;
          if (valueBottom[i] <= 1) valueBottom[i] = 1;
          valueBottom[7] = 1;
          update();
        }
 
        else if (i == 7)
        {
          valueBottom[i]--;
          if (valueBottom[i] <= 1) valueBottom[i] = 1;
          update();
        }
        
        else if (i >=9 && i <= 11 && valueBottom[7] != 1)
        {
          valueBottom[i]--;
          if (valueBottom[i] <= -1) valueBottom[i] = 9;
          save();
          update();
        }
        
        else if (i >=13 && i <= 15)
        {
          valueBottom[i]--;
          if (valueBottom[i] <= -1) valueBottom[i] = 9;
          save();
          update();
        }
      }
    }
    lcd.setCursor (cursorValue, RowBottom);
}


/*
*/
void selectkey()
{
  update();
  save();
  
  delay(500);
  
  digitalWrite(Laser, HIGH);
  
  digitalWrite(backLightSwitch, LOW);

  int i = 0;
  long k = 0;
  long runTimer[totalNm];
  
  long totalTime = 0;
  
  for (i = 0; i < totalNm; i++)
  {
    totalTime = totalTime + Interval[i] + dropletSize[i];
  }
  
lcd.setCursor(0, RowTop);
  long timerA;
  long timerB;
  

  for (i = 0; i < totalNm ; i++)
  {
     runTimer[i] = Interval[i] + dropletSize[i];
     timerA = millis();

     while ((millis() - timerA) <= runTimer[i])
     {
       if (millis() - timerA >= Interval[i]) digitalWrite(Valve, HIGH);
       
       if (soundVal >= 100) 
       {
         timerB = millis();
         
         if (millis() - timerB >= triggerDelay) digitalWrite(Trigger, HIGH);
       }
       
     }
     digitalWrite(Valve, LOW);
  }
   
  if (totalTime <= triggerDelay)
  {
    
    delay(triggerDelay - totalTime);
    digitalWrite(Trigger, HIGH);
    delay(200);
  }
    
    
//   lcd.print(timerA);
//   lcd.print(" ");
//   lcd.print(timerB);
 
  digitalWrite(backLightSwitch, HIGH);
  digitalWrite(Trigger, LOW);
  digitalWrite(Laser, LOW);
  lcd.setCursor(cursorValue, RowBottom);
}



