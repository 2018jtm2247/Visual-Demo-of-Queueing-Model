/*------------------------22/April/2019------------------------------------------------------------
 * Queuenung model M/M/1/5
 * This is client 1 programme
 * By default,client 1 is source(sender)
 * Send msg is selected by up-down button and send by select button
 */
  #include <Wire.h>
  #include <LiquidCrystal.h>
  LiquidCrystal lcd(22,23,5,18,19,21); //(rs,enable,D4,D5,D6,D7)
 // #include <ESP8266WiFi.h>
    #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiServer.h>
 
//------------------------------------------------------------------------------------
// Defining I/O Pins
//------------------------------------------------------------------------------------
  #define       LED0      2         // WIFI Module LED
  #define       LED1      13        // Connectivity With Client #1
  #define       LED2      12        // Connectivity With Client #2
 // #define       BUTTON    15        // Connectivity ReInitiate Button
  #define       TWI_FREQ  400000L   // I2C Frequency Setting To 400KHZ
  const int  Up_buttonPin   = 15;    // the pin that the pushbutton is attached to
  const int  Down_buttonPin = 2;
  const int  Sel_buttonPin  = 4;
  const int  Connect  = 14;         //connect to server
  
//------------------------------------------------------------------------------------
// BUTTON Variables
//------------------------------------------------------------------------------------
  int           ButtonState;
  int           LastButtonState   = LOW;
  int           LastDebounceTime  = 0;
  int           DebounceDelay     = 50;
  const String  ButtonColor       = "BLU";
  int buttonPushCounter = 0;   // counter for the number of button presses
  int up_buttonState = 0;         // current state of the up button
  int up_lastButtonState = 0;     // previous state of the up button
  
  int down_buttonState = 0;         // current state of the up button
  int down_lastButtonState = 0;     // previous state of the up button
  
  int sel_buttonState = 0;         // current state of the up button
  int sel_lastButtonState = 0;     // previous state of the up button
  
  bool bPress = false;
  bool msend = false;
//------------------------------------------------------------------------------------
// LED Delay Variables
//------------------------------------------------------------------------------------
  int           LEDState          = LOW;
  unsigned long CurrMillis        = 0;
  unsigned long PrevMillis        = 0;
  unsigned long Interval          = 1000;
//------------------------------------------------------------------------------------
// Authentication Variables
//------------------------------------------------------------------------------------
  char*         TKDssid;
  char*         TKDpassword;
  IPAddress     TKDServer(192,168,4,1);
  WiFiClient    TKDClient;
//------------------------------------------------------------------------------------
//Frame Variables
//------------------------------------------------------------------------------------
char string[20] ;   //message
int ascii[20];     //Ascii value of message
char binary[20][8]; //Binary value of message
char source_add[] = "00";
char dest_add[] = "10";
char start_bit[] = "1";
char stop_bit[] = "1";
char control_bit[10][4];
char frame[20][40];
String Send_Message;
String Message;
int sequence_no = 0;
//------------------------------------------------------------------------------------
//Extra Variables
//------------------------------------------------------------------------------------
  int ack = 0;
  long time_init;
  long time_final;
  boolean queue_is_free = true;
//====================================================================================

  void setup() 
  {    
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Priyanshu");
    lcd.setCursor(0, 1);
    lcd.print("2018JTM2247");
    delay(2000);
 //***   Wire.begin();                   // Begginning The I2C
    
    // Setting Up The I2C Of The MPU9250 ------------------------------------
 //***   Wire.setClock(TWI_FREQ);        // Setting The Frequency MPU9250 Require
        
    // Setting The Serial Port ----------------------------------------------
    Serial.begin(115200);           // Computer Communication
    
    // Setting The Mode Of Pins ---------------------------------------------   
    pinMode(LED0, OUTPUT);          // WIFI OnBoard LED Light
    pinMode(LED1, OUTPUT);          // Indicator For Client #1 Connectivity
    pinMode(LED2, OUTPUT);          // Indicator For Client #2 Connectivity
 //   pinMode(BUTTON, INPUT_PULLUP);  // Initiate Connectivity
    pinMode( Up_buttonPin , INPUT_PULLUP);
    pinMode( Down_buttonPin , INPUT_PULLUP);
    pinMode( Sel_buttonPin , INPUT_PULLUP);
    pinMode( Connect , INPUT_PULLUP);
    digitalWrite(LED0, !LOW);       // Turn WiFi LED Off
    
    // Print Message Of I/O Setting Progress --------------------------------
    Serial.println("\nI/O Pins Modes Set .... Done");   
    Serial.println("Press connect button to connect to server");
    lcd.clear();
    lcd.print("Press connect ");  
    lcd.setCursor(0, 1);
    lcd.print("button...");    

    // Starting To Connect --------------------------------------------------
    while(!digitalRead(Connect))   //connect to wifi if connect button pressed.
    {
      continue;    
    }
    connect_to_server();
    Serial.println("Select Message"); 
    lcd.clear();
    lcd.print("Select Message"); 
    lcd.setCursor(0, 1);
    lcd.print("UP--Down--SEL"); 
  }

