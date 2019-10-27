/* 
 Created 10 Oct. 2018
  by Peter Chodyra
 */

/*Vertical blind turn range 180 deg 84 beeds, 1 rotation = 16 beeds 
5.25 rotations = 180 deg
2.62 rotations = 90 deg
1.31 rotations or 21 beeds = 45 deg 
*/

#include <ArduinoJson.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager

#include <Stepper.h>
#include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

//#define DEBUG

//IMPORTANT Connect ULN2003 PINs to NodeMCU D pins in order PIN1 -> D1, PIN2 -> D5, PIN3 -> D2,  PIN4 -> D6
#define PIN1 D1 
#define PIN2 D2
#define PIN3 D5
#define PIN4 D6

// ------------------------------------------------------------------------
// ############################# Global Definitions #########################
// ------------------------------------------------------------------------
int photocellPin = A0;     // the cell and 100K pulldown are connected to a4
int MAXSteps = 0; //Global variable fo rthe maximum steps to open blinds = 100% open

const int moveSteps = 100; //Steps to advance the motor at setup
const int stepsPerRevolution = 2048; //this has been changed to 2048 for the 28BYJ-48
int movesLeft = 0; //This hold the current remaining percent of move to make, this will be used in main loop to prevent blocking behaviour of teh stepper function 
int target = 0;
int advanceAmount = 1; //for non blocking stepper set the advance amount each itteration

// ------------------------------------------------------------------------
// ############################# EEPROM Definitions #########################
// ------------------------------------------------------------------------
int addr = 0; //EEPROM Address memory space for the current_position of the blinds
struct EEPROMStruct {
  int MAXstate;
  int NOWstate;
  int OPENstate;
  int LUXstate;
} eepromVar;

//const int revolutions = 1;

// ------------------------------------------------------------------------
// ############################# HTML Definitions #########################
// ------------------------------------------------------------------------

