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

#include "oscinprocessor.h"


using namespace std;

OscInProcessor::OscInProcessor(int oscListenPort, const vector<string>& outputNames, unsigned int monitor) :
    m_monitor(monitor)
{
    m_input = make_unique<OscIn>(oscListenPort, this);
    for (auto& outputName : outputNames){
        auto midiOut = make_unique<MidiOut>(outputName);
        m_outputs.push_back(move(midiOut));
    }
}


void OscInProcessor::oscMessageReceived(const OSCMessage &message)
{
    cout << "Received OSC message!" << endl;
    cout << "Address pattern: " << message.getAddressPattern().toString() << endl;
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
    
    for (auto& output : m_outputs) {
        output->send(MidiMessage::noteOn(1, 32, static_cast<uint8_t>(64)));
    }
}


void OscInProcessor::oscBundleReceived(const OSCBundle &bundle)
{
    cout << "Received OSC bundle. Ignoring for now!" << endl;
}