//====================================================================================
  
  void loop()
  {
   /*if(TKDClient.connect(TKDServer, 9001))
     {       
        ReadButton();
     }
     else
      {
         Serial.println("Server disconnected");
         lcd.clear();
         lcd.setCursor(0,0);
         lcd.print("Server disconnec.."); 
         delay(5000);
         setup();
      }*/
      ReadButton();
  
  }

//====================================================================================

  void ReadButton()
  {
    checkUp();
    checkDown();
    checkSel();
    if(buttonPushCounter > 4) {buttonPushCounter =0;}else if(buttonPushCounter<0) {buttonPushCounter =4;}    //constrain on button count
    if( bPress){
       bPress = false;    
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Select message"); 
       lcd.setCursor(0,1);
       lcd.print(buttonPushCounter);          //count 0,1,2,3,4
       act_on_lcd();      
    }    
  

    // To Iliminate Debounce
    if(msend)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("msg send");      
   
      get_message_to_send();             //i.e call act_on_lcd();     
      conversion_char_to_ascii();
      conversion_ascii_to_binary();
      set_sequence_no();              //set control bit acc..
      get_frame();                                 //we have complete frame ready to send...
      for(int i=0 ; i <strlen(string) ; i++)
       {
        Send_Message = frame[i];              //select frame to be send
        ack = 0;
        while(!ack)                           
        {
          Serial.print("Message send to server : ");
          Serial.println(Send_Message);
          TKDClient.println(Send_Message);    //send_Message
        //  TKDClient.flush();
          //Wait for ack
        //  Serial.print("In readButton while(!ack)  ack =  ");
         // Serial.println(ack);
          ReadAck();                               //Read Ack
        }   
      } 
      clear_frame();
       
    }
  }  

 
//====================================================================================
  void ReadMessage()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Queue is Full");
    lcd.setCursor(0,1);
    lcd.print("Please Wait...");
    //come here if queue is full
    while(!queue_is_free)
    {
      if(TKDClient.available())
      {
        while(TKDClient.available())
        {
          Message = TKDClient.readStringUntil('\r');
          // Here We Print The Message On The Screen
          Serial.println(Message);
          if(Message == "01")              //queue free msg
          {
           Serial.println("Queue is free");
           lcd.clear();
           lcd.setCursor(0,0);
           lcd.print("Queue is Free");
           lcd.setCursor(0,1);
           lcd.print("SEL Msg");
           queue_is_free = true;
           return;
          }
         continue; 
        }
      }
    }  
  }

//========================================================================================
   void ReadAck()
  {
    time_init = millis();
    time_final = 0;
    //Serial.println("In ReadAck loop");
      while((!ack)  &&  (time_final < 5000))
      {
        //Serial.print("while loop in ReadAck time final :");
        //Serial.println(time_final);
        while (TKDClient.available())
        {
          Message = TKDClient.readStringUntil('\r');
         // Here We Print The Message On The Screen
       // Serial.println("In client.available() reutine");
        if(Message == "00")
          {
            Serial.println("ACK received");
            ack = 1;    
            msend = false;     
          }  
         else if(Message == "11" )
          {
            //queue is full...wait
            Serial.println("queue is full, Wait for ACK...");
            queue_is_free = false;
            ReadMessage();                    //Wait in ReadMessage routine
            
          }
        } 
       time_final = millis() - time_init ;
      }
     // Serial.println(time_final);
      if(time_final >= 5000){
      Serial.println("Time-out period over, Resend packet");
      sequence_no--;
      }
   }
 
