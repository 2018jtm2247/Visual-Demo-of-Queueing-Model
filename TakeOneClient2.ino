//------------------------------------------------------------------------------------
  #include <Wire.h>
 // #include <ESP8266WiFi.h>
    #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiServer.h>
 
//------------------------------------------------------------------------------------
// Defining I/O Pins
//------------------------------------------------------------------------------------
  #define       LED0      2         // WIFI Module LED
  #define       LED1      15        // Connectivity With Client #1
  #define       LED2      12        // Connectivity With Client #2
  #define       BUTTON    13        // Connectivity ReInitiate Button
  #define       TWI_FREQ  400000L   // I2C Frequency Setting To 400KHZ
  const int  Connect  = 14;
//------------------------------------------------------------------------------------
// BUTTON Variables
//------------------------------------------------------------------------------------
  int           ButtonState;
  int           LastButtonState   = LOW;
  int           LastDebounceTime  = 0;
  int           DebounceDelay     = 50;
  const String  ButtonColor       = "RED";
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

  String Message;
  char frame[30];
  char binary[20];
  int ascii[20];
  char string[20];   //converted message
  int string_index = 0;
  
//====================================================================================

  void setup() 
  {
    /* ----------------------------------------------------------------------
     * Setting The I2C Pins SDA, SCL
     * Because We Didnt Specify Any Pins The Defult
     * SDA = D4 <GPIO2>, SCL = D5 <GPIO14> For ESP8266 Dev Kit Node MCU v3
     --------------------------------------------------------------------- */
    Wire.begin();                   // Begginning The I2C
    
    // Setting Up The I2C Of The MPU9250 ------------------------------------
    Wire.setClock(TWI_FREQ);        // Setting The Frequency MPU9250 Require
        
    // Setting The Serial Port ----------------------------------------------
    Serial.begin(115200);           // Computer Communication
    
    // Setting The Mode Of Pins ---------------------------------------------
    pinMode(LED0, OUTPUT);          // WIFI OnBoard LED Light
    pinMode(LED1, OUTPUT);          // Indicator For Client #1 Connectivity
    pinMode(LED2, OUTPUT);          // Indicator For Client #2 Connectivity
    pinMode(BUTTON, INPUT_PULLUP);  // Initiate Connectivity
    pinMode( Connect , INPUT_PULLUP);
    digitalWrite(LED0, !LOW);       // Turn WiFi LED Off
    
    // Print Message Of I/O Setting Progress --------------------------------
    Serial.println("\nI/O Pins Modes Set .... Done");
    Serial.println("Press connect button to connect to server");

    // Starting To Connect --------------------------------------------------
    while(!digitalRead(Connect))   //connect to wifi if connect button pressed.
    {
      continue;    
    }
    connect_to_server();
  }

//====================================================================================
  
  void loop()
  {

   // ReadButton();
    ReadMessage();
  }

//====================================================================================

 /* void ReadButton()
  {
    // Reading The Button
    int reading = digitalRead(BUTTON);

    // If It Doest Match The Previous State
    if(reading != LastButtonState)
    {
      LastDebounceTime = millis();
    }

    // To Iliminate Debounce
    if((millis() - LastDebounceTime) > DebounceDelay)
    {
      if(reading != ButtonState)
      {
        ButtonState = reading;
        
        if(ButtonState == LOW)
        {
          LEDState    = !digitalRead(LED1);
          digitalWrite(LED1, LEDState);
          Serial.println("<" + ButtonColor + "-SCORED>");
          TKDClient.println("<" + ButtonColor + "-SCORED>");
          TKDClient.flush();
        }
      }
    }

    // Last Button State Concidered
    LastButtonState = reading;
  }*/

//====================================================================================
 void ReadMessage()
  {
   if( TKDClient.connected())
   {    
    if(TKDClient.available())
    {      
      while(TKDClient.available()) 
      {        
        Message = TKDClient.readStringUntil('\n');
        //Serial.println(Message);
        strcpy(frame,Message.c_str());    //string msg to frame aray conversion
      //  TKDClient.flush();
        // Here We Print The Message On The Screen
        if(strlen(frame) > 10 )
        {
          Serial.print("Message received :  ");
          Serial.println(Message);
         // Serial.print("Message length :  ");
         // Serial.println(strlen(frame));
          if(Message == "<OK>")
           {
             Serial.println("ACK received");
           }
           Decode_frame();
           conversion_binary_to_ascii();
           ascii_to_char();   
           print_message();  
        }     
      }
     } 
    }
  }
//=====================================================================================================
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
     // Serial.println    ("<" + ButtonColor + "-CONNECTED>");
     Serial.println    ("< Client2 -CONNECTED>");
     // TKDClient.println ("<" + ButtonColor + "-CONNECTED>");
    }
  }

//====================================================================================
void Decode_frame()
{
 //strcpy(frame,Message.c_str());    //string msg to frame aray conversion
 for(int i = 5; i <=11 ; i++)
  { 
   binary[i-5] = frame[i];       //get data in the form of 0 1 string
  }
  Serial.print("Data bits--");
  Serial.println(binary);
}
//================================================================================
void conversion_binary_to_ascii()     //binary string to int Ascii value
{
   int base = 2;     
   ascii[0] = strtol(binary, NULL, base);   //
   Serial.print("Ascii value--");
   Serial.println(ascii[0]);

}
//===============================================================================
void ascii_to_char()
{
  string[string_index] = ascii[0];
  
  Serial.print("Char equivalent--");
  Serial.println(string[string_index]);
  string_index ++;
}
//===============================================================================
void print_message()
{
  Serial.print("Original Message--");
  for(int i =0; i< string_index;i++)
  {
    Serial.print(string[i]);
  }
   Serial.println("\n");
}
//====================================================================================
void connect_to_server()
{
    WiFi.mode(WIFI_STA);            // To Avoid Broadcasting An SSID
    WiFi.begin("TAKEONE");          // The SSID That We Want To Connect To

    // Printing Message For User That Connetion Is On Process ---------------
    Serial.println("!--- Connecting To " + WiFi.SSID() + " ---!");

    // WiFi Connectivity ----------------------------------------------------
    CheckConnectivity();            // Checking For Connection

    // Stop Blinking To Indicate Connected ----------------------------------
    digitalWrite(LED0, !HIGH);
    Serial.println("!-- Client Device Connected --!");

    // Printing IP Address --------------------------------------------------
    Serial.println("Connected To      : " + String(WiFi.SSID()));
    Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
    Serial.print  ("Server IP Address : ");
    Serial.println(TKDServer);
    Serial.print  ("Device IP Address : ");
    Serial.println(WiFi.localIP());
    
    // Conecting The Device As A Client -------------------------------------
    TKDRequest();   
}
