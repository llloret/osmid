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

#include <stdexcept>
#include <boost/program_options.hpp>
#include <iostream>
#include "midiin.h"
#include "oscout.h"
#include "midiinprocessor.h"
#include "osc/OscOutboundPacketStream.h"
#include "version.h"
#include "utils.h"

using namespace std;

namespace po = boost::program_options;

void listAvailablePorts()
{
    auto inputs = MidiIn::getInputNames();
    cout << "Found " << inputs.size() << " MIDI inputs." << endl;
    for (unsigned int i = 0; i < inputs.size(); i++) {
        cout << "   (" << i << "): " << inputs[i] << endl;
    }
}

struct ProgramOptions {
    vector<string> midiInputNames;
    bool allMidiInputs;
    string oscOutputHost;
    vector<int> oscOutputPorts;
    bool useOscTemplate;
    string oscTemplate;
    bool oscRawMidiMessage;
    bool oscHeartbeat;
    bool virtualPort;
    unsigned int monitor;
    bool listPorts;
};

void showVersion()
{
    cout << "m2o version " << M2O_VERSION << endl;
}

int setup_and_parse_program_options(int argc, char* argv[], ProgramOptions& programOptions)
{
    po::options_description desc("m2o Usage");

    desc.add_options()
    ("list,l", po::bool_switch(&programOptions.listPorts)->default_value(false), "List input MIDI devices")
    ("virtualport,v", po::bool_switch(&programOptions.virtualPort)->default_value(false), "Create a Virtual MIDI output port that will be monitored for MIDI (useful to have MIDI->OSC inside your favourite DAW)")
    ("midiin,i", po::value<vector<string> >(&programOptions.midiInputNames), "MIDI Input device (default: all) - can be specified multiple times")
    ("oschost,H", po::value<string>(&programOptions.oscOutputHost)->default_value("127.0.0.1"), "OSC Output host (default:127.0.0.1)")
    ("oscport,o", po::value<vector<int> >(&programOptions.oscOutputPorts), "OSC Output port (default:57120) - can be specified multiple times")
    ("osctemplate,t", po::value<string>(&programOptions.oscTemplate), "OSC output template (use $n: midi port name, $i: midi port id, $c: midi channel, $m: message_type")
    ("oscrawmidimessage,r", po::bool_switch(&programOptions.oscRawMidiMessage)->default_value(false), "OSC send the raw MIDI data as part of the OSC message")
    ("heartbeat,b", po::bool_switch(&programOptions.oscHeartbeat)->default_value(false), "OSC send the heartbeat with info about the active MIDI devices")
    ("monitor,m", po::value<unsigned int>(&programOptions.monitor)->default_value(2)->implicit_value(1), "Monitor and logging level (lower more verbose)")
    ("help,h", "Display this help message")
    ("version", "Show the version number");

    po::variables_map args;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), args);
        po::notify(args);
    } catch (const po::unknown_option& e) {
        cout << e.what() << endl;
        return -1;
    }

    if (args.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    if (args.count("version")) {
        showVersion();
    }

    if (args.count("osctemplate"))
        programOptions.useOscTemplate = true;
    else
        programOptions.useOscTemplate = false;

    if (!args.count("midiin")) {
        // by default add all input devices
        programOptions.midiInputNames = MidiIn::getInputNames();
        programOptions.allMidiInputs = true;
    } else {
        programOptions.allMidiInputs = false;
    }

    if (!args.count("oscport")) {
        programOptions.oscOutputPorts.push_back(57120);
    }

    return 0;
}

void prepareMidiProcessors(vector<unique_ptr<MidiInProcessor> >& midiInputProcessors, const ProgramOptions& popts, vector<shared_ptr<OscOutput> >& oscOutputs)
{
    // Should we open all devices, or just the ones passed as parameters?
    vector<string> midiInputsToOpen = (popts.allMidiInputs ? MidiIn::getInputNames() : popts.midiInputNames);

    for (auto& input : midiInputsToOpen) {
        cout << "Opening input: " << input << endl;
        try {
            auto midiInputProcessor = local_utils::make_unique<MidiInProcessor>(input, oscOutputs, false);
            if (popts.useOscTemplate)
                midiInputProcessor->setOscTemplate(popts.oscTemplate);
            midiInputProcessor->setOscRawMidiMessage(popts.oscRawMidiMessage);
            midiInputProcessors.push_back(std::move(midiInputProcessor));
        } catch (const std::out_of_range&) {
            cout << "The device " << input << " does not exist";
            throw;
        }
    }
}

