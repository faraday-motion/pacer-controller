#define IS_TEENSY false
#define TIMEOUT_READ 50
#define READ_INTERVAL 10
#define SIGNAL_CHECK_INTERVAL 250

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ControlPacket.h>
#include <Metro.h>

//Variables
// #if defined IS_TEENSY
// //Teensy
// RF24 myRadio (7, 8);
// #endif
// #ifndef IS_TEENSY
//NODEMCU
RF24 myRadio (4, 15);
// #endif
byte contollerId[6] = {"s0xY2"};
//Note the address is mirrored with the other transiever
byte defaultAddresses[][6] = {"FM01R", "FM01W"};
byte currentAddresses[][6] = {"FM01R", "FM01W"};
byte foundAddresses[6] = {"FM000"};
long m = millis();
int controllerSpeed = -1;
int controllerSteer = -1;
byte controllerEnabled = false;
byte controllerCruise = false;
byte controllerDirection = false;
bool controllerConnected = false;
byte channelDefault = 100;
byte channelSelected = 100;
byte channelFound = 255;
byte sendCommand = 0;
byte packetSize = 7;
byte lastPacketId = 0;
byte sendCount = 0;
unsigned long recieveCounter = 0;
byte connectionStrength = 0;
Metro metroCommunication = Metro(READ_INTERVAL);
Metro metroController = Metro(SIGNAL_CHECK_INTERVAL);
Metro metroHasController = Metro(500); // was 500

bool controllerIsEnabledAndRegistered = false;


bool usingDefaultAddress()
{

  if (currentAddresses[0][0] == defaultAddresses[0][0] && currentAddresses[0][1] == defaultAddresses[0][1] && currentAddresses[0][2] == defaultAddresses[0][2] &&  currentAddresses[0][3] == defaultAddresses[0][3])
  {
    return true;
  }
  return false;
}

void printAddresses()
{
  Serial.println();
  Serial.print("Radio.h READ ADDRESS :: ");
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
  Serial.print("Radio.h Write ADDRESS :: ");
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

}

void printRequestPacket(ControlPacket* requestPacket)
{
  Serial.println("Current Request:: ");
  Serial.print(requestPacket->Id);
  Serial.print(" ");
  Serial.print(requestPacket->Command);
  Serial.print(" ");
  Serial.print(requestPacket->Value1);
  Serial.print(" ");
  Serial.print(requestPacket->Value2);
  Serial.print(" ");
  Serial.print(requestPacket->Value3);
  Serial.print(" ");
  Serial.print(requestPacket->Value4);
  Serial.print(" ");
  Serial.print(requestPacket->Value5);
  Serial.print(" ");
  Serial.println();
}

void printResponsePacket(ControlPacket* responsePacket)
{
  Serial.println("Packet Response:: ");
  Serial.print(responsePacket->Id);
  Serial.print(" ");
  Serial.print(responsePacket->Command);
  Serial.print(" ");
  Serial.print(responsePacket->Value1);
  Serial.print(" ");
  Serial.print(responsePacket->Value2);
  Serial.print(" ");
  Serial.print(responsePacket->Value3);
  Serial.print(" ");
  Serial.print(responsePacket->Value4);
  Serial.print(" ");
  Serial.print(responsePacket->Value5);
  Serial.print(" ");
  Serial.println();
}



void openPipes()
{
  myRadio.openReadingPipe(1, currentAddresses[0]);
  myRadio.openWritingPipe(currentAddresses[1]);
  myRadio.startListening();
}

void setChannel(byte channel)
{
  myRadio.setChannel(channel);
  channelSelected = channel;
}

//Note the address is mirrored with the other transiever
void setAddress(byte address[])
{
  for (byte i = 0; i < 5; i++)
  {
    currentAddresses[0][i] = address[i];
    currentAddresses[1][i] = address[i];
  }
  currentAddresses[0][4] = 'W';
  currentAddresses[1][4] = 'R';
}

void resetConnection()
{
  Serial.println("RESET CONN");
  controllerConnected = false;
  setChannel(channelDefault);
  setAddress(defaultAddresses[0]);
  openPipes();
  sendCommand = 0;

}

int readAnalogSensorPin(byte pin)
{
  byte measurements = 5;
  float totalMeasurement = 0;
  for (byte i = 0; i < measurements; i++)
  {
    totalMeasurement += analogRead(pin);
  }
  return totalMeasurement / measurements;
}

