
typedef enum
  {
    NOTE_OFF		    = 0x80, // (nibble)channel, (byte)num, (byte)vel
    NOTE_ON		    = 0x90, // (nibble)channel, (byte)num, (byte)vel
    POLYPHONIC_AFTERTOUCH   = 0xA0, // (nibble)channel, (byte)num, (byte)vel
    MODE_CHANGE		    = 0xB0, // (nibble)channel, (byte)mode,(byte)see below
    CONTROL_CHANGE          = 0xB0, // synonym for MODE_CHANGE
    PROGRAM_CHANGE	    = 0xC0, // (nibble)channel, (byte)mode
    CHANNEL_AFTERTOUCH	    = 0xD0, // (nibble)channel, (byte)vel
    PITCH_WHEEL		    = 0xE0, // (short)pitch
    SYSTEM_EXCLUSIVE	    = 0xF0, //data bytes, described below
    META_EVENT              = 0xFF, //data bytes, described below
  }mkMIDIStatusByte_t;

typedef enum 
  {
    CHANNEL_1  = 0x00,
    CHANNEL_2  = 0x01,
    CHANNEL_3  = 0x02,
    CHANNEL_4  = 0x03,
    CHANNEL_5  = 0x04,
    CHANNEL_6  = 0x05,
    CHANNEL_7  = 0x06,
    CHANNEL_8  = 0x07,
    CHANNEL_9  = 0x08,
    CHANNEL_10 = 0x09,
    CHANNEL_11 = 0x0A,
    CHANNEL_12 = 0x0B,
    CHANNEL_13 = 0x0C,
    CHANNEL_14 = 0x0D,
    CHANNEL_15 = 0x0E,
    CHANNEL_16 = 0x0F,
  }mkMIDIChannel_t;

