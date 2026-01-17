#pragma once
#include<Arduino.h>
#include<thread>
#include<SPIFFS.h>
#include<FS.h>
#define gsm Serial2

String _buffer;
String Buffer;

static int tries_=0;
static int contentL=0;
enum POWER_STATE:uint8_t{
    ACTIVE, SLEEP
};

static RTC_DATA_ATTR POWER_STATE power_state;
 
void parseATText(byte b);

enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};

enum _actionState {
  AS_IDLE,
  AS_WAITING_FOR_RESPONSE
};

char buffer[100];

byte actionState = AS_IDLE;
unsigned long lastActionTime = 0;

byte parseState = PS_DETECT_MSG_TYPE;
byte pos = 0;

int contentLength = 0;

constexpr uint8_t delayTime=1;
class GSM_{  

    protected:

        static void resetBuffer() {
            memset(buffer, 0, sizeof(buffer));
            pos = 0;
        }

        static  void sendCommand(const String& command,const bool& i=1) {
            gsm.println(command);
            if(i==1){
            Synchronize();
            }else{
                String check;
                long startTime=millis();
            
                while(!gsm.available()){
                    if(millis()-startTime>5000){
                        break;
                    }
                }
                Serial.print(F("{\n"));

                while(gsm.available()>0){ 
                    char character=gsm.read();
                   // check+=character;    
                    Serial.print(character);
                    //delay(10);

                    // if(check.indexOf("OK")>=0){
                    //     break;
                    // }

                }
                Serial.println(F("}"));
            }
        } 
        static bool trigger;
        static String message;
    public:
        
        static void Write(const char* command){
            gsm.println(command);
        }

       static void Synchronize(){
            delay(500);
            long start=millis();
            while(!gsm.available()){
                if(millis()-start>3000){
                    break;
                }
            }

            while(gsm.available()>0){     
                //message=gsm.readString();    
                //Serial.println(message); //Kwa ajili ya debugging.
                Serial.print((char)gsm.read());
                delay(10);
            }
        }
        
        static bool Synchronize(unsigned delay_){
            delay(delay_);
            if(gsm.available()>0){   
                message=gsm.readString();  
                return true;
            }
            return false;
        }


        String getMessage(){  
            return this->message;
        }

        static void Synchronize_(){
            RecieveMessage();
        }

        static void Begin(unsigned freq){
            gsm.begin(freq);
        }

        static void call(const String& phone){
            gsm.println("ATD"+phone+";");      
         }


         static void hangUp(){
            gsm.println("ATH");
         }

        static void isConnected(){
            sendCommand(F("AT+CREG?"));
        }