void readJoystick()
{
  controllerSpeed = readAnalogSensorPin(A0);
  //controllerSteer = readAnalogSensorPin();

  Serial.print("SPEED ::: ");
  Serial.println(controllerSpeed / 4);


  //Serial.print("  STEER ::: ");
  //Serial.print(controllerSteer / 4);
  Serial.println();
  // controllerEnabled = !digitalRead(16);
  // controllerCruise = !digitalRead(17);
  // controllerDirection = !digitalRead(18);
}


bool tryReadBytes(::ControlPacket* data)
{
  bool timeout = false;
  bool success = false;
  long started_waiting_at = millis();
  while (!myRadio.available()) {
    yield();
    if (millis() - started_waiting_at > TIMEOUT_READ) {
      timeout = true;
      Serial.println("Readtimeout");
      break;
    }
  }

  if (!timeout) {
    success = true;
    controllerConnected = true;
    metroHasController.reset();
    //Serial.println("ReadBytes:");
    //Serial.println(packetSize);
    myRadio.read(data, packetSize);

    //Serial.println("DIFF");
    //Serial.println(max(data -> Id, lastPacketId) - min(data -> Id, lastPacketId));
    if (max(data -> Id, lastPacketId) - min(data -> Id, lastPacketId) > 5 )//TODOSet to some realistic good value
    {
      //We have some connection issues
      //Not sure what to do for now. We can use the
    }
    if (data -> Id == 255)
      lastPacketId = 0;
    else
      lastPacketId = data -> Id;

    printResponsePacket(data);
  }
  else
  {
    Serial.println("Reading from base failed - timeout");
  }
  return success;
}

bool tryWriteBytes(::ControlPacket data)
{

  printAddresses();

  bool success = false;
  data.Id = sendCount;
  //Dont listen while writing
  myRadio.stopListening();
  //Serial.println("tryWriteBytes");
  //if (true)
  if (myRadio.write(&data, packetSize, 1))
  {
    sendCount++;
    success = true;
    printRequestPacket(&data);
  }
  else
  {
    Serial.println("WRITING BYTES FAILED");
  }
  //This function should be called as soon as transmission is finished to drop the radio back to STANDBY-I mode
  myRadio.txStandBy();
  //Enable listening after writing
  myRadio.startListening();
  return success;
}

void handleRead()
{
  //Listen for incoming name
  ControlPacket packet;
  if (tryReadBytes(&packet))
  {
    if (packet.Command == 1)
    {
      //We have a name packet
      Serial.println("NAME Requested");
      sendCommand = 5;
    }
    else if (packet.Command == 10)
    {
      //We have an address change packet
      Serial.println("Address change Requested");
      foundAddresses[0] = packet.Value1;
      foundAddresses[1] = packet.Value2;
      foundAddresses[2] = packet.Value3;
      foundAddresses[3] = packet.Value4;
      foundAddresses[4] = packet.Value5;
      for (int i = 0; i < 6; i++)
      {
        Serial.print(foundAddresses[i]);
        Serial.print(" ");
      }
      Serial.println();
      sendCommand = 15;
    }
    else if (packet.Command == 20 )
    {
      //We have an channel change packet
      Serial.println("Channel change Requested");
      channelFound = packet.Value1;
      sendCommand = 25;
    }
    else if (packet.Command == 40 && !usingDefaultAddress())
    {
      Serial.println("Stand Idle Command 40");
      sendCommand = 44;
    }
    else if (packet.Command == 50 && !usingDefaultAddress())
    {
      //Controller input requested
      recieveCounter++;
      Serial.println("Controller input Requested");
      sendCommand = 55;
      controllerIsEnabledAndRegistered = true;
    }
  }
}

