/*
 * ESP8266 (NodeMCU) Handling Web form data basic example
 * https://circuits4you.com
 */
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <MFRC522.h>
#include <ESP8266WebServer.h>
#include <Wire.h>       //I2C library
#include <RtcDS3231.h>  //RTC library
#include <FS.h>
bool    fsOK = false;
const char* fsName = "SPIFFS";
FS* fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
String prevUID;
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
const char MAIN_page[] = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/pass" method="POST">
    <center>
    PASSWORD: <input type="password" name="PASSWORD">
    <input type="submit" value="Submit">
    </center>
  </form><br>
</body></html>)rawliteral";


const char datetime[] =R"rawliteral(
<!DOCTYPE HTML><html><head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/setRTC">
  <center>
    MM/YY: <input type="datetime-local" name="datetime" >
    <input type="submit" value="Set">
    </center>
  </form><br>
</body></html>)rawliteral";
//SSID and Password of your WiFi router
const char* ssid = "Bus number 1";
const char* password = "12345678";
const char* serverPassword="admin";

#define RST_PIN         16         // Configurable, see typical pin layout above
#define SS_PIN          2          // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
byte len=18;
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
String flname;
int prev_month;
RtcDS3231<TwoWire> rtcObject(Wire);
ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleMainPage() //front page
{
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
void replyNotFound(String msg) 
{
  server.send(404, FPSTR("text/plain"), msg);
}


String NowDate()
{
RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC
    char date[12];   //declare a string as an array of chars
    sprintf(date, "%d/%d/%d",     //%d allows to print an integer to the string
          currentTime.Day(),   //get day method
          currentTime.Month(),  //get month method
          currentTime.Year()    //get year method
        );
  return String(date);
}

String NowTime()
{
RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC
  char time[10];
  sprintf(time,"%d:%d",
          currentTime.Hour(),   //get hour method
          currentTime.Minute() //get minute method
         );
  return String(time);
}

void handleSetDateAndTime()
{
  String s = datetime; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handelRtc()//to set date and time
{
  String datetime = server.arg("datetime");
  Serial.println(datetime);
  server.send(200, "text/html","<a href='/pass?PASSWORD=admin'><center><h2> Click to return home</h2> </center> </a>" );
  String date = datetime.substring(0,9) , time=datetime.substring(11);
  time = time + ":00";
  int dd= (datetime.substring(5,7)).toInt();
   Serial.println("dd : ");
    Serial.println(dd);
  String dat;
  switch(dd)//to format data
  {
    case 1:
    dat ="Jan";
    break;
    case 2:
    dat ="Feb";
    break;
    case 3:
    dat ="Mar";
    break;
    case 4:
    dat ="Apr";
    break;
    case 5:
    dat ="May";
    break;
    case 6:
    dat ="Jun";
    break;
    case 7:
    dat ="Jul";
    break;
    case 8:
    dat ="Aug";
    break;
    case 9:
    dat ="Sep";
    break;
    case 10:
    dat ="Oct";
    break;
    case 11:
    dat ="Nov";
    break;
    case 12:
    dat ="Dec";
    break;
  }

  dat=dat+" " +datetime.substring(8,10) + " " + datetime.substring(0,4);
  Serial.println("date : "+ dat + " time : "+ time);
  RtcDateTime currentTime = RtcDateTime(dat.c_str(), time.c_str()); //define date and time object
  rtcObject.SetDateTime(currentTime); //configure the RTC with object
  Serial.println("Current time set : ");
  delay(10);


}
  


//password authentication
//=============================================
void handlePassword() 
{
 String pass = server.arg("PASSWORD"); 

 Serial.print("password:");
 Serial.println(pass);
 if(strcmp(pass.c_str(),serverPassword)==0)
 {
    Serial.println("access granted");
   
    String s=R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/date">
  <center>
    <h1>xxx</h1>
    <h2>yyy</h1>
    MM/YY: <input type="month" name="fname">
    <input type="submit" value="Submit">
    <h3><a href="/set">Set date and time</a></h3>
    </center>
  </form><br>
</body></html>)rawliteral";

s.replace("xxx",NowDate());
s.replace("yyy",NowTime());
Serial.println(NowDate());
Serial.println(NowTime());

    server.send(200, "text/html", s); //Send web page
 }
 else
 {
   Serial.println("Wrong password");
 }
}

//create link for file download
//==================================
void handledownload() 
{
  String fname ="/";
  fname= fname + server.arg("fname") +".csv";
String s = "<center><h2><a href='";
s = s+ fname;
s= s + "'> Click to DOWNLOAD  </a></h2></center>"; //link to download file
 server.send(200, "text/html", s);
 delay(300);

}
void replyServerError(String msg)
 {
  Serial.println(msg);
  server.send(500, FPSTR("text/plain"), msg + "\r\n");
}

// streming file to user

bool handleFileRead(String path) {
  Serial.println(String("handleFileRead: ") + path);
  if (!fsOK) {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/")) {
    path += "index.htm";
  }

  String contentType;
  if (server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      Serial.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}
//download and file note found is done hear
//==========================================
void handleNotFound() 
{//download and file note found is done hear
  if (!fsOK) {
    return replyServerError(FPSTR("FS INIT ERROR"));
  }

  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  if (handleFileRead(uri)) {
    return;
  }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i = 0; i < server.args(); i++) {
    message += F(" NAME:");
    message += server.argName(i);
    message += F("\n VALUE:");
    message += server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += server.arg("path");
  message += '\n';
  Serial.print(message);

  return replyNotFound(message);
}

String readCard(byte block)//read data from differetn vlocks in card
{
  len=18;
   for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
   byte buffer[18];
   String data;
   status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
   if (status != MFRC522::STATUS_OK) 
   {
     Serial.print(F("Authentication failed: "));
     Serial.println(mfrc522.GetStatusCodeName(status));
     return "ERROR";
   }
   else
   {
     status = mfrc522.MIFARE_Read(block, buffer, &len);
     if (status != MFRC522::STATUS_OK) 
     {
       Serial.print(F("Reading failed: "));
       Serial.println(mfrc522.GetStatusCodeName(status));
       len=18;     
       return "ERROR";
     }
     else
     {
       data = String((char*)buffer); 
       return data; 
     }
   }       
   
} 

void deleteAllFiles()
{ 
  Serial.println("Initiated file formating.....!!");
  if(SPIFFS.format())
  {
    Serial.println("File System Formated");
    replyNotFound("File System Formated");
  }
  else
  {
    Serial.println("File System Formatting Error");
    replyNotFound("File System Formatting Error");
  }
}

//===========================================to create new file and delete old files                         
void CreateNew()
{
    // initilising file name
  RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC

  prev_month=currentTime.Month();
   char fln[14];   //declare a string as an array of chars
 
    if(currentTime.Month() < 10)
    {
       sprintf(fln, "/%d-0%d.csv",     //set file name
        currentTime.Year(),    //get year method
        currentTime.Month()  //get month method
     ); 
    }
    else
    {
      sprintf(fln, "/%d-%d.csv",     //set file name
       currentTime.Year(),    //get year method
       currentTime.Month()  //get month method
        );
    }                
   flname = String(fln);
  File fl = SPIFFS.open(flname.c_str(), "r");
  
  if (!fl) 
  {
    Serial.println("file open failed");
    File fl = SPIFFS.open(flname.c_str(), "w");
    if (!fl) 
    {
     Serial.println("file open failed");
    }
    else
   {
      //Write data to file
      Serial.println("Writing heading to new file");
      fl.print("UID,ID,FIRST_NAME,LAST_NAME,DATE,TIME");
      fl.close();  //Close file
   }
  }
  else
  {
  fl.close();
  }
// delete 1 year old file
    if(currentTime.Month() < 10)
    {
       sprintf(fln, "/%d-0%d.csv",     //set file name
        currentTime.Year()-1,    //get year method
        currentTime.Month()  //get month method
     ); 
    }
    else
    {
      sprintf(fln, "/%d-%d.csv",     //set file name
       currentTime.Year()-1,    //get year method
       currentTime.Month()  //get month method
        );
    }
String delFname =String(fln);
SPIFFS.remove(delFname.c_str());
}

//==============================================================
//                  SETUP
//==============================================================
void setup()
{
  Serial.begin(115200);
  SPI.begin();    
  rtcObject.Begin();                                              // Init SPI bus
  prevUID="0000000";
  mfrc522.PCD_Init();
  Serial.println(F("Read personal data on a MIFARE PICC:")); 
  if(SPIFFS.begin())  //initiating spiffs file system
  {
    Serial.println("SPIFFS Initialize....ok");
    fsOK = true;
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
    fsOK = false;
  }


  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  WiFi.softAPIP();
  Serial.println("AP IP address: ");
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.softAPIP());

  server.on("/", handleMainPage);    //sends mainpage
  server.on("/pass", handlePassword);//password authentication
  server.on("/date",handledownload); //create link to download file
  server.on("/set",handleSetDateAndTime);//set date and time
  server.on("/format",deleteAllFiles);
  server.on("/setRTC",handelRtc);
  server.onNotFound(handleNotFound); //handle file download and errors

  server.begin();                  //Start server
  Serial.println("HTTP server started");

  void CreateNew(); // create new file if the file doesnot exist
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests

  len=18;  

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
//get data from card
  String firstName,lastName,id,uidString;
  firstName = readCard(4);
  lastName =  readCard(5);
  id = readCard(2);
  uidString = String(mfrc522.uid.uidByte[0]) + String(mfrc522.uid.uidByte[1]) + String(mfrc522.uid.uidByte[2]) + String(mfrc522.uid.uidByte[3]);
  
  Serial.print("first name : ");
  Serial.println(firstName);
  Serial.print("last name : ");
  Serial.println(lastName);
  Serial.print("ID : ");
  Serial.println(id);
  Serial.print("Card uid : ");
  Serial.println(uidString);

  Serial.println(F("\n**End Reading**\n"));

  RtcDateTime currentTime = rtcObject.GetDateTime();    //get the time from the RTC
  if( prev_month != currentTime.Month())
  CreateNew();

  

  if(strcmp(firstName.c_str(),"ERROR")==0 || strcmp(lastName.c_str(),"ERROR")==0 || strcmp(id.c_str(),"ERROR")==0 || strcmp(prevUID.c_str(),uidString.c_str())==0 )
  {
   Serial.println("Error occured");
  }
  else
  {
    //open door
    prevUID=uidString;
    String dataToWrite = uidString +","+ id +","+ firstName +","+ lastName +","+ NowDate() +","+NowTime(); 
    dataToWrite.replace("\n","");//removes white spaces
    Serial.println("Data written to file : " + dataToWrite);
    dataToWrite ="\n"+dataToWrite;
    //write dataToWrite to file
    File f = SPIFFS.open(flname.c_str(), "a");
    if (!f) 
    {
     Serial.println("file open failed");
    }
    else
   {
      //Write data to file
      Serial.println("Writing Data to File");
      f.print(dataToWrite.c_str());
      f.close();  //Close file
      Serial.println("file name : "+ flname);
   }

  }
  delay(40);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(2000);
  Serial.println(NowDate());
}