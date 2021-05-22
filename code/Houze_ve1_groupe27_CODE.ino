//LEDS
#define LED1 2
#define LED2 3
#define LED3 4


//Motor
#define MOTOR 5

//Bufer
#define BUFER 6
int melody[] = {31, 33, 35, 37, 39, 41, 44, 46};
int toneDuration = 300;  //duration de chaque tone

//Tonos originales descomentar si quiere probarlos
//#include "pitches.h"
//int melody[] = {NOTE_C1, NOTE_D2, NOTE_E3, NOTE_F4, NOTE_G5, NOTE_A6, NOTE_B7, NOTE_C7};

//Screen
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//Button
#define BT 13

//Potentiometer
#define FORCE A0


//Photo resistors
#define PH1 A1 //gauche
#define PH2 A2 //droit

//valeurs des photoresistors
int vph1 = 0; //gauche
int vph2 = 0; //droit

int ambientLight = 0;
int maxLight = 0;

//rang
int rang = 20;


//control du moteur
int f = 200;
int f_ant = 0;

//temps de rotation en secondes
int tmpR = 10;


//donnes du jeu
int jeuxGagne = 0;
int jeuxJoues = 0;



//contreul du temps
unsigned long tm = 0;
unsigned long tm1 = 0;



//vrai si le jeu est en cours d'exécution
boolean initialize = false;



void setup() {

  //Init Components
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  
  pinMode(MOTOR,OUTPUT);
  
  pinMode(BUFER,OUTPUT);

  pinMode(BT,INPUT);
  digitalWrite(BT, HIGH);

  lcd.begin(16, 2);
  Serial.begin(9600);

  
  //reset components
  analogWrite(MOTOR, 0);
  digitalWrite(BUFER, LOW);
  leds(0);
  tm1 = 0;

}

void loop() {
  
  //à attente du demarrage du jeu
  if(!initialize &&  digitalRead(BT) ){
    initialize = true;
    start();
    initialize = false;
  }
  
  
  //mise à jour de l'écran
  if(millis()  - tm1 > 200){
    tm1 = millis();
    f = map(analogRead(FORCE), 0, 1023, 180, 255);
    printData(false);
    
  }


}


/**
* Démarrage du jeu
*/
void start(){
 Serial.println("---------------- Start game-----------------");

 //réinitialiser les valuers des photorésistances
 rstLights(); 
 
 //nombre d'échantillons pour obtenir la lumière d'ambiante
 int ech = 3;
 
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("    Start in   ");
 
 leds(1);
 //obtenir la moyenne de la lumière ambiante
 for(int i = ech; i > 0; i--){
   lcd.setCursor(7, 1);
   lcd.print(i);
   ambientLight += calcAL(); 
   delay(1000);
 }
 ambientLight /= ech;
 
 lcd.clear();
 lcd.setCursor(0, 0);
 lcd.print("    Good look");
 
 tone(BUFER, melody[5], toneDuration);
  
 //Control
 Serial.print("Ambient light: ");
 Serial.println(ambientLight);
  

 rotate();
 
 leds(0);
 verify();
 
 jeuxJoues++;
 printData(true);
 
 //Control
 Serial.print("MAX Light: ");
 Serial.println(maxLight);
 Serial.println("-----------------------------\n");
 
 
 
}

/**
* Rotation du moteur.
*
*/
void rotate(){
  
  //pourcentage de rotation
  int prc = 0;
  int prcAnt = 0;
  
  //réduire la force linéairement
  int forceAct = 0;
  
  tm = millis();
  
  //réduction de la puissance du moteur et affichage du temps restant
  while(millis() - tm < (tmpR * 1000) ){
    
     prc = 10 - ((int)(millis() - tm)/1000);
     if( prc != prcAnt){
       lcdClearUpDown( 1, 7);
       lcd.print(prc);
       prcAnt = prc;
     }
     
     //réduction de la puissance
     forceAct = (int)(prc * f / 10);
     analogWrite(MOTOR,forceAct );
     
     //obtenir la valeur maximum de lumière
     maxLight = max(maxLight, calcML());
     
  }
  
  //arret du moteur
  analogWrite(MOTOR,0 );
  delay(1500);
  
 

}



