# Arduino-MIDI-damper
A simple MIDI-enabled damper/sustain pedal created with an Arduino Uno.

*Disclaimer*: this worked on my keyboard, but I am not responsible of any demage if you connect your keyboard to an Arduino.

## The problem

My second piano keyboard (a pretty cheap one) has ever had two problems:
* lack of a working MIDI connection: an USB connection is present, but no drivers are present for Windows. Instead, it works on Linux, but has a terrible problem: no more than two key can be pressed together (Really? No one ever tested this device?). The piano samples on it are pretty bad, while with a MIDI connection you can use some free soundfont, for example using Qsynth on Linux. 
* lack of a damper pedal: every piano player, even the beginner one, knows that a damper pedal is necessary to play quite all songs. Well, my keyboard does not have it, nor any input for it.

The lock-down due to COVID-19 lead me to the only possible solution: DIY.

## Let's reverse-engineer it
I opened the keyboard, looking for some info about the chip, maybe other users online have the very same problem with a similar controller.

It was a surprise when I discovered that the chip driving the Usb connector was an AtTiny 2313. It may look familiar to you, because it is a microcontroller very similar to the one used on the Arduino series.

Being a low-end microcontroller, it was impossible to it to pilot the sound synthesis. A more deep inspection revealed that it was indeed connected to nothing but power/gnd, the usb ports, some components for the clock and another chip with only two traces. Two traces. Oscilloscope in hands, I discovered that one of the line presented some activity while the other was low all the time.

Activity? Really? The MIDI protocol is actually a two line protocol (+ GND).

Connecting these two line to a serial-to-usb converter (be aware of the TTL voltages, for me it was 5V but a logic voltage converter may be needed) confirmed that the messages exchanged between the two chips where actually MIDI messages!

## How the sketch works
Actually the sketch is very simple.
It creates a virtual SerialPort on pin 11 and 10, that are used respectively to send messages to and to receive messages from the keyboard. Every message received on the virtual SerialPort is sent on the phisical Arduino serial port. On the Uno, it is the one connected to PC via usb cable. Remember to connect also a GND from the keyboard to the Arduino GND. Be aware of ground loops!

Moreover, the firmware continuously listen to variation on pin 8. A button/pedal should be connected between pin 8 and GND. As it is configured as pullup, a value of HIGH means that it is disconnected from GND i.e. the circuit is open; if value is LOW then the circuit is closed and the pedal is pressed. When the button is pressed, the specfic command is sent to the PC.

As MIDI is a not so easy protocol, some logic in the firmware helps to not overlap the pedal command and the real MIDI commands. you can find further info in comments in the code (quite strange but I have actually commented it!).

If the damper is sent to the pc but does not work, maybe it is set to a wrong channel. On my kyboard, the keyboard note message are sent on channel 5 (0x04). You can set this by changing the `MIDI_CHANNEL` constant.

## But I want to play
Yes, I know what you are thinking. Now we have only a serial port on PC that spams incomprensible characters on the screen. How we can use this device to actually play some more music?

Well, [TTYMidi](https://github.com/cjbarnes18/ttymidi) is what you are looking for. It creates a virtual MIDI device that replicates every command received from the serial line. 

You can start it as simply as `./ttymidi -v -s /dev/ttyACM0 -b 115200`. Remember to change the serial device if needed.

You can now use `aconnect` to redirect the output of the virtual device to any application on your PC. 

The ArchLinux ~~bible~~ wiki contains [some very useful information](https://wiki.archlinux.org/index.php/USB_MIDI_keyboards) about the MIDI subsystem on Linux.

If you use Windows, well... Good luck! ;)