const String HTMLheader = "<!DOCTYPE html><html><head>"\
"<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.3.1/css/all.css\" integrity=\"sha384-mzrmE5qonljUremFsqc01SB46JvROS7bZs3IO2EmfFsd15uHvIt+Y8vEf7N7fWAU\" crossorigin=\"anonymous\">"\
"</head>"\
"<style>"\
" .koobee_wrap{   "\
"    width: 100%;"\
"    margin: 0 auto;"\
"    background-color: grey;}"\
" .koobee {"\
"padding:10px; margin:5px;}"\
"  .blue_top{"\
"    background: #4096ee;"\ 
"background: -moz-linear-gradient(top, #4096ee 0%, #60abf8 44%, #7abcff 100%); "\
"background: -webkit-linear-gradient(top, #4096ee 0%,#60abf8 44%,#7abcff 100%); "\
"background: linear-gradient(to bottom, #4096ee 0%,#60abf8 44%,#7abcff 100%);}"\
"  .blue {"\
"    background: #7abcff; "\
"background: -moz-linear-gradient(top, #7abcff 0%, #60abf8 44%, #4096ee 100%); "\
"background: -webkit-linear-gradient(top, #7abcff 0%,#60abf8 44%,#4096ee 100%); "\
"background: linear-gradient(to bottom, #7abcff 0%,#60abf8 44%,#4096ee 100%); }"\
"  .green {"\
"    background: #f8ffe8; "\
"background: -moz-linear-gradient(top, #f8ffe8 0%, #e3f5ab 33%, #b7df2d 100%); "\
"background: -webkit-linear-gradient(top, #f8ffe8 0%,#e3f5ab 33%,#b7df2d 100%); "\
"background: linear-gradient(to bottom, #f8ffe8 0%,#e3f5ab 33%,#b7df2d 100%); }"\
"  .lavender {"\
"    background: #c3d9ff; "\
"background: -moz-linear-gradient(top, #c3d9ff 0%, #b1c8ef 41%, #98b0d9 100%);"\ 
"background: -webkit-linear-gradient(top, #c3d9ff 0%,#b1c8ef 41%,#98b0d9 100%); "\
"background: linear-gradient(to bottom, #c3d9ff 0%,#b1c8ef 41%,#98b0d9 100%); }"\
"  .orange{"\
"    background: #ffa84c; "\
"background: -moz-linear-gradient(top, #ffa84c 0%, #ff7b0d 100%); "\
"background: -webkit-linear-gradient(top, #ffa84c 0%,#ff7b0d 100%); "\
"background: linear-gradient(to bottom, #ffa84c 0%,#ff7b0d 100%); }"\
" .red{"\
"background: #ff776b;"\
"background: -moz-linear-gradient(top, #ff776b 0%, #f25252 100%); "\
"background: -webkit-linear-gradient(top, #ff776b 0%,#f25252 100%);"\
"background: linear-gradient(to bottom, #ff776b 0%,#f25252 100%);};"\
"  .koobee h1 {font-family: Gotham, \"Helvetica Neue\", Helvetica, Arial, \"sans-serif\";}"\
"  .koobee p {font-family: Gotham, \"Helvetica Neue\", Helvetica, Arial, \"sans-serif\";font-size: 28px;}"\
"  .btnrow1{text-align: center;}"\
"  .btns{"\
"    text-align: center;"\
"    width: 100%}"\
"  .menu {padding: 5px;}"\
"  .menu a {color: black;"\
"  text-decoration: none;}"\
".koobee button2 {"\
"     width: 105px;"\
"    display: inline-block;"\
"    border: none;"\
"    padding: 1rem 2rem;"\
"    margin: 0;"\
"    text-decoration: none;"\
"    background: #0069ed;"\
"    color: #ffffff;"\
"    font-family: sans-serif;"\
"    font-size: 1rem;"\
"    line-height: 1;"\
"    cursor: pointer;"\
"    text-align: center;"\
"    transition: background 250ms ease-in-out, transform 150ms ease;"\
"    -webkit-appearance: none;"\
"    -moz-appearance: none;"\
"}"\
".koobee button2:hover,button:focus {background: #0053ba;}"\
".koobee button2:focus {outline: 1px solid #fff;outline-offset: -4px;}"\
".koobee button2:active {transform: scale(0.99);}"\
".koobee button {"\
"     width: 115px;"\
"-moz-box-shadow: 0px 10px 14px -7px #3e7327;"\
" -webkit-box-shadow: 0px 10px 14px -7px #3e7327;"\
"  box-shadow: 0px 10px 14px -7px #3e7327;"\
"  background-color:#77b55a;"\
"  -moz-border-radius:4px;"\
"  -webkit-border-radius:4px;"\
"  border-radius:4px;"\
"  border:1px solid #4b8f29;"\
"  display:inline-block;"\
"  cursor:pointer;"\
"  color:#ffffff;"\
"  font-family:Arial;"\
"  font-size:2em;"\
"  font-weight:bold;"\
"  padding:6px 12px;"\
"  margin-bottom: 40px;"\
"  text-decoration:none;"\
"  text-shadow:0px 1px 0px #5b8a3c;}"\
".koobee button:hover { background-color:#72b352;}"\
".koobee button:active {position:relative;top:1px;}"\
"  </style>"\
"<body><div class=\"koobee_wrap\">";

const String HTMLfooter = "</div></body></html>";

const String HTMLheading =" <div class=\"koobee blue_top\">"\
"      <h1 style=\"text-align: center;\">KOOBEE blinds<br>Control Panel</h1>"\
"  </div>";