static std::atomic<bool> g_wantToExit(false);

#if WIN32
BOOL ctrlHandler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT) {
        g_wantToExit = true;
    }
    return TRUE;
}
#else
void ctrlHandler(int signal)
{
    cout << "Ctrl-C event" << endl;
    g_wantToExit = true;
}
#endif

void sendHeartBeat(const vector<unique_ptr<MidiInProcessor> >& midiProcessors, const vector<shared_ptr<OscOutput> >& oscOutputs)
{
    char buffer[2048];
    osc::OutboundPacketStream p(buffer, 2048);
    p << osc::BeginMessage("/midi/heartbeat");
    for (const auto& midiProcessor : midiProcessors) {
        p << midiProcessor->getInputId() << midiProcessor->getInputPortname().c_str();
    }
    p << osc::EndMessage;
    MonitorLogger::getInstance().debug("sending OSC: [/o2m/heartbeat] -> ");
    for (const auto& midiProcessor : midiProcessors) {
        MonitorLogger::getInstance().debug("   {}, {}", midiProcessor->getInputId(), midiProcessor->getInputPortname());
    }

    for (auto& output : oscOutputs) {
        output->sendUDP(p.Data(), p.Size());
        local_utils::logOSCMessage(p.Data(), p.Size());
    }
}

int main(int argc, char* argv[])
{
    // midiInputProcessors will contain the list of active MidiIns at a given time
    vector<unique_ptr<MidiInProcessor> > midiInputProcessors;
    // oscOutputs will contain the list of active OSC output ports
    vector<shared_ptr<OscOutput> > oscOutputs;
    ProgramOptions popts;

    int rc = setup_and_parse_program_options(argc, argv, popts);
    if (rc != 0) {
        return rc;
    }

    if (popts.listPorts) {
        listAvailablePorts();
        return 0;
    }

    MonitorLogger::getInstance().setLogLevel(popts.monitor);

    // Open the OSC output ports
    for (auto port : popts.oscOutputPorts) {
        auto oscOutput = make_shared<OscOutput>(popts.oscOutputHost, port);
        oscOutputs.push_back(std::move(oscOutput));
    }

    // Will configure logging on the first OSC port (may want to change this in the future, so that it sends to every port, or be able to select one)
    MonitorLogger::getInstance().setOscOutput(oscOutputs[0]);

// Create the virtual output port?
#ifndef WIN32
    unique_ptr<MidiInProcessor> virtualIn;
    if (popts.virtualPort) {
        virtualIn = local_utils::make_unique<MidiInProcessor>("TO Virtual osmid", oscOutputs, true);
    }
#endif

    // Open the MIDI input ports
    try {
        prepareMidiProcessors(midiInputProcessors, popts, oscOutputs);
    } catch (const std::out_of_range&) {
        return -1;
    }

    // Exit nicely with CTRL-C
#if WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlHandler, TRUE);
#else
    struct sigaction intHandler;

    intHandler.sa_handler = ctrlHandler;
    sigemptyset(&intHandler.sa_mask);
    intHandler.sa_flags = 0;
    sigaction(SIGINT, &intHandler, NULL);
#endif


    // For hotplugging
    vector<string> lastAvailablePorts = MidiIn::getInputNames();
    while (!g_wantToExit) {
        std::chrono::milliseconds timespan(1000);
        std::this_thread::sleep_for(timespan);
        vector<string> newAvailablePorts = MidiIn::getInputNames();
        // Was something added or removed?
        if (newAvailablePorts != lastAvailablePorts) {
            midiInputProcessors.clear();
            prepareMidiProcessors(midiInputProcessors, popts, oscOutputs);
            lastAvailablePorts = newAvailablePorts;
            listAvailablePorts();
        }
        if (popts.oscHeartbeat)
            sendHeartBeat(midiInputProcessors, oscOutputs);
    }
}