        static void configGprs() {
            Serial.println(F(" --- Configuring GPRS --- "));
            sendCommand(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\""));
            delay(2000);
            sendCommand(F("AT+SAPBR=3,1,\"APN\",\"internet\""));
            delay(2000);
        }


        static void SendMessage(const String& msg,const String& phone){
            ////Serialprintln(F("Sending message!!"));
            delay(500);
            gsm.println(F("AT+CMGF=1"));
            Synchronize();
            gsm.println(("AT+CMGS=\""+phone+"\""));
            Synchronize();  
            gsm.print(msg);
            Synchronize();
            gsm.write(26);
        }
    

        static void RecieveMessage(){
            gsm.println(F("AT+CMGF=1"));  
            Synchronize();
            gsm.println(F("AT+CPMS=\"SM\",\"SM\",\"SM\""));
            Synchronize();
            gsm.println(F("AT+CNMI=2,1,0,0,0"));
            Synchronize();
      
        }

        static void storeMessage(){
            gsm.println("AT+CMGW");
            Synchronize();
        }

        static String _readSerial(uint32_t timeout)
        {

            uint64_t timeOld = millis();

            while (!gsm.available() && !(millis() > timeOld + timeout))
            {
                delay(13);
            }

            String str;

            while(gsm.available())
            {
                if (gsm.available()>0)
                {
                    str += (char) gsm.read();
                  //Serial.print(str[str.length()-1]);
                }
                delay(2);
            }

            return str;

        }     

        static  bool checkUnreadMessages() {
            gsm.println("AT+CMGL=\"REC UNREAD\"");
            long start=millis();
            while(gsm.available()<0){
                if(millis()-start>10000){
                 //   Serial.println(F("Unread message check timeout."));
                    break;
                }
            }


            // Read the response from the SIM800L module
            //Synchronize();
            String response = gsm.readString();
            // Check if the response contains "+CMGL: "
            if (response.indexOf("+CMGL: ") != -1) {
                return true; // Unread messages found
            } else {
                return false; // No unread messages
            }
        }

        static void getUnreadMessages(){
            gsm.println(F("at+cmgl=\"ALL\""));
            Synchronize();
        }
        
        static void setPowerState(POWER_STATE state){
            power_state=state;
        }

        POWER_STATE getPowerState(){
            return power_state;
        }

        static bool deleteMessages(){
            static int counter=0;
            
            while(true){
                gsm.print(F("AT+CMGD=1,4\r"));
                long start=millis();
                while(!gsm.available()){
                if(millis()-start>3000){
                        break;
                    }  
                }
                
                String msg=gsm.readString();
                Serial.println(msg);
                if(msg.indexOf("OK")>=0){
                    return true;
                }else{
                Serial.println("Restarting...");
                    //ESP.restart();
                }

                if(counter>3){
                    break;
                }
                counter++;

            }
            return false;
          //  return (strstr_P(_buffer,"OK")!=NULL);
        }

        static void sleep(){
            gsm.println(F("AT+CSCLK=2"));
            setPowerState(SLEEP);
            delay(100);
        }

        static void HTTP_OFF(){
            sendCommand(F("AT+HTTPTERM"));
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            sendCommand(F("AT+SAPBR=0,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        static void wakeUp(){
            gsm.println(F("AT"));
            delay(200);
            gsm.println(F("AT+CSCLK=0"));
            delay(200);
            setPowerState(ACTIVE);

        }

        static void checkSignalStrenth(){
            gsm.println(F("AT+CSQ"));
            delay(100);
            Synchronize();
        }

        String getMessage_(){
            return _buffer;
        }

        void setBaudRate(uint32_t baud){
            sendCommand("AT+IPR="+String(baud));
        }

        void saveBaudRate(){
            sendCommand("AT&W");
        }

         static void HttpPost(const String& postdata,const String& url) {
            Serial.println(F(" --- Start GPRS & HTTP --- "));
            configGprs();
            sendCommand(F("AT+SAPBR=1,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            sendCommand(F("AT+SAPBR=2,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            sendCommand(F("AT+HTTPINIT"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            sendCommand(F("AT+HTTPPARA=CID,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            sendCommand("AT+HTTPPARA=\"URL\","+url+"");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            // sendCommand(F("AT+HTTPSSL=1"));  // for SSL support.
            // delay(1000);
            // sendCommand(F("AT+SSLOPT=0,1"));
            // delay(1000);

            sendCommand(F("AT+HTTPPARA=\"CONTENT\",\"application/json\""));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            sendCommand("AT+HTTPDATA="+String(postdata.length()+1)+",5000");
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            sendCommand(postdata);
             std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            sendCommand(F("AT+HTTPACTION=1"));
             std::this_thread::sleep_for(std::chrono::milliseconds(4000));
            sendCommand(F("AT+HTTPREAD"));
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            sendCommand(F("AT+HTTPTERM"));
             std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            sendCommand(F("AT+SAPBR=0,1"));
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        static uint16_t readData(){
            uint16_t i=0;
            String buffer;
    
            while(gsm.available()>0){
                buffer+=(char)gsm.read();
                Serial.print(buffer[buffer.length()-1]);
                i++;
            }

             if(buffer.indexOf("+HTTPREAD: ")>=0){
                Serial.print("^");
                buffer=buffer.substring(buffer.indexOf('\n')+14,buffer.length());
            }
            //Buffer+=buffer;
            return buffer.length();
        }
        static void parseATText(const byte& b, File& file) {

          

            if ( pos >= (sizeof(buffer)) ){
                resetBuffer(); // just to be safe
                parseState=PS_HTTPREAD_CONTENT;
            }
            
            buffer[pos++] = b;
            
            switch (parseState) {
            case PS_DETECT_MSG_TYPE: 
                {
                if ( b == '\n' )
                    resetBuffer();
                else {        
                    if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
                    parseState = PS_IGNORING_COMMAND_ECHO;
                    }
                    else if ( b == ':' ) {
                    //Serial.print("Checking message type: ");
                    //Serial.println(buffer);

                    if ( strcmp(buffer, "+HTTPACTION:") == 0 ) {
                        Serial.println("Received HTTPACTION");
                        parseState = PS_HTTPACTION_TYPE;
                    }
                    else if ( strcmp(buffer, "+HTTPREAD:") == 0 ) {
                        Serial.println("Received HTTPREAD");            
                        parseState = PS_HTTPREAD_LENGTH;
                    }
                    resetBuffer();
                    }
                }
                }
                break;

            case PS_IGNORING_COMMAND_ECHO:
                {
                if ( b == '\n' ) {
                    Serial.print("Ignoring echo: ");
                    Serial.println(buffer);
                    parseState = PS_DETECT_MSG_TYPE;
                    resetBuffer();
                }
                }
                break;

            case PS_HTTPACTION_TYPE:
                {
                if ( b == ',' ) {
                    Serial.print("HTTPACTION type is ");
                    Serial.println(buffer);
                    parseState = PS_HTTPACTION_RESULT;
                    resetBuffer();
                }
                }
                break;

            case PS_HTTPACTION_RESULT:
                {
                if ( b == ',' ) {
                    Serial.print("HTTPACTION result is ");
                    Serial.println(buffer);
                    parseState = PS_HTTPACTION_LENGTH;
                    resetBuffer();
                }
                }
                break;

            case PS_HTTPACTION_LENGTH:
                {
                if ( b == '\n' ) {
                    Serial.print("HTTPACTION length is ");
                    Serial.println(buffer);
                    
                    // now request content
                    gsm.print("AT+HTTPREAD=0,");
                    gsm.println(buffer);
                    
                    parseState = PS_DETECT_MSG_TYPE;
                    resetBuffer();
                }
                }
                break;

            case PS_HTTPREAD_LENGTH:
                {
                if ( b == '\n' ) {
                    contentLength = atoi(buffer);
                    Serial.print("HTTPREAD length is ");
                    Serial.println(contentLength);
                    
                    Serial.print("HTTPREAD content: ");
                    
                    parseState = PS_HTTPREAD_CONTENT;
                    resetBuffer();
                }
                }
                break;

            case PS_HTTPREAD_CONTENT:
                {
                // for this demo I'm just showing the content bytes in the serial monitor
                Serial.write(b);
                file.write(b); 
                contentLength--;
                contentL++;
                
                if ( contentLength <= 0 ) {

                    // all content bytes have now been read

                    parseState = PS_DETECT_MSG_TYPE;
                    resetBuffer();
                    
                    Serial.print("\n\n\n");
                    
                    actionState = AS_IDLE;
                }
                }
                break;
            }
        }
        
        static bool download(const int& size){
             File firmwareFile = SPIFFS.open("/firmware.bin", "w");

            if (!firmwareFile) {
                Serial.println("Error opening firmware file for writing");
                return false;
            }

            int bytesRead = 0;
            const int chunkSize = 5120; // 5KB chunk size

            while (bytesRead < size) {
                sendCommand("ATE0");
                gsm.print("AT+HTTPREAD=");
                gsm.print(bytesRead);
                gsm.print(",");
                gsm.println(chunkSize);

                delay(2000);

                while (gsm.available()) {
                char c = gsm.read();
                firmwareFile.write(c);
                bytesRead++;
                }
                delay(500);
            }

            firmwareFile.close();

            Serial.println("Firmware saved to SPIFFS successfully");
            sendCommand("ATE1");
            return bytesRead==size;
        }
        static bool downloadFirmware(const String& postdata,const String& url) {
            tries_=0;
            int size=0;
            http:
            Serial.println(F(" --- Start GPRS & HTTP --- "));
            configGprs();
            sendCommand(F("AT+SAPBR=1,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            sendCommand(F("AT+SAPBR=2,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            
            
            sendCommand(F("AT+HTTPINIT"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
            sendCommand(F("AT+HTTPPARA=CID,1"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            sendCommand("AT+HTTPPARA=\"URL\","+url+"");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            // sendCommand(F("AT+HTTPSSL=1"));  // for SSL support.
            // delay(1000);
            // sendCommand(F("AT+SSLOPT=0,1"));
            // delay(1000);

            sendCommand(F("AT+HTTPPARA=\"CONTENT\",\"application/json\""));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            sendCommand("AT+HTTPDATA="+String(postdata.length()+1)+",15000");
             std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            sendCommand(postdata);
             std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            sendCommand(F("AT+HTTPACTION=1"));
            int start=millis();

            while(millis()-start<200000){
                if(gsm.available()>0){
                    
                    String buf=gsm.readString();
                    if(buf.indexOf("200")>=0){

                        Serial.print("Should work!!");
                        
                        size=buf.substring(buf.indexOf("200")+4,buf.length()).toInt();
                        Serial.print("SIZE:");
                        Serial.println(size);
                        break;
                    }else{
                        tries_++;
                        Serial.print("UUGHHH!!");Serial.print(" Try:"); Serial.println(tries_);
                        if(tries_<=4){
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            goto http;
                        }else{
                            sendCommand(F("AT+HTTPTERM"));
                            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                            sendCommand(F("AT+SAPBR=0,1"));
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            return false;
                        }
                        
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(15000));
            bool status=download(size);
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            sendCommand(F("AT+HTTPTERM"));
             std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            sendCommand(F("AT+SAPBR=0,1"));
             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return status;
        }
    
    
        static const uint8_t checkForSMS()
        {
        _buffer = _readSerial(100);
        if(_buffer.length() == 0)
        {
         return 0;
        }
        _buffer += _readSerial(1000);
        
        // ////Serialprintln(_buffer);
        // +CMTI: "SM",1
        if(_buffer.indexOf("CMTI") == -1)
        {
        return 0;
        }
        return _buffer.substring(_buffer.indexOf(',')+1).toInt();
}

        
static String  readSms(uint8_t index)
{
    // Can take up to 5 seconds

    if(( _readSerial(5000).indexOf("ER")) != -1)
    {
    	return "";
    }

    gsm.print(F("AT+CMGR="));
    gsm.print (index);
    gsm.print ("\r");
    _buffer=_readSerial(5000);
   // phone=
    if (_buffer.indexOf("CMGR") == -1)
    {
    	return "";
    }
    
	_buffer = _readSerial(10000);
	byte first = _buffer.indexOf('\n', 2) + 1;
	byte second = _buffer.indexOf('\n', first);
    return _buffer.substring(first, second);
}



    static String getNumberSms(uint8_t index)
    {
        _buffer=readSms(index);
        //////Serialprintln(_buffer.length());
        if (_buffer.length() > 10) //avoid empty sms
        {
            uint8_t _idx1=_buffer.indexOf("+CMGR:");
            _idx1=_buffer.indexOf("\",\"",_idx1+1);
            return _buffer.substring(_idx1+3,_buffer.indexOf("\",\"",_idx1+4));
        }
        else
        {
            return "";
        }
}
    GSM_()=default;        
    
    ~GSM_(){
    
    }
};

String GSM_::message;
bool GSM_::trigger;

