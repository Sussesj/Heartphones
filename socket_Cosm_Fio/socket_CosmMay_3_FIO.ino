/*
This project is a collaboration between Alex Samuel and Susse Sønderby Jensen, on sending heart beat to cosm, 
having this heart beat calculate at your max hearrate, mim heart rate when sitting in fron of your computer*/

//THIS IS FOR WIFI CONNECTION
#include <WiFlyHQ.h>
#include <Software//Serial.h>
WiFly wifly;

// function definitions
const int resetPin = 11;

/* Change these to match your WiFi network - Need modem  */
const char mySSID[] = "internetz";
const char myPassword[] = "1nt3rn3tz"; 
char site[] = "api.cosm.com";
//char Serialdata[220];
//calibration variables
boolean calibrated = false; //Asking when it is calibrated or not. 

//for accelerometer
int x,y,z;
boolean movement = false;
int xAvg;
int zAvg;
int yAvg;
int pxAvg;
int pyAvg;
int pzAvg;
int numSampl = 50;

int currentState = 0;
int readingCounter = 0;
int calibratedBPM; // holds the calibrated BPM average. 
int bpmCalibrationAdder; // just for calibration averaging
/*int age = 25;*/  

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

//  VARIABLES
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePinBlue = 7;              // pin to do fancy classy fading blink at each beat
int fadePinGreen = 6;
int fadePinRed = 5;               
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin
int numberOfBeats = 0;
int averageBPM = 0;
int averageNumber = 10;
long previousMillis = 0;
long interval = 1000;
boolean subscribed = false;
boolean gotData = false;

// Your Cosm key to let you upload data
char feedId[] = "120070"; //FEED ID
char cosmKey[] = "s5G_p9FMnRRrTezCwJBwJYpIFRySAKxmUDJnRC85aGlNQT0g"; //API KEY
char sensorId[] ="BPM"; //This should not contain a space ' ' char
char sensorId2[] ="currentState"; //This should not contain a space ' ' char
//char sensorId3[] ="currentState"; //This should not contain a space ' ' char
//To include multiple feeds, We can add more. BPM, State when 20 % lower average, State when 20 % higher, state when 40 % higher.  
//char sensorId[] = {"testone","lowHeart"}; //This should not contain a space ' ' char

void setup() {
  // put your setup code here, to run once:
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(fadePinBlue,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(fadePinGreen,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(fadePinRed,OUTPUT);          // pin that will fade to your heartbeat!
  pinMode(resetPin,OUTPUT);
  digitalWrite(resetPin, HIGH);
  Serial.begin(9600);             // we agree to talk fast!
  
//  while (!//Serial) {
//    ; // wait for //Serial port to connect. Needed for Leonardo only
//  }
  ////Serial.println("Starting PULSE upload to Cosm...");
  ////Serial.println();
  
  char buf[32];

  ////Serial.println("Starting");
  ////Serial.print("Free memory: ");
  ////Serial.println(wifly.getFreeMemory(),DEC);
  ////Serial1.begin(115200);
  
  if (!wifly.begin(&Serial)) {
    ////Serial.println("Failed to start wifly");
  }

  /* Join wifi network if not already associated */
  if (!wifly.isAssociated()) {
    /* Setup the WiFly to connect to a wifi network */
    //Serial.println("Joining network");
    wifly.setSSID(mySSID);
    wifly.setPassphrase(myPassword);
    wifly.enableDHCP();

    if (wifly.join()) {
      //Serial.println("Joined wifi network");
    } 
    else {
      //Serial.println("Failed to join wifi network");
    }
  } 
  else {
    //Serial.println("Already joined network");
  }

  //Serial.print("MAC: ");
  //Serial.println(wifly.getMAC(buf, sizeof(buf)));
  //Serial.print("IP: ");
  //Serial.println(wifly.getIP(buf, sizeof(buf)));
  //Serial.print("Netmask:  ");
  //Serial.println(wifly.getNetmask(buf, sizeof(buf)));
  //Serial.print("Gateway: ");
  //Serial.println(wifly.getGateway(buf, sizeof(buf)));

  wifly.setDeviceID("Wifly-WebClient");
  //Serial.print("DeviceID:   ");
  //Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

  if (wifly.isConnected()) {
    //Serial.println("Old connection active. Closing");
    wifly.close();
  }

  if (wifly.open(site, 8081)) {
    //Serial.print("Connected to ");
    //Serial.println(site);
    //subscribeToCosmDatastream();
    /* Send the request */
    //	wifly.println("GET / HTTP/1.0");
    //	wifly.println();
  } 
  else {
    //Serial.println("Failed to connect");
  }
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
}

void loop() {
  if(!subscribed){
    subscribeToCosmDatastream(sensorId2);
    subscribed = true;
  }
    unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) { 
    previousMillis = currentMillis;
    gotData = false;
  }
  
  
  if (wifly.available() > 0 && gotData==false) {
    //char //Serialdata[220];
    //Serial.println("got");
    //int lf = 10;
    int parsed = wifly.parseInt();
    gotData = true;
    wifly.flush();
    //wifly.readBytesUntil(lf, //Serialdata, 220);
    //char ch = wifly.read();
    ////Serial.println(parsed);
    
    switch (parsed){
    case 1:
      //This is the stuff for case 1
      //LED BLUE
      //Serial.println("BLUE");
      digitalWrite(7,HIGH);
      digitalWrite(6,LOW);
      digitalWrite(5,LOW);
      break;
    case 2:
      //this is case 2
      // LED PURPLE
      //Serial.println("PURPLE");
      digitalWrite(7,LOW);
      digitalWrite(6,HIGH);
      digitalWrite(5,LOW);
      break;
    case 3:
      //this is case 3
      // LED RED
      //Serial.println("RED");
      digitalWrite(7,LOW);
      digitalWrite(6,LOW);
      digitalWrite(5,HIGH);
      break;
    }
  }else{
    wifly.read();
  }
  
  //----------------Accelerometer Values-----------------------
  xAvg = 0;
  yAvg = 0;
  zAvg = 0; 
  for (int i = 0; i < numSampl; i++){
    x = analogRead(4);       // read analog input pin 0
    y = analogRead(5);       // read analog input pin 1
    z = analogRead(6);       // read analog input pin 1
    xAvg = (xAvg+x); 
    yAvg = (yAvg+y);
    zAvg = (zAvg+z); 
  }
  xAvg = (xAvg/numSampl);
  yAvg = (yAvg/numSampl);
  zAvg = (zAvg/numSampl);
  //Serial.print(xAvg);
  int deltaX = abs(xAvg - pxAvg);
  //Serial.println(deltaX); 
  pxAvg = xAvg; 
  
  int deltaY = abs(yAvg - pyAvg);
  //Serial.println(deltaY); 
  pyAvg = yAvg; 
  
  int deltaZ = abs(zAvg - pzAvg);
  //Serial.println(deltaZ); 
  pzAvg = zAvg; 

  if(deltaZ >= 200 || deltaX >= 200 || deltaY >= 200) {
    movement = true;
  }
  else{
    movement = false;
  }
//}  
  //sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
  if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
    //fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
    //sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
    //sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
    sendDataToCosm(BPM, movement);             //send data to cosm sockets using arduino ethernet/shield
//    sendDataToCosm(calibratedBPM);      //send data to cosm sockets using arduino ethernet/shield
   // sendTwoDataToCosm(currentState);
    //sendDataToCosm(lowHeart);             //send data to cosm sockets using arduino ethernet/shield
    //Calibrate your BPM after 10 sec, so that you don't send data imidiately 
    if(calibrated == true){
      }
      else{
        currentState = 0;
        //Serial.print("BPM");
        //ledFadeToBeat();
      }
    } 
    else{ 
      // -- when not calibrated
      calibrate();
    }
    QS = false;                      // reset the Quantified Self flag for next time    
  }

