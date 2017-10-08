#ifndef RadioHandshake_h
#define RadioHandshake_h
#include <Arduino.h>
#include "Radio.h"
#include <Metro.h>
#include "RadioPacket.h"

class RadioHandshake {

private:
  Radio* _radio;

  // Timer Constants
  unsigned int _LOST_CONNECTION = 500;

  // Timers
  Metro* handshakeTimeout;


  // Packets
  RadioPacket requestPacket;
  RadioPacket responsePacket;

  // Addresses
  byte defaultAddresses[2][6];
  byte currentAddresses[2][6];
  byte foundAddresses[6];

  // Channel
  byte foundChannel = 255;
  byte channelMax = 125;
  byte channelMin = 100;

  void clearPayload(RadioPacket* packet);
  void findChannel();
  void managePipes();

public:

  //TODO:: This should not be hardcoded.
  byte controllerId[6];
  // Flags
  bool handshaking = false; // flag handshake in progress.
  bool hasPendingDevice = false; // flag handshake successfully finished.

  RadioHandshake(Radio* radio);

  // Main loop
  void handleHandshake();
  // Process Response. Take action if required and populate request.
  void processResponse();

};

#endif