typedef enum
  {
    MIDI_PITCH_C_NEG1 = 0x00,
    MIDI_PITCH_Cs_NEG1 = 0x01,
    MIDI_PITCH_Db_NEG1 = 0x01,
    MIDI_PITCH_D_NEG1 = 0x02,
    MIDI_PITCH_Ds_NEG1 = 0x03,
    MIDI_PITCH_Eb_NEG1 = 0x03,
    MIDI_PITCH_E_NEG1 = 0x04,
    MIDI_PITCH_F_NEG1 = 0x05,
    MIDI_PITCH_Fs_NEG1 = 0x06,
    MIDI_PITCH_Gb_NEG1 = 0x06,
    MIDI_PITCH_G_NEG1 = 0x07,
    MIDI_PITCH_Gs_NEG1 = 0x08,
    MIDI_PITCH_Ab_NEG1 = 0x08,
    MIDI_PITCH_A_NEG1 = 0x09,
    MIDI_PITCH_As_NEG1 = 0x0A,
    MIDI_PITCH_Bb_NEG1 = 0x0A,
    MIDI_PITCH_BNEG1 = 0x0B,
    
    MIDI_PITCH_C_0 = 0x0C,
    MIDI_PITCH_Cs_0 = 0x0D,
    MIDI_PITCH_Db_0 = 0x0D,
    MIDI_PITCH_D_0 = 0x0E,
    MIDI_PITCH_Ds_0 = 0x0F,
    MIDI_PITCH_Eb_0 = 0x0F,
    MIDI_PITCH_E_0 = 0x10,
    MIDI_PITCH_F_0 = 0x11,
    MIDI_PITCH_Fs_0 = 0x12,
    MIDI_PITCH_Gb_0 = 0x12,
    MIDI_PITCH_G_0 = 0x13,
    MIDI_PITCH_Gs_0 = 0x14,
    MIDI_PITCH_Ab_0 = 0x14,
    MIDI_PITCH_A_0 = 0x15,
    MIDI_PITCH_As_0 = 0x16,
    MIDI_PITCH_Bb_0 = 0x16,
    MIDI_PITCH_B_0 = 0x17,
    
    MIDI_PITCH_C_1 = 0x18,
    MIDI_PITCH_Cs_1 = 0x19,
    MIDI_PITCH_Db_1 = 0x19,
    MIDI_PITCH_D_1 = 0x1A,
    MIDI_PITCH_Ds_1 = 0x1B,
    MIDI_PITCH_Eb_1 = 0x1B,
    MIDI_PITCH_E_1 = 0x1C,
    MIDI_PITCH_F_1 = 0x1D,
    MIDI_PITCH_Fs_1 = 0x1E,
    MIDI_PITCH_Gb_1 = 0x1E,
    MIDI_PITCH_G_1 = 0x1F,
    MIDI_PITCH_Gs_1 = 0x20,
    MIDI_PITCH_Ab_1 = 0x20,
    MIDI_PITCH_A_1 = 0x21,
    MIDI_PITCH_As_1 = 0x22,
    MIDI_PITCH_Bb_1 = 0x22,
    MIDI_PITCH_B_1 = 0x23,    
    
    MIDI_PITCH_C_2 = 0x24,
    MIDI_PITCH_Cs_2 = 0x25,
    MIDI_PITCH_Db_2 = 0x25,
    MIDI_PITCH_D_2 = 0x26,
    MIDI_PITCH_Ds_2 = 0x27,
    MIDI_PITCH_Eb_2 = 0x27,
    MIDI_PITCH_E_2 = 0x28,
    MIDI_PITCH_F_2 = 0x29,
    MIDI_PITCH_Fs_2 = 0x2A,
    MIDI_PITCH_Gb_2 = 0x2A,
    MIDI_PITCH_G_2 = 0x2B,
    MIDI_PITCH_Gs_2 = 0x2C,
    MIDI_PITCH_Ab_2 = 0x2C,
    MIDI_PITCH_A_2 = 0x2D,
    MIDI_PITCH_As_2 = 0x2E,
    MIDI_PITCH_Bb_2 = 0x2E,
    MIDI_PITCH_B_2 = 0x2F,
    
    MIDI_PITCH_C_3 = 0x30,
    MIDI_PITCH_Cs_3 = 0x31,
    MIDI_PITCH_Db_3 = 0x31,
    MIDI_PITCH_D_3 = 0x32,
    MIDI_PITCH_Ds_3 = 0x33,
    MIDI_PITCH_Eb_3 = 0x33,
    MIDI_PITCH_E_3 = 0x34,
    MIDI_PITCH_F_3 = 0x35,
    MIDI_PITCH_Fs_3 = 0x36,
    MIDI_PITCH_Gb_3 = 0x36,
    MIDI_PITCH_G_3 = 0x37,
    MIDI_PITCH_Gs_3 = 0x38,
    MIDI_PITCH_Ab_3 = 0x38,
    MIDI_PITCH_A_3 = 0x39,
    MIDI_PITCH_As_3 = 0x3A,
    MIDI_PITCH_Bb_3 = 0x3A,
    MIDI_PITCH_B_3 = 0x3B,
    
    MIDI_PITCH_C_4 = 0x3C,
    MIDI_PITCH_Cs_4 = 0x3D,
    MIDI_PITCH_Db_4 = 0x3D,
    MIDI_PITCH_D_4 = 0x3E,
    MIDI_PITCH_Ds_4 = 0x3F,
    MIDI_PITCH_Eb_4 = 0x3F,
    MIDI_PITCH_E_4 = 0x40,
    MIDI_PITCH_F_4 = 0x41,
    MIDI_PITCH_Fs_4 = 0x42,
    MIDI_PITCH_Gb_4 = 0x42,
    MIDI_PITCH_G_4 = 0x43,
    MIDI_PITCH_Gs_4 = 0x44,
    MIDI_PITCH_Ab_4 = 0x44,
    MIDI_PITCH_A_4 = 0x45,
    MIDI_PITCH_As_4 = 0x46,
    MIDI_PITCH_Bb_4 = 0x46,
    MIDI_PITCH_B_4 = 0x47,
    
    MIDI_PITCH_C_5 = 0x48,
    MIDI_PITCH_Cs_5 = 0x49,
    MIDI_PITCH_Db_5 = 0x49,
    MIDI_PITCH_D_5 = 0x4A,
    MIDI_PITCH_Ds_5 = 0x4B,
    MIDI_PITCH_Eb_5 = 0x4B,
    MIDI_PITCH_E_5 = 0x4C,
    MIDI_PITCH_F_5 = 0x4D,
    MIDI_PITCH_Fs_5 = 0x4E,
    MIDI_PITCH_Gb_5 = 0x4E,
    MIDI_PITCH_G_5 = 0x4F,
    MIDI_PITCH_Gs_5 = 0x50,
    MIDI_PITCH_Ab_5 = 0x50,
    MIDI_PITCH_A_5 = 0x51,
    MIDI_PITCH_As_5 = 0x52,
    MIDI_PITCH_Bb_5 = 0x52,
    MIDI_PITCH_B_5 = 0x53,
    
    MIDI_PITCH_C_6 = 0x54,
    MIDI_PITCH_Cs_6 = 0x55,
    MIDI_PITCH_Db_6 = 0x55,
    MIDI_PITCH_D_6 = 0x56,
    MIDI_PITCH_Ds_6 = 0x57,
    MIDI_PITCH_Eb_6 = 0x57,
    MIDI_PITCH_E_6 = 0x58,
    MIDI_PITCH_F_6 = 0x59,
    MIDI_PITCH_Fs_6 = 0x5A,
    MIDI_PITCH_Gb_6 = 0x5A,
    MIDI_PITCH_G_6 = 0x5B,
    MIDI_PITCH_Gs_6 = 0x5C,
    MIDI_PITCH_Ab_6 = 0x5C,
    MIDI_PITCH_A_6 = 0x5D,
    MIDI_PITCH_As_6 = 0x5E,
    MIDI_PITCH_Bb_6 = 0x5E,
    MIDI_PITCH_B_6 = 0x5F,
    
    MIDI_PITCH_C_7 = 0x60,
    MIDI_PITCH_Cs_7 = 0x61,
    MIDI_PITCH_Db_7 = 0x61,
    MIDI_PITCH_D_7 = 0x62,
    MIDI_PITCH_Ds_7 = 0x63,
    MIDI_PITCH_Eb_7 = 0x63,
    MIDI_PITCH_E_7 = 0x64,
    MIDI_PITCH_F_7 = 0x65,
    MIDI_PITCH_Fs_7 = 0x66,
    MIDI_PITCH_Gb_7 = 0x66,
    MIDI_PITCH_G_7 = 0x67,
    MIDI_PITCH_Gs_7 = 0x68,
    MIDI_PITCH_Ab_7 = 0x68,
    MIDI_PITCH_A_7 = 0x69,
    MIDI_PITCH_As_7 = 0x6A,
    MIDI_PITCH_Bb_7 = 0x6A,
    MIDI_PITCH_B_7 = 0x6B,
    
    MIDI_PITCH_C_8 = 0x6C,
    MIDI_PITCH_Cs_8 = 0x6D,
    MIDI_PITCH_Db_8 = 0x6D,
    MIDI_PITCH_D_8 = 0x6E,
    MIDI_PITCH_Ds_8 = 0x6F,
    MIDI_PITCH_Eb_8 = 0x6F,
    MIDI_PITCH_E_8 = 0x70,
    MIDI_PITCH_F_8 = 0x71,
    MIDI_PITCH_Fs_8 = 0x72,
    MIDI_PITCH_Gb_8 = 0x72,
    MIDI_PITCH_G_8 = 0x73,
    MIDI_PITCH_Gs_8 = 0x74,
    MIDI_PITCH_Ab_8 = 0x74,
    MIDI_PITCH_A_8 = 0x75,
    MIDI_PITCH_As_8 = 0x76,
    MIDI_PITCH_Bb_8 = 0x76,
    MIDI_PITCH_B_8 = 0x77,
    
    MIDI_PITCH_C_9 = 0x78,
    MIDI_PITCH_Cs_9 = 0x79,
    MIDI_PITCH_Db_9 = 0x79,
    MIDI_PITCH_D_9 = 0x7A,
    MIDI_PITCH_Ds_9 = 0x7B,
    MIDI_PITCH_Eb_9 = 0x7B,
    MIDI_PITCH_E_9 = 0x7C,
    MIDI_PITCH_F_9 = 0x7D,
    MIDI_PITCH_Fs_9 = 0x7E,
    MIDI_PITCH_Gb_9 = 0x7E,
    MIDI_PITCH_G_9 = 0x7F,
  }mkMIDIPitch_t;

