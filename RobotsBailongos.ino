/*
  RobotGeek 'Chip-E' Biped -> https://www.robotgeek.com/robotgeek-chip-e.aspx
  Remix utilizando servos MG996R, driver PCA9685, y NRF2401L. Javier Vargas. El Hormiguero.
  https://creativecommons.org/licenses/by/4.0/
*/

//COLOR DEL ROBOT -> Cada robot tiene sus propios ajustes

#define Rojo
//#define Verde
//#define Amarillo
//#define Azul

//---PINES---
//Servos (Conexion a PCA9685)
const int PIN_YL = 2; //Cintura izquierda
const int PIN_YR = 3; //Cintura derecha
const int PIN_RL = 0; //Pie izquierdo
const int PIN_RR = 1; //Pie derecho
const int BUZZER_PIN = 7; //Zumbador
const int pinCE = 9; //Modulo NRF24
const int pinCSN = 10; //Modulo NRF24

//--Chip-E
/*
            ______________
           /              |
          |   _________   |
          |  [_________]  |
          |               |
          |               |
          |               |
          |       O       |
   YR ==> |               | <== YL
           ---------------
              ||     ||
              ||     ||
              ||     ||
   RR ==>   -----   ------  <== RL
            -----   ------
*/

#ifdef Rojo
const int TRIM_RL = -11; //Offset en tobillo izquierdo
const int TRIM_RR = 3; //Offset en tobillo derecho
const int TRIM_YL = -9; //Offset en cintura izquierda
const int TRIM_YR = -7; //Offset en cintura izquierda
#endif

#ifdef Verde
const int TRIM_RL = -10;
const int TRIM_RR = 6;
const int TRIM_YL = -5;
const int TRIM_YR = -6;
#endif

#ifdef Amarillo
const int TRIM_RL = 7;
const int TRIM_RR = -21;
const int TRIM_YL = -5;
const int TRIM_YR = -5;
#endif

#ifdef Azul
const int TRIM_RL = 2;
const int TRIM_RR = -22;
const int TRIM_YL = -5;
const int TRIM_YR = -16;
#endif

//Robot
#include <ChipE.h>
ChipE myChipE;

//Receptor 2.4GHZ NRF24L01
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
byte direccion[5] = {'a', 'b', 'c', 'd', 'e'};
RF24 radio(pinCE, pinCSN);

//Sonidos del zumbador
#include <PiezoEffects.h>
PiezoEffects mySounds( BUZZER_PIN );

//Pantalla lcd
#include <ChipE_LCD.h>

int estado = 0;
unsigned long m0 = 0;
unsigned long m1 = 0;

void setup() {

  //Semilla aleatoria
  randomSeed(analogRead(A0));

  //Tiempo aleatorio
  m0 = TiempoAleatorio(5000, 40000);
  m1 = TiempoAleatorio(2000, 10000);

  //Inicialización del buzzer
  mySounds.play(soundConnection);

  //Inicializacion del robot
  myChipE.init( PIN_YL, PIN_YR, PIN_RL, PIN_RR );
  myChipE.setTrims( TRIM_YL, TRIM_YR, TRIM_RL, TRIM_RR ); //Set the servo trims
  myChipE.home(); //Move feet to home position

  //Inicializacion de la pantalla LCD
  myLCD.begin();
  myLCD.backlight();
  myLCD.createChar(0, char_heart);
  myLCD.createChar(1, char_block);
  myLCD.createChar(2, char_line);
  myLCD.setCursor(0, 0);
#ifdef Rojo
  myLCD.print("------ROJO------");
#endif
#ifdef Verde
  myLCD.print("-----VERDE------");
#endif
#ifdef Amarillo
  myLCD.print("----AMARILLO----");
#endif
#ifdef Azul
  myLCD.print("------AZUL------");
#endif
  delay(2000);
  drawEyes(EYES_CENTER);

  //Inicialicacion RF
  radio.begin();
  radio.openReadingPipe(1, direccion);
  radio.stopListening();
}

/////////////////////////////////
///////////////LOOP//////////////
/////////////////////////////////

