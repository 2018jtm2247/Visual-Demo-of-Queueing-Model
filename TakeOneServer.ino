//-- Libraries Included --------------------------------------------------------------
  #include <ESP8266WiFi.h>
 //  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiServer.h>
 
//------------------------------------------------------------------------------------
  // Define I/O Pins
  #define     LED0      2           // WIFI Module LED
  #define     LED1      12          // Connectivity With Client #1
  #define     LED2      14          // Connectivity With Client #2
  #define     BUTTON    13          // Connectivity ReInitiate Button
//------------------------------------------------------------------------------------
  // Authentication Variables
  char*       TKDssid;              // SERVER WIFI NAME
  char*       TKDpassword;          // SERVER PASSWORD
//------------------------------------------------------------------------------------
  #define     MAXSC     3           // MAXIMUM NUMBER OF CLIENTS
  WiFiServer  TKDServer(9001);      // THE SERVER AND THE PORT NUMBER
  WiFiClient  TKDClient[MAXSC];     // THE SERVER CLIENTS

  String Message;
  int count =0;
//====================================================================================
  //Queue variables
  const int max_queue_length = 5;     //set max queue length
  char queue[max_queue_length][40];   //queue length is 7
  int index_queue = 0;
  char temp[25];
  long time_init = millis();
  long time_final;
  int sequence_no[5];
  boolean queue_full = false;


//===================================================================================

  void setup()
  {
    // Setting The Serial Port
    Serial.begin(115200);           // Computer Communication
    
    // Setting The Mode Of Pins
    pinMode(LED0, OUTPUT);          // WIFI OnBoard LED Light
    pinMode(LED1, OUTPUT);          // Indicator For Client #1 Connectivity
    pinMode(LED2, OUTPUT);          // Indicator For Client #2 Connectivity
    pinMode(BUTTON, INPUT_PULLUP);  // Initiate Connectivity
    
    // Print Message Of I/O Setting Progress
    Serial.println();
    Serial.println("I/O Pins Modes Set .... Done");

    // Setting Up A Wifi Access Point
    SetWifi("TAKEONE", "");
  }

//====================================================================================
  
  void loop()
  {
    
    IsClients2();
    
  }

//====================================================================================
  
  void SetWifi(char* Name, char* Password)
  {
    // Stop Any Previous WIFI
    WiFi.disconnect();

    // Setting The Wifi Mode
    WiFi.mode(WIFI_AP_STA);
    Serial.println("WIFI Mode : AccessPoint Station");
    
    // Setting The Access Point
    TKDssid      = Name;
    TKDpassword  = Password;
    
    // Starting The Access Point
    WiFi.softAP(TKDssid, TKDpassword);
    Serial.println("WIFI < " + String(TKDssid) + " > ... Started");
    
    // Wait For Few Seconds
    delay(1000);
    
    // Getting Server IP
    IPAddress IP = WiFi.softAPIP();
    
    // Printing The Server IP Address
    Serial.print("AccessPoint IP : ");
    Serial.println(IP);

    // Starting Server
    TKDServer.begin();
    Serial.println("Server Started");
  }

//====================================================================================

