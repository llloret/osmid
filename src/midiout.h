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

#include <vector>
#include <map>
#include <string>
#include "midicommon.h"
#include "../JuceLibraryCode/JuceHeader.h"

// This class manages a MIDI output device as seen by JUCE
class MidiOut : public MidiCommon 
{
    public:
        MidiOut(std::string portName);
        MidiOut(const MidiOut&) = delete;
        MidiOut& operator=(const MidiOut&) = delete;

        ~MidiOut();
        
        void send(const MidiMessage &message);

        static std::vector<std::string> getOutputNames();

    protected:
        void updateMidiDevicesNamesMapping() override;

    private:
        MidiOutput *m_midiOut;
    };
