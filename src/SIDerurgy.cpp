#include "SIDerurgy.h"

#include <inttypes.h>

SIDerurgy::SIDerurgy() {
  reset_SID();
}

SIDerurgy::~SIDerurgy() {
}


void SIDerurgy::reset_SID() {
  memset(&sid_, 0, sizeof(sid_));
  for (int voice = 0; voice < 3; voice++) {
    reg_CONTROL_prev_[voice] = -1;
    prev_gates_[voice] = -1; // Gates are unknown.
    prev_notes_[voice] = -1; // No note is playing.
    prev_shape_[voice] = -1;
    prev_pulse_width_[voice] = -1;
  }
  prev_filter_cutoff_ = -1;
  prev_filter_resonance_ = -1;
}

bool SIDerurgy::is_voice_filtered(int voice) {
  return sid_.is_voice_filtered(voice);
}

bool SIDerurgy::is_note_on_detected(int voice, int new_note) {
  bool cur_gate = sid_.get_CONTROL(voice) & 0x01;
  if (!cur_gate) return false; // No note gate.
  int prev_gate = prev_gates_[voice];
  if (prev_gate < 0) prev_gate = 0;
  if (prev_gate == 1) {
    // Gate never went off.
    if (prev_notes_[voice] == new_note) {
      return false; // Same note.
    }
  }
  return cur_gate; // Previously off, now on.
}


bool SIDerurgy::is_shape_change_detected(int voice, int new_shape) {
  return (prev_shape_[voice] != new_shape);
}

bool SIDerurgy::is_pulse_width_change_detected(int voice, int new_pulse_width) {
  return (prev_pulse_width_[voice] != new_pulse_width);
}

bool SIDerurgy::is_filter_cutoff_change_detected(int new_cutoff) {
  return (prev_filter_cutoff_ != new_cutoff);
}

bool SIDerurgy::is_filter_resonance_change_detected(int new_resonance) {
  return (prev_filter_resonance_ != new_resonance);
}


bool SIDerurgy::send_filter_cutoff(int cutoff) {
  bool res = true;
  float f = cutoff / 2047.0f;
  // Send to all channels of voices being filtered.
  for (int i = 0; i <= 2; i++) {
    if (is_voice_filtered(i)) res &= midi_out_->send_filter_cutoff(i, f);
  }
  // Send also to channel 4 (global).
  res &= midi_out_->send_filter_cutoff(4, f);
  return res;
}

bool SIDerurgy::send_filter_resonance(int resonance) {
  bool res = true;
  float f = resonance / 15.0f;
  // Send to all channels of voices being filtered.
  for (int i = 0; i <= 2; i++) {
    if (is_voice_filtered(i)) res &= midi_out_->send_filter_resonance(i, f);
  }
  // Send also to channel 4 (global).
  res &= midi_out_->send_filter_resonance(4, f);
  return res;
}

GM::Instrument SIDerurgy::get_GM_instrument_for_shape(uint8_t shape) {
  if (shape & Sid::Shape::NOISE) return GM::Instrument::Breath_Noise;
  else if (shape & Sid::Shape::TRI) return GM::Instrument::Ocarina;
  else if (shape & Sid::Shape::PULSE) return GM::Instrument::Lead_1_square;
  else if (shape & Sid::Shape::SAW) return GM::Instrument::Lead_2_sawtooth;
  return GM::Instrument::Lead_1_square; // Should never happen.
}


