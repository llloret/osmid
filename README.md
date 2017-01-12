[![Build Status](https://travis-ci.org/llloret/osmid.svg?branch=master)](https://travis-ci.org/llloret/osmid)

# osmid

osmid aims to provide a lightweight, portable, easy to use tool to convert MIDI to OSC and OSC to MIDI.

I have started it as a proof of concept for the MIDI components for Sonic PI. I started playing a bit with MidiOSC, but after a while I decided to implement my own tools for more flexibility on the design and architecture.

The idea now if to see what features are required (or not), and use that to evolve the project. Of course, osmid is available for other projects and uses too, so any ideas and feedback are very welcome.

osmid is divided in 2 tools:
* m2o: MIDI to OSC conversion
* o2m: OSC to MIDI conversion

Having two separate tools follows Unix ideas of having a number of smaller standalone tools instead of bigger monolithic ones. Since some projects might want to use just one direction for the conversion, it makes sense to keep this separation.

## m2o features
* Portable: Works under Windows, Linux and Mac
* Compact
* Very low latency
* Customizable: can open a number of midi inputs, or all, and can configure the output UDP socket (if it is necessary to forward differnet MIDI devices to different OSC destinations, it is possible to do that invoking the program multiple times, with the appropriate arguments)
* OSC address templates, that is, the format of the OSC address can be passed as an argument to the program. If the template parameter is not passed, then templates are NOT used (for example, if you REALLY care about latency - but we are talking about tens of microseconds here...). 


## o2m features
* At the moment very basic OSC parsing implementation that will send RAW MIDI messages to all MIDI output devices
* Parses OSC messages with Address pattern /<whatever>/raw, with 3 integers or a blob with 3 bytes, that will be converted to MIDI and send as is.


## Building
For build instruction see INSTALL.md.

osmid is built with C++11/14. The build system is based on cmake. Target compiler in Windows is MSVC 2015 Win64, and in Linux is gcc 4.9 or later. Prepare using something like: `cmake -G "Visual Studio 14 2015 Win64" .. -DBOOSTROOT=d:\sdk\boost_1_61_0`

osmid uses the following libs:
* JUCE for the midi handling and OSC handling (included in the tree)
* oscpack, for the OSC handling and UDP networking (included in the tree)
* spdlog, for logging (included in the tree)
* boost, for program options parsing (external dependency)



## m2o parameters
* --midiin or -i <MIDI Input device>: open the specified input device - can be specified multiple times to open more than one device
* --oschost or -H <hostname or IP address>: send the OSC output to the specified host
* --oscport or -o <UDP port number>: send the OSC output to the specified port - can be specified multiple times to send to more than one port
* --osctemplate or -t <OSC template>: use the specified OSC output template (use $n: midi port name, $i midi port id, $c: midi channel, $m: message_type). For example: -t /midi/$c/$m
* --oscrawmidimessage or -r: send the raw MIDI data in the OSC message, instead of a decoded version
* --monitor or -m: dump information to standard output about the MIDI messages received and the OSC messages encoded and sent. Use -m 2 to see sent OSC over the wire
* --list: List input MIDI devices
* --heartbeat or -b: sends OSC heartbeat message
* --help: Display this help message
* --version: Show the version number

## m2o outgoing OSC message format
The address by default is: /midi/<port id>/<channel>(if the message contains channel information).
The address can be templated with the -t argument.

The message body is: 
* By default: (int)<port id>, (string)<port name>, <decoded message data>(i.e. for note_on messages, it will be 2 integers: note, velocity)
* if -r specified: (int)<port id>, (string)<port name>, (blob)<raw midi data>.


There is also an optional heartbeat message which sends periodic messages with the following format:
OSC address pattern: /midi/heartbeat. Message body is OSC array of pairs <midi device id>, <midi device name>



## o2m parameters (Need to update this)
- The default port to listen for OSC is 57200

## o2m incoming OSC message format
- The expected OSC address pattern is /(string)<out midi device name or id>/(string)<midi command>
- Recognized midi commands, and the expected OSC body:
	- raw: send a midi command as is. Body can be either a blob or a sequence of int32s
	- note_on: Body is (int32)channel, (int32)note, (int32)velocity
	- note_off: Body is (int32)channel, (int32)note, (int32)velocity
	- control_change: Body is (int32)channel, (int32)control number, (int32)control value
	- pitch_bend: Body is (int32)channel, (int32)value
	- channel_pressure: Body is (int32)channel, (int32)value
	- poly_pressure: Body is (int32)channel, (int32)note, (int32)value
	- program_change: Body is (int32)channel, (int32)program number
        - clock: Body is empty
        - start: Body is empty
        - stop: Body is empty
        - continue: Body is empty        
        - active_sense: Body is empty                

## TODO
* "all" devices should mean not only the devices that are found at the beginning of the run, but ALL the devices, even the ones that get plugged in later. (I think this is in now)
* Installers

## LICENSE
See LICENSE.md file for details.