const String HTMLsetup = "<div class=\"koobee red\">"\
"  <h1 style=\"text-align: center;\">Setup</h1>"\
"  <p style=\"text-align:justify\">Use the MORE and LESS buttons to adjust the OPEN limit of the blinds. Click SAVE to save your settings. RESET will reset the blinds to a closed position. Clicking MORE or LESS will move the motor by 100 steps. To increase the number of steps and advance faster, change the query string in the browser URL to e.g.<u>\\api\\position?move=1000</u>.</p>"\
"    <table class=\"btns\">"\
"    <tr>"\
"      <td style=\"text-align: center;\"><a href=\"\\api\\position?move=100\"><button>More</button></a></td>"\
"      <td></td>"\
"      <td style=\"text-align: center;\"> <a href=\"\\api\\position?move=-100\"><button>Less</button></a></td>"\
"    </tr>"\
"    <tr>"\
"      <td colspan=\"3\" style=\"text-align: center;\">"\
"         <a href=\"\\api\\save\"><button>Save</button></a>"\
"      </td>"\
"    </tr>"\
"    <tr>"\
"      <td colspan=\"3\" style=\"text-align: center;\">"\
"         <a href=\"\\api\\reset\"><button>Reset</button></a>"\
"      </td>"\
"    </tr>"\
"      </table>"\
"    </div>";

const String HTMLhelp = "  <div class=\"koobee green\">"\
"      <h1 style=\"text-align: center;\">Information</h1>"\
"      <p style=\"text-align:justify\">To control the motor via WiFi and the builtin API, use the following GET and PUT API calls.</p>"\
"        <p>"\
"        <ul style=\"font-size: 1.5em\">"\
"          <li><b>/api/lux</b> - GET : returns the light level as {\"lightlevel\":240}</li>"\
"          <li><b>/api/status</b> - GET : returns the motor position in % {\"position\":20}</li>"\
"          <li><b>/api/blinds?open=[x]</b> - PUT : moves the motor to position x%</li>"\
"        </ul>"\
"      </p>"\
"  </div>";

const String HTMLsavedone = "<div class=\"koobee green\" style=\"text-align: center;\"><p><h1>Setup Complete!</h1></p></div>";
const String HTMLresetdone = "<div class=\"koobee green\" style=\"text-align: center;\"><p><h1>Reset Complete!</h1></p></div>";

const String HTMLmove = "<div class=\"koobee blue\">"\
  "<h1 style=\"text-align: center;\">Operation</h1>"\
  "<p>Click on the % buttons to move the blinds to the desired open state. Ensure you have gone through the setup process first.</p>"\
    "<table class=\"btns\">"\
    "<tr>"\
      "<td><a href=\"\\api\\blinds?open=0\"><button>0%</button></a></td>"\
      "<td></td>"\
      "<td> <a href=\"\\api\\blinds?open=100\"><button>100%</button></a></td>"\
    "</tr>"\
    "<tr>"\
      "<td><a href=\"\\api\\blinds?open=10\"><button>10%</button></a></td>"\
      "<td><a href=\"\\api\\blinds?open=20\"><button>20%</button></a></td>"\
      "<td><a href=\"\\api\\blinds?open=30\"><button>30%</button></a></td>"\
    "</tr>"\
    "<tr>"\
      "<td><a href=\"\\api\\blinds?open=40\"><button>40%</button></a></td>"\
      "<td><a href=\"\\api\\blinds?open=50\"><button>50%</button></a></td>"\
      "<td><a href=\"\\api\\blinds?open=60\"><button>60%</button></a></td>"\
    "</tr>"\
    "<tr>"\
      "<td><a href=\"\\api\\blinds?open=70\"><button>70%</button></a></td>"\
      "<td><a href=\"\\api\\blinds?open=80\"><button>80%</button></a></td>"\
      "<td><a href=\"\\api\\blinds?open=90\"><button>90%</button></a></td>"\
    "</tr>"\
    "</table>"\
  "</div>";

