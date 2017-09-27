#ifndef Radio_h
#define Radio_h
#include <SPI.h>
#include <RF24.h>
#include <Metro.h>
#include <nRF24L01.h>
#include "RadioPacket.h"


class Radio {
private:
  RF24* _radio;
  Metro* connectionLostTimer;
  Metro* readInterval;

  // Connection defaults, flags and inits.
  byte defaultAddresses[2][6];
  byte currentAddresses[2][6];
  byte foundAddresses[6];
  byte channelDefault  = 100;
  byte channelSelected = 100;
  byte channelFound = 255;
  byte channelMax = 125;
  byte channelMin = 100;
  byte connectionStrength = 0;
  bool handShaking  = false;
  byte packetSize   = 7;
  byte sendCount    = 0;

  RadioPacket responsePacket;
  RadioPacket requestPacket;

  //Intervals
  byte _TIMEOUT_READ = 50;
  byte _READ_INTERVAL = 10;
  unsigned int _LOST_CONNECTION = 500;

  byte lastPacketId = 0;

  void processResponse();

  // Connection
  void findChannel();
  void setChannel(byte channel);
  void changeChannel();
  void generateRandomAddress();
  void setAddress(byte address[]);
  void openPipes();
  void resetConnection();
  void initPackets();
  bool usingDefaultAddress();

  // Controller
  byte controllerId[6];
  int controllerSample = -1;

public:
  Radio();
  void setup();

  // Communication
  void readWrite();

  // Read/Write packets
  bool tryReadBytes(RadioPacket* response);
  bool tryWriteBytes(RadioPacket* request);

  void setControllerSample(int sample);

  // Debug
  void printRequestPacket();
  void printResponsePacket();
  void printAddresses();

};

#endif
