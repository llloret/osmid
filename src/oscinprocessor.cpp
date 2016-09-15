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
        cout << "Received OSC message with address pattern: " << addressPattern << endl;
    }
    regex addressRegex("/([[:alnum:]]+)/([[:alnum:]]+)(/([[:digit:]]+))?");
    smatch match;
        
    if (regex_match(addressPattern, match, addressRegex)) {
        cout << "Match (" << match[0] << ", " << match[1] << ", " << match[2] << ", " << match[3] << ", " << match[4] << endl;
        // We are interested in groups [1], [2] y [4]. [1] -> device, [2] -> command / raw, [4] -> channel

    }
    else {
        cout << "No match on address pattern" << endl;
    }

    // Check what the adress patter contains
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
    for (auto& output : m_outputs) {
        output->send(MidiMessage::noteOn(1, 32, static_cast<uint8_t>(64)));
    }
}


void OscInProcessor::ProcessBundle(const osc::ReceivedBundle& b, const IpEndpointName& remoteEndpoint)
{
    cout << "Received OSC bundle. Ignoring for now!" << endl;
}

