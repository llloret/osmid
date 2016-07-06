[![Build Status](https://travis-ci.org/llloret/osmid.svg?branch=master)](https://travis-ci.org/llloret/osmid)

# osmid

osmid aims to provide a lightweight, portable, easy to use tool to convert MIDI to OSC and OSC to MIDI.

I have started it as a proof of concept for the MIDI components for Sonic PI. I started playing a bit with MidiOSC, but after a while I decided to implement my own tools for more flexibility on the design and architecture.

The idea now if to see what features are required (or not), and use that to evolve the project. Of course, osmid is available for other projects and uses too, so any ideas and feedback are very welcome.

osmid is divided in 2 tools:
* m2o: MIDI to OSC conversion
* o2m: OSC to MIDI conversion (not implemented yet)

Having two separate tools follows Unix ideas of having a number of smaller standalone tools instead of bigger monolithic ones. Since some projects might want to use just one direction for the conversion, it makes sense to keep this separation.

## m2o features
* Portable: Works under Windows and Linux (not tested on Mac yet, but the idea is to have it working there, too)
* Compact
* Very low latency
* Customizable: can open a number of midi inputs, or all, and can configure the output UDP socket (if it is necessary to forward differnet MIDI devices to different OSC destinations, it is possible to do that invoking the program multiple times, with the appropriate arguments)
* OSC address templates, that is, the format of the OSC address can be passed as an argument to the program. If the template parameter is not passed, then templates are NOT used (for example, if you REALLY care about latency - but we are talking about tens of microseconds here...). 


o2m, the OSC to MIDI conversion is not implemented yet.

## Building
For build instruction see INSTALL.md.

osmid is built with C++11/14. The build system is based on cmake. Target compiler in Windows is MSVC 2015 Win64, and in Linux is gcc 4.9 or later.

osmid uses the following libs:
* RtMidi, for the midi handling (included in the tree)
* oscpack, for the OSC handling (and optionally UDP networking) (included in the tree)
* boost, for program options parsing and udp networking (external dependency) 


## m2o parameters
* --midiin or -i <MIDI Input device>: open the specified input device - can be specified multiple times to open more than one device
* --oschost or -H <hostname or IP address>: send the OSC output to the specified host
* --oscport or -o <UDP port number>: send the OSC output to the specified port - can be specified multiple times to send to more than one port
* --osctemplate or -t <OSC template>: use the specified OSC output template (use $n: midi port name, $i midi port id, $c: midi channel, $m: message_type). For example: -t /midi/$c/$m
* --monitor or -m: dump information to standard output about the MIDI messages received and the OSC messages sent
* --list: List input MIDI devices
* --help: Display this help message
* --version: Show the version number

## m2o OSC message format
The address by default is: /midi/<port id>/<channel>(if the message contains channel information).

The message body is: (int)<port id>, (string)<port name>, (blob)<raw midi data>.

The address can be templated with the -t argument.

## TODO
* Hot-plugging / unplugging.
* "all" devices should mean not only the devices that are found at the beginning of the run, but ALL the devices, even the ones that get plugged in later.
* Installers
* TCP as well as UDP for packet transport? (not sure if there is any interest on this)

## LICENSE
See LICENSE.md file for details.
