#include "Radio.h"

Radio::Radio() :
defaultAddresses{"FM01R", "FM01W"},  // initialize defaultAddresses
currentAddresses{"FM01R", "FM01W"},  // initialize currentAddresses
foundAddresses{"FM000"}             // initialize foundAddresses
{
  _radio=  new RF24(10,15);
}

void Radio::setup()
{
  Serial.println("Setting Up Radio Connection");

  //Setup Metro Interval
  this->connectionLostTimer = new Metro(_LOST_CONNECTION);

  // Setup NRF24.
  _radio->begin();
  _radio->setAutoAck(false);
  _radio->setRetries(0, 0);
  _radio->setPALevel(RF24_PA_MAX);
  _radio->setPayloadSize(packetSize);

  this->setChannel(channelDefault);

  _radio->failureDetected = 0;
  _radio->printDetails();

  delay(50);
  this->findChannel();

  delay(50);
  this->setAddress(defaultAddresses[0]);

  delay(50);
  this->generateRandomAddress();
  this->openPipes();

  this->initPackets();

  Serial.println("Finished setting up radio connection");
  yield();
}


/**
  Method that processes a responsePacket and assembles a new requestPacket.
*/

void Radio::processResponse()
{
  // TODO:: Make this thing into a switch.

  if (responsePacket.Command = 1)
  {
    Serial.println("Name requested by FMV");
    requestPacket.Command = 5; // Send name.
  }

  else if (responsePacket.Command == 10)
  {
    Serial.println("Address change requested");
    foundAddresses[0] = responsePacket.Value1;
    foundAddresses[1] = responsePacket.Value2;
    foundAddresses[2] = responsePacket.Value3;
    foundAddresses[3] = responsePacket.Value4;
    foundAddresses[4] = responsePacket.Value5;

    // Changing to the new address.
    this->setAddress(foundAddresses);
    this->openPipes();

    requestPacket.Command = 15; // Confirming address received.
  }

  else if (responsePacket.Command == 20)
  {
    Serial.println("Channel change requested");
    channelFound = responsePacket.Value1;
    setChannel(channelFound);

    requestPacket.Command = 25;
  }
}



/**
  Method that reads and processes a responsePacket,
  and then assembles and write a new requestPacket.
*/
void Radio::readWrite()
{
  if(_radio->failureDetected)
  {
    Serial.println("RF24 Failure Detected. Re-running the setup");
    this->setup();
  }

  if (this->tryReadBytes(&responsePacket)) { // Populates the responsePacket.
    this->printResponsePacket();
    delay(5);
    this->processResponse();
  }

  // Breathing space.
  yield();
  delay(5);

  // Write the requestPacket
  this->tryWriteBytes(&requestPacket);
}


/**
   Attempts to send the requestPacket over.
   @return bool
*/
bool Radio::tryWriteBytes(RadioPacket *request)
{
  bool success = false;
  sendCount++;
  request->Id = sendCount; // Sets ID to a packet.

  // Dont'listen while writing.
  _radio->stopListening();

  if ( _radio->write(request, packetSize, 1) )
    success = true;
  else
    Serial.println("Radio failed to write bytes");

  // This function should be called as soon as transmission
  // is finished to drop the radio back to STANDBY-I mode.
  _radio->txStandBy();
  _radio->startListening();

  yield(); // Breathing space.

  return success;
}


/**
   Attempts to read the requestPacket.
   @return bool
*/
bool Radio::tryReadBytes(RadioPacket *response)
{
  // Keep track of performance.
  bool timeout = false;
  bool success = false;
  long started_waiting_at = millis();

  // Check if we've been trying to read for too long.
  if ( connectionLostTimer->check() == 1 )
  {
    Serial.println("Radio Connection Lost when trying to read.");

    // Reset the connection.
    this->resetConnection();
    // Clear the response/requestPacket
    this->initPackets();
  }

  // Check if we have available bytes to read.
  while ( !_radio->available() )
  {
    yield();
    if ( millis() - started_waiting_at > _TIMEOUT_READ )
    {
      timeout = true;
      Serial.println("Readio Read Timeaout Reached of tryReadBytes()");
      break;
    }
  }

  // No issues with connection
  if ( !timeout )
  {
    success =  true;
    connectionLostTimer->reset(); // reset lost connection timer.

    // Populate the responsePacket
    _radio->read(response, packetSize);
    lastPacketId = response->Id; // TODO:: Make use of this for validation.
    yield();
  }

  return success;
}


/**
   Finds the less crowded channel and sets it as the currentChannel.
*/
void Radio::findChannel()
{
  byte currentChannel = channelSelected;
  for (byte i = channelMax; i >= channelMax; i--)
  {
    setChannel(i);
    _radio->startListening();

    // We need to wait to get proper results from teh tests.
    delayMicroseconds(750);

    if(!_radio->testCarrier())
    {
      //Let's make sure that the next channel is a different one than teh last one and that there is some badnwith between
      if ( currentChannel != i && abs(currentChannel -1) > 6 ) // The value 6 is an offset for searching channels that are 6 channels away from the previous channelSelected.
      {
        channelFound = i;
        break;
      }
    }
    yield();
  }

  // If a good channel is not found, continue with the default one.
  if ( channelFound == 255 )
    channelFound = channelDefault;

  _radio->stopListening();
  setChannel(currentChannel);
}

/**
   Sets a radio channel.
   @parameter byte channel.
*/
void Radio::setChannel(byte channel)
{
  _radio->setChannel(channel);
  channelSelected = channel;
}

/**
  Generates a new address block to be used for writing and reading.
*/
void Radio::generateRandomAddress()
{
  randomSeed(analogRead(A0));
  foundAddresses[0] = random(0, 255);
  foundAddresses[1] = random(0, 255);
  foundAddresses[2] = random(0, 255);
  foundAddresses[3] = random(0, 255);
  foundAddresses[4] = 0; // This is a slot for read/write flag
}

/**
   Takes an address block and sets the current read/write addresses.
*/
void Radio::setAddress(byte address[])
{
  // Here we assemble address;
  for (byte i = 0; i < 5; i++)
  {
    currentAddresses[0][i] = address[i];
    currentAddresses[1][i] = address[i];
  }
  currentAddresses[0][4] = 'W'; // Setting read address
  currentAddresses[1][4] = 'R'; // Setting write address
}

/**
   Enable the current read/write addresses.
*/
void Radio::openPipes()
{
  _radio->openReadingPipe(1, currentAddresses[0]);
  _radio->openWritingPipe(currentAddresses[1]);
  _radio->startListening();
}

/**
   Resets the radio connection
   1. Sets Default channel
   2. Sets Default read/write addresses
   3. Generates a new set of random read/write addresses
   4. Enabbles the default read/write addresses
*/
void Radio::resetConnection()
{
  Serial.println("START RESET RADIO CONN");
  setChannel(channelDefault);
  setAddress(defaultAddresses[0]);
  generateRandomAddress();
  openPipes();
  delay(1);
}

/**
   Initialize response/requestPackets.
*/
void Radio::initPackets()
{
  // Init.
  responsePacket.Id      = 0;
  responsePacket.Command = 0;
  responsePacket.Value1  = 0;
  responsePacket.Value2  = 0;
  responsePacket.Value3  = 0;
  responsePacket.Value4  = 0;
  responsePacket.Value5  = 0;

  // Init.
  requestPacket.Id      = 0;
  requestPacket.Command = 1; // default command for seeking.
  requestPacket.Value1  = 0;
  requestPacket.Value2  = 0;
  requestPacket.Value3  = 0;
  requestPacket.Value4  = 0;
  requestPacket.Value5  = 0;
}
