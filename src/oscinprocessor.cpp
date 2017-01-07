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
#include "oscinprocessor.h"
#include "utils.h"


using namespace std;

OscInProcessor::OscInProcessor(int oscListenPort, unsigned int monitor) : m_monitor(monitor)
{
    m_input = make_unique<OscIn>(oscListenPort, this);
}


void OscInProcessor::prepareOutputs(const vector<string>& outputNames)
{
    m_outputs.clear();
    for (auto& outputName : outputNames) {
        auto midiOut = make_unique<MidiOut>(outputName);
        m_outputs.push_back(move(midiOut));
    }
}



void OscInProcessor::ProcessMessage(const osc::ReceivedMessage& message, const IpEndpointName& remoteEndpoint)
{
    string addressPattern(message.AddressPattern());
    if (m_monitor) {
        cout << timestamp() << "Received OSC message with address pattern: " << addressPattern << endl;
    }
    if (m_monitor > 1) {
        dumpOscBody(message);
    }

    regex addressRegex("/([[:alnum:]]+)/(([[:alnum:]]|_)+)");
    smatch match;

    if (regex_match(addressPattern, match, addressRegex)) {
        cout << "Match (" << match[0] << ", " << match[1] << ", " << match[2] << endl;
        // We are interested in groups [1] and [2]. [1] -> device, [2] -> command / raw
        const string& outDevice = match[1];
        const string& command = match[2];

        if (command == "clock") {
          processClockMessage(outDevice);
        }
        else if (command == "raw") {
            processRawMessage(outDevice, message);
        }
        else if (command == "note_on") {
            processNoteOnMessage(outDevice, message);
        }
        else if (command == "note_off") {
            processNoteOffMessage(outDevice, message);
        }
        else if (command == "control_change") {
            processControlChangeMessage(outDevice, message);
        }
        else if (command == "pitch_bend") {
            processPitchBendMessage(outDevice, message);
        }
        else if (command == "channel_pressure") {
            processChannelPressureMessage(outDevice, message);
        }
        else if (command == "poly_pressure") {
            processPolyPressureMessage(outDevice, message);
        }
        else if (command == "start") {
          processStartMessage(outDevice);
        }
        else if (command == "continue") {
          processContinueMessage(outDevice);
        }
        else if (command == "stop") {
          processStopMessage(outDevice);
        } else if (command == "active_sense") {
          processActiveSenseMessage(outDevice);
        }
        else if (command == "program_change") {
            processProgramChangeMessage(outDevice, message);
        }
        else {
            cout << "Unknown command on OSC message. Ignoring";
        }
    }
    else {
        cout << "No match on address pattern" << endl;
    }

    // Check what the adress pattern contains

/*
    cout << "Got " << message.size() << "arguments" << endl;
    for (int i = 0; i < message.size(); i++){
        if (message[i].isFloat32())
            cout << "F: " << message[i].getFloat32() << endl;
        else if (message[i].isInt32())
            cout << "I: " << message[i].getInt32() << endl;
        else if (message[i].isString())
            cout << "S: " << message[i].getString() << endl;
        else if (message[i].isBlob())
            cout << "B: this is a blob" << endl;
    }
  */  
}


void OscInProcessor::dumpOscBody(const osc::ReceivedMessage& message)
{
    cout << "Got " << message.ArgumentCount() << " arguments" << endl;
    auto arg = message.ArgumentsBegin();
    while(arg != message.ArgumentsEnd()){
        if (arg->IsFloat())
            cout << "F: " << arg->AsFloat() << endl;
        else if (arg->IsInt32())
            cout << "I: " << arg->AsInt32() << endl;
        else if (arg->IsString())
            cout << "S: " << arg->AsString() << endl;
        else if (arg->IsBlob())
            cout << "B: this is a blob" << endl;
        else
            cout << "X: not sure what this field is" << endl;
        
        arg++;
    }
}



// FIXME: For now send to all outputs. Later send only to the appropriate outputs
void OscInProcessor::processRawMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto arg = message.ArgumentsBegin();
    if (arg->IsBlob()) {
        const void *blobData;
        int blobSize;
        arg->AsBlob(blobData, blobSize);
        MidiMessage raw(blobData, blobSize);
        for (auto& output : m_outputs) {
            output->send(raw);
        }
    }
    else {
        unsigned char midiMessage[1024];
        int midiMessageSize = 0;

        while (arg != message.ArgumentsEnd()) {
            if (arg->IsInt32()) {
                midiMessage[midiMessageSize++] = arg->AsInt32();
            }
            arg++;
        }
        MidiMessage raw(midiMessage, midiMessageSize);
        for (auto& output : m_outputs) {
            output->send(raw);
        }
    }
}


// note_on OSC messages have this layout: channel (int32), note (int32), velocity (int32)
void OscInProcessor::processNoteOnMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();  
    int channel;
    int note;
    int velocity;

    try {
        args >> channel >> note >> velocity >> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC note_on message: Error parsing args. Expected int32, int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::noteOn(channel, note, (uint8)velocity) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}


