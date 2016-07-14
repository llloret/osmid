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

#include "midiin.h"

using namespace std;

map<string, unsigned int> MidiIn::m_midiInputNameToRtmidiId;
map<string, unsigned int> MidiIn::m_midiInputNameToStickyId;
vector<string> MidiIn::m_midiRtmidiIdToName;
unsigned int MidiIn::m_nStickyIds = 0;


MidiIn::MidiIn(string portName) {
    cout << "MidiIn contructor for " << portName << endl;
    updateMidiDevicesNamesMapping();
    m_portName = portName;
    if (!nameInStickyTable(m_portName))
        m_stickyId = addNameToStickyTable(m_portName);
    else
        m_stickyId = getStickyIdFromName(m_portName);

    m_rtmidiId = getRtmidiIdFromName(m_portName);
    m_midiIn.openPort(m_rtmidiId);
    m_midiIn.ignoreTypes(false, false, false);
}

MidiIn::~MidiIn() {
    cout << "MidiIn destructor for " << m_portName << endl;
}


void MidiIn::setCallback(RtMidiIn::RtMidiCallback callback, void *userData)
{
    m_midiIn.setCallback(callback, userData); 
}

string MidiIn::getPortName() const
{
    return m_portName;
}

int MidiIn::getPortId() const
{
    return m_stickyId;
}

// Checks if the name matches the id. They may stop matching because of adding or removing MIDI devices while running
// This should be called after we detect a change in the list of MIDI devices, for finer control of which MidiIns to keep
bool MidiIn::checkValid() const
{
    RtMidiIn midiIn;
    unsigned int nPorts = midiIn.getPortCount();
    if (m_rtmidiId >= nPorts)
        return false;

    string nameForId = midiIn.getPortName(m_rtmidiId);
    if (nameForId != m_portName)
        return false;

    return true;
}

vector<string> MidiIn::getInputNames()
{
    RtMidiIn midiIn;
    unsigned int nPorts = midiIn.getPortCount();
    vector<string> names(nPorts);
    for (unsigned int i = 0; i < nPorts; i++) {
        names[i] = midiIn.getPortName(i);
    }
    return names;
}

void MidiIn::updateMidiDevicesNamesMapping()
{
    m_midiRtmidiIdToName = MidiIn::getInputNames();
    for (int i = 0; i < m_midiRtmidiIdToName.size(); i++) {
        m_midiInputNameToRtmidiId[m_midiRtmidiIdToName[i]] = i;
    }
}


int MidiIn::getRtmidiIdFromName(string portName)
{
    return m_midiInputNameToRtmidiId.at(portName);
}

bool MidiIn::nameInStickyTable(string portName)
{
    auto search = m_midiInputNameToStickyId.find(portName);
    return (search != m_midiInputNameToStickyId.end());
}

unsigned int MidiIn::addNameToStickyTable(string portName)
{
    m_midiInputNameToStickyId[portName] = m_nStickyIds;
    return m_nStickyIds++;
}

unsigned int MidiIn::getStickyIdFromName(string portName)
{    
    return m_midiInputNameToStickyId[portName];
}
