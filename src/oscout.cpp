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

using namespace std;
using boost::asio::ip::udp;

OscOutput::OscOutput(string dstOscHost, int dstOscPort, unsigned int monitor) : m_monitor(monitor)
{
#ifdef USE_UDP_OSCPACK
    m_transmitSocket = make_unique<UdpTransmitSocket>(IpEndpointName(dstOscHost, dstOscPort));
#else
    prepareUDPSocket(dstOscPort);
#endif
#ifdef USE_UDP_BOOST_ASYNC
    m_ioServiceThread = std::move(thread(std::bind(&MidiIn::ioServiceThread_func, this)));
#endif  
}


#ifndef USE_UDP_OSCPACK
void OscOutput::prepareUDPSocket(int port)
{
    udp::resolver resolver(m_ioService);
    udp::resolver::query query(udp::v4(), "localhost", to_string(port));
    m_receiverEndpoint = std::move(udp::endpoint(*resolver.resolve(query)));

    m_socket = make_unique<udp::socket>(m_ioService);
    m_socket->open(udp::v4());
}
#endif


void OscOutput::dumpMessage(const char *data, size_t size)
{
    
    cout << "INFO sent UDP message: ";
    for (int i = 0; i < size; i++){
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
#ifdef USE_UDP_OSCPACK
    m_transmitSocket->Send(data, size);
#elif defined(USE_UDP_BOOST_ASYNC)
    m_socket->async_send_to(boost::asio::buffer(data, size), m_receiverEndpoint,
        [](const boost::system::error_code& error, std::size_t bytes_transferred) {});
#else
    try {
        m_socket->send_to(boost::asio::buffer(data, size), m_receiverEndpoint);
        if (m_monitor >= 2)
            dumpMessage(data, size);
        
    }
    catch (const boost::system::system_error& e) {
        cout << "ERROR sending through udp socket: " << e.what() << endl;
    }
#endif
}

#ifdef USE_UDP_BOOST_ASYNC
void OscOutput::ioServiceThread_func()
{
    m_ioService.run();
}
#endif
