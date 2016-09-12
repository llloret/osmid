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

#include <iostream>
#include "midiin.h"

using namespace std;

map<string, unsigned int> MidiIn::m_midiInputNameToJuceMidiId;
map<string, unsigned int> MidiIn::m_midiInputNameToStickyId;
vector<string> MidiIn::m_midiJuceMidiIdToName;
unsigned int MidiIn::m_nStickyIds = 0;


MidiIn::MidiIn(string portName, MidiInputCallback *midiInputCallback) {
    cout << "MidiIn contructor for " << portName << endl;
    updateMidiDevicesNamesMapping();
    m_portName = portName;
    if (!nameInStickyTable(m_portName))
        m_stickyId = addNameToStickyTable(m_portName);
    else
        m_stickyId = getStickyIdFromName(m_portName);

    m_juceMidiId = getJuceMidiIdFromName(m_portName);
	// FIXME: need to check if name does not exist
    m_midiIn = MidiInput::openDevice(m_juceMidiId, midiInputCallback);
    m_midiIn->start();
}

MidiIn::~MidiIn() {
    cout << "MidiIn destructor for " << m_portName << endl;
	m_midiIn->stop();
	delete m_midiIn;
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
    auto strArray = MidiInput::getDevices();
    int nPorts = strArray.size();
    if (m_juceMidiId >= nPorts)
        return false;

    string nameForId = strArray[m_juceMidiId].toStdString();
    if (nameForId != m_portName)
        return false;

    return true;
}

vector<string> MidiIn::getInputNames()
{    
    auto strArray = MidiInput::getDevices();
    int nPorts = strArray.size();
    vector<string> names(nPorts);

    for (unsigned int i = 0; i < nPorts; i++) {
        names[i] = strArray[i].toStdString();
    }
    return names;
}

void MidiIn::updateMidiDevicesNamesMapping()
{
    m_midiJuceMidiIdToName = MidiIn::getInputNames();
    for (int i = 0; i < m_midiJuceMidiIdToName.size(); i++) {
        m_midiInputNameToJuceMidiId[m_midiJuceMidiIdToName[i]] = i;
    }
}


int MidiIn::getJuceMidiIdFromName(string portName)
{
    return m_midiInputNameToJuceMidiId.at(portName);
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
