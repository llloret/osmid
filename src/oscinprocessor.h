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
#include <memory.h>
#include <vector>
#include "../JuceLibraryCode/JuceHeader.h"
#include "oscin.h"


class OscInProcessor : public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback> {
public:
	OscInProcessor(int oscListenPort, std::vector<std::shared_ptr<MidiOutput>> outputs, unsigned int monitor = 0);
    virtual void oscMessageReceived(const OSCMessage &message) override;
    virtual void oscBundleReceived(const OSCBundle &) override;

private:
    std::unique_ptr<OscIn> m_input;

    unsigned int m_monitor;
};