const String HTMLmenu = "<div class=\"koobee orange\">"\
 "<div class=\"menu\">"\
  "<table style=\"width: 100%;\"><tr style=\"text-align: center\">"\
    "<td id=\"home\"><a href=\"\\\"><i class=\"fas fa-home fa-4x\"></i></a></td>"\
    "<td id=\"operate\"><a href=\"\\api\\blinds\"><i class=\"fas fa-arrows-alt-h fa-4x\"></i></a></td>"
    "<td id=\"setup\"><a href=\"\\api\\setup\"><i class=\"fas fa-cog fa-4x\"></i></a></td>"\
    "<td id=\"info\"><a href=\"\\api\\help\"><i class=\"fas fa-info-circle fa-4x\"></i></a></td>"\
  "</tr></table></div></div>";

String HTMLstatus ="";

//Stepper setup
const int clockwise = 1;
const int counterclockwise = -1;

// initialize the stepper library on pins 1 through 4:        
Stepper myStepper(stepsPerRevolution, PIN1,PIN2,PIN3,PIN4);   //note the modified sequence D1, D5, D2, D6 

//Web server setup
ESP8266WebServer server(80);   //Web server object. Will be listening in port 80 (default for HTTP)

// ------------------------------------------------------------------------
// ############################# getHTMLstatus() #############################
// ------------------------------------------------------------------------
void getHTMLstatus(){
  eepromVar.LUXstate = getLux(photocellPin);
  HTMLstatus ="<div class=\"koobee lavender\"><p style=\"text-align:center;\">"\
  "Open LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"\
  "Current position is: "+String(eepromVar.NOWstate)+"<br>"\
  "Open position is: "+String(eepromVar.OPENstate)+"%<br>"\
  "Light level is: "+String(eepromVar.LUXstate)+"</p></div>";
}
// ------------------------------------------------------------------------
// ############################# handleRoot() #############################
// ------------------------------------------------------------------------
void handleRoot() {
  server.send(200, "text/plain", HTMLheader+HTMLhelp+HTMLsetup+HTMLmove+HTMLfooter);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// ------------------------------------------------------------------------
// ############################# getBlindsPosition() #############################
// ------------------------------------------------------------------------

int getBlindsPosition(){
  return eepromVar.OPENstate;
}

// ------------------------------------------------------------------------
// ############################# jsonOutput() #############################
// ------------------------------------------------------------------------

String jsonOutput(String jName,int jValue){  
  String output;
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root[jName] = jValue;
  root.printTo(output);
  return output;
}


// ------------------------------------------------------------------------
// ############################# advanceBlinds() #############################
// ------------------------------------------------------------------------

void advanceBlinds() {
  //non blocking stepper move
  int advanceSteps = map(advanceAmount,0,100,0,eepromVar.MAXstate);
  int targetSteps = map(target,0,100,0,eepromVar.MAXstate);
  
  if (targetSteps-eepromVar.NOWstate > 0){
    //move the motor clockwise
    eepromVar.OPENstate=eepromVar.OPENstate+advanceAmount;
    eepromVar.NOWstate=eepromVar.NOWstate+advanceSteps;
    //move the motor
    myStepper.step(advanceSteps * clockwise);
  } else if (targetSteps-eepromVar.NOWstate < 0) {
    //move the motor counterclockwise
    eepromVar.OPENstate=eepromVar.OPENstate-advanceAmount;
    eepromVar.NOWstate=eepromVar.NOWstate-advanceSteps;
    //move the motor
    myStepper.step(advanceSteps * counterclockwise);
   
  } else {
    //is 0 and no meed to move the blinds
  }
     EEPROM.put(addr, eepromVar);
    // write the data to EEPROM
    boolean ok2 = EEPROM.commit();
#ifdef DEBUG     
    Serial.println((ok2) ? "Commit OK" : "Commit failed");
#endif   

  //Power off the motor
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN3,LOW);
  digitalWrite(PIN4,LOW);
}

