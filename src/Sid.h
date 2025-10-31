/// SID helper class.
/// Contains the SID status.
///
/// https://github.com/0x444454/SIDerurgy
///
/// Revision history [authors in square brackets]:
///   2023-2014 Original version (see license below) [Alex Smith]
///   2023-2025 Modified for SIDerurgy [DDT]
/// 
///------------------------------------------------------------------------------------------------------
/// ORIGINAL LICENSE TEXT from https://github.com/twistedelectrons/TherapSID
/// 
/// Copyright 2014 - 2023 Alex Smith.
/// Permission to use, copy, modify, and /or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
/// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
///------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <string>

using namespace std;

class Sid {

public:

  enum Shape {
		NOISE = 1 << 7,
		PULSE = 1 << 6,
		SAW = 1 << 5,
		TRI = 1 << 4
	};

	enum VoiceRegister {
		FREQ_LO = 0,
		FREQ_HI,
		PULSEWIDTH_LO,
		PULSEWIDTH_HI,
		CONTROL,
		ATTACK_DECAY,
		SUSTAIN_RELEASE
	};
	
	enum FilterMode {
		HIGHPASS = 1 << 6,
		BANDPASS = 1 << 5,
		LOWPASS = 1 << 4
	};

	enum Register {
		FREQ1_LO = 0,
		FREQ1_HI,
		PULSEWIDTH1_LO,
		PULSEWIDTH1_HI,
		CONTROL1,
		ATTACK_DECAY1,
		SUSTAIN_RELEASE1,

		FREQ2_LO,
		FREQ2_HI,
		PULSEWIDTH2_LO,
		PULSEWIDTH2_HI,
		CONTROL2,
		ATTACK_DECAY2,
		SUSTAIN_RELEASE2,

		FREQ3_LO,
		FREQ3_HI,
		PULSEWIDTH3_LO,
		PULSEWIDTH3_HI,
		CONTROL3,
		ATTACK_DECAY3,
		SUSTAIN_RELEASE3,

		FILTER_CUTOFF_LO,
		FILTER_CUTOFF_HI,
		FILTER_RESONANCE_ROUTING,
		FILTER_MODE_VOLUME,

		POTX,
		POTY,
		OSC3_RANDOM,
		ENV3
	};


public:
	int next_pair_ = 0;
	int chip_enable_bit_ = 1;
	bool force_initial_update_ = true;
	uint8_t registers_[25];
	uint8_t registers_sent_[25];


public:
	Sid();

	int get_register(int index);
	
	/// Get the value of the control register for the given voice [0..2].
	int get_CONTROL(int voice);

	void set_freq(int voice, int value);
	int get_freq(int voice);
	float get_freq_Hz(int voice);
	int get_MIDI_note(int voice);
	wstring get_note_as_string(int voice);

	bool get_gate(int voice);
	void set_gate(int voice, bool on);

	bool get_sync(int voice);
	bool get_ring_mod(int voice);

	/// attack,decay in [0; 15]
	void set_attack_decay(int voice, int attack, int decay);
	/// sustain,release in [0; 15]
	void set_sustain_release(int voice, int sustain, int release);



	/// @param pulsewidth Pulse width [0..4095].
	void set_pulsewidth(int voice, int pulsewidth);
	int get_pulsewidth(int voice);

	/// @param cutoff Filter cutoff [0..2047].
	void set_filter_cutoff(int cutoff);
	int get_filter_cutoff();

	/// @param resonance The resonance value [0..15].
	void set_filter_resonance(int resonance);
	int get_filter_resonance();

	/// @param resonance The resonance value [0..15].
	//void set_resonance_and_filter_enable(int resonance, bool en1, bool en2, bool en3, bool en_ext);

	/// Set gate, shape, sync and ring.
	void set_reg_control(int voice, int value);

	int get_shape(int voice) const;


	void set_filter_mode(int value);
	int get_filter_mode();
	bool is_voice_filtered(int voice);

	void set_volume(int value);

	wstring get_voice_as_string(int voice);
	wstring to_string();

private:
	bool is_voice_playing(int voice);
	bool is_update_allowed(int register_index);
};