//bass instrument - gameover melody
const char bass_gameover[] PROGMEM = {
0,	PC_WAVE,		8,
0,	PC_TREMOLO_RATE,	1,
0,	PC_TREMOLO_LEVEL,	64,
0,	PC_ENV_VOL,		52,
0,	PC_NOTE_HOLD,		0,
1,	PC_ENV_SPEED,		-8,
10,	PATCH_END,		0,
};

//bass instrument - title melody
const char bass_title[] PROGMEM = {
0,	PC_WAVE,		4,
0,	PC_TREMOLO_RATE,	222,
0,	PC_TREMOLO_LEVEL,	128,
0,	PC_ENV_VOL,		33,
0,	PC_NOTE_HOLD,		0,
1,	PC_ENV_SPEED,		-1,
2,	PC_ENV_SPEED,		-2,
0,	PC_WAVE,		8,
14,	PATCH_END,		0,
};

//lead instrument - title melody
const char lead_title[] PROGMEM = {
0,	PC_WAVE,		4,
0,	PC_TREMOLO_RATE,	222,
0,	PC_TREMOLO_LEVEL,	128,
0,	PC_ENV_VOL,		28,
0,	PC_NOTE_HOLD,		0,
1,	PC_ENV_SPEED,		-2,
0,	PC_WAVE,		8,
14,	PATCH_END,		0,
};

// main song - bass drum
const char noise_bass_drum[] PROGMEM = {   
  0, PC_NOISE_PARAMS, 18,
  0, PC_ENV_VOL, 46,
  0, PC_ENV_SPEED, -4,
  1, PC_NOISE_PARAMS, 23,
  1, PC_NOISE_PARAMS, 18,
  1, PC_ENV_SPEED, -12, 
  0, PATCH_END
};

// main song - crash cymbal
const char noise_crash_cymbal[] PROGMEM = {   
  0, PC_NOISE_PARAMS, 2,
  0, PC_ENV_VOL, 34,
  0, PC_ENV_SPEED, -1,
  1, PC_NOISE_PARAMS, 1,
  1, PC_NOISE_PARAMS, 2,
  2, PC_NOISE_PARAMS, 1,
  5, PC_ENV_SPEED, -2,
  64, PATCH_END
};

const struct PatchStruct patches[] PROGMEM = {
{0,0,lead_title,0,0},
{0,0,bass_title,0,0},
{0,0,bass_gameover,0,0},

{1, 0, noise_bass_drum, 0, 0},
{1, 0, noise_crash_cymbal, 0, 0},
};

