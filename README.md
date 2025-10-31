# SIDerurgy - Forge your favorite SID
## A realtime ASID to MIDI/CV converter

This program is mainly aimed at creating a custom Commodore 64 "SID chip" using a **modular synth** via a MIDI to CV interface.  
However, it also works with **any MIDI device** (each SID voice is sent to a different channel).  
General MIDI is supported, but this will only produce good results with simple SID tunes.  

Watch the video: https://www.youtube.com/watch?v=yzq5XAfxcBw

[![Watch the video](https://img.youtube.com/vi/yzq5XAfxcBw/maxresdefault.jpg)](https://www.youtube.com/watch?v=yzq5XAfxcBw)

## Is it magic ?

No.
You are in charge of remastering your favorite SID tune.  
This is just a tool, the magic is in your brain (and patch ;-)  

I use a Polyend Poly2 to convert SIDerurgy's MIDI stream to CV.  
See my setup below.  

This is still a very alpha version, but enough to have fun.   

## Question: Do I need a modular synthesizer like the one in the picture above ?

**No. Not at all.**
That is my general purpose polyphonic modular synth, rearranged in a rush to test SIDerurgy. 
An optimized modular setup for SID tunes remastering or live performance would be very different.  
The good thing is you only need just a fractions of the modules you see in my synth.  
The bad thing is a modular synthesizer is one of the most expensive hobbies there are.  

**WARNING** - New awesome modules are continuosuly released, and they are expensive. Very expensive.  
**You have been warned.**  

## Question: I have a MIDI synthesizer. Can I use that ?

Sure you can. But it will only work for simple SID tunes.  
Several SID tunes use unique features like Ring Modulation, Hard Sync, and dynamic filtering.  
For this reason, a custom modular synth provides the highest flexibility and the best results.  

## Question: I have a GM (General MIDI) keyboard/synth. Can I use that ?

Yes. But same as above.  
SIDerurgy will try sending GM channel program changes to match the waveform of each SID voice.  
But keep your expectations low.  Results will depend on your device and how well can you hack it.  

## How do I send a SID tune to SIDerurgy via MIDI (ASID) ?

Check the demo video above, and/or read this:

1) Install a good loopback MIDI driver, like Tobias Erichsen's loopMIDI (it's free):
https://www.tobias-erichsen.de/software/loopmidi.html

2) Run loopMIDI.

3) Run SIDerurgy specifying loopMIDI (or your other virtual interface) as input:

```bash
SIDerurgy -i loop
```

NOTE: This will use your default MIDI output device.
You can also specify the output device, if needed. This is my command line to use my Poly2 device:  

```bash
SIDerurgy -i loop -o POLY2
```

**NOTE**: If SIDerurgy prints errors about MIDI devices, close all your other MIDI apps, **including your browser**.

4) Then open your browser and go to DeepSID to access the High Voltage SID Collection:
https://deepsid.chordian.net/

5) Select the SID tune to play.

6) Change DeepSID play mode to "ASID (MIDI)".

7) Wait at least two secs, for DeepSID to "digest" your change. If you rush, you may have problems.

8) Make sure the "MIDI port for ASID" is set to the input device you selected in SIDerurgy using the -i option.


## Question: Why don't you just read a ".sid" file as input ?

Because I am lazy, and I prefer to use DeepSID with its nice UI and features.  
Using a .sid file requires integrating a 6502 CPU emulator (e.g. siddump).  
It can be done, but let's see if people really want it first.  


## The MIDI to CV module (for modular synths)

SIDerurgy maintains the state of a virtual SID chip, and converts it to MIDI in realtime  
Each of the 3 SID voices is mapped to a MIDI channel: 1, 2, 3.  
However, a modular synth only understands CV (Control Voltage).  
So a special module (or other device) is needed to convert MIDI to CV.  

There are several MIDI to CV converters.  
I use a *Polyend Poly2* because it is very flexible. 
If you plan to remaster complex tunes, you need a very flexible converter.  
Note that for live performances, you may just memorize the patch changes you need to do in realtime (e.g. osc sync).  

## Polyend Poly2 recommended configuration

[TBD]


## Question: SID tunes with percussions are tricky.

Indeed.  
A perfectly emulated modular SID with automatic switches for Ring Modulation and Sync (RM/S) should have no problem. However this requires several modules, and RM/S are not yet sent as CC (they will in the next version).  
Another approach is mapping the percussion channel(s) notes to a drum module, or use comparators to split percussions to different modules.  
You may also program a drum machine to respond to drum notes on the channels (voices) used for SID percussions.  
Experiment.  

## SIDerurgy MIDI implementation chart

### MIDI IN

ASID protocol only.

### MIDI OUT


n = MIDI channel hex [0..F], i.e. decimal [1..16] for the user.  

All table values in hex.  

- c = Channel hex [0..F], i.e. Decimal MIDI channel [1..16] for the user.  
- All table values in hex.  

| CC | Byte1 | Byte2 | Description | Currently Implemented ?|
| :---: | :---: | :---: | --- | --- |
| 8c | kk | 40       | Note Off                     | &check; |
| 9c | kk | 40       | Note On                      | &check; |
| Bc | 07 | {00, 7F} | Volume                       | <sup>1</sup> |
| Bc | 15 | [00..7F] | Pulse Width Modulation       | &check; |
| Bc | 47 | [00..7F] | Filter resonance             | &check;<sup>2</sup> |
| Bc | 4A | [00..7F] | Filter cutoff                | &check;<sup>2</sup> |
| Bc | 50 | {00, 7F} | Hard Sync: 0=OFF, 7F=ON      | <sup>3</sup> |
| Bc | 51 | {00, 7F} | Ring Mod: 0=OFF, 7F=ON       | <sup>4</sup> |
| Bc | 78 | 00       | All sounds off               | &check; |
| Bc | 79 | 00       | Reset all controllers        | &check; |
| Bc | 7B | 00       | All notes off                | &check; |
| Cc | pp | 00       | Program Change               | &check;<sup>5</sup> |

Notes:  
1. Volume is global and sent to all voice channels (1,2,3) and channel 4.
2. Filter resonance and cutoff CC are sent on MIDI channel 4 and channels of voices (1,2,3) currently routed through the filter.
3. Hard Sync slaves the voice to a master sync source voice. Masters for voices (1,2,3) are (3,1,2) respectively.
5. Program Change is sent to match the shape of a SID voice to the most similar GM instrument.

<!--
| Bn   | 10 | vv | EG ATTACK vv=[1..7F] | |
| Bn   | 11 | vv | EG DECAY vv=[1..7F] | |
| Bn   | 12 | vv | EG SUSTAIN vv=[1..7F] | |
| Bn   | 13 | vv | EG RELEASE vv=[1..7F] | |
-->



# LICENSE

Creative Commons, CC BY

https://creativecommons.org/licenses/by/4.0/deed.en

Please add a link to this github project.