//====================================================================================

  void IsClients2()
  {
    if (TKDServer.hasClient())
    {
      for(int i = 0; i < MAXSC; i++)
      {
        //find free/disconnected spot
        if (!TKDClient[i] || !TKDClient[i].connected())
        {
          if(TKDClient[i]) TKDClient[i].stop();
          TKDClient[i] = TKDServer.available();
          Serial.print("New Client : "); Serial.print(String(i+1) + " - ");
          Serial.println(TKDClient[i].remoteIP());
          Serial.println("");
          
          continue;
        }
      }
      // no free / disconnected spot so reject
      digitalWrite(LED1, HIGH);
      WiFiClient TKDClient = TKDServer.available();
      TKDClient.stop();
    }

    //----------------check clients for data -------------------------------------------------------
    
    for(int i = 0; i < MAXSC; i++)
    {
      if (TKDClient[i] && TKDClient[i].connected())
      {
        time_final = millis() - time_init ;
        if(TKDClient[i].available())                           //if data available
        {
          // If Any Data Was Available We Read IT
          while(TKDClient[i].available())                     //we read data from client 1 only(bcoz client 2 is sink and client 1 is source)
          {            
            // Read From Client
            Message = TKDClient[i].readStringUntil('\n');     //get message
            Serial.print("Message received :  ");
            Serial.println(Message);                         //Read Frame(Message)
            Message.toCharArray(temp,22);                    //change of variable
         //   Serial.println(temp);                  //store msg in temp                            //add frame(packet) in queue 
          //  Serial.println(strcmp(temp,"< Client1 -CONNECTED>"));  
            if(strcmp(temp,"< Client1 -CONNECTED>"))                  //To avoid first message (strcmp return 0 if str1 = str2)     
            {
              Message.toCharArray(queue[index_queue],20);      //add frame(packet) in queue  
              get_sequence_no();                               //Get the sequence no from feame
              //***to do ---no check if two sequence no is same?
              Serial.print("queue length : ");
              Serial.println(index_queue);                     //old queue index
              Serial.print("queue  : ");
       //----------------------------------------------------------------------------------       
              for(int i =0; i<= index_queue;i++)               //print queue
              {
                Serial.print(queue[i]);
                Serial.print(" ");
              }  
              Serial.println(" ");
        //-------------------------------------------------------------------------------
              if(index_queue >= (max_queue_length - 1))       //if queue is full
               {
                //send ack not to send more packet...11
                TKDClient[i].println("11");    //queue is full
                queue_full = true;
                
               }
              else
               {
                 index_queue++;        //Index is not full
               }
       //-------------------------------------------------------------------------------
              // Reply To Client except 5 th multiple of packet
              count++;            
              if ((count % 5) != 0   && index_queue <=(max_queue_length - 1) )
              {
              TKDClient[i].println("00");         //ack that packet received        
              }
        //-------------------------------------------------------------------------------      
            }
          } 
        }
        if ((TKDClient[1] && TKDClient[1].connected())  || (TKDClient[2] && TKDClient[2].connected()) )     //if queue index is > 2,then send data to sink.
        {          
          // Reply to client 2...add some delay
          if((queue[0][0] != '\0'))  //if queue is not empty
          {
            if(queue[index_queue-1][3] == '1' && queue[index_queue-1][4] == '0')
            {
              send_frame_to_client_2();
            }  
            else if(queue[index_queue-1][3] == '1' && queue[index_queue-1][4] == '1')
            {
              send_frame_to_client_3();
            }  
            
          }
        }
              
      }
    }
  }

//====================================================================================
void send_frame_to_client_2()
{
  // if(time_final > 3000) 
     // {
      Serial.println("packet Send to client 2");   
      TKDClient[1].println(queue[0]);            //data send to client 2     
      shift_data_to_left_in_frame();             //update frame
     // index_queue--;
      time_init = millis();
      time_final = 0;
   //   }
  
}

void send_frame_to_client_3()
{
  // if(time_final > 3000) 
     // {
      Serial.println("packet Send to client 3");      
      TKDClient[2].println(queue[0]);            //data send to client 3 
      shift_data_to_left_in_frame();             //update frame
     // index_queue--;
      time_init = millis();
      time_final = 0;
   //   }
  
}


void shift_data_to_left_in_frame()
{
  //Serial.print("Update queue i is-  "); 
  int i =0;  
  for(i = 0; i < (index_queue ) ; i++)
   {
    //Serial.print(i); 
    // queue[i] = queue[i+1];
     strcpy(queue[i],queue[i+1]);
   }
   //Serial.print(queue[0]); 
  // Serial.print(" -Q1- "); 
   //Serial.println(queue[1]); 
   index_queue--;
  // strcpy(queue[i],'\0');

  //-------send queue free msg if quee was full before
  if(queue_full == true)
   {
    TKDClient[0].println("01");   //queue is free send msg to client 1
    queue_full = false;
   }
}

void get_sequence_no()
{
  int sum = 0;
  if(queue[index_queue][14] == '1'){sum = sum + 1;} else { sum = sum + 0;}
  if(queue[index_queue][13] == '1'){sum = sum + 2;} else { sum = sum + 0;}
  if(queue[index_queue][12] == '1'){sum = sum + 4;} else { sum = sum + 0;} 
  Serial.print("Sequence no is : ");
  Serial.println(sum);
  sequence_no[index_queue] = sum;  
}