//=====================================================================================  


  void CheckConnectivity()
  {
    while(WiFi.status() != WL_CONNECTED)
    {
      for(int i=0; i < 10; i++)
      {
        digitalWrite(LED0, !HIGH);
        delay(250);
        digitalWrite(LED0, !LOW);
        delay(250);
        Serial.print(".");
      }
      Serial.println("");
    }
  }

//====================================================================================

  void TKDRequest()
  {
    // First Make Sure You Got Disconnected
    TKDClient.stop();

    // If Sucessfully Connected Send Connection Message
    if(TKDClient.connect(TKDServer, 9001))
    {
      Serial.println    ("< Client1 -CONNECTED>");
      TKDClient.println ("< Client1 -CONNECTED>");
    }
  }

//====================================================================================
//------------------------------------------------------------
void conversion_char_to_ascii()
{
  Serial.println("------Ascii conversion------");
  for(int i =0;i<strlen(string) ;i++)
    {
    Serial.print(string[i]); 
    Serial.print("--"); 
    ascii[i] = string[i];                    //int eqivalant of char is Ascii value
    Serial.println(ascii[i]);
    }  
}

//----------------------------------------------------------
void conversion_ascii_to_binary()         //generate 7 bit outpit
{ 
  Serial.println("------Binary conversion------");                                                                                                                                                              
  for(int i=0 ; i <strlen(string) ; i++)
   {
      Serial.print(ascii[i]);
      String result = "";
     if(ascii[i] == 0)
     {
      result = "0";
     }
     else
     {
      while(ascii[i] > 0)
       {
        if((ascii[i] % 2) ==0)
        {
          result = "0" + result;
        }
        else
        {
          result = "1" + result;
        }
        
        ascii[i] /= 2;  //we are updating ascii value,we should have used a variable instead
       }  
     }
     //Serial.println(result); 
     result.toCharArray(binary[i],8);     
     int j = strlen(binary[i]);
     if(j != 7)                 //if length is not 7 make it seven and add 0 at beginning
     {
      for(j ; j>0 ; j--)
      {
        binary[i][j] = binary[i][j-1];
      }
     //strcat("0",binary[i]);
        binary[i][j] = '0';
     }
     
     Serial.print("--");
     Serial.println(binary[i]);
    //Serial.println(strlen(binary[i]));
   }
}
//----------------------------------------------------------
void set_sequence_no()
{
  for(int i=0 ; i <strlen(string) ; i++)
    {
      if(sequence_no == 0){strcpy(control_bit[i],"000");}  
      else if(sequence_no == 1){strcpy(control_bit[i],"001");}
      else if(sequence_no == 2){strcpy(control_bit[i],"010");}
      else if(sequence_no == 3){strcpy(control_bit[i],"011");}
      else if(sequence_no == 4){strcpy(control_bit[i],"100");}
      else if(sequence_no == 5){strcpy(control_bit[i],"101");}
      else if(sequence_no == 6){strcpy(control_bit[i],"110");}
      else if(sequence_no == 7){strcpy(control_bit[i],"111");}
      if(sequence_no < (strlen(string)-1))                                //for msg0 strlen = 4,our constrain is 4-1 = 3,
      sequence_no++;
      else
      sequence_no = 0;
      //Serial.println(control_bit[i]);
    }  
}

//----------------------------------------------------------
void get_frame()
{
  Serial.println("------Frame building------"); 
  for(int i=0 ; i <strlen(string) ; i++)
    {
    strcat(frame[i],start_bit);
    Serial.print("Add Start bit :       ");
    Serial.println(frame[i]);
    strcat(frame[i],source_add);
    Serial.print("Add Source add :      ");
    Serial.println(frame[i]);
    strcat(frame[i],dest_add);
    Serial.print("Add Destination add : ");
    Serial.println(frame[i]);
    strcat(frame[i],binary[i]);
    Serial.print("Add Data bits :       ");
    Serial.println(frame[i]);
    strcat(frame[i],control_bit[i]);
    Serial.print("Add control bit :     ");
    Serial.println(frame[i]);
    strcat(frame[i],stop_bit);
    Serial.print("Add Stop bit :        ");
    Serial.println(frame[i]);
    Serial.println(" ");
    }
}