typedef enum
  {
    VELOCITY_OFF = 0,
    VELOCITY_PPP = 15,
    VELOCITY_PP = 31,
    VELOCITY_P = 47,
    VELOCITY_MP = 63,
    VELOCITY_MF = 79,
    VELOCITY_F = 95,
    VELOCITY_FF = 111,
    VELOCITY_FFF = 127,
  }mkMIDIVelocity_t;

enum sysExByte
  {
    MTC_QUARTER_FRAME		        = 0xF1,
    SONG_POSITION_POINTER	        = 0xF2,
    SONG_SELECT				= 0xF3,
    //undefined				= 0xF4,
    //undefined				= 0xF5,
    TUNE_REQUEST			= 0xF6, // none
    END_OF_EXCLUSIVE		        = 0xF7, // none
    TIMING_CLOCK			= 0xF8, // none
    //undefined				= 0xF9, 
    START				= 0xFA, // none
    CONTINUE				= 0xFB, // none
    STOP				= 0xFC, // none
    //undefined				= 0xFD,
    ACTIVE_SENSING			= 0xFE, // none
    SYSTEM_RESET			= 0xFF, // none
  };

typedef enum
  {
    BANK_SELECT_MSB			= 0x00,
    MODULATION_WHEEL_MSB		= 0x01,
    BREATH_CONTROLLER_MSB		= 0x02,
    //undefined				= 0x03,
    FOOT_CONTROLLER_MSB			= 0x04,
    PORTAMENTO_TIME_MSB			= 0x05,
    DATA_ENTRY_MSB			= 0x06,
    CHANNEL_VOLUME_MSB			= 0x07,
    BALANCE_MSB				= 0x08,
    //undefined				= 0x09,
    PAN_MSB				= 0x0A,
    EXPRESSION_CONTROLLER_MSB	        = 0x0B,
    EFFECT_CTRL_1_MSB			= 0x0C,
    EFFECT_CTRL_2_MSB			= 0x0D,
    //undefined				= 0x0E,
    //undefined				= 0x0F,
    GENERAL_PURPOSE_CTRL_1_MSB	        = 0x10,
    GENERAL_PURPOSE_CTRL_2_MSB	        = 0x11,
    GENERAL_PURPOSE_CTRL_3_MSB	        = 0x12,
    GENERAL_PURPOSE_CTRL_4_MSB	        = 0x13,
    //undefined				= 0x14,
    //undefined				= 0x15,
    //undefined				= 0x16,
    //undefined				= 0x17,
    //undefined				= 0x18,
    //undefined				= 0x19,
    //undefined				= 0x1A,
    //undefined				= 0x1B,
    //undefined				= 0x1C,
    //undefined				= 0x1D,
    //undefined				= 0x1E,
    //undefined				= 0x1F,
    BANK_SELECT_LSB			= 0x20,
    MODULATION_WHEEL_LSB		= 0x21,
    BREATH_CONTROLLER_LSB		= 0x22,
    //undefined				= 0x23,
    FOOT_CONTROLLER_LSB			= 0x24,
    PORTAMENTO_TIME_LSB			= 0x25,
    DATA_ENTRY_LSB			= 0x26,
    CHANNEL_VOLUME_LSB			= 0x27,
    BALANCE_LSB				= 0x28,
    //undefined				= 0x29,
    PAN_LSB				= 0x2A,
    EXPRESSION_CONTROLLER_LSB	        = 0x2B,
    EFFECT_CTRL_1_LSB			= 0x2C,
    EFFECT_CTRL_2_LSB			= 0x2D,
    //undefined				= 0x2E,
    //undefined				= 0x2F,
    GENERAL_PURPOSE_CTRL_1_LSB	        = 0x30,
    GENERAL_PURPOSE_CTRL_2_LSB	        = 0x31,
    GENERAL_PURPOSE_CTRL_3_LSB	        = 0x32,
    GENERAL_PURPOSE_CTRL_4_LSB	        = 0x33,
    //undefined				= 0x34,
    //undefined				= 0x35,
    //undefined				= 0x36,
    //undefined				= 0x37,
    //undefined				= 0x38,
    //undefined				= 0x39,
    //undefined				= 0x3A,
    //undefined				= 0x3B,
    //undefined				= 0x3C,
    //undefined				= 0x3D,
    //undefined				= 0x3E,
    //undefined				= 0x3F,
    
    /*
      = 0x40,
      = 0x41,
      = 0x42,
      = 0x43,
      = 0x44,
      = 0x45,
      = 0x46,
      = 0x47,
      = 0x48 ,
      = 0x49,
      = 0x4A,
      = 0x4B,
      = 0x4C,
      = 0x4D,
      = 0x4E,
      = 0x4F,
      = 0x50,
      = 0x51,
      = 0x52,
      = 0x53,
      = 0x54,
      = 0x55,
      = 0x56,
      = 0x57,
      = 0x58,
      = 0x59,
      = 0x5A,
      = 0x5B,
      = 0x5C,
      = 0x5D,
      = 0x5E,
      = 0x5F,
    */
    DATA_INCREMENT               = 0x60,
    DATA_DECREMENT               = 0x61,
    NON_REGISTERED_PARAMETER_LSB = 0x62,
    NON_REGISTERED_PARAMETER_MSB = 0x63,
    REGISTERED_PARAMETER_LSB     = 0x64,
    REGISTERED_PARAMETER_MSB     = 0x65,
    /*
      = 0x66,
      = 0x67,
      = 0x68,
      = 0x69,
      = 0x6A,
      = 0x6B,
      = 0x6C,
      = 0x6D,
      = 0x6E,
      = 0x6F,
      = 0x70,
      = 0x71,
      = 0x72,
      = 0x73,
      = 0x74,
      = 0x75,
      = 0x76,
      = 0x77,
      = 0x78,
      = 0x79,
      = 0x7A,
      = 0x7B,
      = 0x7C,
      = 0x7D,
      = 0x7E,
      = 0x7F,  
    */
  }mkMIDIModeChangeByte_t;

