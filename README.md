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
I really have fun with this, and I hope you will too !  

## Question: Do I need a modular synthesizer like the one in the picture above (or demo videos) ?

**No. Not at all.**
That is my general purpose polyphonic modular synth, rearranged in a rush to test SIDerurgy. 
An optimized modules setup for SID tunes remastering would be very different.  
The good thing is you only need just a fractions of the modules you see in my synth.  
The bad thing is a modular synthesizer is one of the most expensive hobbies there are.  

**WARNING** - New awesome modules are continuosuly released, and they are expensive. Very expensive.  
**You have been warned.**  

## Question: I have a MIDI synthesizer. Can I use that ?

Sure you can. But this will only work for simple SID tunes.  
Several SID tunes use unique features like Ring Modulation, Hard Sync, and dynamic filtering.  
For this reason, a custom modular synth provides the highest flexibility.  

## Question: I have a GM (General MIDI) keyboard/synth. Can I use that ?

Sure you can. But same as above.  
SIDerurgy will try sending a GM channel program change to match the waveform of each SID voice.  
But keep your expectations low.  Results will depend on your device.  

## How to send a SID tune to SIDerurgy via MIDI (ASID).

Check the demo video, or read this:

1) Install a good loopback MIDI driver, like Tobias Erichsen's loopMIDI (it's free):
https://www.tobias-erichsen.de/software/loopmidi.html

3) Run loopMIDI.

4) Run SIDerurgy specifying loopMIDI (or your other virtual interface) as input:

```bash
SIDerurgy -i loop
```

NOTE: This will use your default MIDI output device.
You can also specify the output device, if needed (this is for my Poly2 device, see below):

```bash
SIDerurgy -i loop -o POLY2
```

**NOTE**: If SIDerurgy prints errors about MIDI devices, close all your other MIDI apps, **including your browser**.

5) Then open your browser and go to DeepSID to access the High Voltage SID Collection:
https://deepsid.chordian.net/

6) Select the SID tune to play.

7) Change DeepSID play mode to "ASID (MIDI)".

8) Wait at least two secs, for DeepSID to "digest" your change. If you rush, you may have problems.

9) Make sure the "MIDI port for ASID" is set to the input device you selected in SIDerurgy using the -i option.


## Question: Why don't you just read a ".sid" file as input ?

Because I am lazy, and I prefer to use DeepSID with its nice UI and stuff.  
Using a .sid file requires integrating a 6502 CPU emulator (e.g. siddump).  
Let's see if people really want this feature.  


## The MIDI to CV module

SIDerurgy converts the SID internal state to MIDI, one voice per channel.  
However, a modular synth only understands CV (Control Voltage).  
So a special module (or other device) is needed to convert MIDI to CV.  

There are several MIDI to CV converters.  
I use a *Polyend Poly2* because it is very flexible. 
If you plan to remaster complex tunes, you definitely a very flexible module.  

## Polyend Poly2 recommended configuration

[TBD]


# LICENSE

Creative Commons, CC BY

https://creativecommons.org/licenses/by/4.0/deed.en

Please add a link to this github project.