void loop() {

  //Recibimos ordenes de radio con el dispositivo NRF24L01
  switch (ControlRemoto()) {

    //Girar izquierda
    case 0:
      myChipE.turn(1, 2000, LEFT);
      estado = 0;
      break;

    //Girar derecha
    case 1:
      myChipE.turn(1, 2000, RIGHT);
      estado = 1;
      break;

    //Avanzar
    case 2:
      drawEyes(EYES_DOWN);
      myChipE.walk(1, 2000, FORWARD);
      estado = 2;
      break;

    //Retroceder
    case 3:
      drawEyes(EYES_UP);
      myChipE.walk(1, 2000, BACKWARD );
      estado = 3;
      break;

    //Boton baile A
    case 4:
      drawEyes(EYES_UP);
      myChipE.flapping(1, 600, 10, FORWARD );
      estado = 4;
      break;

    //Boton baile B
    case 5:
      drawEyes(EYES_UP);
      myChipE.updown(1, 500, BIG);
      estado = 5;
      break;

    //Boton baile C
    case 6:
      drawEyes(EYES_UP);
      myChipE.crusaito(1, 1000, 20, LEFT );
      estado = 6;
      break;

    //Boton D, preparar baile
    case 7:
      if (estado != 7) {
        myChipE.home();
        myLCD.clear();
        myLCD.print("   A BAILAR!");
        mySounds.play(soundUp);
        estado = 7;
      }
      break;

    //Botono D mantenido, baile sincronizado
    case 8:
      BaileSincronizado();
      estado = 8;
      break;

    //Reposo
    default:
      if (estado != -1) {
        drawEyes(EYES_CENTER);
        myChipE.home();
        m0 = TiempoAleatorio(10000, 40000);
        m1 = TiempoAleatorio(2000, 10000);
        estado = -1;
      }
      //Sonido y parpadeo aleatorio si no estamos esperando a sincronizar el baile
      if (estado != 7) {
        SonidoAleatorio();
        ParpadeoAleatorio();
      }
      break;
  }
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////

int ControlRemoto() {
  float datos[7];
  static int e = -1;

  radio.startListening(); //Recibimos datos
  while (!radio.available()) { //Esperamos hasta tener datos disponible
  }
  radio.stopListening(); //Dejamos de recibir
  radio.read(datos, sizeof(datos)); //Leemos los datos recibidos

  //Devolvemos un número en funcion del dato recibido
  if (abs(datos[0]) > abs(datos[1])) {
    if (datos[0] < 1) e = 0; //Girar izquierda
    else if (datos[0] > 1) e = 1; //Girar derecha
  }
  else if (abs(datos[0]) < abs(datos[1])) {
    if (datos[1] > 1) e = 2; //Avanzar
    else if (datos[1] < 1) e = 3; //retroceder
  }

  else if (datos[2]) e = 4;
  else if (datos[3]) e = 5;
  else if (datos[4]) e = 6;
  else if (datos[6]) e = 8;
  else if (datos[5]) e = 7;

  else e = -1; //Home
  return e;

}

void SonidoAleatorio() {
  if (millis() > m0) {
    m0 = TiempoAleatorio(10000, 40000);
    byte s = random(7);
    switch (s) {
      case 0:
        mySounds.play(soundHappy);
        break;
      case 1:
        mySounds.play(soundConfused);
        break;
      case 2:
        mySounds.play(soundSleeping);
        break;
      case 3:
        mySounds.play(soundSad);
        break;
      case 4:
        mySounds.play(soundOhOoh);
        break;
      case 5:
        mySounds.play(soundWhistle);
        break;
      case 6:
        mySounds.play(soundLaugh);
        break;
    }
  }
}

void ParpadeoAleatorio() {
  if (millis() > m1) {
    m1 = TiempoAleatorio(2000, 10000);
    drawEyes(EYES_BLINK);
  }
}

unsigned long TiempoAleatorio(unsigned long tmin, unsigned long tmax) {
  unsigned long m_ = millis();
  int r = random(tmin, tmax);
  m_ += r;
  return m_;
}

///////////////////////////////////////
/////////////////BAILE/////////////////
///////////////////////////////////////

void BaileSincronizado() {
  int T0 = 2000;
  int T1 = 200;
  int T2 = 200;
  int T3 = 200;
  int T4 = 200;
  int T5 = 1000;
  int T6 = 1000;
  int T7 = 1000;
  int T8 = 200;


  mySounds.play(soundOhOoh);
  delay(500);
  //------------------------------------------
#ifdef Rojo
  //0
  drawEyes(EYES_UP);
  myChipE.updown(6 , T0, 15);
  delay(T0 * 3 / 4);
  //1
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T1, 20);
  delay(T1 * 3);
  //2
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T2, 0, 40);
    myChipE.justFoots(T2, 0, -40);
  }
  //3
  delay(T3 * 3);
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T3, 20);
  //4
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T4, 40, 0);
    myChipE.justFoots(T4, -40, 0);
  }
  //5
  myChipE.bodyRight(1000, 0);
  drawEyes(EYES_CENTER);
  myChipE.swing(6, T5, 20);
  //6
  myChipE.moonwalker(6, T6, 20, RIGHT);
  //7
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T7, 30);
  delay(T7);
  delay(T7);
  //8
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T8, 40, 0);
    myChipE.justFoots(T8, -40, 0);
  }
  myChipE.bodyRight(1000, 0);
