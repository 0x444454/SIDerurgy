#include "MIDI_Device.h"

#include <stdio.h>
#include <inttypes.h>

const int BUF_SIZE = 256 * 1024;


MIDI_Device::MIDI_Device(int dev_id_in, int dev_id_out, MIDI_Listener* listener) {
  dev_id_in_ = dev_id_in;
  dev_id_out_ = dev_id_out;
  listener_ = listener;

  // MIDI IN
  if (dev_id_in_ >= 0) {
    // Open MIDI IN
    last_err_ = midiInOpen(&hDev_in_, dev_id_in_, (DWORD_PTR)(void*) cb_midi_in, (DWORD_PTR) this, CALLBACK_FUNCTION);
    if (last_err_ != MMSYSERR_NOERROR) {
      wprintf(L"ERROR: midiInOpen() failed. Res=%d\n", last_err_);
      return;
    }

    // Allocate buffer.
    pmh_ = (MIDIHDR*) malloc(sizeof(MIDIHDR));
    if (!pmh_) {
      wprintf(L"ERROR allocating MIDIHDR.\n");
      return;
    }
    memset(pmh_, 0, sizeof(MIDIHDR));
    buffer_ = (unsigned char*) malloc(BUF_SIZE);
    pmh_->lpData = (LPSTR) buffer_;
    pmh_->dwBufferLength = BUF_SIZE; // The size of the buffer
    pmh_->dwFlags = 0;
    setup_buffer(pmh_);

    // Start MIDI IN
    last_err_ = midiInStart(hDev_in_);
    if (last_err_ != MMSYSERR_NOERROR) {
      wprintf(L"ERROR: midiInStart() failed. Res=%d\n", last_err_);
    }
  }

  // MIDI OUT
  if (dev_id_out_ >= 0) {
    // Open MIDI OUT
    last_err_ = midiOutOpen(&hDev_out_, dev_id_out_, (DWORD_PTR)(void*)cb_midi_out, (DWORD_PTR)this, CALLBACK_FUNCTION);
    //last_err_ = midiOutOpen(&hDev_out_, dev_id_out_, 0, 0, CALLBACK_NULL);
    if (last_err_ != MMSYSERR_NOERROR) {
      wprintf(L"ERROR: midiOutOpen() failed. Res=%d\n", last_err_);
    }
  }

  // Reset.
  reset();
  //output_note(1, 60);
}


MIDI_Device::~MIDI_Device() {
  if (hDev_in_) {
    midiInStop(hDev_in_);
    midiInClose(hDev_in_);
  }
  if (hDev_out_) {
    midiOutReset(hDev_out_);
    midiOutClose(hDev_out_);
  }
  free(buffer_);
  free(pmh_);
}


MIDI_Device* MIDI_Device::create(wstring prefix, Mode mode, MIDI_Listener* listener) {
  // MIDI INPUT
  int dev_id_in = -1;
  wstring device_name_in;
  if (mode & Mode::MIDI_IN) {
    wprintf(L"Available MIDI IN devices:\n");
    const int num_devs_in = midiInGetNumDevs();
    for (int i = 0; i < num_devs_in; i++) {
      MIDIINCAPSW mic{};
      midiInGetDevCapsW(i, &mic, sizeof(mic));
      wprintf(L"  MIDI I[%d]: \"%ls\"\n", i, mic.szPname);
      wstring name = mic.szPname;
      if (device_name_in.empty() && (name.find(prefix, 0) != std::wstring::npos)) {
        // Found.
        dev_id_in = i;
        device_name_in = name;
      }
    }
    if (dev_id_in < 0) {
      // NOT FOUND.
      wprintf(L"ERROR: Could not find MIDI IN device matching \"%ls\"\n", prefix.c_str());
      wprintf(L"Remember: Matching is case-sensitive.\n");
      return nullptr;
    }
  }
  

  // MIDI OUTPUT
  int dev_id_out = -1;
  wstring device_name_out;
  if (mode & Mode::MIDI_OUT) {
    wprintf(L"Available MIDI OUT devices:\n");
    const int num_devs_out = midiOutGetNumDevs();
    for (int i = 0; i < num_devs_out; i++) {
      MIDIOUTCAPSW moc{};
      midiOutGetDevCapsW(i, &moc, sizeof(moc));
      wprintf(L"  MIDI O[%d]: \"%ls\"\n", i, moc.szPname);
      wstring name = moc.szPname;
      if (device_name_out.empty() && (name.find(prefix, 0) != std::wstring::npos)) {
        // Found.
        dev_id_out = i;
        device_name_out = name;
        break;
      }
    }
    if (dev_id_out < 0) {
      // NOT FOUND.
      wprintf(L"ERROR: Could not find MIDI OUT device matching \"%ls\"\n", prefix.c_str());
      wprintf(L"Remember: Matching is case-sensitive.\n");
      return nullptr;
    }
  }

  MIDI_Device* d = new MIDI_Device(dev_id_in, dev_id_out, listener);
  d->device_name_in_ = device_name_in;
  d->device_name_out_ = device_name_out;
  return d;
}