typedef enum
  {
    PITCH_BEND_SENSITIVITY  = 0x00,
    CHANNEL_FINE_TUNING     = 0x01,  //resolution 100/8192 cents  
    CHANNEL_COARSE_TUNING   = 0x02, //resolution 100 cents, MSB only
    TUNING_PROGRAM_CHANGE   = 0x03,
    TUNING_BANK_SELECT      = 0x04,
    MODULATION_DEPTH        = 0x05,
    NULL_PARAMATER          = 0x7F7F
  }mkMIDIRegisterdParameter_t;

typedef enum //followed by length of data to follow
{
  SEQUENCE_NUMBER      = 0x00, //length 2
  INSERT_TEXT          = 0x01, //variable length, text
  COPYRIGHT            = 0x02, //variable length, text
  TRACK_NAME           = 0x03, //variable length, text
  INSTRUMENT_NAME      = 0x04, //variable length, text
  INSERT_LYRIC         = 0x05, //variable length, text
  MARKER               = 0x06, //variable length, text
  CUE_POINT            = 0x07, //variable length, text
  
  MIDI_CHANNEL         = 0x20, //length 1, channel
  MIDI_PORT            = 0x21, // length 1, port
  END_OF_TRACK         = 0x2F, // length 0
  TEMPO_CHANGE         = 0x51, //length 3, microsends per quarter
  SMPTE_OFFSET         = 0x54, //length 5, hours, minutes, seconds, frames, subframes
  TIME_SIGNATURE       = 0x58, //length 4, numerator, denominator, clocks/metronome, 32nd notes/quarter note
  KEY_SIGNATURE        = 0x59, //length 2, num accidentals(neg for flat), mode (0=MAJ, 1 = MIN)
}mkMIDIMetaEvent_t;

typedef enum
{
  TRANSPOSE = 0x5B,
  SCALE_CHANGE = 0x5C, // (toggle Major/Minor);
}mkMIDIUnregisteredParameters_t;