// ------------------------------------------------------------------------
// ############################# moveBlinds() #############################
// ------------------------------------------------------------------------
void moveBlinds(int percent){

  //map percentage to number of steps where 2860 is fully open or 100%
  int newSteps = map(percent, 0,100,0,eepromVar.MAXstate);
  
  if (newSteps != eepromVar.NOWstate) {
    myStepper.step(newSteps - eepromVar.NOWstate);
    eepromVar.NOWstate = newSteps;
    eepromVar.OPENstate=percent;
    EEPROM.put(addr, eepromVar);
    // write the data to EEPROM
    boolean ok2 = EEPROM.commit();
#ifdef DEBUG     
    Serial.println((ok2) ? "Commit OK" : "Commit failed");
#endif     
  }
  else {
    //do nothing newSteps = currentSteps
  }

 #ifdef DEBUG 
    Serial.print("Moving Percent : ");
    Serial.print(percent);   
    Serial.print(" Moving Steps : ");
    Serial.print(newSteps-eepromVar.NOWstate);
    Serial.print(" currentSteps is: ");
    Serial.print(eepromVar.NOWstate);
    Serial.print(" newSteps is: ");
    Serial.print(newSteps);
    Serial.print(" MAXSteps is: ");
    Serial.println(eepromVar.MAXstate);
#endif  

#ifdef DEBUG 
  Serial.print("Blinds are at ");
  Serial.print(percent);   
  Serial.println("%");
#endif 
  //Power off the motor
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN3,LOW);
  digitalWrite(PIN4,LOW);

}
// ------------------------------------------------------------------------
// ############################# setBlindsMore() #############################
// ------------------------------------------------------------------------
void setBlindsMore(){

#ifdef DEBUG    
    Serial.print("Moving Steps to: ");
    Serial.println(moveSteps*clockwise);
#endif    
    myStepper.step(moveSteps*clockwise);
    eepromVar.MAXstate = eepromVar.MAXstate + moveSteps;
    eepromVar.NOWstate = eepromVar.NOWstate + moveSteps;
    eepromVar.OPENstate=100; //During setup always assume the current position is at 100% open
    
#ifdef DEBUG 
  Serial.print("Blinds are at ");
  Serial.print(eepromVar.MAXstate);   
  Serial.println(" steps");
#endif 
  //Power off the motor
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN3,LOW);
  digitalWrite(PIN4,LOW);

}
// ------------------------------------------------------------------------
// ############################# setBlindsLess() #############################
// ------------------------------------------------------------------------
void setBlindsLess(){

#ifdef DEBUG    
    Serial.print("Moving Steps to: ");
    Serial.print(moveSteps*counterclockwise);
#endif
    if (eepromVar.NOWstate != 0) {     
      myStepper.step(moveSteps*counterclockwise);
      eepromVar.MAXstate = eepromVar.MAXstate - moveSteps;
      eepromVar.NOWstate = eepromVar.NOWstate - moveSteps;
      //MAXSteps=MAXSteps-moveSteps;
      //saveEEPROMState(MAXSteps, addr2); //Store the current blind position in the non volotile memory  
      //saveEEPROMState(MAXSteps, addr); //Save the currentSteps position 
      eepromVar.OPENstate=100; //During setup always assume the current position is at 100% open
    } else {
      //can not go past 0 steps  
    }
    
#ifdef DEBUG 
  Serial.print(" Blinds are at ");
  Serial.print(eepromVar.MAXstate); 
  Serial.println(" steps");  
#endif 
  //Power off the motor
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN3,LOW);
  digitalWrite(PIN4,LOW);

}
// ------------------------------------------------------------------------
// ############################# saveBlindsPosition() #############################
// ------------------------------------------------------------------------
void saveBlindsPosition(){  

    eepromVar.OPENstate=100; //At the completion of the setup the current position is at 100% open 
    EEPROM.put(addr, eepromVar);
    // write the data to EEPROM
    boolean ok2 = EEPROM.commit();
#ifdef DEBUG     
    Serial.println((ok2) ? "Commit OK" : "Commit failed");
#endif     


}

