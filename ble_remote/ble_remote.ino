#include "CurieBle.h"

static const char* bluetoothDeviceName = "LedRemote";

static const int   characteristicTransmissionLength = 2; 

// Commands

static const uint8_t bleCommandFooterPosition = 1;
static const uint8_t bleCommandDataPosition = 0;

static const uint8_t bleCommandFooter = 1;

static const uint8_t bleCommandLedOn = 1;
static const uint8_t bleCommandLedOff = 2;


// Responses
static const uint8_t bleResponseFooterPosition = 1;
static const uint8_t bleResponseDataPosition = 0;

static const uint8_t bleResponseErrorFooter = 0;
static const uint8_t bleResponseConfirmationFooter = 1;

static const uint8_t bleResponseLedError = 0;
static const uint8_t bleResponseLedOn = 1;
static const uint8_t bleResponseLedOff = 2;

// Peripheral Properties

static const byte ledPin = 13;
static const unsigned int ledError = 0;
static const unsigned int ledOn = 1;
static const unsigned int ledOff = 2;
int ledState = ledOff;

// internal state

char bleMessage[characteristicTransmissionLength];
const char* uuid;
bool bleCommandReceived = false;


BLEService service("180C");
BLECharacteristic commandCharacteristic(
  "2A56",
  BLEWrite,
  characteristicTransmissionLength
);

BLECharacteristic responseCharacteristic(
  "2A57",
  BLERead | BLENotify,
  characteristicTransmissionLength
);

BLEPeripheral blePeripheral;


void onCharacteristicWritten(BLECentral& central, 
  BLECharacteristic &characteristic) {
    
  bleCommandReceived = true;
  uuid = characteristic.uuid();
  
  strcpy((char*) bleMessage, (const char*) characteristic.value());
}

void sendBleCommandResponse(int ledState) {
  byte confirmation[characteristicTransmissionLength] = {0x0};
  confirmation[bleResponseDataPosition] = (byte)ledState;
  confirmation[bleResponseFooterPosition] = (byte)bleResponseConfirmationFooter;
  responseCharacteristic.setValue((const unsigned char*) confirmation, characteristicTransmissionLength);
}


// Central connected.  Print MAC address
void onCentralConnected(BLECentral& central) {
  Serial.print("Central connected: ");
  Serial.println(central.address());
}

// Central disconnected
void onCentralDisconnected(BLECentral& central) {
  Serial.println("Central disconnected");
}


void setup() {
  Serial.begin(9600);
  while (!Serial) {;}

  digitalWrite(ledPin, LOW); // start with LED off
  
  blePeripheral.setLocalName(bluetoothDeviceName);
  
  // attach callback when central connects
  blePeripheral.setEventHandler(
    BLEConnected,
    onCentralConnected
  );
  // attach callback when centlal disconnects
  blePeripheral.setEventHandler(
    BLEDisconnected,
    onCentralDisconnected
  );

  
  blePeripheral.setAdvertisedServiceUuid(service.uuid());
  blePeripheral.addAttribute(service);
  blePeripheral.addAttribute(commandCharacteristic);
  blePeripheral.addAttribute(responseCharacteristic);

  commandCharacteristic.setEventHandler(
    BLEWritten,
    onCharacteristicWritten
  );

  Serial.print("Starting ");
  Serial.println(bluetoothDeviceName);
  blePeripheral.begin();
}

void loop() {
  if (bleCommandReceived) {
    bleCommandReceived = false; // ensures only executed once
  
    // incoming command is one byte
    unsigned int command = bleMessage[bleCommandDataPosition]; 
    if (command == bleCommandLedOn) {
      Serial.println("Turning LED on");
      ledState = HIGH;
      sendBleCommandResponse(ledState);
    } else {
      Serial.println("Turning LED off");
      ledState = LOW;
      sendBleCommandResponse(ledState);
    }

    digitalWrite(ledPin, ledState);
  }

}
