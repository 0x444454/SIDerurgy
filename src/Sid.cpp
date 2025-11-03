#include "Sid.h"
#include <cassert>


static void write_mask(uint8_t& value, int mask, int new_value) {
	value = (value & (~mask)) | (new_value & mask);
}


Sid::Sid() {
	for (int i = 0; i < 25; i++) {
		registers_[i] = 0;
		registers_sent_[i] = 0;
	}
	//registers_[CONTROL1] = 0x01;
	//registers_[CONTROL2] = 0x01;
	//registers_[CONTROL3] = 0x01;
	registers_[FILTER_CUTOFF_LO] = 0xFF;
	registers_[FILTER_CUTOFF_HI] = 0xFF;
	registers_[FILTER_RESONANCE_ROUTING] = 0xFF;
	registers_[FILTER_MODE_VOLUME] = 0x0F; // Filter off full vol
}


void Sid::set_pulsewidth(int voice, int pulsewidth) {
	registers_[7 * voice + PULSEWIDTH_HI] = pulsewidth >> 8;
	registers_[7 * voice + PULSEWIDTH_LO] = pulsewidth & 0xFF;
}

int Sid::get_pulsewidth(int voice) {
	int pw = registers_[7 * voice + PULSEWIDTH_HI] << 8;
	pw |= registers_[7 * voice + PULSEWIDTH_LO];
	return pw;
}


void Sid::set_attack_decay(int voice, int attack, int decay) {
	registers_[7 * voice + ATTACK_DECAY] = decay | (attack << 4);
}


void Sid::set_sustain_release(int voice, int sustain, int release) {
	registers_[7 * voice + SUSTAIN_RELEASE] = release | (sustain << 4);
}


int Sid::get_CONTROL(int voice) {
	return registers_[7 * voice + CONTROL];
}


void Sid::set_freq(int voice, int value) {
	registers_[7 * voice + FREQ_LO] = value & 0xFF;
	registers_[7 * voice + FREQ_HI] = value >> 8;
}


int Sid::get_freq(int voice) {
	int v = (registers_[7 * voice + FREQ_HI] << 8) | (registers_[7 * voice + FREQ_LO]);
	return v;
}

float Sid::get_freq_Hz(int voice) {
	float f = (float)get_freq(voice);
	return f * 1000000.0f / 16777216.0f; // i.e. SID_Clock / 16777216
}

//float Sid::get_freq_Hz(int voice) {
//	float f_reg = (float) get_freq(voice);
//	return f_reg * sid_clock_ / 16777216.0f; // i.e. SID_Clock / 16777216
//}


float Sid::get_freq_CV(int voice) {
	const float f_C0 = 16.3516f; // Frequency of C0 (i.e. base freq when CV = 0V).
	const float K = sid_clock_ / 16777216.0f / f_C0; // K = 0.0036451873
	float voltage = log2f(max(1.0f, get_freq(voice) * K)); // Lower bound to 1.
	return voltage;
}


int Sid::get_MIDI_note(int voice) {
	float f = get_freq_Hz(voice);
	if (f <= 0.0f) return -1;
	int note = static_cast<int>((12.0f * (log10f(f / 440.0f) / log10f(2.0f)) + 57.0f) + 0.5f); // TODO: Should we use round() ?
	return note;
}

wstring Sid::get_note_as_string(int voice) {
	int note = get_MIDI_note(voice);
	if (note < 0) return L"  ";
	static const wstring note_names[]{
		L"C", L"C#", L"D", L"D#", L"E", L"F", L"F#", L"G", L"G#", L"A", L"A#", L"B"
	};
	wstring str_note = note_names[note % 12];
	if (str_note.length() == 1) str_note.append(L" ");
	int octave = (note / 12) - 1;
	if (octave < 0) str_note.append(L" ");
	str_note.append(std::to_wstring(octave));
	
	return str_note;
}


bool Sid::get_gate(int voice) {
	return registers_[7 * voice + CONTROL] & 0x01;
}

void Sid::set_gate(int voice, bool on) {
	registers_[7 * voice + CONTROL] &= 0xfe;
	registers_[7 * voice + CONTROL] |= (on ? 0x01 : 0x00);
}

bool Sid::get_sync(int voice) {
	return registers_[7 * voice + CONTROL] & 0x02;
}

bool Sid::get_ring_mod(int voice) {
	return registers_[7 * voice + CONTROL] & 0x04;
}


void Sid::set_reg_control(int voice, int control) {
	registers_[7 * voice + CONTROL] = control;
}


int Sid::get_shape(int voice) const {
	return registers_[7 * voice + CONTROL] & 0xf0;
}


void Sid::set_filter_mode(int value) {
	write_mask(registers_[FILTER_MODE_VOLUME], HIGHPASS | BANDPASS | LOWPASS, value);
}


void Sid::set_volume(int value) {
	registers_[FILTER_MODE_VOLUME] = (registers_[FILTER_MODE_VOLUME] & 0xF0) | (value & 0x0F);
}


int Sid::get_filter_mode() {
	return registers_[FILTER_MODE_VOLUME] & (HIGHPASS | BANDPASS | LOWPASS);
}

bool Sid::is_voice_filtered(int voice) {
	return (registers_[FILTER_RESONANCE_ROUTING] & (1 << voice));
}


wstring Sid::get_voice_as_string(int voice) {
	wstring str;
	//str.append(std::to_string(voice + 1)); str.append(" ");
	str.append(get_note_as_string(voice));
  str.append(L" ");
	if (get_gate(voice)) str.append(L"g"); else str.append(L"-");
	if (get_sync(voice)) str.append(L"s"); else str.append(L"-");
	if (get_ring_mod(voice)) str.append(L"r"); else str.append(L"-");
	return str;
}


wstring Sid::to_string() {
	wstring str;
	for (int voice = 0; voice < 3; voice++) {
		if (voice > 0) str.append(L"        ");
		str.append(get_voice_as_string(voice));
	}
	return str;
}


bool Sid::is_voice_playing(int voice) {
	return registers_[7 * voice + CONTROL] & 1;
}


//void Sid::set_resonance_and_filter_enable(int resonance, bool en1, bool en2, bool en3, bool en_ext) {
//	registers_[FILTER_RESONANCE_ROUTING] = (resonance << 4) | (en1 ? 1 : 0) | (en2 ? 2 : 0) | (en3 ? 4 : 0) | (en_ext ? 8 : 0);
//}


void Sid::set_filter_cutoff(int cutoff) {
	registers_[FILTER_CUTOFF_LO] = (cutoff & 0x07);
	registers_[FILTER_CUTOFF_HI] = (cutoff >> 3);
}


int Sid::get_filter_cutoff() {
	return (registers_[FILTER_CUTOFF_HI] << 3) | (registers_[FILTER_CUTOFF_LO] & 0x07);
}


void Sid::set_filter_resonance(int resonance) {
	int frr = registers_[FILTER_RESONANCE_ROUTING];
	frr = (frr & 0x0f) | (resonance << 4);
	registers_[FILTER_RESONANCE_ROUTING] = frr;
}


int Sid::get_filter_resonance() {
	return registers_[FILTER_RESONANCE_ROUTING] >> 4;
}


bool Sid::is_update_allowed(int register_index) {
	if (register_index == SUSTAIN_RELEASE1 || register_index == SUSTAIN_RELEASE2 || register_index == SUSTAIN_RELEASE3) {
		int voice = register_index / 7;
		return !is_voice_playing(voice);
	}
	else {
		return true;
	}
}


int Sid::get_register(int index) {
	return registers_[index];
}

