#include "RadioHandshake.h"
#include "Commands.h"

RadioHandshake::RadioHandshake(Radio* radio) :
defaultAddresses{"FM01R", "FM01W"},   // initialize defaultAddresses
currentAddresses{"FM01R", "FM01W"},   // initialize currentAddresses
foundAddresses{"FM000"},              // initialize foundAddresses
controllerId{"s0xY2"}                 // TODO:: THIS SHOULD NOT BE HARDCODED
{
  this->_radio = radio;
  this->handshakeTimeout = new Metro(_LOST_CONNECTION);
  this->handshakeTimeout->reset();
}

// Main loop for read and write.
void RadioHandshake::handleHandshake()
{

  if (this->handshakeTimeout->check() == 1) {
    Serial.println("CTRL WARNING:: RadioHandhake timedout.");
    this->clearPayload(&requestPacket);
    this->clearPayload(&responsePacket);
    return;
  }

  if ( _radio->tryReadBytes(&responsePacket) ) // populate the responsePacket.
  {
    delay(5); // TODO:: Do we really need this here?
    this->_radio->printPacket(&responsePacket, false);
    this->processResponse();
  }
  this->_radio->printPacket(&requestPacket, true);
  this->_radio->tryWriteBytes(&requestPacket); // poplate requestPacket.

  this->managePipes();
}

void RadioHandshake::managePipes()
{
  // Intercept address change.
  if (requestPacket.Command == CONFIRM_CHANGE_ADDRESS) {
    Serial.println("Changing Address from handshake");
    this->_radio->setAddress(foundAddresses);
  }
  // Intercept channel change.
  if (requestPacket.Command == CONFIRM_CHANGE_CHANNEL) {
    Serial.println("Changing Channel");
    this->_radio->setChannel(foundChannel);
  }
}

// Proess response and assemble new request.
void RadioHandshake::processResponse()
{
  switch (responsePacket.Command)
  {
    // Name Received from pending device.
    case REQUEST_NAME:
      Serial.println("REQUEST_NAME");
      this->handshaking = true;
      //Assemble Request Packet and request address change
      requestPacket.Command = SAVE_NAME;
      requestPacket.Value1  = controllerId[0];
      requestPacket.Value2  = controllerId[1];
      requestPacket.Value3  = controllerId[2];
      requestPacket.Value4  = controllerId[3];
      requestPacket.Value5  = controllerId[4];
      break;

    //  Adress change confirmation.
    case SAVE_ADDRESS:
      Serial.println("SAVE_ADDRESS");

      foundAddresses[0] = responsePacket.Value1;
      foundAddresses[1] = responsePacket.Value2;
      foundAddresses[2] = responsePacket.Value3;
      foundAddresses[3] = responsePacket.Value4;
      foundAddresses[4] = responsePacket.Value5;

      requestPacket.Command = CONFIRM_SAVE_ADDRESS;
      requestPacket.Value1 = responsePacket.Value1;
      requestPacket.Value2 = responsePacket.Value2;
      requestPacket.Value3 = responsePacket.Value3;
      requestPacket.Value4 = responsePacket.Value4;
      requestPacket.Value5 = responsePacket.Value5;

      break;
    case CHANGE_ADDRESS:
      Serial.println("CHANGE_ADDRESS");
      // Address change happens in the intercept in the handleHandshake method.

      requestPacket.Command = CONFIRM_CHANGE_ADDRESS;
      this->clearPayload(&requestPacket);
      break;

    case SAVE_CAHNNEL:
      Serial.println("SAVE_CAHNNEL");

      foundChannel = responsePacket.Value1;

      requestPacket.Command = CONFIRM_SAVE_CHANNEL;
      requestPacket.Value1  = foundChannel;
      break;
      
    case CHANGE_CHANNEL:
      Serial.println("CHANGE_CHANNEL");
      // Channel change happens in the intercept in the handleHandshake method.
      requestPacket.Command = CONFIRM_CHANGE_CHANNEL;
      this->clearPayload(&requestPacket);
      break;

    case WAIT_REGISTER:
      Serial.println("WAIT_REGISTER");

      requestPacket.Command = CONFIRM_WAIT_REGISTER;
      this->clearPayload(&requestPacket);
      break;

    case FAILED_REGISTER:
      Serial.println("FAILED_REGISTER");

      Serial.println("Controller is not registered.");
      Serial.print("Reason: ");
      Serial.println(responsePacket.Value1);
      break;

    case SET_MODE_IDLE:
      Serial.println("SET_MODE_IDLE");
      requestPacket.Command = CONFIRM_SET_MODE_IDLE;
      break;

    case SET_MODE_ACTIVE:
      Serial.println("SET_MODE_ACTIVE");
      requestPacket.Command = CONFIRM_SET_MODE_ACTIVE;
      break;

    case REQUEST_INPUT:
      Serial.println("REQUEST_INPUT");
      requestPacket.Command = CTRL_HAS_INPUT;
      requestPacket.Value2  = 111;
      break;
  }
}

// Clears the payload so we don't send it with a wrong command.
void RadioHandshake::clearPayload(RadioPacket* packet)
{
  packet->Value1 = 0;
  packet->Value2 = 0;
  packet->Value3 = 0;
  packet->Value4 = 0;
  packet->Value5 = 0;
}