void OscInProcessor::processClockMessage(const string& outDevice)
{
  MidiMessage midiMessage{ MidiMessage::midiClock() };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

void OscInProcessor::processStartMessage(const string& outDevice)
{
  MidiMessage midiMessage{ MidiMessage::midiStart() };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

void OscInProcessor::processContinueMessage(const string& outDevice)
{
  MidiMessage midiMessage{ MidiMessage::midiContinue() };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

void OscInProcessor::processStopMessage(const string& outDevice)
{
  MidiMessage midiMessage{ MidiMessage::midiStop() };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

void OscInProcessor::processActiveSenseMessage(const string& outDevice)
{
  MidiMessage midiMessage{ MidiMessage() };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}


// note_off OSC messages have this layout: channel (int32), note (int32), velocity (int32)
void OscInProcessor::processNoteOffMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();
    int channel;
    int note;
    int velocity;

    try {
        args >> channel >> note >> velocity >> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC note_off message: Error parsing args. Expected int32, int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::noteOff(channel, note, (uint8)velocity) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

// control_change OSC messages have this layout: channel (int32), note (int32), velocity (int32)
void OscInProcessor::processControlChangeMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();
    int channel;
    int number;
    int value;

    try {
        args >> channel >> number >> value>> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC control_change message: Error parsing args. Expected int32, int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::controllerEvent(channel, number, value) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}


// pitch_bend OSC messages have this layout: channel (int32), value (int32). Note that the midi resolution for pitch_bend value is 14 bits
void OscInProcessor::processPitchBendMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();
    int channel;
    int value;

    try {
        args >> channel >> value >> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC pitch_bend message: Error parsing args. Expected int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::pitchWheel(channel, value) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

// channel_pressure OSC messages have this layout: channel (int32), value (int32).
void OscInProcessor::processChannelPressureMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();
    int channel;
    int value;

    try {
        args >> channel >> value >> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC channel_pressure message: Error parsing args. Expected int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::channelPressureChange(channel, value) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}


// poly_pressure OSC messages have this layout: channel (int32), note (int32), velocity (int32)
void OscInProcessor::processPolyPressureMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();
    int channel;
    int note;
    int value;

    try {
        args >> channel >> note >> value >> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC poly_pressure message: Error parsing args. Expected int32, int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::aftertouchChange(channel, note, value) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}

// program_change OSC messages have this layout: channel (int32), program (int32)
void OscInProcessor::processProgramChangeMessage(const string& outDevice, const osc::ReceivedMessage& message)
{
    auto args = message.ArgumentStream();
    int channel;
    int program;

    try {
        args >> channel >> program >> osc::EndMessage;
    }
    catch (osc::Exception& e) {
        cout << "OSC change_program message: Error parsing args. Expected int32, int32. Ignoring message: " << e.what() << endl;
        return;
    }

    MidiMessage midiMessage{ MidiMessage::programChange(channel, program) };
    for (auto& output : m_outputs) {
        output->send(midiMessage);
    }
}


// For example, gets a string of the form "iifsb", and checks that it got 4 arguments (int32, int32, float, string, blob)
// warningText is printed when something wrong is found, so that the user knows what triggered it
// FIXME: perhaps we do not need this, and we can rely on oscpack message parsing as stream
#if 0
bool OscInProcessor::validateMessage(const string& warningPre, const string& validationString, const osc::ReceivedMessage& message)
{
    int nArgs = validationString.size();
    if (message.ArgumentCount() != nArgs) {
        cout << warningPre << "Expected " << message.ArgumentCount() << " OSC arguments and got " << nArgs << ". IGNORING MESSAGE" << endl;
        return false;
    }

    auto arg = message.ArgumentsBegin();
    for (int i = 0; i < nArgs; i++) {
        switch (validationString[i]) {
            case 'i':
                if (!arg->IsInt32()) {
                    cout << warningPre << "Expected int32 for OSC argument " << i << " and got somethng else. IGNORING MESSAGE" << endl;
                    return false;
                }
                break;
            case 'f':
                if (!arg->IsFloat()) {
                    cout << warningPre << "Expected float for OSC argument " << i << " and got somethng else. IGNORING MESSAGE" << endl;
                    return false;
                }
                break;
            case 's':
                if (!arg->IsString()) {
                    cout << warningPre << "Expected string for OSC argument " << i << " and got somethng else. IGNORING MESSAGE" << endl;
                    return false;
                }
                break;
            case 'b':
                if (!arg->IsBlob()) {
                    cout << warningPre << "Expected blob for OSC argument " << i << " and got somethng else. IGNORING MESSAGE" << endl;
                    return false;
                }
                break;
            default:
                assert("Unknown validation character");
                break;
        }
    }


    return true;
}

#endif

void OscInProcessor::ProcessBundle(const osc::ReceivedBundle& b, const IpEndpointName& remoteEndpoint)
{
    cout << "Received OSC bundle. Ignoring for now!" << endl;
}

int OscInProcessor::getNMidiOuts()
{
    return m_outputs.size();
}

int OscInProcessor::getMidiOutId(int n)
{
    return m_outputs[n]->getPortId();
}


string OscInProcessor::getMidiOutName(int n)
{
    return m_outputs[n]->getPortName();
}
