//#include <Esplora.h>

#include <EEPROM.h>
#include <SoftwareSerial.h>

// software serial #1: RX = digital pin 2, TX = digital pin 3
SoftwareSerial portOne(2, 3);
int cptReadSms = 50;

boolean sir = false;

String psw = "i2c";

int cptStart = 0;

int s1 = 8;
int s2 = 9;
int s3 = 10;
int s4 = 11;

int sirene    = 12;
int inverseur = 13;

int led_funct  = 5;
int led_alarme = 6;


int indice_appel = 1;
int cptAppel = 0;

int cptSmsStop = 0;
String numTb[3];

// *** initialisation *** //
void setup() {
  Serial.begin(9600);
  portOne.begin(9600);
  
   pinMode(led_funct,  OUTPUT);
   pinMode(led_alarme, OUTPUT);

  pinMode(s1, INPUT);
  pinMode(s2, INPUT);
  pinMode(s3, INPUT);
  pinMode(s4, INPUT);

  pinMode(sirene,    OUTPUT);
  pinMode(inverseur, OUTPUT);
    


String rn = readEepromStr(6,6);
if(!rn.equals("0") || !rn.equals("1") || !rn.equals("2") || !rn.equals("3") || !rn.equals("4") || !rn.equals("5") || !rn.equals("6") || !rn.equals("7") || !rn.equals("8") || !rn.equals("9")) StrToEeprom("1", 6);
    String val = readEepromStr(6,6);
    cptStart = val.toInt() * 60;

   numTb[0] = readEepromStr(10,19);
    numTb[1] = readEepromStr(20,29);
     numTb[2] = readEepromStr(30,39);
}
// ********************** //





//****************** Main Loop ***************************
void loop() {
//++++++++++Lecture port sim800L+++++++++++++++++++++++++++++++  
if(cptReadSms <= 0){
   cptReadSms = 50;
   portOne.listen();
   smsStartStop();
}
   cptReadSms--;
   
//++++++++++Lecture START/ARRET EEPROM+++++++++++++++++++++++++++++++  
   
if(readEepromStr(8,8).equals("1")){
  start();
        digitalWrite(led_funct, HIGH);
        if(cptReadSms > 0){
          delay(300);
          digitalWrite(led_funct, LOW);
          delay(300);
        }
  
}
else{
  Stop();
        digitalWrite(led_alarme, LOW);
        digitalWrite(led_funct, LOW);  
        delay(300);
}
//++++++++++Lecture PORT SERIE BT HC05+++++++++++++++++++++++++++++++  
if (Serial.available()) {
   String mot = Serial.readString();
    if(mot.substring(0,3).equals(psw)){
      String conf = mot;
      mot = mot.substring(3, mot.length());
     
      if(mot.equals("START")){
        // Start
        start();
      }else if(mot.equals("STOP")){
        // Stop
        Stop();
      }else{
        // Configuration
        msgConfig(conf);
      }
    }
 }
 
 
}
//********************************************************
// ----------------- Lecture depuis l'eeprom de la position x jusqu'a la position y-------------
String readEepromStr(int x, int y){
  String str = "";
  int i = 0;
  char c = "";
  for(i = x; i<=y; i++){
    c = char(EEPROM.read(i));
    str = String(str + c);
  }
  return str;
}
//------------------- ecriture du str dans la position x de l'eeprom----------------------------
void StrToEeprom(String str, int x){
  int str_len = str.length();
  int i = x;
  int j = 0;
  char c = "";
  int s = 0;
  for( i = x; i < x + str_len; i++){
    c = str.charAt(j);
    j++;
    s = c;
    EEPROM.write(i, s);
  }
}
//------------------- test d'activité de la position sensor-------------------------------------
boolean snActivePosition(int pos){
    String str = readEepromStr(pos, pos);   
    if(str.equals("1")){
      return true;
    }else{
      return false;
    }
}
//------------------- test d'alert sur l'une des positions--------------------------------------
boolean alarme(int s1, int s2, int s3, int s4){
  boolean ok = false;
  if(snActivePosition(0)){
    if(s1 == 1) ok = true;
  }
  if(snActivePosition(1)){
    if(s2 == 1) ok = true;
  }
  if(snActivePosition(2)){
    if(s3 == 1) ok = true;
  }
  if(snActivePosition(3)){
    if(s4 == 1) ok = true;
  }
  return ok;
}
//------------------- Procédure du message de configuration a envoyer par blueTooth-------------
void msgConfig(String msg){

if(!readEepromStr(7,7).equals("1")){
String str = msg.substring(0, 3);  
if(str.equals(psw)){

        int boucle = 0;
        for(boucle = 0; boucle < 10; boucle++){
                                                digitalWrite(led_funct, HIGH); 
                                                digitalWrite(led_alarme, HIGH);
                                                delay(80);
                                                digitalWrite(led_alarme, LOW);
                                                delay(80);
                                              }
  str = msg.substring(3,10);
        StrToEeprom(str, 0);
        str = msg.substring(10, msg.length());
        StrToEeprom(str, 10);
           numTb[0] = readEepromStr(10,19);
           numTb[1] = readEepromStr(20,29);
           numTb[2] = readEepromStr(30,39);
      }else {
                 erreur();
            }
} 

 }
//------------------- SMS Start/Stop System ---------------------------------------------------------

