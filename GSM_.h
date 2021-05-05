#include<SoftwareSerial.h>

class GSM_{
    private:
    String PhoneNumber;
      
    protected:
      String message;
      SoftwareSerial* gsm;
    public:
    
    GSM_(const uint8_t& rx,const uint8_t& tx){
      this->gsm=new SoftwareSerial(rx,tx);
    }

    void Write(const char* command){
      this->gsm->println(command);
    }

    String getSenderNumber(const char* countryCode){
      delay(500);
      unsigned index=this->message.indexOf(countryCode);
      if(index>10){
        return this->message.substring(index,index+13);
      }
      return "";
      
    }

  void initiateCommunication(const uint8_t& rx,const uint8_t& tx){
          this->gsm=new SoftwareSerial(rx,tx);
  }
    
    GSM_()=default;

    void Synchronize(){
        delay(200);
          while(gsm->available()>0){         
              this->message=gsm->readString();
              Serial.println(this->message);
        }
    }
    
    
    bool Synchronize(unsigned delay_){
        delay(delay_);
          while(gsm->available()>0){         
              this->message=gsm->readString();
              Serial.println(this->message);
              return true;
        }
        return false;
    }

    void Begin(unsigned freq){
      this->gsm->begin(freq);
    }
    
  void SendMessage(const char * msg){
          delay(500);
          this->gsm->println("AT+CMGF=1");
          this-> Synchronize();
          this->gsm->println(("AT+CMGS=\""+String(this->PhoneNumber)+"\"").c_str());
          this->Synchronize();  
          this->gsm->print(msg);
          this->Synchronize();
          this->gsm->write(26);
    }
    void RecieveMessage(){
          this->gsm->println("AT+CMGF=1");  
          this->Synchronize();
          this->gsm->println("AT+CNMI=2,2,0,0,0");
          delay(1000);
          this->Synchronize();
    }
    void deleteMessages(){
      delay(500);
      gsm->println("AT+CMGD=4");
      Synchronize();
    }

    String getMessage(){
      return this->message;
    }

    void checkSignalStrenth(){
        this->gsm->println("AT+CSQ");
        delay(100);
        this->Synchronize();
    }

    bool setPhoneNumber(const char* number){
        if(strlen(number)<12){
          Serial.print("invaid number! :");
          Serial.println(number);
          return false;
        }
        this->PhoneNumber=number;
        return true;
    }
    
  virtual ~GSM_(){
    this->gsm->~SoftwareSerial();
    }
};

