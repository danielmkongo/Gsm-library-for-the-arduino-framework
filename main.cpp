#include<Arduino.h>
#include<GSM_.h>

//declare gsm object.
GSM_ gsm(2,3);

void setup(){
  //set baudrate to 9600.
  gsm.Begin(9600);

  //delete all messages on the sim card to save space for new incomming messages.
  gsm.deleteMessages();

//set phone number to send messages to.
  gsm.setPhoneNumber("+255........");

//set the gsm in recieve mode so that you can recieve texts.
  gsm.RecieveMessage();

  //gsm.Write() this enables you to write AT commands directly. 

}


void loop(){

  //listens for any incomming messages or calls.
  if(gsm.Synchronize(200)){
    //sets phone number to sender's number;
    gsm.setPhoneNumber(gsm.getSenderNumber("+255").c_str());

    //sends a message to the phone number set by gsm.setPhoneNumber();
    gsm.SendMessage(gsm.getMessage().c_str());
  }

//other processes.
  delay(100);
}
