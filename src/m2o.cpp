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

#include "midiin.h"
#include "oscout.h"
#include "midiinprocessor.h"
#include "version.h"

using namespace std;

namespace po = boost::program_options;


void listAvailablePorts() {
    auto inputs = MidiIn::getInputNames();
    cout << "Found " << inputs.size() << " MIDI inputs." << endl;
    for(unsigned int i = 0; i < inputs.size(); i++) {
        cout << "   (" << i << "): " << inputs[i] << endl;
    }
}


struct ProgramOptions
{
    vector<string> midiInputNames;
    bool allMidiInputs;
    vector<int> oscOutputPorts;
    bool useOscTemplate;
    string oscTemplate;
    bool oscRawMidiMessage;
};


void showVersion()
{
    cout << "m2o version " << M2O_VERSION << endl;
}

int setup_and_parse_program_options(int argc, char* argv[], ProgramOptions &programOptions)
{
    po::options_description desc("mos Usage");

    desc.add_options()
        ("list", "List input MIDI devices")
        ("midiin,i", po::value<vector<string>>(&programOptions.midiInputNames), "MIDI Input device (default: all) - can be specified multiple times")
        ("oscout,o", po::value<vector<int>>(&programOptions.oscOutputPorts), "OSC Output port (default:57120) - can be specified multiple times")
        ("osctemplate,t", po::value<string>(&programOptions.oscTemplate), "OSC output template (use $n: midi port name, $i: midi port id, $c: midi channel, $m: message_type")
        ("oscrawmidimessage,r", po::bool_switch(&programOptions.oscRawMidiMessage)->default_value(false), "OSC send the raw MIDI data as part of the OSC message")
        ("help", "Display this help message")
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

    if (args.count("osctemplate"))
        programOptions.useOscTemplate = true;
    else
        programOptions.useOscTemplate = false;

    if (!args.count("midiin")) {
        // by default add all input devices
        programOptions.midiInputNames = MidiIn::getInputNames();
        programOptions.allMidiInputs = true;
    }
    else {
        programOptions.allMidiInputs = false;
    }

    if (!args.count("oscout")) {
        programOptions.oscOutputPorts.push_back(57120);
    }

    return 0;
}


void prepareMidiProcessors(vector<shared_ptr<MidiInProcessor>>& midiInputProcessors, const ProgramOptions& popts, vector<shared_ptr<OscOutput>> oscOutputs)
{
    // Should we open all devices, or just the ones passed as parameters?
    vector<string> midiInputsToOpen = (popts.allMidiInputs ? MidiIn::getInputNames() : popts.midiInputNames);

    for (auto& input : midiInputsToOpen) {
        cout << "Opening input: " << input << endl;
        try {
            auto midiInput = make_unique<MidiIn>(input);
            auto midiInputProcessor = make_unique<MidiInProcessor>(move(midiInput), oscOutputs);
            if (popts.useOscTemplate)
                midiInputProcessor->setOscTemplate(popts.oscTemplate);
            midiInputProcessor->setOscRawMidiMessage(popts.oscRawMidiMessage);
            midiInputProcessors.push_back(move(midiInputProcessor));
        }
        catch (const std::out_of_range&) {
            cout << "The device " << input << " does not exist";
            throw;
        }
    }
}


int main(int argc, char* argv[]) {
    // midiInputProcessors will contain the list of active MidiIns at a given time
    vector<shared_ptr<MidiInProcessor>> midiInputProcessors;
    // oscOutputs will contain the list of active OSC output ports 
    vector<shared_ptr<OscOutput>> oscOutputs;
    ProgramOptions popts;

    int rc = setup_and_parse_program_options(argc, argv, popts);
    if (rc != 0) {
        return rc;
    }
    
    // Open the OSC output ports
    for (auto port : popts.oscOutputPorts) {
        auto oscOutput = make_shared<OscOutput>("localhost", port);
        oscOutputs.push_back(move(oscOutput));
    }   
    
    // Open the MIDI input ports
    try {
        prepareMidiProcessors(midiInputProcessors, popts, oscOutputs);
    }
    catch (const std::out_of_range&) {
        return -1;
    }

    // For hotplugging
    vector<string> lastAvailablePorts = MidiIn::getInputNames();
    while(true){
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
    };
}
    