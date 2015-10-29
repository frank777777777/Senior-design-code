/*
This software is written and produced by Yili Zou, credit to Peng Gao in pulse counting function.
The author of this software is in no way responsible for any damage or extending liability.
Redistributions of source code must retain this disclaimer notice. 
Any comercial use of this software must obtain Yili Zou's authorization.
Modification of this software must retain the author's name.
*/
#include <LiquidCrystal.h>
#include <DueFlashStorage.h>
#include <IRremote2.h>

int RECV_PIN = 2;

DueFlashStorage dueFlashStorage;  //object of dueFlashStorage
IRrecv irrecv(RECV_PIN);      //set up the receiving pin
IRsend irsend;

decode_results results;
 

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
int BUTTON_PIN_1=52;  //Button 1
int BUTTON_PIN_2=50;  //Button 2
int BUTTON_PIN_3=48;  //Button 3
int BUTTON_PIN_4=46;  //Button 4
int P_BUTTON_PIN_1=53;  //ground pin1
int P_BUTTON_PIN_2=51;  //ground pin2
int P_BUTTON_PIN_3=49;  //ground pin3
int P_BUTTON_PIN_4=47;  //ground pin4
int lastButtonState;  //Previous ButtonState
int ButtonState;      //Current ButtonState
int Command_Index=1; //initialize the Command_Index, from 1 to 200
int Mode_Switch=2;  //A flag to represent which Mode it is on, initialize the device with 2, and 0 is EDIT mode, 1 is Listenning mode 
int pulseRead = A0;   //pulse reading pin
int lowFlag = 0; //Flag for a low in pulse reading
int count=0;  //pulse counter

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(BUTTON_PIN_1,INPUT);
  pinMode(BUTTON_PIN_2,INPUT);
  pinMode(BUTTON_PIN_3,INPUT);
  pinMode(BUTTON_PIN_4,INPUT);
  pinMode(P_BUTTON_PIN_1,OUTPUT);
  pinMode(P_BUTTON_PIN_2,OUTPUT);
  pinMode(P_BUTTON_PIN_3,OUTPUT);
  pinMode(P_BUTTON_PIN_4,OUTPUT);
  digitalWrite(P_BUTTON_PIN_1, HIGH);
  digitalWrite(P_BUTTON_PIN_2, HIGH);
  digitalWrite(P_BUTTON_PIN_3, HIGH);
  digitalWrite(P_BUTTON_PIN_4, HIGH);
  lcd.begin(20,4);      //initialize the 4x20 crystal LCD
  lcd.setCursor(0,0);   // set up the initialized user interface
  lcd.print("***Hello  Welcome***");
  lcd.setCursor(0,1);
  lcd.print("   Voice  Command   ");
  lcd.setCursor(0,2);
  lcd.print("   IR Environment   ");
  lcd.setCursor(0,3);
  lcd.print("____________________"); 
}


uint8_t codeLen; // The length of the code

uint16_t rawCodes[RAWBUF]; //Ram Buffer to store the array of IR Raw Code


// Most of this code is just logging
void storeCode(decode_results *results) {
int count = results->rawlen;
    Serial.println("\n Received unknown code, saving as raw1");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK -100;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK +100;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
    
        //copy the codeLen to memory from 0-200, IR codes after 200, 200 per array
    dueFlashStorage.write(0+Command_Index-1,codeLen);
    dueFlashStorage.write(200+Command_Index-1,1);  //write 1 to this spot, it means this Command index has been used
    byte temp[sizeof(rawCodes)]; //create byte array to store the rawCodes array
    memcpy(temp, &rawCodes, sizeof(rawCodes)); // copy the rawCodes array to the byte array
    dueFlashStorage.write(Command_Index*200+200, temp, sizeof(rawCodes)); // write byte array to flash 
  }
  
void sendCode() {
  
    byte* temp = dueFlashStorage.readAddress(Command_Index*200+200); //byte array which is read from flash at address 200+Command_index*200
    uint16_t array_new[RAWBUF];
    memcpy(array_new,temp,sizeof(rawCodes)); 
    codeLen = dueFlashStorage.read(0+Command_Index-1);
    // Assume 38 KHz1
    irsend.sendRaw(array_new, codeLen, 38);
    Serial.println("Sent raw1");
}



