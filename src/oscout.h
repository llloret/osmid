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
#include <string>
#include <memory>
#ifdef USE_UDP_BOOST_ASYNC
    #include <thread>
#endif


#ifdef USE_UDP_OSCPACK
	#include "ip/UdpSocket.h"
#else
	#include <boost/asio.hpp>
#endif



class OscOutput {
public:
	OscOutput(std::string dstOscHost, int dstOscPort, bool monitor = false);
	void sendUDP(const char *data, std::size_t size);

private:
    void dumpMessage(const char *data, size_t size);
#ifdef USE_UDP_OSCPACK
    std::unique_ptr<UdpTransmitSocket> m_transmitSocket;
#else
    boost::asio::io_service m_ioService;
    std::unique_ptr<boost::asio::ip::udp::socket> m_socket;
    boost::asio::ip::udp::endpoint m_receiverEndpoint;
    void prepareUDPSocket(int port);
#endif
#ifdef USE_UDP_BOOST_ASYNC
    std::thread m_ioServiceThread;
    void ioServiceThread_func();
#endif
    bool m_monitor;

};