// ------------------------------------------------------------------------
// ############################# resetBlindsPosition() #############################
// ------------------------------------------------------------------------
void resetBlindsPosition(){  

    //Reset the position of the blinds to 0  
    eepromVar.MAXstate=0;
    eepromVar.NOWstate=0;
    eepromVar.OPENstate=0;
    EEPROM.put(addr, eepromVar);
    // write the data to EEPROM
    boolean ok2 = EEPROM.commit();
#ifdef DEBUG     
    Serial.println((ok2) ? "Commit OK" : "Commit failed");
#endif     


}

// ------------------------------------------------------------------------
// ############################# getLux() #############################
// ------------------------------------------------------------------------

int getLux(int Pin){
  // the analog reading from the sensor divider
  int photocellReading = analogRead(Pin);
  //Convert phocell analog to lux
  int lux = map(photocellReading, 0, 1023, 0, 800);

#ifdef DEBUG 
  Serial.print("Analog reading RAW = ");
  Serial.print(photocellReading);     // the raw analog reading
  Serial.print(" LUX= ");
  Serial.println(lux);
#endif

  return lux;
}
// ------------------------------------------------------------------------
// ############################# handleOpenArgs() #############################
// ------------------------------------------------------------------------
void handleOpenArgs(){

  String message = "";

  if (server.arg("open")== ""){     //Parameter not found
    message = "Open Argument not found";
  }else{     //Parameter found
    message = "Manual input trigered Open Argument = ";
    message += server.arg("open");     //Gets the value of the query parameter
    //Move the blinds
    //moveBlinds(atoi(server.arg("open").c_str()));
    target=atoi(server.arg("open").c_str()); //target to open to
    movesLeft=abs(eepromVar.OPENstate-target); //change the global variale to percent to move the blinds
  }
 #ifdef DEBUG  
  //server.send(200, "text/plain", message);          //Returns the HTTP response
 #endif   

}
// ------------------------------------------------------------------------
// ############################# handleMoveArgs() #############################
// ------------------------------------------------------------------------
void handleMoveArgs(){

  String message = "";
  int moveSteps = 0; //Ths value can either be positive -> clockwise rotation or negative -> counetrclockwise rotation
  
  if (server.arg("move")== ""){     //Parameter not found
    message = "Move Argument not found";
  }else{     //Parameter found
    message = "Manual input trigered Open Argument = ";
    message += server.arg("move");     //Gets the value of the query parameter
    //Move the blinds
    moveSteps = atoi(server.arg("move").c_str());
    
    if (eepromVar.NOWstate + moveSteps >= 0) {     
      myStepper.step(moveSteps);
      eepromVar.MAXstate = eepromVar.MAXstate + moveSteps;
      eepromVar.NOWstate = eepromVar.NOWstate + moveSteps;
      //MAXSteps=MAXSteps-moveSteps;
      //saveEEPROMState(MAXSteps, addr2); //Store the current blind position in the non volotile memory  
      //saveEEPROMState(MAXSteps, addr); //Save the currentSteps position 
      eepromVar.OPENstate=100; //During setup always assume the current position is at 100% open
    } else {
      //can not go past 0 steps  
    }
  }
  //Power off the motor
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN3,LOW);
  digitalWrite(PIN4,LOW);
}
// ------------------------------------------------------------------------
// ############################# setup() #############################
// ------------------------------------------------------------------------

