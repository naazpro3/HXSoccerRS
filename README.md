# HXSoccerRS
this repository contains the code for HXSoccer Robosoccer Robots used in Mekathlon 2025. The code is in its 1st public release and supports both espnow and pwm recievers.

# Requirements
Arduino IDE - https://www.arduino.cc/en/software/

ESP32 Core 2.0.17 (no future versions are supported)

# Installation
There are two variants of this code - 
  SmartElex :
    This version of code drives the robosoccer motors through smartelex input structure (DIR and PWM pins) and recieves signals from ACTransmitter (github.com/naazpro3/ACTransmitter/) and is also capable of recieving pwm rc signals 

  BTS7960 :
    This version of code drives the robosoccer motors through BTS7960 (and other generic drivers) structure (L_PWM and R_PWM pins) and recieves signals from ACTransmitter (github.com/naazpro3/ACTransmitter/) and is also capable of recieving pwm rc signals

How to Flash the ESP32
  1. open the .ino file of your selected variant
  2. select yuor esp32 board type and COM port of the esp32 on the top
  3. upload the code

# Connections
Smartelex :

  L_DIR - 12
  L_PWM - 14
  ----------
  R_DIR - 27
  R_PWM - 26

BTS7960 :

  Left_L_PWM - 12
  Left_R_PWM - 14
  ---------------
  Right_L_PWM - 27
  Right_R_PWM - 26

Reciever (common) :
  Throttle - 36
  Steering - 39

# All done
