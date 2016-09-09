// MIT License

// Copyright (c) 2016 Luis Lloret

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <regex>
#include <algorithm>
#include <sstream>
#include <cassert>
#include "midiinprocessor.h"
#include "utils.h"

using namespace std;

regex MidiInProcessor::regexName{ "\\$n" };
regex MidiInProcessor::regexId{ "\\$i" };
regex MidiInProcessor::regexChannel{ "\\$c" };
regex MidiInProcessor::regexMessageType{ "\\$m" };
regex MidiInProcessor::regexDoubleSlash{ "//" };


MidiInProcessor::MidiInProcessor(unique_ptr<MidiIn>&& input, vector<shared_ptr<OscOutput>> outputs, unsigned int monitor): 
    m_input(move(input)), m_outputs(outputs), m_useOscTemplate(false), m_monitor(monitor)
{
	m_input->setCallback(&MidiInProcessor::onMidi, this);
}


void MidiInProcessor::onMidi(double deltatime, std::vector<unsigned char> *message, void *userData) {    
    unsigned char channel = 0xff, status = 0;
    string message_type;
    size_t nBytes = message->size();

    assert(nBytes > 0);

    if((message->at(0) & 0xf0) != 0xf0) {
        channel = message->at(0) & 0x0f;
        status = message->at(0) & 0xf0;
    } else {
        status = message->at(0);
    }

    // Get the object instance
    MidiInProcessor *midiInputProcessor = static_cast<MidiInProcessor *>(userData);
    if (midiInputProcessor->m_monitor) {
        midiInputProcessor->dumpMIDIMessage(message);
    }


    // Process the message
    switch(status) {
        case 0x80:
            message_type = "note_off";
            assert(nBytes == 3);
            break;

        case 0x90:
            message_type = "note_on";
            assert(nBytes == 3);
            break;

        case 0xA0:
            message_type = "polyphonic_key_pressure";
            assert(nBytes == 3);
            break;

        case 0xB0:
            message_type = "control_change";
            assert(nBytes == 3);
            break;

        case 0xC0:
            message_type = "program_change";
            assert(nBytes == 2);
            break;

        case 0xD0:
            message_type = "channel_pressure";
            assert(nBytes == 2);
            break;

        case 0xE0:
            message_type = "pitch_bend_change";
            assert(nBytes == 3);
            break;

        case 0xF0:
            message_type = "sysex";
            // Remove the end of message marker if raw message is not specified
            if (!midiInputProcessor->m_oscRawMidiMessage)
                nBytes--;
            break;

        case 0xF1:
            message_type = "syscommon_MTC";
            assert(nBytes == 2);
            break;

        case 0xF2:
            message_type = "syscommon_song_position";
            assert(nBytes == 3);
            break;

        case 0xF3:
            message_type = "syscommon_song_select";
            assert(nBytes == 2);
            break;

        case 0xF4:
        case 0xF5:
            message_type = "syscommon_undefined";
            assert(nBytes == 1);
            break;

        case 0xF6:
            message_type = "syscommon_tune_request";
            assert(nBytes == 1);
            break;

        case 0xF8:
            message_type = "sysrt_timing_tick";
            assert(nBytes == 1);
            break;

        case 0xF9:
        case 0xFD: 
            message_type = "sysrt_undefined";
            assert(nBytes == 1);
            break;

        case 0xFA:
            message_type = "sysrt_start_song";
            assert(nBytes == 1);
            break;

        case 0xFB:
            message_type = "sysrt_continue_song";
            assert(nBytes == 1);
            break;

        case 0xFC:
            message_type = "sysrt_stop_song";
            assert(nBytes == 1);
            break;

        case 0xFE:
        	message_type = "sysrt_active_sensing";
            assert(nBytes == 1);
        	break;

        default:
            message_type = "unknown_message";
            break;
    }

    // Prepare the OSC address
    //auto start_time = chrono::high_resolution_clock::now();
    
    stringstream path;
    string portNameWithoutSpaces(midiInputProcessor->m_input->getPortName());
    int portId = midiInputProcessor->m_input->getPortId();
    replace_chars(portNameWithoutSpaces, ' ', '_');

    // Was a template specified?
    if (midiInputProcessor->m_useOscTemplate){
        string templateSubst(midiInputProcessor->m_oscTemplate);
        midiInputProcessor->doTemplateSubst(templateSubst, portNameWithoutSpaces, portId, channel, message_type);
        path << templateSubst;
    }
    else{
        path << "/midi/" << portId;
        if(channel != 0xff) {
            path << "/" << (int)channel;
        }
        path << "/" << message_type;
    }
    //auto end_time = chrono::high_resolution_clock::now();
    //auto e = chrono::duration <double, micro>(end_time - start_time).count();
    //cout << "Prepare string: " << e << endl << flush;

    // And now prepare the OSC message body
    //p << osc::BeginMessage(path.str().c_str());
    OSCMessage msg(path.str().c_str());

    // send device id and name as part of the message
    //p << static_cast<int>(portId) << portNameWithoutSpaces.c_str();
    msg.addInt32(static_cast<int>(portId));
    msg.addString(portNameWithoutSpaces);
        
    // send the raw midi message as part of the body
    // do we want a raw midi message?
    if (midiInputProcessor->m_oscRawMidiMessage) {
        if (!message->empty()) {
            //p << osc::Blob(&((*message)[0]), static_cast<osc::osc_bundle_element_size_t>(message->size()));
            MemoryBlock mb(&((*message)[0]), message->size());
            msg.addBlob(mb);
        }
    }
    else {
        for (int i = 1; i < nBytes; i++) {
            //p << (int)message->at(i);
            msg.addInt32((int)message->at(i));
        }
    }
    //p << osc::EndMessage;

    // Dump the OSC message
    if (midiInputProcessor->m_monitor) {
        cout << "INFO prepared OSC: [" << path.str() << "]" << " -> " << portId << ", " << portNameWithoutSpaces;
        if (midiInputProcessor->m_oscRawMidiMessage) {
            if (!message->empty()) {
                cout << ", <raw_midi_message>" << endl;
            }
        }
        else {
            for (int i = 1; i < nBytes; i++) {
                cout << ", " << (int)message->at(i);
            }
            cout << endl;
        }
    }


    //start_time = chrono::high_resolution_clock::now();

    // And send the message to the specified output ports
    for (auto& output : midiInputProcessor->m_outputs) {
        output->sendUDP(msg);
    }

    //end_time = chrono::high_resolution_clock::now();
    //e = chrono::duration <double, micro>(end_time - start_time).count();
    //cout << "SEND: " << e << endl << flush;
    
}

void MidiInProcessor::setOscTemplate(const std::string& oscTemplate)
{ 
    m_oscTemplate = oscTemplate; 
    m_useOscTemplate = true; 
};

void MidiInProcessor::setOscRawMidiMessage(bool oscRawMidiMessage)
{
    m_oscRawMidiMessage = oscRawMidiMessage;
}

void MidiInProcessor::doTemplateSubst(string &str, const string& portName, int portId, int channel, const string& message_type) const
{
    str = regex_replace(regex_replace(regex_replace(regex_replace(str,
        regexMessageType, message_type),
        regexChannel, (channel != 0xff ? to_string(channel) : "")),
        regexId, to_string(portId)),
        regexName, portName);

    // And now remove potential double slashes when the message does not have a channel, and remove potential slash at the end
    str = regex_replace(str, regexDoubleSlash, "/");
    if (str.back() == '/') {
        str.pop_back();
    }
}

void MidiInProcessor::dumpMIDIMessage(vector<unsigned char> *message) const
{
    cout << "INFO received MIDI message: ";
    for (int i = 0; i < message->size(); i++) {
        cout << hex << "[" << (unsigned int)message->at(i) << /*setw(2) << setfill('0') <<*/ "]" << dec;
    }
    cout << endl;

}