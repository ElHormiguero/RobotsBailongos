//--------------------------------------------------------------
//-- Oscillator.pde
//-- Generate sinusoidal oscillations in the servos
//--------------------------------------------------------------
//-- (c) Juan Gonzalez-Gomez (Obijuan), Dec 2011
//-- GPL license
//--------------------------------------------------------------
//-- Added servos controlled with PCA9685 by Javier Vargas

#ifndef Oscillator_h
#define Oscillator_h

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

//-- Macro for converting from degrees to radians
#ifndef DEG2RAD
  #define DEG2RAD(g) ((g)*M_PI)/180
#endif



class Oscillator
{
  public:
    Oscillator(int trim=0) {_trim=trim;};
    void attach(int pin, bool rev =false);
    void detach();

    void SetA(unsigned int A) {_A=A;};
    void SetO(unsigned int O) {_O=O;};
    void SetPh(double Ph) {_phase0=Ph;};
    void SetT(unsigned int T);
    void SetTrim(int trim){_trim=trim;};
    int getTrim() {return _trim;};
    void SetPosition(int position);
    void Stop() {_stop=true;};
    void Play() {_stop=false;};
    void Reset() {_phase=0;};
    void refresh();
    void pwmMINMAX(int SERVOMIN_, int SERVOMAX_);

  private:
    bool next_sample();

  private:

    Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); //Driver I2C 0x40
    //-- Servo that is attached to the oscillator
    //Servo _servo;
    int ServoDir;
    void SetServo(int angulo);
    int SERVOMIN; // this is the 'Minimum' pulse length count (out of 4096)
    int SERVOMAX; // this is the 'maximum' pulse length count (out of 4096)

    //-- Oscillators parameters
    unsigned int _A;  //-- Amplitude (degrees)
    unsigned int _O;  //-- Offset (degrees)
    unsigned int _T;  //-- Period (miliseconds)
    double _phase0;   //-- Phase (radians)

    //-- Internal variables
    int _pos;         //-- Current servo pos
    int _trim;        //-- Calibration offset
    double _phase;    //-- Current phase
    double _inc;      //-- Increment of phase
    double _N;        //-- Number of samples
    unsigned int _TS; //-- sampling period (ms)

    long _previousMillis;
    long _currentMillis;

    //-- Oscillation mode. If true, the servo is stopped
    bool _stop;

    //-- Reverse mode
    bool _rev;
};

#endif
