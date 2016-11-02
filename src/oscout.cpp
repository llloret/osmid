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
#include <iomanip>
#include <iostream>
#include "oscout.h"
#include "utils.h"

using namespace std;

OscOutput::OscOutput(string dstOscHost, int dstOscPort, unsigned int monitor) : m_monitor(monitor)
{
    m_transmitSocket = make_unique<UdpTransmitSocket>(IpEndpointName(dstOscHost.c_str(), dstOscPort));
}


void OscOutput::dumpMessage(const char *data, size_t size)
{

    cout << timestamp() << "DEBUG sent UDP message: ";
    for (int i = 0; i < size; i++) {
        const unsigned char udata = (unsigned char)(data[i]);
        // is it printable?
        if (udata >= 32 && udata <= 127)
            cout << udata;
        else
            cout << hex << "[" << (unsigned int)udata << /*setw(2) << setfill('0') <<*/ "]" << dec;
    }
    cout << endl;
}


void OscOutput::sendUDP(const char *data, size_t size)
{
    // it is not thread safe to share udp objects...
    lock_guard<mutex> lock(m_sendMutex);
    m_transmitSocket->Send(data, size);
    if (m_monitor > 2)
        dumpMessage(data, size);
}