/*
* Verifier le resultat du jeu
*/
void verify(){
  int vph1_temp = 0;
  int vph2_temp = 0;
  
  //nombre d'échantillons pour obtenir la les resultats
  int ech = 10;
  
  //obtenir la moyenne des photorésistances
  for(int i = 0; i<ech; i++){
    calcPH();
    vph1_temp += vph1;
    vph2_temp += vph2;
  }
  vph1_temp /= ech;
  vph2_temp /= ech;
  
  
  //Control
  controlLights(vph1_temp, vph2_temp, ambientLight, maxLight);
  
  
  
  if( (ambientLight + rang) >= maxLight ) 
    MauvaisReglageLumiereMax(); 
  else if ( (vph1_temp + rang) >= maxLight || (vph2_temp + rang) >= maxLight)
    gagner();
  else
    perdre();
  
  printData( true );
}

/**
* calculer les valeurs des photorésistances
**/
void calcPH(){
  vph1 = analogRead(PH1);
  vph2 = analogRead(PH2);
}


/**
* calculer la lumière ambiante
*/
int calcAL(){
  calcPH();
  int al= min(vph1, vph2);
  controlLights( vph1, vph2, al, maxLight );
  return al;
}

/**
* calculer la lumière maximale
*/
int calcML(){
  calcPH();
  return max(vph1, vph2);
}


/**
* réinitialiser les valeurs des photorésistances
*/
void rstLights(){
  ambientLight = 0;
  maxLight = 0;
}

/**
* imprimer les données du jeu sur lcd
*/
void printData( boolean ob){
  
  if( f != f_ant || ob){
    lcd.clear();
    lcd.setCursor(0, 0);
    
    //Force de rotation
    lcd.print("F: ");
    lcd.print(f);
    
    
    //jeux jouées
    lcd.print(" PJ: ");
    lcd.print(jeuxJoues);
    
    lcd.setCursor(0, 1);
  
    //jeux gagnés
    lcd.print("PG: ");
    lcd.print(jeuxGagne);
    
    f_ant = f;
  }
  
  
  
}

/**
* contrôle de la lumière obtenue par les photorésistances
*/
void controlLights( int phr1, int phr2, int al,int ml  ){
  
  //lumière obtenue par le photorésist gauche
  Serial.print("PHR_1: ");
  Serial.print(phr1);
  
  //lumière obtenue par le photorésist droite
  Serial.print("\tPHR_2: ");
  Serial.print(phr2);
  
  //lumière ambiante
  Serial.print("\tAmbient_Light: ");
  Serial.print(al);
  
  //lumière maximale
  Serial.print("\t Max_Light: ");
  Serial.println(ml);
}


/**
* contrôle des leds d'information
*/
void leds(int m){
  //jugar con los leds
  if(m == 0){
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
  else if(m == 1){
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
  }
  else{
  digitalWrite(LED1, random(0, 2));
  digitalWrite(LED2, random(0, 2));
  digitalWrite(LED3, random(0, 2));
  }
}


/**
* protocole exécuté quand une partie est gagnée
*/
void gagner(){

  jeuxGagne++;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     You Win!!!    ");
  
  Serial.println("     You Win!!!    ");
  
  for(int i = 0; i<2; i++){  
    
    for (int thisNote = 0; thisNote < 8; thisNote++) {

      tone(BUFER, melody[thisNote], toneDuration);
      leds(thisNote % 2);
      delay(toneDuration);
    }
   
  }
   leds(0);
}

/**
* protocole exécuté quand une partie est perdue
*/
void perdre(){
  leds(0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     You lost...    ");
  
  Serial.println("     You lost...    ");
  
    for (int thisNote = 7; thisNote >= 0; thisNote--) {
      tone(BUFER, melody[thisNote], toneDuration);
      delay(toneDuration);
    }

}


/**
* protocole exécuté lorsque la lumière maximale
* n'a pas pu être calculée correctement
*/

void MauvaisReglageLumiereMax(){
  leds(0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game failed");
  lcd.setCursor(0, 1);
  lcd.print("increase pw led");
  
  Serial.println("Game failed");
  Serial.println("increase pw led");
  
  for(int i =0; i<20; i++){
     if( i%2 == 0 )
       tone(BUFER, melody[0], toneDuration);
     delay(toneDuration);
  }
  
}


/**
* effacer une partie de l'écran LCD
*/
void lcdClearUpDown( boolean down, int pos){
  lcd.setCursor(0, down);
  lcd.print("                ");
  lcd.setCursor(pos, down);
}