void SIDerurgy::on_receive_long_data(int64_t timestamp, unsigned char* buf, int len) {
  // Debug stuff.
  if (debug_level_ >= 2) {
    printf("===> PARSE LONG DATA: len=%d ts=%" PRId64 "\n", len, timestamp);
    for (int i = 0; i < len; i++) {
      printf(" %02X", buf[i]);
    }
    printf("\n");
  }

  // Parser.
  if ((len < 2) || (buf[0] != 0xF0)) return; // Not a sysex data.

  // Check for sysex integrity.
  if (buf[len - 1] != 0xF7) {
    printf("  ERROR: Incomplete sysex.\n");
    return;
  }

  // Check for 0x2D (1 byte European ID: "Neve").
  if (buf[1] != 0x2D) {
    printf("  ERROR: Not an ASID sysex (ID = %02X).\n", buf[1]);
    return; // Not an ASID sysex.
  }

  // Check command.
  uint8_t cmd = buf[2];

  if (cmd == 0x4C) {
    // START PLAYBACK
    if (debug_level_ >= 2) printf("  CMD: START PLAYBACK\n");
    reset_SID();
    midi_out_->reset();
  }
  else if (cmd == 0x4D) {
    // STOP PLAYBACK
    if (debug_level_ >= 2) printf("  CMD: STOP PLAYBACK\n");
    midi_out_->reset();
  }
  else if (cmd == 0x4F) {
    // DISPLAY CHARACTERS
    if (debug_level_ >= 2) printf("  CMD: DISPLAY CHARACTERS\n");
  }
  else if (cmd == 0x4E) {
    // SID CONTROL
    if (debug_level_ >= 2) printf("  CMD: SID CONTROL\n");
    if (len < 11) {
      printf("  ERROR: Not enough SID control bytes.\n");
      return;
    }
    
    // Save previous CONTROL registers.
    reg_CONTROL_prev_[0] = sid_.registers_[Sid::Register::CONTROL1];
    reg_CONTROL_prev_[1] = sid_.registers_[Sid::Register::CONTROL2];
    reg_CONTROL_prev_[2] = sid_.registers_[Sid::Register::CONTROL3];

    int mask1 = buf[3];
    int mask2 = buf[4];
    int mask3 = buf[5];
    int mask4 = buf[6];
    int msb1 = buf[7];
    int msb2 = buf[8];
    int msb3 = buf[9];
    int msb4 = buf[10];

    int mask = mask1 | (mask2 << 7) | (mask3 << 14) | (mask4 << 21);
    int msb = msb1 | (msb2 << 7) | (msb3 << 14) | (msb4 << 21);
    int data_idx = 11;
    for (int reg_idx = 0; reg_idx <= 27; reg_idx++) {
      if (mask & 1) {
        int reg_offset = g_table_id_to_offset_[reg_idx];
        // Remap secondaries for CONTROL registers.
        switch (reg_offset) {
          case 25: reg_offset = 4; break;
          case 26: reg_offset = 11; break;
          case 27: reg_offset = 18; break;
        }
        if (data_idx >= len) {
          printf("  ERROR: Not enough SID data bytes.\n");
          return;
        }
        uint8_t reg_value = buf[data_idx];
        if (msb & 1) reg_value |= 0x80;
        // Set SID register.
        sid_.registers_[reg_offset] = reg_value;
        /*
        if (reg_offset == 11) {
          if (reg_value & 0x02) {
            int a = 7;
          }
        }
        */
        data_idx++;
      }
      mask >>= 1;
      msb >>= 1;
    }
    
    // DEBUG: Print SID state.
    if (debug_level_ >= 2) wprintf(L"----------------------------> %ls\n", sid_.to_string().c_str());

    if (!midi_out_) return; // NO MIDI OUTPUT TO HANDLE

    // Handle per-voice SID state.

    for (int voice = 0; voice < 3; voice++) {
      int gate = (sid_.get_gate(voice) ? 1 : 0);
      int note = sid_.get_MIDI_note(voice);
      if (note <= 0) {
        // Voice is muted (freq == 0).
        if (prev_notes_[voice] >= 0) {
          midi_out_->send_note_off(voice, prev_notes_[voice]);
          prev_notes_[voice] = -1;
        }
      }
      else {
        // We have a valid note (frequency).
        // Check if new note.
        if (note != prev_notes_[voice]) {
          // NEW NOTE.
          // Previous note goes off.
          midi_out_->send_note_off(voice, prev_notes_[voice]);
          // New note is on or off depending on gate.
          if (gate) midi_out_->send_note_on(voice, note);
          else {
            // New note but no gate.
            // This seems to be ignored by the Poly2 MIDI to CV interface.
            // So we use an ON/OFF trick to convince it to set CV to the note value anyway.
            midi_out_->send_note_on(voice, note); // Yes, it's a valid note !
            midi_out_->send_note_off(voice, note); // ...but it lasts 0 seconds (no gate :-).
          }
        }
        else {
          // SAME NOTE.
          // Check if gate has changed.
          if (gate != prev_gates_[voice]) {
            // Gate has changed.
            if (gate) midi_out_->send_note_on(voice, note);
            else midi_out_->send_note_off(voice, note); // MIDI to CV interface should set CV to the note value anyway.
          }
          else {
            // Gate has not changed.
            // [nothing to do].
          }
        }
        // Set prev state.
        prev_notes_[voice] = note; // Remember playing notes.
        prev_gates_[voice] = gate;
      }

      // Check for waveform change.
      int shape = sid_.get_shape(voice);
      if (is_shape_change_detected(voice, shape)) {
        midi_out_->send_program_change(voice, get_GM_instrument_for_shape(shape));
        prev_shape_[voice] = shape;
      }
      // Check pulse width mod [0..4095].
      int pw = sid_.get_pulsewidth(voice);
      if (is_pulse_width_change_detected(voice, pw)) {
        midi_out_->send_pulse_width(voice, pw / 4095.0f);
        prev_pulse_width_[voice] = pw;
      }
    }
    
    // Handle hlobal SID state.

    // Check filter cutoff [0..2047].
    int cutoff = sid_.get_filter_cutoff();
    if (is_filter_cutoff_change_detected(cutoff)) {
      send_filter_cutoff(cutoff);
      prev_filter_cutoff_ = cutoff;
    }
    // Check filter resonance [0..15].
    int resonance = sid_.get_filter_resonance();
    if (is_filter_resonance_change_detected(resonance)) {
      send_filter_resonance(resonance);
      prev_filter_resonance_ = resonance;
    }
  }
}