void setup() {
  
  digitalWrite(PIN1,OUTPUT);
  digitalWrite(PIN2,OUTPUT);
  digitalWrite(PIN3,OUTPUT);
  digitalWrite(PIN4,OUTPUT);
  
  // set the speed (needed to be reduced for the 28BYJ-48):
  myStepper.setSpeed(6);
  // initialize the serial port:
  Serial.begin(115200);
  Serial.println();

  //Setup WiFiManager
  WiFiManager wfManager;
  //Switchoff debug mode
  wfManager.setDebugOutput(false);

  //exit after config instead of connecting
  wfManager.setBreakAfterConfig(true);
  
  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  
  if (!wfManager.autoConnect()) {
    Serial.println("failed to connect, resetting and attempting to reconnect");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected... ");

  //server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
  
  server.on("/", [](){
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLstatus+HTMLmenu+HTMLfooter);
  });

  server.on("/api/help", []() {
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLhelp+HTMLstatus+HTMLmenu+HTMLfooter);
  });

  //hande querystring /blinds?open=20
  server.on("/api/blinds",[](){
    handleOpenArgs();
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLmove+HTMLstatus+HTMLmenu+HTMLfooter);
  });
  
  server.on("/api/setup", [](){
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLsetup+HTMLstatus+HTMLmenu+HTMLfooter);
  });
  
  server.on("/api/position", [](){
    //When ?move=100 is passed move the blinds 100 steps
    handleMoveArgs();
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLsetup+HTMLstatus+HTMLmenu+HTMLfooter);
  });

  server.on("/api/save", [](){
    saveBlindsPosition();
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLsavedone+HTMLstatus+HTMLmenu+HTMLfooter);
  });
  
    server.on("/api/reset", [](){
    resetBlindsPosition();
    //String HTMLstatus ="<p style=\"text-align:center;\">LIMIT is set to: "+String(eepromVar.MAXstate)+"<br>"+"Current position is: "+String(eepromVar.NOWstate)+"</p>";
    getHTMLstatus();
    server.send(200, "text/html", HTMLheader+HTMLheading+HTMLresetdone+HTMLstatus+HTMLmenu+HTMLfooter);
  });

    server.on("/api/lux", []() {
    eepromVar.LUXstate = getLux(photocellPin);  
    server.send(200, "application/json", jsonOutput("lightlevel",eepromVar.LUXstate));
  });

  server.on("/api/status", [](){
    server.send(200, "application/json", jsonOutput("position",getBlindsPosition()));
  });
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  eepromVar.MAXstate = -1;
  eepromVar.NOWstate = -1;
  eepromVar.OPENstate = -1;
  eepromVar.LUXstate = -1;
  
  EEPROM.begin(sizeof(EEPROMStruct));

  EEPROM.get(addr, eepromVar); //read the eeprom structure
  
  //currentSteps = getEEPROMState(addr); //Get previous steps number
  //MAXSteps = getEEPROMState(addr2); //Get previous MAXsteps number

  if (eepromVar.MAXstate < 0 || eepromVar.NOWstate < 0 || eepromVar.OPENstate < 0) {
    //eeprom has been corrupted or not set
    eepromVar.MAXstate = 0;
    eepromVar.NOWstate = 0;
    eepromVar.OPENstate = 0;
    moveBlinds(0); //rest blind position to closed on startup
    EEPROM.put(addr, eepromVar);
   }
   
  eepromVar.LUXstate = getLux(photocellPin); 
  
  //if (currentSteps == 0) {
    //device was reset
  //  eepromVar.NOWstate = 0;
  //  moveBlinds(0); //rest blind position to closed on startup
  //  EEPROM.put(addr, eepromVar);
 // }

#ifdef DEBUG    
    Serial.print(" currentSteps is: ");
    Serial.print(eepromVar.NOWstate);
    Serial.print(" MAXSteps is: ");
    Serial.println(eepromVar.MAXstate);
#endif  

  //Power off the motor
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN3,LOW);
  digitalWrite(PIN4,LOW);
}

// ------------------------------------------------------------------------
// ############################# loop() #############################
// ------------------------------------------------------------------------
void loop() {

  if(movesLeft != 0) {
    //move the steper motor by advanceAmount 
     advanceBlinds();
#ifdef DEBUG    
    Serial.print("movesLeft is: ");
    Serial.println(movesLeft);
#endif  
     movesLeft=movesLeft-advanceAmount;
  } //else wait for webserver request
  
  //Handle server requests
  server.handleClient();
    
}