//----------------------------- end loop ----------------------------------------------------
void calibrate(){
  //After 25 sec, it starts sending to cosm. 
  if(millis() <= 50000){
    //if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
      fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
      //sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
      //sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
      readingCounter++;
      bpmCalibrationAdder += BPM;
      calibratedBPM = bpmCalibrationAdder/readingCounter;
      //Serial.print("current calibrated BPM:  ");
      //Serial.println(calibratedBPM);
      //QS = false;                      // reset the Quantified Self flag for next time
    //}
  }
  else{
    calibrated = true;
  }
}

void sendDataToProcessing(char symbol, int data, int data2 ){
  //Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
  //Serial.println(data);                // the data to send culminating in a carriage return
}

void sendDataToCosm(int data, int data2){
  //Serial.println("sending:  ");
  //Serial.println(data);
  wifly.print("{\"method\" : \"put\",");
  wifly.print("\"resource\" : \"/feeds/");
  wifly.print(feedId);
  wifly.print("\", \"headers\" :{\"X-ApiKey\" : \"");
  wifly.print(cosmKey);
  wifly.print("\"},\"body\" :{ \"version\" : \"1.0.0\",\"datastreams\" : [");
  
  wifly.print("{\"id\" : \"");
  wifly.print("BPM");
  wifly.print("\",\"current_value\" : \"");
  wifly.print(data);
  wifly.print("\"},");
  
  //Second data stream I'm sending
  wifly.print("{\"id\" : \"");
  wifly.print("movement");
  wifly.print("\",\"current_value\" : \"");
  wifly.print(data2);
  wifly.print("\"}"); 
  
  wifly.print("]}}");
  wifly.println();
}

void getDataFromCosm(int dataStream){
  //Serial.println("getting:  ");
  //Serial.println(dataStream);
  wifly.print("{\"method\" : \"get\",");
  wifly.print("\"resource\" : \"/feeds/");
  wifly.print(feedId);
  wifly.print("datastreams/");
  wifly.print(dataStream);
  wifly.print("\", \"headers\" :{\"X-ApiKey\" : \"");
  wifly.print(cosmKey);
  wifly.print("\"}");
  wifly.println();
}
void subscribeToCosmDatastream(char datastream[]){
  //Serial.println("subscribing:  ");
  //Serial.println(datastream);
  wifly.print("{\"method\" : \"subscribe\",");
    wifly.print("\"resource\" : \"/feeds/");
    wifly.print(feedId);
    wifly.print("/datastreams/");
    wifly.print(datastream);
    wifly.print("\", \"headers\" :{\"X-ApiKey\" : \"");
    wifly.print(cosmKey);
    wifly.print("\"}}");
    wifly.println();
    wifly.println();
}
/*
{
  "method" : "subscribe",
  "resource" : "/feeds/504/datastreams/0",
  "headers" :
    {
      "X-ApiKey" : "API_KEY"
    },
  "token" : "0xabcdef"
}

*/










