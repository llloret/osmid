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
#ifdef WIN32
#include <windows.h>
#endif

#include <stdexcept>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>
#include <boost/program_options.hpp>
#include "midiout.h"
#include "oscin.h"
#include "oscinprocessor.h"
#include "version.h"

using namespace std;

namespace po = boost::program_options;


void listAvailablePorts() {
    auto outputs = MidiOut::getOutputNames();
    cout << "Found " << outputs.size() << " MIDI outputs." << endl;
    for (unsigned int i = 0; i < outputs.size(); i++) {
        cout << "   (" << i << "): " << outputs[i] << endl;
    }
}


struct ProgramOptions
{
    vector<string> midiOutputNames;
    bool allMidiOutputs;
    unsigned int oscInputPort;
    unsigned int monitor;
};


void showVersion()
{
    cout << "o2m version " << O2M_VERSION << endl;
}

int setup_and_parse_program_options(int argc, char* argv[], ProgramOptions &programOptions)
{
    po::options_description desc("o2m Usage");

    desc.add_options()
        ("list,l", "List output MIDI devices")
        ("midiin,i", po::value<vector<string>>(&programOptions.midiOutputNames), "MIDI Output devices (default: all) - can be specified multiple times")
        ("oscport,o", po::value<unsigned int>(&programOptions.oscInputPort), "OSC Input port (default:57200)")
        ("monitor,m", po::value<unsigned int>(&programOptions.monitor)->default_value(0)->implicit_value(1), "Monitor OSC input and MIDI output")
        ("help,h", "Display this help message")
        ("version", "Show the version number");

    po::variables_map args;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), args);
        po::notify(args);
    }
    catch (const po::unknown_option& e) {
        cout << e.what() << endl;
        return -1;
    }

    if (args.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    if (args.count("list")) {
        listAvailablePorts();
    }

    if (args.count("version")) {
        showVersion();
    }

    if (!args.count("midiout")) {
        // by default add all input devices
        programOptions.midiOutputNames = MidiOut::getOutputNames();
        programOptions.allMidiOutputs = true;
    }
    else {
        programOptions.allMidiOutputs = false;
    }

    if (!args.count("oscout")) {
        programOptions.oscInputPort = 57200;
    }

    return 0;
}


static mutex g_oscinMutex;


void prepareOscProcessorOutputs(unique_ptr<OscInProcessor>& oscInputProcessor, const ProgramOptions& popts)
{
    // Should we open all devices, or just the ones passed as parameters?
    vector<string> midiOutputsToOpen = (popts.allMidiOutputs ? MidiOut::getOutputNames() : popts.midiOutputNames);
    {
        lock_guard<mutex> lock(g_oscinMutex);
        oscInputProcessor->prepareOutputs(midiOutputsToOpen);
    }
}


std::atomic<bool> g_wantToExit(false);

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



void asyncBreakThread(OscInProcessor *oscInputProcessor)
{
    while (!g_wantToExit) {
        std::chrono::milliseconds timespan(1000);
        std::this_thread::sleep_for(timespan);
        {
            lock_guard<mutex> lock(g_oscinMutex);
            oscInputProcessor->asyncBreak();
        }
    }
}

int main(int argc, char* argv[]) {

    ProgramOptions popts;

    int rc = setup_and_parse_program_options(argc, argv, popts);
    if (rc != 0) {
        return rc;
    }

    auto oscInputProcessor = make_unique<OscInProcessor>(popts.oscInputPort, popts.monitor);
    // Prepare the OSC input and MIDI outputs
    try {
        prepareOscProcessorOutputs(oscInputProcessor, popts);
    }
    catch (const std::out_of_range&) {
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

    std::thread thr(asyncBreakThread, oscInputProcessor.get());

    // For hotplugging
    vector<string> lastAvailablePorts = MidiOut::getOutputNames();
    while (!g_wantToExit) {
        oscInputProcessor->run(); // will run until asyncBreak is called from another thread
        vector<string> newAvailablePorts = MidiOut::getOutputNames();
        // Was something added or removed?
        if (newAvailablePorts != lastAvailablePorts) {
            prepareOscProcessorOutputs(oscInputProcessor, popts);
            lastAvailablePorts = newAvailablePorts;
            listAvailablePorts();
        }
    }
    thr.join();
}

