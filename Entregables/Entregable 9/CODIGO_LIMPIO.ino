#include <Wire.h>
#include <MPU6050_tockn.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define TCAADDR 0x70

MPU6050 mpu1(Wire);
MPU6050 mpu2(Wire);
MPU6050 mpu3(Wire);

//////////////////////////////////////
// TB6612 Vibradores
//////////////////////////////////////

#define PWMA 18
#define AIN1 23
#define AIN2 5

#define STBY 16

#define BIN1 4
#define BIN2 17
#define PWMB 19

//////////////////////////////////////
// Actuadores
//////////////////////////////////////

#define ACTUADOR1 32
#define ACTUADOR2 33

//////////////////////////////////////
// Variables
//////////////////////////////////////

float refY1=0;
float refY2=0;
float refY3=0;

bool modoAutomatico=true;

bool posturaMala=false;

bool alertaEnviada=false;

bool actuadoresAutomaticos=false;

bool vibrando=false;

unsigned long tiempoMalaPostura=0;

unsigned long tiempoVibracion=0;

const unsigned long DURACION_VIB=15000;

const unsigned long PAUSA_VIB=5000;

const unsigned long TIEMPO_ACTUADORES=45000;

//////////////////////////////////////

void tcaselect(uint8_t i){

Wire.beginTransmission(TCAADDR);

Wire.write(1<<i);

Wire.endTransmission();

}

//////////////////////////////////////

void vibradoresON(){

digitalWrite(AIN1,HIGH);
digitalWrite(AIN2,LOW);

digitalWrite(BIN1,HIGH);
digitalWrite(BIN2,LOW);

}

void vibradoresOFF(){

digitalWrite(AIN1,LOW);
digitalWrite(AIN2,LOW);

digitalWrite(BIN1,LOW);
digitalWrite(BIN2,LOW);

}

//////////////////////////////////////

void actuadoresON(){

digitalWrite(ACTUADOR1,HIGH);

digitalWrite(ACTUADOR2,HIGH);

}

void actuadoresOFF(){

digitalWrite(ACTUADOR1,LOW);

digitalWrite(ACTUADOR2,LOW);

}

//////////////////////////////////////

void calibrar(){

tcaselect(0);

mpu1.update();

refY1=mpu1.getAngleY();

tcaselect(1);

mpu2.update();

refY2=mpu2.getAngleY();

tcaselect(2);

mpu3.update();

refY3=mpu3.getAngleY();

SerialBT.println("CALIBRADO");

}

//////////////////////////////////////

void setup(){

Serial.begin(115200);

SerialBT.begin("POSTURAL_SENSE");

Wire.begin();

pinMode(PWMA,OUTPUT);

pinMode(AIN1,OUTPUT);

pinMode(AIN2,OUTPUT);

pinMode(PWMB,OUTPUT);

pinMode(BIN1,OUTPUT);

pinMode(BIN2,OUTPUT);

pinMode(STBY,OUTPUT);

digitalWrite(STBY,HIGH);

digitalWrite(PWMA,HIGH);

digitalWrite(PWMB,HIGH);

pinMode(ACTUADOR1,OUTPUT);

pinMode(ACTUADOR2,OUTPUT);

vibradoresOFF();

actuadoresOFF();

Serial.println("Sistema iniciado");

}

//////////////////////////////////////

void loop(){

//////////////////////////////////////
// LEER IMUs
//////////////////////////////////////

tcaselect(0);

mpu1.update();

float y1=mpu1.getAngleY();

tcaselect(1);

mpu2.update();

float y2=mpu2.getAngleY();

tcaselect(2);

mpu3.update();

float y3=mpu3.getAngleY();

float e1=abs(y1-refY1);

float e2=abs(y2-refY2);

float e3=abs(y3-refY3);

posturaMala=

(e1>10 ||

 e2>10 ||

 e3>10);

//////////////////////////////////////
// AUTOMATICO
//////////////////////////////////////

if(modoAutomatico){

if(posturaMala){

if(!alertaEnviada){

SerialBT.println("POSTURA_MALA");

alertaEnviada=true;

tiempoMalaPostura=millis();

tiempoVibracion=millis();

}

/////////////////////////////////
// Vibración automática
/////////////////////////////////

if(vibrando){

if(millis()-tiempoVibracion>=DURACION_VIB){

vibradoresOFF();

vibrando=false;

tiempoVibracion=millis();

}

}

else{

if(millis()-tiempoVibracion>=PAUSA_VIB){

vibradoresON();

vibrando=true;

tiempoVibracion=millis();

}

}

/////////////////////////////////
// Actuadores automáticos
/////////////////////////////////

if(

!actuadoresAutomaticos &&

millis()-tiempoMalaPostura>=TIEMPO_ACTUADORES

){

actuadoresON();

actuadoresAutomaticos=true;

SerialBT.println("ACTUADORES_ON");

}

}

else{

if(alertaEnviada){

SerialBT.println("POSTURA_OK");

}

alertaEnviada=false;

actuadoresAutomaticos=false;

vibrando=false;

vibradoresOFF();

actuadoresOFF();

}

}

//////////////////////////////////////
// COMANDOS APP
//////////////////////////////////////

if(SerialBT.available()){

String cmd=

SerialBT.readStringUntil('\n');

cmd.trim();

if(cmd=="CALIBRAR"){

calibrar();

}

else if(cmd=="AUTO_MODE"){

modoAutomatico=true;

}

else if(cmd=="MANUAL_MODE"){

modoAutomatico=false;

}

else if(cmd=="VIB_MANUAL_ON"){

vibradoresON();

}

else if(cmd=="VIB_MANUAL_OFF"){

vibradoresOFF();

}

else if(cmd=="ACT_MANUAL_ON"){

actuadoresON();

}

else if(cmd=="ACT_MANUAL_OFF"){

actuadoresOFF();

}

}

delay(50);

}