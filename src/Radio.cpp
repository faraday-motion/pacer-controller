#include "Radio.h"

Radio::Radio() :
defaultAddress{"FM01"},  // initialize defaultAddresses
currentAddresses{"FM01R", "FM01W"}  // initialize currentAddresses
{
  _receiver =  new RF24(10,15);
}

void Radio::setup()
{
  Serial.println("Setting Up Radio Connection");

  //Setup Metro Interval
  this->connectionLostTimer = new Metro(_LOST_CONNECTION);
  this->connectionLostTimer->reset();

  // Setup NRF24.
  _receiver->begin();
  _receiver->setAutoAck(false);
  _receiver->setRetries(0, 0);
  _receiver->setPALevel(RF24_PA_MAX);
  _receiver->setPayloadSize(packetSize);

  this->setChannel(channelDefault);

  _receiver->failureDetected = 0;
  _receiver->printDetails();

  delay(50);
  this->findChannel();

  delay(50);
  this->setAddress(defaultAddress);

  delay(50);
  this->openPipes();

  Serial.println("Finished setting up radio connection");
  yield();
}


/**
   Finds the less crowded channel and sets it as the currentChannel.
*/
byte Radio::findChannel()
{
  byte foundChannel   = 255;
  byte currentChannel = currentChannel;

  for (byte i = channelMax; i >= channelMax; i--)
  {
    setChannel(i);
    _receiver->startListening();

    //We need to wait to get proper results from the tests
    delayMicroseconds(750);

    if(!_receiver->testCarrier())
    {
      //Let's make sure that the next channel is a different one than teh last one and that there is some badnwith between
      if (currentChannel != i && abs(currentChannel - 1) > 6) // The value 6 is a offset for searching channels that are 6 channels away from the previous currentChannel.
      {
        foundChannel = i;
        break;
      }
    }
    yield();
  }

  //If a good channel is not found, continue with the default one
  if (foundChannel == 255)
    foundChannel = channelDefault;

  _receiver->stopListening();
  setChannel(currentChannel);

  return foundChannel;
}


/**
   Sets a radio channel.
   @parameter byte channel.
*/
void Radio::setChannel(byte channel)
{
  _receiver->setChannel(channel);
  currentChannel = channel;
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
  currentAddresses[0][4] = 'W'; // Setting write address
  currentAddresses[1][4] = 'R'; // Setting read address
  delay(5);
  this->openPipes();
}


/**
   Enable the current read/write addresses.
*/
void Radio::openPipes()
{
  _receiver->openReadingPipe(1, currentAddresses[0]);
  _receiver->openWritingPipe(currentAddresses[1]);
  _receiver->startListening();
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
  delay(1);
  setAddress(defaultAddress);
  delay(1);
}


/**
   Attempts to read the requestPacket.
   @return bool
*/
bool Radio::tryReadBytes(RadioPacket* response)
{
  this->printAddresses();
  // Keep track of performance
  bool timeout = false;
  bool success = false;
  long started_waiting_at = millis();

  // Check if we've been trying to write for too long.
  if (connectionLostTimer->check() == 1)
  {
    Serial.println("Radio Connection has been lost");

    // Reset the connection
    this->resetConnection();
  }

  // Check if we have available bytes to read.
  while (!_receiver->available())
  {
    yield();
    if(millis() - started_waiting_at > _TIMEOUT_READ)
    {
      timeout = true;
      Serial.println("Radio Read timeout reached. On tryReadBytes()");
      break;
    }
  }


  // No issues with connection
  if (!timeout)
  {
    success = true;
    connectionLostTimer->reset(); // reset lost connection timer

    // Populate the responsePacket
    _receiver->read(response, packetSize);
    lastPacketId = response->Id; // TODO:: Make use of this.
    yield();
  }

  return success;
}


/**
   Attempts to send the requestPacket over.
   @return bool
*/
bool Radio::tryWriteBytes(RadioPacket* request)
{
  bool success = false;
  sendCount++;
  request->Id = sendCount;// Sets ID to a pakcet.

  //Don't listen while writing
  _receiver->stopListening();

  if(_receiver->write(request, packetSize, 1))
    success = true;
  else
    Serial.println("Radio Failed to Write Bytes");

  // This function should be called as soon as transmission is finished to drop the radio back to STANDBY-I mode
  _receiver->txStandBy();
  _receiver->startListening();

  yield(); // Breathing space.

  return success;
}


/*********
  Set of print methods used for debugging purposes.
*********/

void Radio::printPacket(RadioPacket *packet, bool isRequest)
{
  if (isRequest){
    Serial.print("SENT request:: ");
  } else {
    Serial.print("RECEIVED response:: ");
  }

  Serial.print(" ");
  Serial.print(packet->Id);
  Serial.print(" ");
  Serial.print(packet->Command);
  Serial.print(" ");
  Serial.print(packet->Value1);
  Serial.print(" ");
  Serial.print(packet->Value2);
  Serial.print(" ");
  Serial.print(packet->Value3);
  Serial.print(" ");
  Serial.print(packet->Value4);
  Serial.print(" ");
  Serial.print(packet->Value5);
  Serial.print(" ");
  Serial.print("Length: ");
  Serial.print(packetSize);
  Serial.println();
}

void Radio::printAddresses()
{
  Serial.println();
  Serial.print("CTRL READ address :: ");
  Serial.print(currentAddresses[0][0]);
  Serial.print(" | ");
  Serial.print(currentAddresses[0][1]);
  Serial.print(" | ");
  Serial.print(currentAddresses[0][2]);
  Serial.print(" | ");
  Serial.print(currentAddresses[0][3]);
  Serial.print(" | ");
  Serial.print(currentAddresses[0][4]);
  Serial.print(" | ");

  Serial.println();
  Serial.print("CTRL READ address :: ");
  Serial.print(currentAddresses[1][0]);
  Serial.print(" | ");
  Serial.print(currentAddresses[1][1]);
  Serial.print(" | ");
  Serial.print(currentAddresses[1][2]);
  Serial.print(" | ");
  Serial.print(currentAddresses[1][3]);
  Serial.print(" | ");
  Serial.print(currentAddresses[1][4]);
  Serial.print(" | ");
  Serial.println();


  Serial.print("CHANNEL: ");
  Serial.println(currentChannel);

}