//---------------------------------------------------
void get_message_to_send()
{
  act_on_lcd();
 // strcpy(string,"ABC");
 // Serial.println(string);
}

//-----------------------------------------------------
void clear_frame()
{
  memset(frame, 0, sizeof(frame));
}

//====================================================================================
void connect_to_server()
  {
    WiFi.mode(WIFI_STA);            // To Avoid Broadcasting An SSID
    WiFi.begin("TAKEONE");          // The SSID That We Want To Connect To

    // Printing Message For User That Connetion Is On Process ---------------
    Serial.println("!--- Connecting To " + WiFi.SSID() + " ---!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.SSID());

    // WiFi Connectivity ----------------------------------------------------
    CheckConnectivity();            // Checking For Connection(call function)

    // Stop Blinking To Indicate Connected ----------------------------------
    digitalWrite(LED0, !HIGH);
    Serial.println("!-- Client Device Connected --!");
    lcd.setCursor(0, 0);
    lcd.print("connection estb");
    delay(1000);

    // Printing IP Address --------------------------------------------------
    Serial.println("Connected To      : " + String(WiFi.SSID()));
    Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
    Serial.print  ("Server IP Address : ");
    Serial.println(TKDServer);
    Serial.print  ("Device IP Address : ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Client IP:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(3000);
    lcd.setCursor(0, 0);
    lcd.print("Server IP:");
    lcd.setCursor(0, 1);
    lcd.print(TKDServer);
    delay(3000);
    // Conecting The Device As A Client -------------------------------------
    TKDRequest();
   // Serial.println  ("Select Message ");    
  }

//====================================================================================
//====================================================================================
/***********************************************************************************************/
void checkUp()
{
  up_buttonState = digitalRead(Up_buttonPin);

  // compare the buttonState to its previous state
  if (up_buttonState != up_lastButtonState) {
    // if the state has changed, increment the counter
    if (up_buttonState == LOW) {
        bPress = true;
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
     // Serial.println("on");
    //  Serial.print("number of button pushes: ");
     // Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  up_lastButtonState = up_buttonState;
}
/************************************************************************************************/
void checkDown()
{
  down_buttonState = digitalRead(Down_buttonPin);

  // compare the buttonState to its previous state
  if (down_buttonState != down_lastButtonState) {
    // if the state has changed, increment the counter
    if (down_buttonState == LOW) {
        bPress = true;
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter--;
     
     // Serial.println("on");
     // Serial.print("number of button pushes: ");
    //  Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  down_lastButtonState = down_buttonState;
}
/****************************************************************************************/
void checkSel()
{
  sel_buttonState = digitalRead(Sel_buttonPin);

  // compare the buttonState to its previous state
  if (sel_buttonState != sel_lastButtonState) {
    // if the state has changed, increment the counter
    if (sel_buttonState == LOW) {
        bPress = true;
        msend = true;           //send command activated
      
     
     // Serial.println("on");
   //   Serial.print("number of button pushes: ");
     // Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  sel_lastButtonState = sel_buttonState;
}
/****************************************************************************************/
void act_on_lcd()
{
  lcd.setCursor(5,1);
 switch(buttonPushCounter)
   {
     
     case 0://if(msend){TKDClient.println("msg0"); }
            strcpy(string,"msg0");
           // Serial.println("msg0");
            lcd.print("msg0");  
            break;
     case 1://if(msend){TKDClient.println("msg1");}
            strcpy(string,"msg1");
           // Serial.println("msg1");
            lcd.print("msg1");  
            break;

     case 2://if(msend){TKDClient.println("msg2");}
             strcpy(string,"msg2");
          // Serial.println("msg2");
            lcd.print("msg2");  
            break;
            
     case 3://if(msend){TKDClient.println("msg3");}
           strcpy(string,"msg3");
          // Serial.println("msg3");
            lcd.print("msg3");  
            break;

     case 4://if(msend){TKDClient.println("msg4");}
             strcpy(string,"msg4");
           //Serial.println("msg4");
            lcd.print("msg4");  
            break;       

     default://client.println("default");
           strcpy(string,"default");
          // Serial.println("default");
            lcd.print("default");  
            delay(50);
            break;                                            

   }
}