void smsStartStop(){
                  portOne.print("AT+CMGF=1");   // Configure le mode SMS
                  portOne.write(13);
                  delay(300);
                  portOne.print("AT+CMGL=\"ALL\"");  // Affiche tous les messages
                  portOne.write(13);
                  String mot = portOne.readString();
                  Serial.print(mot);
                  delay(300);
                  int xx = sms(mot);
                  if(xx == 1){
                           // Start
                     start();
                     portOne.print("AT+CMGD=1,4");
                     portOne.write(13);            
                  }else
                  if(xx == 2){
                      // Stop
                      Stop();
                      portOne.print("AT+CMGD=1,4");
                      portOne.write(13);
                  }else{
                      // Effacé tout les sms
                      portOne.print("AT+CMGD=1,4");
                      portOne.write(13);
                  }
                   delay(100);
}
 
//------------------- Procédure d'appel --------------------------------------------------------
void appel(int i) {
// digitalWrite(inverseur, HIGH);
            
// ----- Lancement appel ----- //  

  portOne.print("ATH");
  portOne.write(13);
  delay(200);
  char val = 0;
  int x = 0;
  portOne.print("ATD");
  if (i == 1) {
    for (x = 10; x < 20; x++) {
      val = EEPROM.read(x);
      if(x==9) val = '0';
      portOne.print(val);
    }
  } else if (i == 2) {
    for (x = 20; x < 30; x++) {
      val = EEPROM.read(x);
      portOne.print(val);
    }
  } else if (i == 3) {
    for (x = 30; x < 40; x++) {
      val = EEPROM.read(x);
      portOne.print(val);
    }
  }
  portOne.println(';');
  portOne.write(13);
  delay(500);
}


//------------------- Procédure d'appel --------------------------------------------------------
void appelS4s() {
  portOne.print("ATH");
  portOne.write(13);
  delay(200);
// ----- Lancement appel ----- //  
  char val = 0;
  int x = 0;
  portOne.print("ATD");
    for (x = 10; x < 20; x++) {
      val = EEPROM.read(x);
      if(x==9) val = '0';
      portOne.print(val);
    }
  portOne.println(';');
  portOne.write(13);
  delay(200);
}


//------------------- Procédure d'activité -----------------------------------------------------
void start(){
   StrToEeprom("1", 8);
if(!sir){
  digitalWrite(sirene, HIGH);
  delay(100);
  digitalWrite(sirene, LOW);
  sir = true;
}

 
   if(cptStart > 0 && readEepromStr(5,5).equals("0")) {
      cptStart--;
    }else{
      StrToEeprom("1",5);
      if(indice_appel > 3) indice_appel = 1;
    
      if(readEepromStr(7,7).equals("1")){
       
        if(snActivePosition(4)) digitalWrite(sirene, HIGH);
                               if(cptAppel > 50){
                                    cptAppel = 0;
                                    appel(indice_appel);
                                    digitalWrite(led_alarme, HIGH);
                                    indice_appel++;
                               }else{
                                    cptAppel++;
                               }
                                 
               
      }else if(alarme(digitalRead(s1), digitalRead(s2), digitalRead(s3), digitalRead(s4))){
           
             if(snActivePosition(4)){
                digitalWrite(sirene, HIGH);
                appelS4s(); 
             }
             delay(3800);
             if(alarme(digitalRead(s1), digitalRead(s2), digitalRead(s3), digitalRead(s4))){
                                           //  appel(1);
                                             digitalWrite(led_alarme, HIGH);
                                             digitalWrite(led_funct, LOW); 
                                             StrToEeprom("1", 7);
                                             indice_appel++;
             }else{
                                             digitalWrite(sirene, LOW);
             }
      }
    }
}
//------------------- Procédure d'arret---------------------------------------------------------
void Stop(){
   digitalWrite(sirene, LOW);
   sir = false;
  String val = readEepromStr(6,6);
  cptStart = val.toInt() * 60;
  StrToEeprom("0", 5);
  StrToEeprom("0", 7);
  StrToEeprom("0", 8); 
}
//------------------- Lecture du message du sms et revoi du code d'emploi-----------------------
int sms(String str){
  int i = 0;
    String num = numTb[0];
    num = num.substring(1, num.length());
      String num1 = numTb[1];
      num1 = num1.substring(1, num1.length());
        String num2 = numTb[2];
        num2 = num2.substring(1, num2.length());
    if((contains(str, num)||contains(str, num1)||contains(str, num2)) && contains(str,"START")){
      i = 1;
    }else if((contains(str, num)||contains(str, num1)||contains(str, num2)) && contains(str,"STOP")){
      i = 2;
    }else{
      i = 0;
    }
    
  return i;
}
//------------------- test Si toFind existe dans str--------------------------------------------
boolean contains(String str, String toFind){
  boolean ok = false;
  int i = 0;
  String c0 = "";
  String c1 = toFind.substring(0,1);
  int toul = toFind.length();
          for(i = 0; i < str.length(); i++){
                 c0 = str.substring(i,i+1);
                 if(c0.equals(c1)){
                      if(str.substring(i,i+toul).equals(toFind)){
                          ok = true;
                          break;
                      }
                 }
          }
  return ok;
}
//------------------- Procédure blink Erreur----------------------------------------------------
void erreur(){
     digitalWrite(led_funct, LOW);
      int pass = 0;

      for (pass = 0; pass < 3; pass++) {
        digitalWrite(led_alarme, HIGH);
        delay(300);
        digitalWrite(led_alarme, LOW);
        delay(300);
      }
}

