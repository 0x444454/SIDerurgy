/// SIDerurgy - Forge your favorite SID.
/// An ASID to MIDI/CV converter.
/// 
/// This class implements the MIDI interface.
/// Author: DDT
/// 
/// https://github.com/0x444454/SIDerurgy
/// 
#pragma once

#include <windows.h>
#include <string>

#include "GM.h"

using namespace std;

class MIDI_Listener {

public:
  virtual void on_receive_long_data(int64_t timestamp, unsigned char* buf, int len) {}
};


class MIDI_Device
{
public:
  enum Mode {
    MIDI_IN = 0x01,
    MIDI_OUT = 0x02,
    MIDI_IN_AND_OUT = MIDI_IN | MIDI_OUT,
  };

  enum ChannelMode {
    ALL_SOUND_OFF         = 120,
    RESET_ALL_CONTROLLERS = 121,
    LOCAL_CONTROL         = 122,
    ALL_NOTES_OFF         = 123,
    OMNI_OFF              = 124,
    OMNI_ON               = 125,
    MONO_ON               = 126,
    POLY_ON               = 127,
  };



public:
  wstring device_name_in_;
  wstring device_name_out_;

  int dev_id_in_ = -1;
  int dev_id_out_ = -1;

  HMIDIIN hDev_in_ = NULL;
  HMIDIOUT hDev_out_ = NULL;

  MIDIHDR* pmh_ = nullptr;
  unsigned char* buffer_ = nullptr;

  MIDI_Listener* listener_ = nullptr;
  MMRESULT last_err_ = MMSYSERR_NOERROR;

  int debug_level_ = 0;

public:
  MIDI_Device(int dev_id_in, int dev_id_out, MIDI_Listener* listener = nullptr);
  virtual ~MIDI_Device();

  wstring get_device_name_in() { return device_name_in_; }
  wstring get_device_name_out() { return device_name_out_; }

  MMRESULT get_error() { return last_err_; }

  static MIDI_Device* create(wstring prefix, Mode mode = Mode::MIDI_IN_AND_OUT, MIDI_Listener* listener = nullptr);

  void set_listener(MIDI_Listener* listener) { listener_ = listener; }

  bool send_note_on(int channel, int note_num);
  bool send_note_off(int channel, int note_num);
  bool send_channel_mode(int channel, ChannelMode channel_mode);
  bool send_program_change(int channel, int program);
  /// Change pulse width on the given channel.
  /// Pulse width value in the range [0..1].
  bool send_pulse_width(int channel, float pw);
  /// Cutoff value in the range[0..1].
  bool send_filter_cutoff(int channel, float cutoff);
  /// Resonance value in the range[0..1].
  bool send_filter_resonance(int channel, float resonance);
  /// Modulation Wheel bend value in the range[0..1].
  bool send_mod_wheel(int channel, float mod_wheel);
  /// Pitch bend value in the range[0..1].
  /// Value:
  ///   - Negative pitch bend: [0..0.5)
  ///   - No pitch bend: 0.5
  ///   - Positive pitch bend: (0..1.0]
  bool send_pitch_bend(int channel, float pitch_bend);
  
  bool send_CC(int channel, int controller, int value);
  void reset();

protected:
  void setup_buffer(MIDIHDR* pmh);
  
  // Callback invoked on MIDI IN.
  void on_midi_in(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  void on_midi_out(HMIDIOUT hMidiout, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  //void parse_long_data(int64_t timestamp, unsigned char* buf, int len);
  
  static void CALLBACK cb_midi_in(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  static void CALLBACK cb_midi_out(HMIDIOUT hMidiout, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
};