void handleWrite()
{
  if (sendCommand == 5)
  {
    Serial.println("Command 5");
    //Send a packet with the name
    ControlPacket namePacket;
    namePacket.Command = 5;
    namePacket.Value1 = contollerId[0];
    namePacket.Value2 = contollerId[1];
    namePacket.Value3 = contollerId[2];
    namePacket.Value4 = contollerId[3];
    namePacket.Value5 = contollerId[4];
    tryWriteBytes(namePacket);
  }
  else if (sendCommand == 15)
  {
    Serial.println("Command 15");
    //Send a packet with the confirmation
    ControlPacket addressConfirmPacket;
    addressConfirmPacket.Command = 15;
    if (tryWriteBytes(addressConfirmPacket))
    {
      //After sending the confirm, change the adress
      Serial.println("We will change address to: ");
      for (int i = 0; i < 4; i++)
      {
        Serial.print(foundAddresses[i]);
        Serial.print(" ");
      }
      Serial.println();
      setAddress(foundAddresses);
      openPipes();
    }
  }
  else if (sendCommand == 25)
  {
    Serial.println("Command 25");
    //Send a packet with the confirmation
    ControlPacket channelConfirmPacket;
    channelConfirmPacket.Command = 25;
    tryWriteBytes(channelConfirmPacket);

    //After sending the confirm, change the channel
    setChannel(channelFound);
  }
  else if (sendCommand == 44)
  {
    // this is I am idle command.
    Serial.println("Command 44");
    ControlPacket idlePacket;
    idlePacket.Command = 44;
    tryWriteBytes(idlePacket);
  }
  else if (sendCommand == 55)
  {
    Serial.println("Command 55");
    readJoystick();
    ControlPacket controllerPacket;
    controllerPacket.Command = 55;
    controllerPacket.Value1 = controllerEnabled;
    controllerPacket.Value2 = controllerSpeed / 4;
    controllerPacket.Value3 = controllerSteer / 4;
    controllerPacket.Value4 = controllerCruise;
    controllerPacket.Value5 = controllerDirection;
    tryWriteBytes(controllerPacket);
  }
}




void setupRadio(byte channel)
{
  myRadio.begin();  // Start up the physical nRF24L01 Radio
  myRadio.setAutoAck(false);
  myRadio.setRetries(0, 0);
  //  myRadio.setDataRate(RF24_250KBPS);
  //myRadio.setPALevel(RF24_1MBPS);
  myRadio.setPALevel(RF24_2MBPS);
  //myRadio.setPALevel(RF24_PA_MIN);
  myRadio.setPALevel(RF24_PA_MAX);  // Uncomment for more power
  myRadio.setPayloadSize(packetSize);
  setChannel(channel);
  myRadio.failureDetected = 0;
  myRadio.printDetails();
}


void setup()
{
  Serial.begin(115200);
  Serial.println("SETUP");

  // TEENSY
  // pinMode(A0, INPUT_PULLUP);
  // pinMode(A1, INPUT_PULLUP);
  // pinMode(16, INPUT_PULLUP);
  // pinMode(17, INPUT_PULLUP);
  // pinMode(18, INPUT_PULLUP);

  // NODEMCU
  pinMode(2, INPUT_PULLUP); // x
  pinMode(A0, INPUT_PULLUP); // y
  // pinMode(16, INPUT_PULLUP);
  // pinMode(17, INPUT_PULLUP);
  // pinMode(18, INPUT_PULLUP);
  delay(50);
  setupRadio(channelDefault);
  delay(50);
  setAddress(defaultAddresses[0]);
  delay(50);
  openPipes();
}

void loop()
{
  if (myRadio.failureDetected)
  {
    Serial.println("failureDetected!!!!!");
    setupRadio(channelDefault);
    resetConnection();
  }
  else
  {
    if (metroCommunication.check() == 1)
    {
      //Serial.print(" Millis: ");
      //Serial.println(millis() - m);
      m = millis();
      //Serial.print(" recieveCounter : ");
      //Serial.println(recieveCounter );
      //Serial.println("*************START READING*************");
      handleRead();
      //Serial.println("*************END READING*************");
      //Check performance of message
      if (metroController.check() == 1)
      {
        //Calculate the strength of the connection based on the recieved controller packets
        int rc = (SIGNAL_CHECK_INTERVAL / READ_INTERVAL) -1;
        connectionStrength = min(float(recieveCounter) / float(rc) * 100, 100);
        recieveCounter = 0;
      }
      //Serial.print("connectionStrength ");
      //Serial.println(connectionStrength);
      //Serial.println("*************START WRITING*************");
      handleWrite();
      //Serial.println("*************END READING*************");
      if (metroHasController.check() == 1) {
        //Check if we had connection problems
        Serial.println(":::::::::::::::::::::::::");
        Serial.println("CONNECTION LOST TO NunchuckController");
        Serial.println(":::::::::::::::::::::::::");

        resetConnection();
      }
    }
  }
}