void Scroll_up()
{
      lcd.clear();   //clear the lcd
      Command_Index--;   // scroll up the index
      if(Command_Index<1)  //if it goes less than 1, then we go to the last one
      {
        Command_Index=200;
      }
      lcd.setCursor(0,0);
      if(Command_Index==1)  //if the index is 1, the previous one is the last one
      {
        if(dueFlashStorage.read(399) == 1)   //check if this command has an IR Code
        lcd.print("  Command 200       *");
        else
        lcd.print("  Command 200");
      }
      else
      {
        lcd.print("  Command "+(String)(Command_Index-1));
        if(dueFlashStorage.read(200+Command_Index-2)==1)   //check if this command has an IR Code)
        {
          lcd.setCursor(19,0);
          lcd.print("*");
        }
      }
      lcd.setCursor(0,1);
      lcd.print("->Command "+(String)Command_Index);
              if(dueFlashStorage.read(200+Command_Index-1)==1)   //check if this command has an IR Code)
              {
                lcd.setCursor(19,1);
                 lcd.print("*");
              }

      lcd.setCursor(0,2);
      if(Command_Index==200)   //if index is 200, the next one if 1, and check if it has been used
      {
        if(dueFlashStorage.read(200) == 1)   //check if this command has an IR Code
        lcd.print("  Command 1        *");
        else
        lcd.print("  Command 1");
      }
      else
      {
        lcd.print("  Command "+(String)(Command_Index+1));
        if(dueFlashStorage.read(200+Command_Index)==1)   //check if this command has an IR Code)
              {
                lcd.setCursor(19,2);
                 lcd.print("*");
              }
      }
      lcd.setCursor(0,3); 
      lcd.print("***  EDIT  MODE  ***");
}
void Scroll_down()
{
      lcd.clear();   //clear the lcd
      Command_Index++;   // scroll up the index
      if(Command_Index>200)  //if it goes less than 1, then we go to the last one
      {
        Command_Index=1;
      }
      lcd.setCursor(0,0);
      if(Command_Index==1)  //if the index is 1, the previous one is the last one
      {
        if(dueFlashStorage.read(399) == 1)   //check if this command has an IR Code
        lcd.print("  Command 200       *");
        else
        lcd.print("  Command 200");
      }
      else
      {
        lcd.print("  Command "+(String)(Command_Index-1));
        if(dueFlashStorage.read(200+Command_Index-2)==1)   //check if this command has an IR Code)
        {
          lcd.setCursor(19,0);
          lcd.print("*");
        }
      }
      lcd.setCursor(0,1);
      lcd.print("->Command "+(String)Command_Index);
              if(dueFlashStorage.read(200+Command_Index-1)==1)   //check if this command has an IR Code)
              {
                lcd.setCursor(19,1);
                 lcd.print("*");
              }

      lcd.setCursor(0,2);
      if(Command_Index==200)   //if index is 200, the next one if 1, and check if it has been used
      {
        if(dueFlashStorage.read(200) == 1)   //check if this command has an IR Code
        lcd.print("  Command 1        *");
        else
        lcd.print("  Command 1");
      }
      else
      {
        lcd.print("  Command "+(String)(Command_Index+1));
        if(dueFlashStorage.read(200+Command_Index)==1)   //check if this command has an IR Code)
              {
                lcd.setCursor(19,2);
                 lcd.print("*");
              }
      }
      lcd.setCursor(0,3); 
      lcd.print("***  EDIT  MODE  ***");
}

bool Mode_Switch_Check()
{
  ButtonState = digitalRead(BUTTON_PIN_4);
  if(lastButtonState==HIGH && ButtonState == LOW)
  {
    return true;
  }
  else
  {
    lastButtonState = ButtonState;
    return false; 
  }
}