bool MIDI_Device::send_note_on(int channel, int note_num) {
  if (note_num < 0) return false;
  DWORD v = 0x400090 | channel; // Use 0x40 for velocity (i.e. no velocity implemented). 
  v |= (note_num << 8);
  last_err_ = midiOutShortMsg(hDev_out_, v);
  return (last_err_ == MMSYSERR_NOERROR);
}
  

bool MIDI_Device::send_note_off(int channel, int note_num) {
  if (note_num < 0) return false;
  DWORD v = 0x400080 | channel; // Use 0x40 for velocity (i.e. no velocity implemented). 
  v |= (note_num << 8);
  last_err_ = midiOutShortMsg(hDev_out_, v);
  return (last_err_ == MMSYSERR_NOERROR);
}


bool MIDI_Device::send_channel_mode(int channel, ChannelMode channel_mode) {
  DWORD v = 0x0000B0 | channel; // Use 0x40 for velocity (i.e. no velocity implemented). 
  v |= (channel_mode << 8);
  last_err_ = midiOutShortMsg(hDev_out_, v);
  return (last_err_ == MMSYSERR_NOERROR);
}


bool MIDI_Device::send_program_change(int channel, int program) {
  DWORD v = 0x0000C0 | channel; // Use 0x40 for velocity (i.e. no velocity implemented). 
  v |= (program << 8);
  last_err_ = midiOutShortMsg(hDev_out_, v);
  return (last_err_ == MMSYSERR_NOERROR);
}

bool MIDI_Device::send_pulse_width(int channel, float pw) {
  int hires_value = (int) (pw * (float)(0x3fff)); // 14-bit MIDI hi-res value (not yet supported).
  int cc = 21; // Use CC 21.
  send_CC(channel, cc, hires_value >> 7);
  return (last_err_ == MMSYSERR_NOERROR);
}



bool MIDI_Device::send_filter_cutoff(int channel, float cutoff) {
  int hires_value = (int)(cutoff * (float)(0x3fff)); // 14-bit MIDI hi-res value (not yet supported).
  int cc = 74; // Use CC 74.
  send_CC(channel, cc, hires_value >> 7);
  //if (channel == 4) printf("CUTOFF[%d]=%d\n", channel, hires_value >> 7);
  return (last_err_ == MMSYSERR_NOERROR);
}


bool MIDI_Device::send_filter_resonance(int channel, float resonance) {
  int hires_value = (int)(resonance * (float)(0x3fff)); // 14-bit MIDI hi-res value (not yet supported).
  int cc = 71; // Use CC 71.
  send_CC(channel, cc, hires_value >> 7);
  return (last_err_ == MMSYSERR_NOERROR);
}


bool MIDI_Device::send_CC(int channel, int controller, int value) {
  DWORD v = 0x0000B0 | channel; // Use 0x40 for velocity (i.e. no velocity implemented). 
  BYTE status = 0xB0 | (channel & 0x0F);     // channel 0..15
  BYTE data1 = (BYTE)(controller & 0x7F);    // controller 0..127
  BYTE data2 = (BYTE)(value & 0x7F);         // value 0..127
  DWORD msg = (DWORD)status | ((DWORD)data1 << 8) | ((DWORD)data2 << 16);
  last_err_ = midiOutShortMsg(hDev_out_, msg);
  return (last_err_ == MMSYSERR_NOERROR);
}


void MIDI_Device::reset() {
  if (hDev_out_) {
    last_err_ = midiOutReset(hDev_out_);
    for (int chn = 0; chn < 16; chn++) {
      send_channel_mode(chn, ChannelMode::ALL_NOTES_OFF);
      send_channel_mode(chn, ChannelMode::ALL_SOUND_OFF);
      send_channel_mode(chn, ChannelMode::RESET_ALL_CONTROLLERS);
      send_CC(chn, 1, 0);   // Mod wheel to 0.
      send_CC(chn, 64, 0);  // Sustain pedal off.
      // All notes off.
      for (int n = 0; n < 128; n++) send_note_off(chn, n);
      send_note_off(0, 0); // Preset default SID frequency to lowest note.
    }
  }
}


