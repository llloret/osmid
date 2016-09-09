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


#pragma once

#include <thread>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "../JuceLibraryCode/JuceHeader.h"

// This class manages a MIDI input device as seen by RtMidi
class MidiIn {
public:
    MidiIn(std::string portName);
    MidiIn(const MidiIn&) = delete;
    MidiIn& operator=(const MidiIn&) = delete;
    
    ~MidiIn();

    bool checkValid() const;

    void setCallback(RtMidiIn::RtMidiCallback callback, void *userData);
    std::string getPortName() const;
    int getPortId() const;

    static std::vector<std::string> getInputNames();
    static int getRtmidiIdFromName(std::string portName);
    static void updateMidiDevicesNamesMapping();

private:
    static bool nameInStickyTable(std::string portName);
    unsigned int addNameToStickyTable(std::string portName);
    unsigned int getStickyIdFromName(std::string portName);
    
    MidiInput m_midiIn;
    std::string m_portName;
    unsigned int m_rtmidiId;
    unsigned int m_stickyId;

    static std::map<std::string, unsigned int> m_midiInputNameToRtmidiId;
    static std::vector<std::string> m_midiRtmidiIdToName;
    static std::map<std::string, unsigned int> m_midiInputNameToStickyId;
    static unsigned int m_nStickyIds;
};