void Edit_Mode()
{
      lcd.clear();   //clear the lcd
      if(Command_Index<1)  //if it goes less than 1, then we go to the last one
      {
        Command_Index=200;
      }
      lcd.setCursor(0,0);
      if(Command_Index==1)  //if the index is 1, the previous one is the last one
      {
        if(dueFlashStorage.read(399) == 1)   //check if this command has an IR Code
        lcd.print("  Command 10       *");
        else
        lcd.print("  Command 10");
      }
      else
      {
        lcd.print("  Command "+(String)(Command_Index-1));
        if(dueFlashStorage.read(200+Command_Index-2)==1)   //check if this command has an IR Code)
        {
          lcd.setCursor(19,0);
          lcd.print("*");
        }
      }
      lcd.setCursor(0,1);
      lcd.print("->Command "+(String)Command_Index);
              if(dueFlashStorage.read(200+Command_Index-1)==1)   //check if this command has an IR Code)
              {
                lcd.setCursor(19,1);
                 lcd.print("*");
              }

      lcd.setCursor(0,2);
      if(Command_Index==200)   //if index is 200, the next one if 1, and check if it has been used
      {
        if(dueFlashStorage.read(200) == 1)   //check if this command has an IR Code
        lcd.print("  Command 1        *");
        else
        lcd.print("  Command 1");
      }
      else
      {
        lcd.print("  Command "+(String)(Command_Index+1));
        if(dueFlashStorage.read(200+Command_Index)==1)   //check if this command has an IR Code)
              {
                lcd.setCursor(19,2);
                 lcd.print("*");
              }
      }
      lcd.setCursor(0,3); 
      lcd.print("***  EDIT  MODE  ***");
}

void Listening_Mode()
{        
        //Reinitialize lastButtonState and ButtonState
        lastButtonState= LOW;
        ButtonState= LOW;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("   Voice  Command   ");
          lcd.setCursor(0,1);
          lcd.print("   I am Listening   ");
          lcd.setCursor(0,2);
          lcd.print("         ...        ");
          lcd.setCursor(0,3);
          lcd.print("***Listening Mode***");
        while(1)
        {
          if(analogRead(pulseRead)>500)                           //This section counts pulses it received, and send out
                                                                  //corresponded signal.
          {
              count ++;
              while(1)
              {
                delayMicroseconds(2000);
                if((analogRead(pulseRead)>500)&&lowFlag>0)
                {
                  count ++;
                  lowFlag = 0;
                }
              if (analogRead(pulseRead)<500)
              {
                lowFlag ++;
              }
              if(lowFlag>5)
              {
                lowFlag = 0;
                break;
              }
              }
           }
           else{
             Serial.print(analogRead(pulseRead));
           }
          if(count>0)
          {
            int temp_index = Command_Index;
            Command_Index = count;
            sendCode();
            Serial.print(Mode_Switch);
            Command_Index = temp_index;
            count = 0;
          }
          if(Mode_Switch_Check())
          {
            Mode_Switch=0;   //switch to Edit mode
            Edit_Mode();     //Switch to Edit mode user interface
            break;          //exit out of the listening mode
          }
        }
        irrecv.enableIRIn(); // Start the receiver
}

void loop()
{
  if(Mode_Switch==2 || Mode_Switch==0)  //only response when the device just stated or it is on Edit mode
  {
  
    while(digitalRead(BUTTON_PIN_1)==HIGH)
    {
      Mode_Switch=0;
      Scroll_up();
      delay(150);
    }
    while(digitalRead(BUTTON_PIN_2)==HIGH)
    {
      Mode_Switch=0;
      Scroll_down();
      delay(150);
    } 
    while(Mode_Switch==0 && digitalRead(BUTTON_PIN_3)==HIGH)
    {
      if(irrecv.decode(&results))
      {
         storeCode(&results);
         irrecv.resume();//resume receiver
         Edit_Mode();  //After receive the signal, fresh the LCD
           lcd.setCursor(19,1); //flash the *, to inform the user it is updated
           lcd.print(" ");
           delay(1000);
           lcd.setCursor(19,1);
           lcd.print("*");
           
      }
    }
  }
  
  if(Mode_Switch_Check())
  {
    if(Mode_Switch==2 || Mode_Switch==0)   //If it is on the initialization Mode or Edit Mode, Switch it to Listenning Mode
      Mode_Switch=1;
    else
     Mode_Switch=0;    //Or If not, Switch it to Edit Mode */
    Listening_Mode();
    //reinitialize the ButtonState and lastButtonState
    lastButtonState=LOW;
    ButtonState=LOW;
  }


}