#endif
  //------------------------------------------
#ifdef Verde
  //0
  delay(T0 * 1 / 4);
  drawEyes(EYES_UP);
  myChipE.updown(6, T0, 15);
  delay(T0 * 2 / 4);
  //1
  delay(T1);
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T1, 20);
  delay(T1 * 2);
  //2
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T2, 0, 40);
    myChipE.justFoots(T2, 0, -40);
  }
  //3
  delay(T3 * 2);
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T3, 15);
  delay(T3);
  //4
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T4, 40, 0);
    myChipE.justFoots(T4, -40, 0);
  }
  //5
  myChipE.bodyRight(1000, 0);
  drawEyes(EYES_CENTER);
  myChipE.swing(6, T5, 20);
  //6
  myChipE.moonwalker(6, T6, 20, RIGHT);
  //7
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T7, 30);
  delay(T7);
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T7, 20);
  //8
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T8, 0, 40);
    myChipE.justFoots(T8, 0, -40);
  }
  myChipE.bodyRight(1000, 0);
#endif
  //------------------------------------------
#ifdef Amarillo
  //0
  delay(T0 * 2 / 4);
  drawEyes(EYES_UP);
  myChipE.updown(6, T0, 15);
  delay(T0 * 1 / 4);
  //1
  delay(T1 * 2);
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T1, 20);
  delay(T1);
  //2
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T2, 0, 40);
    myChipE.justFoots(T2, 0, -40);
  }
  //3
  delay(T3);
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T3, 20);
  delay(T3 * 2);
  //4
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T4, 40, 0);
    myChipE.justFoots(T4, -40, 0);
  }
  //5
  myChipE.bodyRight(1000, 0);
  drawEyes(EYES_CENTER);
  myChipE.swing(6, T5, 20);
  //6
  myChipE.moonwalker(6, T6, 20, LEFT);
  //7
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T7, 30);
  delay(T7);
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T7, 20);
  //8
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T8, 40, 0);
    myChipE.justFoots(T8, -40, 0);
  }
  myChipE.bodyRight(1000, 0);
#endif
  //------------------------------------------
#ifdef Azul
  //0
  delay(T0 * 3 / 4);
  drawEyes(EYES_UP);
  myChipE.updown(6, T0, 15);
  //1
  delay(T1 * 3);
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T1, 20);
  //2
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T2, 0, 40);
    myChipE.justFoots(T2, 0, -40);
  }
  //3
  drawEyes(EYES_RIGHT);
  myChipE.bodyRight(T3, 20);
  delay(T3 * 3);
  //4
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T4, 40, 0);
    myChipE.justFoots(T4, -40, 0);
  }
  //5
  myChipE.bodyRight(1000, 0);
  drawEyes(EYES_CENTER);
  myChipE.swing(6, T5, 20);
  //6
  myChipE.moonwalker(6, T6, 20, LEFT);
  //7
  drawEyes(EYES_LEFT);
  myChipE.bodyLeft(T7, 30);
  delay(T7);
  delay(T7);
  //8
  drawEyes(EYES_CENTER);
  for (int i = 0; i < 5; i++) {
    myChipE.justFoots(T8, 0, 40);
    myChipE.justFoots(T8, 0, -40);
  }
  myChipE.bodyRight(1000, 0);
#endif

  mySounds.play(soundHappy);
}





