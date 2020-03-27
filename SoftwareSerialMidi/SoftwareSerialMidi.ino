
#include <SoftwareSerial.h>


//MIDI specific configuration
#define MIDI_CHANNEL (0x04) // Set this to the MIDI devices common channel used

//MIDI constants
#define MIDI_DAMPER_COMMAND (64)
#define MIDI_CONTROL_CHANGE_COMMAND (0xB0)

//Arduino Pins configuration
#define PIN_DUMPER 8
#define MIDI_RX 10
#define MIDI_TX 11

//Software serial to communicate with the midi in a serial fashion
SoftwareSerial mySerial(MIDI_RX, MIDI_TX);

/// Sends to the connected PC a value change for the dumper pedal,
/// then resend the last usd MIDI command by the peripheral
void inline sendDamper(bool value);

void setup() {
  //Set the dumper pin to be an input with pullup, to save external harware
  pinMode(PIN_DUMPER, INPUT_PULLUP);


  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // Set the data rate for the SoftwareSerial port
  // 31250 is the standard speed for MIDI devices
  mySerial.begin(31250);
}

//Number of bytes sent after the last command, to prevent interferences with standard communication
unsigned messagebyte = 2;

//Default value for last pedal, disconnected from GND
int lastRead = HIGH;

//Last byte received on the MIDI serial port
int midiread = 0;

//Expected MIDI bytes
int expectedMidiBytes = 0; //No bytes at startup to send

//Last byte received from the MIDI serial port that was a command (MSB = 1)
int lastCommand = 0xFF;

void loop() { // run over and over

  // From MIDI to pc
  if (mySerial.available()) {
    //Transmit every byte read from the MIDI port to pc
    midiread = mySerial.read();
    Serial.write(midiread);

    if (midiread & 0x80) { //start byte for command
      lastCommand = midiread; //save last command
      messagebyte = 0; //sync counter
      if ((lastCommand & 0xF0) == 0xC0 || (lastCommand & 0xF0) == 0xD0) {
        expectedMidiBytes = 1;
      } else {
        expectedMidiBytes = 2;
      }
    } else {
      // When we send the correct amount of bytes, reset the counter,
      // because the command is not sent out again (only on command variation!)
      if (messagebyte >= expectedMidiBytes) {
        messagebyte = 1;
      } else {
        messagebyte++;
      }
    }
  }

  //From pc to MIDI, to enable bidirectional communication, uncomment these lines (not tested but should kinda work)
  /*if (Serial.available()) {
    mySerial.write(Serial.read());
    }*/


  // Damper signal
  // Read the damper value, if different from last read, send notification to PC
  int volatile read_now = digitalRead(PIN_DUMPER);

  // The check with messagebytes is needed because otherwise we can disrupt MIDI communication
  // by overlapping between two messages of the same command
  if (messagebyte >= expectedMidiBytes && read_now != lastRead) {
    lastRead = read_now;
    if (lastRead == HIGH) {
      //PULLUP, so HIGH means "not connected to ground"
      sendDamper(false);

    } else {
      //PULLUP, LOW means connected to GND
      sendDamper(true);
    }
  }


}


void inline sendDamper(bool value) {
  //CONTROLLER CHANGE, on channel 4
  //High nibble = command, Low nibble = channel
  Serial.write(MIDI_CONTROL_CHANGE_COMMAND | MIDI_CHANNEL);

  // Damper change command
  Serial.write(MIDI_DAMPER_COMMAND);


  if (value) {
    //if true, send a value greater than 64 = PRESSED
    Serial.write(70);
  } else {
    //if false, send a value less than 64 = RELEASED
    Serial.write(1);
  }

  // As we have sent a different midi command (CONTROLLER CHANGE) but the MIDI device is unaware of that,
  // send again the last command sent by the MIDI device, so that further messages will be processed correctly
  Serial.write(lastCommand);
}
