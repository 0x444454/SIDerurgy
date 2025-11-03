/// SIDerurgy - Forge your favorite SID.
/// An ASID to MIDI/CV converter.
/// 
/// This program is mainly aimed at creating a custom "SID" chip using a modular synth via a MIDI to CV interface.
/// However, it also works with any MIDI device (each SID voice is sent to a different channel).
/// General MIDI is supported, but this will only produce good results with simple SID tunes.
/// 
/// I use a Polyend Poly2 to convert SIDerurgy's MIDI stream to CV.
/// See project page for my setup.
///
/// https://github.com/0x444454/SIDerurgy
///
/// Revision history [authors in square brackets]:
///   2023-09-01 Initial experimental version. MIDI interface working. Some results. [DDT]
///   [...] Two years hiatus.
///   2025-10-26 Started working on this again. [DDT].
///   2025-10-30 Alpha version, working with Polyend Poly2. Some SID features are not yet mapped. [DDT]
/// 
/// This is still a very alpha version, but enough to have fun. 
/// Enjoy !
/// .DDT

#pragma once

#include <stdint.h>
#include <string>

#include "MIDI_Device.h"
#include "Sid.h"

const uint8_t g_table_id_to_offset_[] = {
   0,
   1,
   2,
   3,
   5,
   6,
   7,
   8,
   9,
  10,
  12,
  13,
  14,
  15,
  16,
  17,
  19,
  20,
  21,
  22,
  23,
  24,
   4,
  11,
  18,
  25, // <= secondary for reg 4
  26, // <= secondary for reg 11
  27, // <= secondary for reg 18 MSB
};

class SIDerurgy : public MIDI_Listener {
public:
  bool must_init_GM_presets_ = true;
  Sid sid_;
  MIDI_Device* midi_out_ = nullptr;
  bool enable_pitch_bend_ = false;
  bool enable_GM_ = false;

  int reg_CONTROL_prev_[3]; // Previous control registers.

  int prev_gates_[3];
  int prev_notes_[3];
  int prev_freq_[3];
  int prev_shape_[3];
  int prev_pulse_width_[3];
  int prev_filter_cutoff_ = -1;
  int prev_filter_resonance_ = -1;

  int debug_level_ = 0;

public:
  SIDerurgy();
  ~SIDerurgy();

  /// Reset SID internal state.
  void reset_SID();

  bool is_voice_filtered(int voice);
  bool is_note_on_detected(int voice, int new_note);
  bool is_freq_change_detected(int voice, int new_freq);
  bool is_shape_change_detected(int voice, int new_shape);
  bool is_pulse_width_change_detected(int voice, int new_pulse_width);
  bool is_filter_cutoff_change_detected(int new_cutoff);
  bool is_filter_resonance_change_detected(int new_resonance);

  bool send_note_on(int voice, int note_num);
  bool send_note_off(int voice, int note_num);
  /// Send filter cutoff to all channels affected by the filter.
  bool send_filter_cutoff(int cutoff);
  /// Send filter resonance to all channels affected by the filter.
  bool send_filter_resonance(int resonance);

  /// NOTE: The instrument starts from 0, so it can be directly used for program change.
  static GM::Instrument get_GM_instrument_for_shape(uint8_t shape);

  // MIDI long data callback (receive sysex).
  virtual void on_receive_long_data(int64_t timestamp, unsigned char* buf, int len) override;

};