void MIDI_Device::setup_buffer(MIDIHDR* pmh) {
  if (pmh->dwFlags & MHDR_PREPARED) {
    last_err_ = midiInUnprepareHeader(hDev_in_, pmh, sizeof(MIDIHDR));
    if (last_err_ != MMSYSERR_NOERROR) {
      wprintf(L"ERROR: midiInUnprepareHeader() failed. Res=%d\n", last_err_);
    }
    pmh->lpData = (LPSTR)buffer_;
    pmh->dwBufferLength = BUF_SIZE;
  }
  pmh->dwBytesRecorded = 0;
  pmh->dwFlags &= ~MHDR_DONE;

  last_err_ = midiInPrepareHeader(hDev_in_, pmh, sizeof(MIDIHDR));
  if (last_err_ != MMSYSERR_NOERROR) {
    wprintf(L"ERROR: midiInPrepareHeader() failed. Res=%d\n", last_err_);
    //return;
  }

  last_err_ = midiInAddBuffer(hDev_in_, pmh, sizeof(MIDIHDR)); // Send the buffer to the device driver
  if (last_err_ != MMSYSERR_NOERROR) {
    wprintf(L"ERROR: midiInAddBuffer() failed. Res=%d\n", last_err_);
    //return;
  }
}


void CALLBACK MIDI_Device::cb_midi_in(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  // Wrap on instance.
  MIDI_Device* dev = (MIDI_Device*)dwInstance;
  dev->on_midi_in(hMidiIn, wMsg, dwParam1, dwParam2);
  return;
}


void MIDI_Device::on_midi_in(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  switch (wMsg) {
    // INPUT
    case MIM_OPEN:
      if (debug_level_ >= 2) printf("wMsg=MIM_OPEN\n");
      break;
    case MIM_CLOSE:
      if (debug_level_ >= 2) printf("wMsg=MIM_CLOSE\n");
      break;
    case MIM_DATA:
      if (debug_level_ >= 2) printf("wMsg=MIM_DATA, dwParam1=%" PRIx64 ", dwParam2=%" PRIx64 "\n", dwParam1, dwParam2);
      break;
    case MIM_LONGDATA:
    {
      if (debug_level_ >= 2) printf("wMsg=MIM_LONGDATA\n");
      // dwParam1 = lpMidiHdr
      // dwParam2 = dwTimestamp
      MIDIHDR* pmh = (MIDIHDR*)dwParam1;
      if (listener_) listener_->on_receive_long_data(dwParam2, (unsigned char*) pmh->lpData + pmh->dwOffset, pmh->dwBytesRecorded);
      //parse_long_data(dwParam2, (unsigned char*)pmh->lpData + pmh->dwOffset, pmh->dwBytesRecorded);
      // Handle buffers.
      setup_buffer(pmh);
      break;
    }
    case MIM_ERROR:
      if (debug_level_ >= 2) printf("wMsg=MIM_ERROR\n");
      break;
    case MIM_LONGERROR:
      if (debug_level_ >= 2) printf("wMsg=MIM_LONGERROR\n");
      break;
    case MIM_MOREDATA:
      if (debug_level_ >= 2) printf("wMsg=MIM_MOREDATA\n");
      break;
    // UNKNOWN
    default:
      if (debug_level_ >= 1) printf("wMsg=%x (%" PRIx64 ", %" PRIx64 ")\n", wMsg, dwParam1, dwParam2);
      break;
  }
}

void CALLBACK MIDI_Device::cb_midi_out(HMIDIOUT hMidiout, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  // Wrap on instance.
  MIDI_Device* dev = (MIDI_Device*)dwInstance;
  dev->on_midi_out(hMidiout, wMsg, dwParam1, dwParam2);
  return;
}

void MIDI_Device::on_midi_out(HMIDIOUT hMidiOut, UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
  switch (wMsg) {
    // OUTPUT
    case MOM_OPEN:
      if (debug_level_ >= 2) printf("wMsg=MOM_OPEN\n");
      break;
    case MOM_CLOSE:
      if (debug_level_ >= 2) printf("wMsg=MOM_CLOSE\n");
      break;
      // UNKNOWN
    default:
      if (debug_level_ >= 1) printf("wMsg=%x (%" PRIx64 ", %" PRIx64 ")\n", wMsg, dwParam1, dwParam2);
      break;
  }
}


