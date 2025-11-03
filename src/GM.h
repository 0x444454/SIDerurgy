/// SIDerurgy - Forge your favorite SID.
/// An ASID to MIDI/CV converter.
/// 
/// General MIDI support class.
/// Author: DDT
/// 
/// https://github.com/0x444454/SIDerurgy
/// 

#pragma once
class GM {
public:

  // GM instruments list (starting from 0, not 1.
  enum Instrument {
    // None
    NONE = -1,             // Used for return codes (silence/error).
    // Piano
    Acoustic_Grand_Piano,
    Bright_Acoustic_Piano,
    Electric_Grand_Piano,
    Honky_Tonk_Piano,
    Electric_Piano_1,
    Electric_Piano_2,
    Harpsichord,
    Clavinet,
  
    // Chromatic_Percussion
    Celesta,
    Glockenspiel,
    Music_Box,
    Vibraphone,
    Marimba,
    Xylophone,
    Tubular_Bells,
    Dulcimer,

    // Organ
    Drawbar_Organ,
    Percussive_Organ,
    Rock_Organ,
    Church_Organ,
    Reed_Organ,
    Accordion,
    Harmonica,
    Tango_Accordion,

    // Guitar
    Acoustic_Guitar_nylon,
    Acoustic_Guitar_steel,
    Electric_Guitar_jazz,
    Electric_Guitar_clean,
    Electric_Guitar_muted,
    Overdriven_Guitar,
    Distortion_Guitar,
    Guitar_harmonics,

    // Bass
    Acoustic_Bass,
    Electric_Bass_finger,
    Electric_Bass_pick,
    Fretless_Bass,
    Slap_Bass_1,
    Slap_Bass_2,
    Synth_Bass_1,
    Synth_Bass_2,

    // String
    Violin,
    Viola,
    Cello,
    Contrabass,
    Tremolo_Strings,
    Pizzicato_Strings,
    Orchestral_Harp,
    Timpani,
    String_Ensemble_1,
    String_Ensemble_2,
    Synth_Strings_1,
    Synth_Strings_2,
    Choir_Aahs,
    Voice_Oohs,
    Synth_Voice,
    Orchestra_Hit,

    // Brass
    Trumpet,
    Trombone,
    Tuba,
    Muted_Trumpet,
    French_Horn,
    Brass_Section,
    Synth_Brass_1,
    Synth_Brass_2,

    // Reed
    Soprano_Sax,
    Alto_Sax,
    Tenor_Sax,
    Baritone_Sax,
    Oboe,
    English_Horn,
    Bassoon,
    Clarinet,

    // Pipe
    Piccolo,
    Flute,
    Recorder,
    Pan_Flute,
    Blown_Bottle,
    Shakuhachi,
    Whistle,
    Ocarina,

    // Synth_Lead
    Lead_1_square,
    Lead_2_sawtooth,
    Lead_3_calliope,
    Lead_4_chiff,
    Lead_5_charang,
    Lead_6_voice,
    Lead_7_fifths,
    Lead_8_bass_lead,

    // Synth_Pad
    Pad_1_new_age,
    Pad_2_warm,
    Pad_3_polysynth,
    Pad_4_choir,
    Pad_5_bowed,
    Pad_6_metallic,
    Pad_7_halo,
    Pad_8_sweep,

    // Synth_Effects
    FX_1_rain,
    FX_2_soundtrack,
    FX_3_crystal,
    FX_4_atmosphere,
    FX_5_brightness,
    FX_6_goblins,
    FX_7_echoes,
    FX_8_sci_fi,

    // Ethnic
    Sitar,
    Banjo,
    Shamisen,
    Koto,
    Kalimba,
    Bag_pipe,
    Fiddle,
    Shanai,

    // Percussive
    Tinkle_Bell,
    Agogo,
    Steel_Drums,
    Woodblock,
    Taiko_Drum,
    Melodic_Tom,
    Synth_Drum,

    // Sound_effects
    Reverse_Cymbal,
    Guitar_Fret_Noise,
    Breath_Noise,
    Seashore,
    Bird_Tweet,
    Telephone_Ring,
    Helicopter,
    Applause,
    Gunshot,
  };

public:

};

