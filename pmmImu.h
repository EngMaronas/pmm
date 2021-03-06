// GY-80 IMU Library especially written for use with Teensy 3.6
// Code written by Marcelo Maronas @ Minerva Rockets (Federal University of Rio de Janeiro Rocketry Team) - February 19, 2018
// Using Adafruit Libraries.
// Contact : marcelomaronas at poli.ufrj.br
// For more codes : github.com/engmaronas

/*
ReadMe ver 1.0, library GY80TEENSY - February 20, 2018

This is a necessary adaptation of my own GY80 library for using the GY80 IMU module with the microcontroller Teensy version 3.6.
This the very basic code using the library.The library was written using the best libraries I could find at the moment for each GY80 sub-module (Gyro, Accel, Magne, BMP085) and
putting them together in an lightweight and easy to understand code.Dont use it with Arduino, there's a lighter version of GY80 library that doesnt need so much memory, check in my GitHub.
The libraries for each sub-modules are, in majority, adapted adafruit libraries, and because of it,
they are very heav.But in the counterpart, they also are very robust and have methods for everything that you need to do with the sensor.

Libraries required for the code to work (they are all included into the AdafruitNeededLibraries.rar file) :
-Adafruit_ADXL345-master
-Adafruit_BMP085_Unified-master
-Adafruit_HMC5883_Unified-master
-Adafruit_Sensor-master
-l3g-arduino-master (Non-Adafruit library)


Contact : marcelomaronas at poli.ufrj.br
For more codes : github.com/engmaronas
*/

#ifndef PMM_IMU_h
#define PMM_IMU_h

#define DELAY_MS_BAROMETER 100 //random value

#include <Wire.h>
#include "Adafruit_Sensor-master/Adafruit_Sensor.h"
#include "Adafruit_BMP085_Unified-master/Adafruit_BMP085_U.h"
#include "Adafruit_ADXL345-master/Adafruit_ADXL345_U.h"
#include "l3g-arduino-master/L3G.h"
#include "Adafruit_HMC5883_Unified-master/Adafruit_HMC5883_U.h"
#include <pmmErrorsAndSignals.h>

class PmmImu
{
private:
    Adafruit_BMP085_Unified mBmpObject;
    Adafruit_ADXL345_Unified mAccelerometerObject;
    Adafruit_HMC5883_Unified mMagnetometerObject;
    L3G mGyroscopeObject;
    sensors_event_t mEvent;
    unsigned long mNextMillisBarometer;
    PmmErrorsAndSignals *pmmErrorsAndSignals;

public:
    //
    PmmImu();
    int initAccelerometer(); //ADXL45 SETUP
    int initGyroscope(); //L2G4200D Setup
    int initMagnetometer(); //HMC5884 Setup
    int initBMP();  //BMP085 Setup

    int init(PmmErrorsAndSignals *pmmErrorsAndSignals); // Must be executed, so the object is passed

    //
    int getGyroscope();
    int getAccelerometer();
    int getMagnetometer();
    int getBMP();

    int update(); // Gets all the sensor above

    float accelerometer[3]; //Posicoes 1,2,3, respectivamente sao as Aceleracoes em x,y,z
    float magnetometer[3]; //Posicoes 1,2,3, respectivamente sao as Campos Magneticos em x,y,z
    float gyroscope[3]; //Posicoes 1, 2, 3, respectivamente sao a velocidade angular em x,y,z
    float barometer[2]; //Posicoes 1,2,3 respectivamente sao Pressao, Altura e Temperatura
    float temperature;
};

#endif
