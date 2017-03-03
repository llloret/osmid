/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_audio_devices
  vendor:           juce
  version:          4.3.1
  name:             JUCE audio and MIDI I/O device classes
  description:      Classes to play and record from audio and MIDI I/O devices
  website:          http://www.juce.com/juce
  license:          ISC

  dependencies:     juce_audio_basics, juce_events
  OSXFrameworks:    CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:    CoreAudio CoreMIDI AudioToolbox AVFoundation
  linuxPackages:    alsa
  mingwLibs:        winmm

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_AUDIO_DEVICES_H_INCLUDED
#define JUCE_AUDIO_DEVICES_H_INCLUDED

#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
/** Config: JUCE_ASIO
    Enables ASIO audio devices (MS Windows only).
    Turning this on means that you'll need to have the Steinberg ASIO SDK installed
    on your Windows build machine.

    See the comments in the ASIOAudioIODevice class's header file for more
    info about this.
*/
#ifndef JUCE_ASIO
 #define JUCE_ASIO 0
#endif

/** Config: JUCE_WASAPI
    Enables WASAPI audio devices (Windows Vista and above). See also the
    JUCE_WASAPI_EXCLUSIVE flag.
*/
#ifndef JUCE_WASAPI
 #define JUCE_WASAPI 1
#endif

/** Config: JUCE_WASAPI_EXCLUSIVE
    Enables WASAPI audio devices in exclusive mode (Windows Vista and above).
*/
#ifndef JUCE_WASAPI_EXCLUSIVE
 #define JUCE_WASAPI_EXCLUSIVE 0
#endif


/** Config: JUCE_DIRECTSOUND
    Enables DirectSound audio (MS Windows only).
*/
#ifndef JUCE_DIRECTSOUND
 #define JUCE_DIRECTSOUND 1
#endif

/** Config: JUCE_ALSA
    Enables ALSA audio devices (Linux only).
*/
#ifndef JUCE_ALSA
 #define JUCE_ALSA 1
#endif

/** Config: JUCE_JACK
    Enables JACK audio devices (Linux only).
*/
#ifndef JUCE_JACK
 #define JUCE_JACK 0
#endif

/** Config: JUCE_USE_ANDROID_OPENSLES
    Enables OpenSLES devices (Android only).
*/
#ifndef JUCE_USE_ANDROID_OPENSLES
 #if JUCE_ANDROID_API_VERSION > 8
  #define JUCE_USE_ANDROID_OPENSLES 1
 #else
  #define JUCE_USE_ANDROID_OPENSLES 0
 #endif
#endif

//==============================================================================
namespace juce
{

#include "audio_io/juce_AudioIODevice.h"
#include "audio_io/juce_AudioIODeviceType.h"
#include "audio_io/juce_SystemAudioVolume.h"
#include "midi_io/juce_MidiInput.h"
#include "midi_io/juce_MidiMessageCollector.h"
#include "midi_io/juce_MidiOutput.h"
#include "sources/juce_AudioSourcePlayer.h"
#include "sources/juce_AudioTransportSource.h"
#include "audio_io/juce_AudioDeviceManager.h"

}

#endif   // JUCE_AUDIO_DEVICES_H_INCLUDED
