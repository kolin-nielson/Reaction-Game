
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

/* ======================= PIN SETUP ======================= */

// PLAYER 1
int P1_BTN[5] = {12, 10, 8, 6, 4}; //BUTTONS
int P1_LED[5] = {13, 11, 9, 7, 5}; //

// PLAYER 1 MATRIX
#define P1_CLK 50
#define P1_CS  48
#define P1_DIN 46

// PLAYER 2
int P2_BTN[5] = {31, 39, 45, 51, 35};   // button 5 is D35
int P2_LED[5] = {33, 41, 47, 53, 23};   // LED 5 is D23

// PLAYER 2 MATRIX
#define P2_CLK 25
#define P2_CS  27
#define P2_DIN 29

// COMMUNITY
#define START_BTN 2
#define SOLO_BTN  1
#define HERO_BTN  43
#define ULTRA_BTN 15
#define ULTRA_TRIG 21
#define ULTRA_ECHO 20
#define SPEAKER   3

/* ======================= MATRICES ======================= */

MD_Parola p1Matrix(MD_MAX72XX::FC16_HW, P1_DIN, P1_CLK, P1_CS, 4);
MD_Parola p2Matrix(MD_MAX72XX::FC16_HW, P2_DIN, P2_CLK, P2_CS, 4);

/* ======================= GAME ======================= */

enum GameState {
  WAITING,
  COUNTDOWN,
  SOLO_PLAYING,
  BONUS_COUNTDOWN,
  BONUS,
  VS_PLAYING,
  VICTORY,
  HERO_STATE,
  HERO_END,
  ULTRA_STATE,
  RHYTHM_SELECT,
  RHYTHM_PLAYING,
  RHYTHM_END
};

enum GameMode {
  MODE_NONE,
  MODE_SOLO,
  MODE_VS
};

GameState gameState = WAITING;
GameMode  gameMode  = MODE_NONE;

enum VictoryType {
  VICTORY_WIN,
  VICTORY_FAIL
};

VictoryType victoryType = VICTORY_WIN;

/* ======================= TIMING ======================= */

const unsigned long DEBOUNCE = 200;
unsigned long lastPress[10] = {0};

unsigned long vsVictoryStart = 0;
bool vsVictoryStarted = false;

unsigned long bonusEndStart = 0;
bool bonusEndAnimating = false;

unsigned long soloVictoryStart = 0;
bool soloVictoryStarted = false;

/* ======================= SCORES ======================= */

int soloScore = 0;
int p1Score = 0;
int p2Score = 0;
int winner = 0;

/* ======================= SOLO MODE ======================= */

unsigned long soloStartTime = 0;
unsigned long bonusStartTime = 0;

const unsigned long SOLO_DURATION  = 30000;
const unsigned long BONUS_DURATION = 10000;

int soloActive[5];
int soloCount = 1;

/* ======================= VS MODE ======================= */

int p1Active = -1;
int p2Active = -1;

/* ======================= HERO MODE ======================= */

const int HERO_MAX_TARGETS = 7;
const int HERO_TOTAL_ROUNDS = 45;
const unsigned long HERO_END_TIME = 3500;

int heroTargets[HERO_MAX_TARGETS];
bool heroHit[HERO_MAX_TARGETS];

int heroRound = 1;
bool heroWon = false;

unsigned long heroRoundStart = 0;
unsigned long heroEndStart = 0;

/* ======================= ULTRA MODE ======================= */

float ultraRecordedDistance = -1.0;
unsigned long ultraStartTime = 0;
const unsigned long ULTRA_SHOW_TIME = 4000;

/* ======================= COUNTDOWN ======================= */

unsigned long cdNext = 0;
int cdBlinkCount = 0;
bool cdOn = false;

/* ======================= MUSIC ======================= */

enum MusicMode { MUSIC_HP, MUSIC_TETRIS, MUSIC_WIN, MUSIC_NONE };
MusicMode musicMode = MUSIC_HP;

unsigned long musicNext = 0;
int musicIndex = 0;

/* ======================= NOTES ======================= */

#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_G4 392
#define NOTE_AS4 466
#define NOTE_A4 440
#define NOTE_F4 349
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_G5 784
#define NOTE_B4 494
#define NOTE_C6 1047
#define NOTE_GS4 415   // G#4 / Ab4
#define NOTE_CS4 277   // C#4 / Db4
#define NOTE_DS5 622   // D#5 / Eb5
#define NOTE_CS5 554   // C#5 / Db5
#define NOTE_F5  698   // F5
#define NOTE_A5  880   // A5
#define NOTE_GS5 831   // G#5 / Ab5
#define REST 0

// Extra note defines for rhythm songs
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D3  147
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS4 370
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B5  988
#define NOTE_FS5 740
#define NOTE_DS4 311
#define NOTE_F6  1397
#define NOTE_E6  1319
#define NOTE_D6  1175
#define NOTE_G6  1568
#define NOTE_AS3 233
#define NOTE_CS6 1109
#define NOTE_AS5 932

/* ======================= MUSIC DATA ======================= */

//  HARRY POTTER

int hpMel[] = {
  REST, NOTE_D4,
  NOTE_G4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_D5,
  NOTE_C5, NOTE_A4,
  NOTE_G4, NOTE_AS4, NOTE_A4,
  NOTE_F4, NOTE_GS4,
  NOTE_D4, NOTE_D4,

  NOTE_G4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_D5,
  NOTE_F5, NOTE_E5,
  NOTE_DS5, NOTE_B4,
  NOTE_DS5, NOTE_D5, NOTE_CS5,
  NOTE_CS4, NOTE_B4,
  NOTE_G4, NOTE_AS4,

  NOTE_D5, NOTE_AS4,
  NOTE_D5, NOTE_AS4,
  NOTE_DS5, NOTE_D5,
  NOTE_CS5, NOTE_A4,
  NOTE_AS4, NOTE_D5, NOTE_CS5,
  NOTE_CS4, NOTE_D4,
  NOTE_D5, REST, NOTE_AS4,

  NOTE_D5, NOTE_AS4,
  NOTE_D5, NOTE_AS4,
  NOTE_F5, NOTE_E5,
  NOTE_DS5, NOTE_B4,
  NOTE_DS5, NOTE_D5, NOTE_CS5,
  NOTE_CS4, NOTE_AS4,
  NOTE_G4
};

int hpDur[] = {
  2, 4,
  4, 8, 4,
  2, 4,
  2, 2,
  4, 8, 4,
  2, 4,
  1, 4,

  4, 8, 4,
  2, 4,
  2, 4,
  2, 4,
  4, 8, 4,
  2, 4,
  1, 4,

  2, 4,
  2, 4,
  2, 4,
  2, 4,
  4, 8, 4,
  2, 4,
  1, 4, 4,

  2, 4,
  2, 4,
  2, 4,
  2, 4,
  4, 8, 4,
  2, 4,
  1
};

const int HP_LEN = sizeof(hpDur) / sizeof(int);

// ======================= TETRIS (FULL SONG) =======================

int tMel[] = {
  NOTE_E5, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_B4,
  NOTE_A4, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5,
  NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5,
  NOTE_C5, NOTE_A4, NOTE_A4, REST,

  NOTE_D5, NOTE_F5, NOTE_A5, NOTE_G5, NOTE_F5,
  NOTE_E5, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5,
  NOTE_B4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5,
  NOTE_C5, NOTE_A4, NOTE_A4, REST,

  NOTE_E5, NOTE_C5,
  NOTE_D5, NOTE_B4,
  NOTE_C5, NOTE_A4,
  NOTE_B4, REST,

  NOTE_E5, NOTE_C5,
  NOTE_D5, NOTE_B4,
  NOTE_C5, NOTE_E5, NOTE_A5,
  NOTE_GS5, REST,

  // repeat / ending
  NOTE_E5, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_B4,
  NOTE_A4, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5,
  NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5,
  NOTE_C5, NOTE_A4, NOTE_A4
};

int tDur[] = {
  4, 8, 8, 4, 8, 8,
  4, 8, 8, 4, 8, 8,
  4, 8, 4, 4,
  4, 4, 4, 4,

  4, 8, 4, 8, 8,
  4, 8, 4, 8, 8,
  4, 8, 8, 4, 4,
  4, 4, 4, 4,

  2, 2,
  2, 2,
  2, 2,
  1, 4,

  2, 2,
  2, 2,
  4, 4, 2,
  1, 4,

  // repeat / ending
  4, 8, 8, 4, 8, 8,
  4, 8, 8, 4, 8, 8,
  4, 8, 4, 4,
  4, 4, 4
};

const int T_LEN = sizeof(tDur) / sizeof(int);

// Victory
int wMel[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
int wDur[] = {4, 4, 4, 2};
const int W_LEN = 4;

/* ======================= RHYTHM GAME DATA ======================= */

struct RhythmNote {
  uint16_t time_ms;
  uint8_t  lane;
  uint16_t freq_hz;
};

// Song 1: Seven Nation Army (Easy) - iconic riff
const RhythmNote PROGMEM song0_notes[] = {
  // Main riff repeats - E E G E D C B
  {0, 2, NOTE_E4}, {400, 2, NOTE_E4}, {800, 3, NOTE_G4}, {1200, 2, NOTE_E4},
  {1600, 1, NOTE_D4}, {2200, 0, NOTE_C4}, {2800, 1, NOTE_B3},
  // Repeat
  {4000, 2, NOTE_E4}, {4400, 2, NOTE_E4}, {4800, 3, NOTE_G4}, {5200, 2, NOTE_E4},
  {5600, 1, NOTE_D4}, {6200, 0, NOTE_C4}, {6800, 1, NOTE_B3},
  // Repeat higher octave feel
  {8000, 3, NOTE_E5}, {8400, 3, NOTE_E5}, {8800, 4, NOTE_G5}, {9200, 3, NOTE_E5},
  {9600, 2, NOTE_D5}, {10200, 1, NOTE_C5}, {10800, 2, NOTE_B4},
  // Back to main
  {12000, 2, NOTE_E4}, {12400, 2, NOTE_E4}, {12800, 3, NOTE_G4}, {13200, 2, NOTE_E4},
  {13600, 1, NOTE_D4}, {14200, 0, NOTE_C4}, {14800, 1, NOTE_B3},
  // Bridge section
  {16000, 0, NOTE_G3}, {16500, 1, NOTE_A3}, {17000, 2, NOTE_B3}, {17500, 3, NOTE_C4},
  {18000, 4, NOTE_D4}, {18500, 3, NOTE_C4}, {19000, 2, NOTE_B3}, {19500, 1, NOTE_A3},
  // Final riffs
  {20500, 2, NOTE_E4}, {20900, 2, NOTE_E4}, {21300, 3, NOTE_G4}, {21700, 2, NOTE_E4},
  {22100, 1, NOTE_D4}, {22700, 0, NOTE_C4}, {23300, 1, NOTE_B3},
  {24500, 2, NOTE_E4}, {24900, 2, NOTE_E4}, {25300, 3, NOTE_G4}, {25700, 2, NOTE_E4},
  {26100, 1, NOTE_D4}, {26700, 0, NOTE_C4}, {27300, 1, NOTE_B3},
};
#define SONG0_LEN 46

// Song 2: Sweet Child O Mine (Medium) - opening riff
const RhythmNote PROGMEM song1_notes[] = {
  // D D A# G A# A G A pattern
  {0, 1, NOTE_D5}, {200, 1, NOTE_D5}, {400, 4, NOTE_A5}, {600, 3, NOTE_G5},
  {800, 4, NOTE_A5}, {1000, 2, NOTE_FS5}, {1200, 3, NOTE_G5}, {1400, 2, NOTE_FS5},
  // Repeat slightly shifted
  {1800, 1, NOTE_E5}, {2000, 1, NOTE_E5}, {2200, 4, NOTE_A5}, {2400, 3, NOTE_G5},
  {2600, 4, NOTE_A5}, {2800, 2, NOTE_FS5}, {3000, 3, NOTE_G5}, {3200, 2, NOTE_FS5},
  // Third round
  {3600, 1, NOTE_G5}, {3800, 1, NOTE_G5}, {4000, 4, NOTE_B5}, {4200, 3, NOTE_A5},
  {4400, 4, NOTE_B5}, {4600, 2, NOTE_A5}, {4800, 3, NOTE_G5}, {5000, 2, NOTE_FS5},
  // Back to D
  {5400, 1, NOTE_D5}, {5600, 1, NOTE_D5}, {5800, 4, NOTE_A5}, {6000, 3, NOTE_G5},
  {6200, 4, NOTE_A5}, {6400, 2, NOTE_FS5}, {6600, 3, NOTE_G5}, {6800, 2, NOTE_FS5},
  // Chorus feel
  {7500, 0, NOTE_D4}, {7800, 1, NOTE_E4}, {8100, 2, NOTE_G4}, {8400, 3, NOTE_A4},
  {8700, 4, NOTE_B4}, {9000, 4, NOTE_D5}, {9300, 3, NOTE_A4}, {9600, 2, NOTE_G4},
  // Second verse riff
  {10200, 1, NOTE_D5}, {10400, 1, NOTE_D5}, {10600, 4, NOTE_A5}, {10800, 3, NOTE_G5},
  {11000, 4, NOTE_A5}, {11200, 2, NOTE_FS5}, {11400, 3, NOTE_G5}, {11600, 2, NOTE_FS5},
  {12000, 1, NOTE_E5}, {12200, 1, NOTE_E5}, {12400, 4, NOTE_A5}, {12600, 3, NOTE_G5},
  {12800, 4, NOTE_A5}, {13000, 2, NOTE_FS5}, {13200, 3, NOTE_G5}, {13400, 2, NOTE_FS5},
  // Ending
  {14000, 0, NOTE_D4}, {14300, 1, NOTE_G4}, {14600, 2, NOTE_A4}, {14900, 3, NOTE_D5},
  {15200, 4, NOTE_G5}, {15600, 3, NOTE_FS5}, {16000, 2, NOTE_E5}, {16400, 1, NOTE_D5},
};
#define SONG1_LEN 60

// Song 3: Enter Sandman (Medium) - main riff
const RhythmNote PROGMEM song2_notes[] = {
  // E5 open, chromatic riff
  {0, 2, NOTE_E4}, {300, 2, NOTE_E4}, {500, 2, NOTE_E4},
  {800, 3, NOTE_G4}, {1000, 4, NOTE_AS4}, {1200, 3, NOTE_G4},
  {1400, 2, NOTE_E4}, {1600, 2, NOTE_E4}, {1800, 2, NOTE_E4},
  {2100, 1, NOTE_D4}, {2300, 1, NOTE_CS4}, {2500, 2, NOTE_E4},
  // Repeat pattern
  {3200, 2, NOTE_E4}, {3500, 2, NOTE_E4}, {3700, 2, NOTE_E4},
  {4000, 3, NOTE_G4}, {4200, 4, NOTE_AS4}, {4400, 3, NOTE_G4},
  {4600, 2, NOTE_E4}, {4800, 2, NOTE_E4}, {5000, 2, NOTE_E4},
  {5300, 1, NOTE_D4}, {5500, 1, NOTE_CS4}, {5700, 2, NOTE_E4},
  // Bridge/verse riff
  {6400, 0, NOTE_B3}, {6700, 1, NOTE_D4}, {7000, 2, NOTE_E4}, {7300, 3, NOTE_G4},
  {7600, 2, NOTE_E4}, {7900, 1, NOTE_D4}, {8200, 0, NOTE_B3}, {8600, 0, NOTE_B3},
  // Heavy part
  {9200, 4, NOTE_E5}, {9400, 3, NOTE_D5}, {9600, 2, NOTE_B4}, {9800, 3, NOTE_D5},
  {10000, 4, NOTE_E5}, {10300, 4, NOTE_E5}, {10600, 3, NOTE_D5}, {10900, 2, NOTE_B4},
  // End section
  {11500, 2, NOTE_E4}, {11700, 2, NOTE_E4}, {11900, 3, NOTE_G4},
  {12200, 4, NOTE_AS4}, {12400, 3, NOTE_G4}, {12700, 2, NOTE_E4},
  {13000, 1, NOTE_D4}, {13300, 0, NOTE_B3}, {13600, 2, NOTE_E4},
  {14000, 2, NOTE_E4}, {14300, 3, NOTE_G4}, {14600, 2, NOTE_E4},
  {15000, 1, NOTE_D4}, {15400, 2, NOTE_E4},
};
#define SONG2_LEN 52

// Song 4: Crazy Train (Med-Hard) - iconic opening riff
const RhythmNote PROGMEM song3_notes[] = {
  // FS FS FS A FS E FS pattern
  {0, 2, NOTE_FS4}, {150, 2, NOTE_FS4}, {300, 2, NOTE_FS4},
  {450, 3, NOTE_A4}, {600, 2, NOTE_FS4}, {750, 1, NOTE_E4},
  {900, 2, NOTE_FS4}, {1100, 2, NOTE_FS4}, {1300, 2, NOTE_FS4},
  {1450, 3, NOTE_A4}, {1600, 2, NOTE_FS4}, {1750, 4, NOTE_B4},
  {1900, 3, NOTE_A4}, {2100, 2, NOTE_FS4}, {2300, 1, NOTE_E4},
  // Second round faster
  {2700, 2, NOTE_FS4}, {2850, 2, NOTE_FS4}, {3000, 2, NOTE_FS4},
  {3150, 3, NOTE_A4}, {3300, 2, NOTE_FS4}, {3450, 1, NOTE_E4},
  {3600, 2, NOTE_FS4}, {3800, 2, NOTE_FS4}, {4000, 2, NOTE_FS4},
  {4150, 3, NOTE_A4}, {4300, 2, NOTE_FS4}, {4450, 4, NOTE_B4},
  {4600, 3, NOTE_A4}, {4800, 2, NOTE_FS4}, {5000, 1, NOTE_E4},
  // Verse riff
  {5500, 0, NOTE_D4}, {5700, 1, NOTE_E4}, {5900, 2, NOTE_FS4},
  {6100, 3, NOTE_A4}, {6300, 4, NOTE_B4}, {6500, 3, NOTE_A4},
  {6700, 2, NOTE_FS4}, {6900, 1, NOTE_E4},
  // Bridge
  {7300, 4, NOTE_D5}, {7500, 3, NOTE_CS5}, {7700, 2, NOTE_A4},
  {7900, 1, NOTE_E4}, {8100, 2, NOTE_FS4}, {8300, 3, NOTE_A4},
  {8500, 4, NOTE_CS5}, {8700, 4, NOTE_D5},
  // Repeat main riff
  {9200, 2, NOTE_FS4}, {9350, 2, NOTE_FS4}, {9500, 2, NOTE_FS4},
  {9650, 3, NOTE_A4}, {9800, 2, NOTE_FS4}, {9950, 1, NOTE_E4},
  {10100, 2, NOTE_FS4}, {10300, 2, NOTE_FS4}, {10500, 2, NOTE_FS4},
  {10650, 3, NOTE_A4}, {10800, 2, NOTE_FS4}, {10950, 4, NOTE_B4},
  {11100, 3, NOTE_A4}, {11300, 2, NOTE_FS4}, {11500, 1, NOTE_E4},
  // Final run
  {12000, 0, NOTE_D4}, {12150, 1, NOTE_E4}, {12300, 2, NOTE_FS4},
  {12450, 3, NOTE_A4}, {12600, 4, NOTE_B4}, {12800, 4, NOTE_D5},
  {13000, 3, NOTE_CS5}, {13200, 2, NOTE_A4}, {13400, 1, NOTE_FS4},
  {13600, 0, NOTE_E4}, {13800, 1, NOTE_FS4}, {14000, 2, NOTE_A4},
};
#define SONG3_LEN 72

// Song 5: Thunderstruck (Hard) - fast alternate picking
const RhythmNote PROGMEM song4_notes[] = {
  // Fast B string hammer-on pattern
  {0, 4, NOTE_B4}, {150, 3, NOTE_A4}, {300, 4, NOTE_B4}, {450, 2, NOTE_FS4},
  {600, 4, NOTE_B4}, {750, 1, NOTE_E4}, {900, 4, NOTE_B4}, {1050, 2, NOTE_FS4},
  {1200, 4, NOTE_B4}, {1350, 3, NOTE_A4}, {1500, 4, NOTE_B4}, {1650, 3, NOTE_A4},
  {1800, 4, NOTE_B4}, {1950, 2, NOTE_FS4}, {2100, 4, NOTE_B4}, {2250, 1, NOTE_E4},
  // Second phrase
  {2500, 4, NOTE_B4}, {2650, 3, NOTE_A4}, {2800, 4, NOTE_B4}, {2950, 2, NOTE_FS4},
  {3100, 4, NOTE_B4}, {3250, 1, NOTE_E4}, {3400, 4, NOTE_B4}, {3550, 0, NOTE_D4},
  {3700, 4, NOTE_B4}, {3850, 1, NOTE_E4}, {4000, 4, NOTE_B4}, {4150, 2, NOTE_FS4},
  {4300, 4, NOTE_B4}, {4450, 3, NOTE_A4}, {4600, 4, NOTE_B4}, {4750, 2, NOTE_FS4},
  // Power chord section
  {5200, 0, NOTE_E4}, {5400, 0, NOTE_E4}, {5600, 2, NOTE_A4}, {5800, 2, NOTE_A4},
  {6000, 4, NOTE_B4}, {6200, 4, NOTE_B4}, {6400, 2, NOTE_A4}, {6600, 0, NOTE_E4},
  // Resume fast picking
  {7000, 4, NOTE_B4}, {7130, 3, NOTE_A4}, {7260, 4, NOTE_B4}, {7390, 2, NOTE_FS4},
  {7520, 4, NOTE_B4}, {7650, 1, NOTE_E4}, {7780, 4, NOTE_B4}, {7910, 2, NOTE_FS4},
  {8040, 4, NOTE_B4}, {8170, 3, NOTE_A4}, {8300, 4, NOTE_B4}, {8430, 3, NOTE_A4},
  {8560, 4, NOTE_B4}, {8690, 2, NOTE_FS4}, {8820, 4, NOTE_B4}, {8950, 1, NOTE_E4},
  // Climax
  {9300, 0, NOTE_E4}, {9450, 1, NOTE_FS4}, {9600, 2, NOTE_A4}, {9750, 3, NOTE_B4},
  {9900, 4, NOTE_D5}, {10050, 4, NOTE_E5}, {10200, 3, NOTE_D5}, {10350, 2, NOTE_B4},
  {10500, 1, NOTE_A4}, {10650, 0, NOTE_E4}, {10800, 1, NOTE_FS4}, {10950, 2, NOTE_A4},
  {11100, 3, NOTE_B4}, {11250, 4, NOTE_D5}, {11400, 4, NOTE_E5}, {11600, 3, NOTE_B4},
  // End
  {12000, 4, NOTE_B4}, {12150, 3, NOTE_A4}, {12300, 4, NOTE_B4}, {12450, 2, NOTE_FS4},
  {12600, 4, NOTE_B4}, {12750, 1, NOTE_E4}, {12900, 4, NOTE_B4}, {13100, 0, NOTE_D4},
  {13300, 2, NOTE_E4}, {13500, 4, NOTE_B4},
};
#define SONG4_LEN 78

// Song 6: Through the Fire and Flames (Extreme) - speed metal
const RhythmNote PROGMEM song5_notes[] = {
  // Ultra fast intro hammer-ons
  {0, 2, NOTE_E5}, {100, 3, NOTE_F5}, {200, 4, NOTE_G5}, {300, 3, NOTE_F5},
  {400, 2, NOTE_E5}, {500, 1, NOTE_D5}, {600, 0, NOTE_C5}, {700, 1, NOTE_D5},
  {800, 2, NOTE_E5}, {900, 3, NOTE_F5}, {1000, 4, NOTE_G5}, {1100, 4, NOTE_A5},
  {1200, 3, NOTE_G5}, {1300, 2, NOTE_F5}, {1400, 1, NOTE_E5}, {1500, 0, NOTE_D5},
  // Second fast run
  {1700, 0, NOTE_C5}, {1800, 1, NOTE_D5}, {1900, 2, NOTE_E5}, {2000, 3, NOTE_G5},
  {2100, 4, NOTE_A5}, {2200, 3, NOTE_G5}, {2300, 2, NOTE_E5}, {2400, 1, NOTE_D5},
  {2500, 0, NOTE_C5}, {2600, 1, NOTE_E5}, {2700, 2, NOTE_G5}, {2800, 4, NOTE_B5},
  {2900, 3, NOTE_A5}, {3000, 2, NOTE_G5}, {3100, 1, NOTE_E5}, {3200, 0, NOTE_C5},
  // Power section
  {3500, 2, NOTE_E4}, {3600, 2, NOTE_E4}, {3700, 3, NOTE_G4}, {3800, 4, NOTE_A4},
  {3900, 4, NOTE_B4}, {4000, 3, NOTE_A4}, {4100, 2, NOTE_G4}, {4200, 1, NOTE_E4},
  {4400, 0, NOTE_D4}, {4500, 1, NOTE_E4}, {4600, 2, NOTE_G4}, {4700, 3, NOTE_A4},
  {4800, 4, NOTE_B4}, {4900, 4, NOTE_D5}, {5000, 3, NOTE_B4}, {5100, 2, NOTE_A4},
  // Solo section - crazy fast
  {5400, 4, NOTE_E5}, {5500, 3, NOTE_D5}, {5600, 2, NOTE_C5}, {5700, 1, NOTE_B4},
  {5800, 0, NOTE_A4}, {5900, 1, NOTE_B4}, {6000, 2, NOTE_C5}, {6100, 3, NOTE_D5},
  {6200, 4, NOTE_E5}, {6280, 4, NOTE_F5}, {6360, 3, NOTE_E5}, {6440, 2, NOTE_D5},
  {6520, 1, NOTE_C5}, {6600, 0, NOTE_B4}, {6700, 1, NOTE_C5}, {6800, 2, NOTE_D5},
  {6900, 3, NOTE_E5}, {7000, 4, NOTE_G5}, {7100, 3, NOTE_F5}, {7200, 2, NOTE_E5},
  // Sweeping arpeggios
  {7500, 0, NOTE_C4}, {7580, 1, NOTE_E4}, {7660, 2, NOTE_G4}, {7740, 3, NOTE_C5},
  {7820, 4, NOTE_E5}, {7900, 3, NOTE_C5}, {7980, 2, NOTE_G4}, {8060, 1, NOTE_E4},
  {8200, 0, NOTE_D4}, {8280, 1, NOTE_FS4}, {8360, 2, NOTE_A4}, {8440, 3, NOTE_D5},
  {8520, 4, NOTE_FS5}, {8600, 3, NOTE_D5}, {8680, 2, NOTE_A4}, {8760, 1, NOTE_FS4},
  // Final blitz
  {9000, 2, NOTE_E5}, {9080, 3, NOTE_F5}, {9160, 4, NOTE_G5}, {9240, 4, NOTE_A5},
  {9320, 3, NOTE_G5}, {9400, 2, NOTE_E5}, {9480, 1, NOTE_D5}, {9560, 0, NOTE_C5},
  {9700, 2, NOTE_E5}, {9800, 4, NOTE_G5}, {9900, 2, NOTE_E5}, {10000, 0, NOTE_C5},
};
#define SONG5_LEN 88

// Song 7: Free Bird (Hard) - solo section
const RhythmNote PROGMEM song6_notes[] = {
  // Opening gentle part
  {0, 2, NOTE_G4}, {500, 3, NOTE_A4}, {1000, 4, NOTE_B4}, {1500, 3, NOTE_A4},
  {2000, 2, NOTE_G4}, {2500, 1, NOTE_E4}, {3000, 2, NOTE_G4}, {3500, 3, NOTE_A4},
  // Solo begins - picking up speed
  {4000, 2, NOTE_G5}, {4200, 3, NOTE_A5}, {4400, 4, NOTE_B5}, {4600, 3, NOTE_A5},
  {4800, 2, NOTE_G5}, {5000, 1, NOTE_E5}, {5200, 2, NOTE_G5}, {5400, 4, NOTE_B5},
  // Fast pentatonic runs
  {5800, 0, NOTE_D5}, {5950, 1, NOTE_E5}, {6100, 2, NOTE_G5}, {6250, 3, NOTE_A5},
  {6400, 4, NOTE_B5}, {6550, 3, NOTE_A5}, {6700, 2, NOTE_G5}, {6850, 1, NOTE_E5},
  {7000, 0, NOTE_D5}, {7150, 1, NOTE_E5}, {7300, 2, NOTE_G5}, {7500, 4, NOTE_B5},
  // Double-time section
  {7800, 4, NOTE_B5}, {7950, 3, NOTE_A5}, {8100, 2, NOTE_G5}, {8250, 1, NOTE_E5},
  {8400, 0, NOTE_D5}, {8550, 1, NOTE_E5}, {8700, 2, NOTE_G5}, {8850, 3, NOTE_A5},
  {9000, 4, NOTE_B5}, {9130, 4, NOTE_D6}, {9260, 3, NOTE_B5}, {9390, 2, NOTE_A5},
  {9520, 1, NOTE_G5}, {9650, 0, NOTE_E5}, {9800, 1, NOTE_G5}, {9950, 2, NOTE_A5},
  // Climax runs
  {10200, 0, NOTE_E5}, {10320, 1, NOTE_G5}, {10440, 2, NOTE_A5}, {10560, 3, NOTE_B5},
  {10680, 4, NOTE_D6}, {10800, 3, NOTE_B5}, {10920, 2, NOTE_A5}, {11040, 1, NOTE_G5},
  {11200, 0, NOTE_E5}, {11350, 1, NOTE_G5}, {11500, 2, NOTE_A5}, {11650, 3, NOTE_B5},
  {11800, 4, NOTE_D6}, {11950, 4, NOTE_E6}, {12100, 3, NOTE_D6}, {12250, 2, NOTE_B5},
  // Ending phrase
  {12500, 4, NOTE_G5}, {12700, 3, NOTE_E5}, {12900, 2, NOTE_D5}, {13100, 1, NOTE_B4},
  {13300, 0, NOTE_G4}, {13600, 1, NOTE_A4}, {13900, 2, NOTE_B4}, {14200, 3, NOTE_D5},
  {14500, 4, NOTE_G5}, {14800, 2, NOTE_E5}, {15100, 0, NOTE_D5}, {15500, 2, NOTE_G4},
};
#define SONG6_LEN 68

// Song metadata
const uint8_t NUM_SONGS = 7;

const uint8_t songLengths[] PROGMEM = {
  SONG0_LEN, SONG1_LEN, SONG2_LEN, SONG3_LEN,
  SONG4_LEN, SONG5_LEN, SONG6_LEN
};

// Pointers to song arrays (stored in a regular array of PROGMEM pointers)
const RhythmNote* const songData[] PROGMEM = {
  song0_notes, song1_notes, song2_notes, song3_notes,
  song4_notes, song5_notes, song6_notes
};

// Song names in PROGMEM
const char sName0[] PROGMEM = "7 Nation Army";
const char sName1[] PROGMEM = "Sweet Child";
const char sName2[] PROGMEM = "Enter Sandman";
const char sName3[] PROGMEM = "Crazy Train";
const char sName4[] PROGMEM = "Thunderstruck";
const char sName5[] PROGMEM = "Fire & Flames";
const char sName6[] PROGMEM = "Free Bird";

const char* const songNames[] PROGMEM = {
  sName0, sName1, sName2, sName3, sName4, sName5, sName6
};

// Difficulty strings in PROGMEM
const char sDiff0[] PROGMEM = "Easy";
const char sDiff1[] PROGMEM = "Medium";
const char sDiff2[] PROGMEM = "Medium";
const char sDiff3[] PROGMEM = "Med-Hard";
const char sDiff4[] PROGMEM = "Hard";
const char sDiff5[] PROGMEM = "Extreme";
const char sDiff6[] PROGMEM = "Hard";

const char* const songDiffs[] PROGMEM = {
  sDiff0, sDiff1, sDiff2, sDiff3, sDiff4, sDiff5, sDiff6
};

/* ======================= RHYTHM GAME STATE ======================= */

const unsigned long RHYTHM_DEBOUNCE = 120;
unsigned long rhythmLastPress[5] = {0};

// Current song state
uint8_t  rhythmSongIdx = 0;
uint8_t  rhythmNoteCount = 0;
uint16_t rhythmSongLen_ms = 0;  // total song duration

// Playback
unsigned long rhythmStartTime = 0;
uint8_t  rhythmNextNote = 0;       // next note index to auto-play
uint8_t  rhythmNextHitNote = 0;    // earliest note not yet judged

// Scoring
unsigned long rhythmScore = 0;
uint16_t rhythmCombo = 0;
uint16_t rhythmMaxCombo = 0;
uint16_t rhythmPerfects = 0;
uint16_t rhythmGreats = 0;
uint16_t rhythmGoods = 0;
uint16_t rhythmMisses = 0;

// Display
unsigned long rhythmLastFrame = 0;
const unsigned long RHYTHM_FRAME_MS = 33; // ~30fps
const int HIGHWAY_COLS = 32;
const uint8_t laneRows[5] = {1, 3, 4, 5, 7}; // row mapping for 5 lanes
const unsigned long HIGHWAY_WINDOW_MS = 3000; // notes visible 3s ahead

// Hit feedback
unsigned long rhythmHitFlash[5] = {0};
uint8_t rhythmHitType[5] = {0}; // 0=none,1=perfect,2=great,3=good,4=miss

// Auto-play sound
unsigned long rhythmToneEnd = 0;
bool rhythmToneOverride = false;
unsigned long rhythmOverrideEnd = 0;

// Song select
unsigned long rhythmSelectTime = 0;
unsigned long rhythmPreviewStart = 0;
uint8_t rhythmPreviewIdx = 0;
bool rhythmPreviewPlaying = false;
unsigned long rhythmPreviewNext = 0;
unsigned long rhythmPulseTime = 0;

// Results
unsigned long rhythmEndTime = 0;

// Temp buffer for reading PROGMEM song names
char rhythmBuf[20];
char rhythmBuf2[20];

/* ======================= HELPERS ======================= */

void startBonusEndAnimation() {
  p1Matrix.displayClear();
  p2Matrix.displayClear();

  p1Matrix.displayScroll("FINAL SCORE", PA_CENTER, PA_SCROLL_LEFT, 80);
  p2Matrix.displayScroll("FINAL SCORE", PA_CENTER, PA_SCROLL_LEFT, 80);

  bonusEndStart = millis();
  bonusEndAnimating = true;
}

void startVSVictoryAnimation() {
  p1Matrix.displayClear();
  p2Matrix.displayClear();

  if (winner == 1) {
    p1Matrix.displayScroll("WINNER", PA_CENTER, PA_SCROLL_LEFT, 80);
    p2Matrix.displayScroll("LOSER",  PA_CENTER, PA_SCROLL_LEFT, 80);
  } else {
    p2Matrix.displayScroll("WINNER", PA_CENTER, PA_SCROLL_LEFT, 80);
    p1Matrix.displayScroll("LOSER",  PA_CENTER, PA_SCROLL_LEFT, 80);
  }

  vsVictoryStart = millis();
  vsVictoryStarted = true;
}

void allLEDsOff() {
  for (int i = 0; i < 5; i++) digitalWrite(P1_LED[i], LOW);
  for (int i = 0; i < 5; i++) digitalWrite(P2_LED[i], LOW);
}

int randomNoRepeat(int prev, int maxV) {
  int r;
  do { r = random(0, maxV); } while (r == prev);
  return r;
}

/* ======================= VS SPAWN ======================= */

void spawnVS_P1() {
  for (int i = 0; i < 5; i++) digitalWrite(P1_LED[i], LOW);
  p1Active = randomNoRepeat(p1Active, 5);
  digitalWrite(P1_LED[p1Active], HIGH);
}

void spawnVS_P2() {
  for (int i = 0; i < 5; i++) digitalWrite(P2_LED[i], LOW);
  p2Active = randomNoRepeat(p2Active, 5);
  digitalWrite(P2_LED[p2Active], HIGH);
}

/* ======================= MUSIC ENGINE ======================= */

void updateMusic() {
  if (millis() < musicNext) return;

  int note = REST, dur = 4;

  if (musicMode == MUSIC_HP) {
    note = hpMel[musicIndex];
    dur  = hpDur[musicIndex];
    musicIndex = (musicIndex + 1) % HP_LEN;
  }
  else if (musicMode == MUSIC_TETRIS) {
    note = tMel[musicIndex];
    dur  = tDur[musicIndex];
    musicIndex = (musicIndex + 1) % T_LEN;
  }
  else if (musicMode == MUSIC_WIN) {
    if (musicIndex >= W_LEN) {
      musicMode = MUSIC_NONE;
      return;
    }
    note = wMel[musicIndex];
    dur  = wDur[musicIndex++];
  }

  int len = 1000 / dur;
  if (note == REST) noTone(SPEAKER);
  else tone(SPEAKER, note, len * 0.9);

  musicNext = millis() + len * 1.3;
}

bool isSoloActive(int idx) {
  for (int i = 0; i < soloCount; i++) {
    if (soloActive[i] == idx) return true;
  }
  return false;
}

int randomSoloLED() {
  int r;
  do {
    r = random(0, 10);   // 0-4 P1, 5-9 P2
  } while (isSoloActive(r));
  return r;
}

void turnOn(int idx) {
  if (idx < 5) digitalWrite(P1_LED[idx], HIGH);
  else         digitalWrite(P2_LED[idx - 5], HIGH);
}

void turnOff(int idx) {
  if (idx < 5) digitalWrite(P1_LED[idx], LOW);
  else         digitalWrite(P2_LED[idx - 5], LOW);
}

void spawnSolo(int count) {
  allLEDsOff();
  soloCount = count;

  for (int i = 0; i < count; i++) {
    soloActive[i] = randomSoloLED();
    turnOn(soloActive[i]);
  }
}

void replaceSolo(int slot) {
  turnOff(soloActive[slot]);
  soloActive[slot] = randomSoloLED();
  turnOn(soloActive[slot]);
}

void setPlayer2Normal() {
  p2Matrix.setZoneEffect(0, false, PA_FLIP_UD);
  p2Matrix.setZoneEffect(0, false, PA_FLIP_LR);
}

void setPlayer2Rotated() {
  p2Matrix.setZoneEffect(0, true, PA_FLIP_UD);
  p2Matrix.setZoneEffect(0, true, PA_FLIP_LR);
}

/* ======================= HERO HELPERS ======================= */

int heroButtonsThisRound() {
  if (heroRound <= 10) return 3;
  if (heroRound <= 15) return 4;
  if (heroRound <= 20) return 5;
  if (heroRound <= 25) return 6;
  return 7;
}

unsigned long heroRoundTimeMs() {
  if (heroRound <= 30) return 3000;
  if (heroRound <= 35) return 2000;
  if (heroRound <= 40) return 1500;
  return 1000;
}

bool heroContainsCurrentRound(int idx) {
  int targetCount = heroButtonsThisRound();
  for (int i = 0; i < targetCount; i++) {
    if (heroTargets[i] == idx) return true;
  }
  return false;
}

int heroFindTargetSlot(int idx) {
  int targetCount = heroButtonsThisRound();
  for (int i = 0; i < targetCount; i++) {
    if (heroTargets[i] == idx) return i;
  }
  return -1;
}

bool heroRoundCleared() {
  int targetCount = heroButtonsThisRound();
  for (int i = 0; i < targetCount; i++) {
    if (!heroHit[i]) return false;
  }
  return true;
}

void showHeroStatus(float timeLeft) {
  if (timeLeft < 0) timeLeft = 0;

  char tBuf[6];
  dtostrf(timeLeft, 4, 1, tBuf);

  char rBuf[8];
  sprintf(rBuf, "R%d", heroRound);

  p1Matrix.displayClear();
  p2Matrix.displayClear();

  p1Matrix.displayText(tBuf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  p2Matrix.displayText(rBuf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
}

void startHeroRound() {
  allLEDsOff();

  int targetCount = heroButtonsThisRound();

  for (int i = 0; i < HERO_MAX_TARGETS; i++) {
    heroTargets[i] = -1;
    heroHit[i] = false;
  }

  for (int i = 0; i < targetCount; i++) {
    int r;
    do {
      r = random(0, 10);
    } while (heroContainsCurrentRound(r));

    heroTargets[i] = r;
    turnOn(r);
  }

  heroRoundStart = millis();
}

void startHeroGame() {
  gameMode = MODE_NONE;
  musicMode = MUSIC_NONE;
  noTone(SPEAKER);

  heroRound = 1;
  heroWon = false;

  setPlayer2Normal();
  startHeroRound();
  gameState = HERO_STATE;
}

void startHeroEnd(bool wonGame) {
  heroWon = wonGame;
  heroEndStart = millis();

  allLEDsOff();
  p1Matrix.displayClear();
  p2Matrix.displayClear();

  if (heroWon) {
    p1Matrix.displayScroll("YOU WIN", PA_CENTER, PA_SCROLL_LEFT, 80);
    p2Matrix.displayScroll("45 ROUNDS", PA_CENTER, PA_SCROLL_LEFT, 80);
  } else {
    p1Matrix.displayScroll("TIME UP", PA_CENTER, PA_SCROLL_LEFT, 80);

    char rBuf[12];
    sprintf(rBuf, "ROUND %d", heroRound);
    p2Matrix.displayScroll(rBuf, PA_CENTER, PA_SCROLL_LEFT, 80);
  }

  gameState = HERO_END;
}

void updateHeroInputs() {
  for (int i = 0; i < 10; i++) {
    bool pressed = (i < 5) ? !digitalRead(P1_BTN[i])
                           : !digitalRead(P2_BTN[i - 5]);

    if (pressed && millis() - lastPress[i] > DEBOUNCE) {
      lastPress[i] = millis();

      int slot = heroFindTargetSlot(i);
      if (slot >= 0 && !heroHit[slot]) {
        heroHit[slot] = true;
        turnOff(i);
        tone(SPEAKER, 1200, 60);
      }
    }
  }
}

/*ULTRA HELPERS*/

float readUltraCM() {
  digitalWrite(ULTRA_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRA_TRIG, LOW);

  unsigned long duration = pulseIn(ULTRA_ECHO, HIGH, 30000);
  if (duration == 0) return -1.0;

  return duration * 0.0343 / 2.0;
}

void startUltraState() {
  gameMode = MODE_NONE;
  musicMode = MUSIC_NONE;
  noTone(SPEAKER);
  allLEDsOff();
  setPlayer2Normal();

  ultraRecordedDistance = readUltraCM();
  ultraStartTime = millis();

  if (ultraRecordedDistance >= 0) tone(SPEAKER, 1400, 150);
  else tone(SPEAKER, 180, 250);

  gameState = ULTRA_STATE;
}

void showUltraDistance() {
  p1Matrix.displayClear();
  p2Matrix.displayClear();

  if (ultraRecordedDistance < 0) {
    p1Matrix.displayText("NO", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    p2Matrix.displayText("READ", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    return;
  }

  char distBuf[10];
  dtostrf(ultraRecordedDistance, 4, 1, distBuf);

  p1Matrix.displayText("DIST", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  p2Matrix.displayText(distBuf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
}

/* ======================= RHYTHM HELPERS ======================= */

// Read a single note from PROGMEM
RhythmNote rhythmReadNote(uint8_t songIdx, uint8_t noteIdx) {
  RhythmNote n;
  const RhythmNote* ptr = (const RhythmNote*)pgm_read_ptr(&songData[songIdx]);
  memcpy_P(&n, &ptr[noteIdx], sizeof(RhythmNote));
  return n;
}

uint8_t rhythmGetSongLen(uint8_t idx) {
  return pgm_read_byte(&songLengths[idx]);
}

void rhythmGetSongName(uint8_t idx, char* buf, uint8_t bufLen) {
  const char* ptr = (const char*)pgm_read_ptr(&songNames[idx]);
  strncpy_P(buf, ptr, bufLen - 1);
  buf[bufLen - 1] = '\0';
}

void rhythmGetDiffName(uint8_t idx, char* buf, uint8_t bufLen) {
  const char* ptr = (const char*)pgm_read_ptr(&songDiffs[idx]);
  strncpy_P(buf, ptr, bufLen - 1);
  buf[bufLen - 1] = '\0';
}

uint8_t rhythmGetMultiplier() {
  if (rhythmCombo >= 50) return 4;
  if (rhythmCombo >= 25) return 3;
  if (rhythmCombo >= 10) return 2;
  return 1;
}

void rhythmSetLED(uint8_t lane, uint8_t brightness) {
  int pin = P1_LED[lane];
  // Pin 7 is not PWM capable on Mega
  if (pin == 7) {
    digitalWrite(pin, brightness > 127 ? HIGH : LOW);
  } else {
    analogWrite(pin, brightness);
  }
}

void startRhythmSelect() {
  gameMode = MODE_NONE;
  musicMode = MUSIC_NONE;
  noTone(SPEAKER);
  allLEDsOff();
  setPlayer2Normal();

  rhythmSongIdx = 0;
  rhythmSelectTime = millis();
  rhythmPreviewPlaying = false;
  rhythmPreviewStart = millis();
  rhythmPulseTime = millis();

  // Show first song
  rhythmGetSongName(0, rhythmBuf, sizeof(rhythmBuf));
  rhythmGetDiffName(0, rhythmBuf2, sizeof(rhythmBuf2));

  p1Matrix.displayClear();
  p2Matrix.displayClear();
  p1Matrix.displayScroll(rhythmBuf, PA_CENTER, PA_SCROLL_LEFT, 60);
  p2Matrix.displayText(rhythmBuf2, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  gameState = RHYTHM_SELECT;
}

void rhythmSwitchSong() {
  noTone(SPEAKER);
  rhythmPreviewPlaying = false;
  rhythmPreviewStart = millis();

  rhythmGetSongName(rhythmSongIdx, rhythmBuf, sizeof(rhythmBuf));
  rhythmGetDiffName(rhythmSongIdx, rhythmBuf2, sizeof(rhythmBuf2));

  p1Matrix.displayClear();
  p2Matrix.displayClear();
  p1Matrix.displayScroll(rhythmBuf, PA_CENTER, PA_SCROLL_LEFT, 60);
  p2Matrix.displayText(rhythmBuf2, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
}

void startRhythmPlaying() {
  uint8_t len = rhythmGetSongLen(rhythmSongIdx);
  rhythmNoteCount = len;

  // Get last note time to know song duration
  RhythmNote lastN = rhythmReadNote(rhythmSongIdx, len - 1);
  rhythmSongLen_ms = lastN.time_ms + 500; // 500ms after last note

  // Reset all scoring
  rhythmScore = 0;
  rhythmCombo = 0;
  rhythmMaxCombo = 0;
  rhythmPerfects = 0;
  rhythmGreats = 0;
  rhythmGoods = 0;
  rhythmMisses = 0;
  rhythmNextNote = 0;
  rhythmNextHitNote = 0;
  rhythmToneOverride = false;
  rhythmLastFrame = 0;

  for (uint8_t i = 0; i < 5; i++) {
    rhythmHitFlash[i] = 0;
    rhythmHitType[i] = 0;
    rhythmLastPress[i] = 0;
  }

  allLEDsOff();
  noTone(SPEAKER);

  p1Matrix.displayClear();
  p2Matrix.displayClear();

  rhythmStartTime = millis();
  gameState = RHYTHM_PLAYING;
}

void rhythmDrawHighway(unsigned long elapsed) {
  MD_MAX72XX* mx = p1Matrix.getGraphicObject();

  // Clear all points
  for (uint8_t r = 0; r < 8; r++) {
    for (uint8_t c = 0; c < 32; c++) {
      mx->setPoint(r, c, false);
    }
  }

  // Draw hit zone indicator (columns 0-1, all lane rows dim)
  for (uint8_t i = 0; i < 5; i++) {
    mx->setPoint(laneRows[i], 0, true);
  }

  // Draw notes
  for (uint8_t i = 0; i < rhythmNoteCount; i++) {
    RhythmNote n = rhythmReadNote(rhythmSongIdx, i);

    // How far in the future is this note?
    long noteOffset = (long)n.time_ms - (long)elapsed;

    // Skip notes that are too far past or too far ahead
    if (noteOffset < -300) continue;
    if (noteOffset > (long)HIGHWAY_WINDOW_MS) break;

    // Map time offset to column: 0 = hit zone, 31 = far right
    // noteOffset=0 -> col 1, noteOffset=HIGHWAY_WINDOW_MS -> col 31
    int col = (int)((long)noteOffset * 30L / (long)HIGHWAY_WINDOW_MS) + 1;

    if (col >= 0 && col < 32 && n.lane < 5) {
      uint8_t row = laneRows[n.lane];
      mx->setPoint(row, col, true);
      // Make notes wider (2 pixels) when close
      if (col > 0) mx->setPoint(row, col - 1, true);
    }
  }
}

void rhythmAutoPlay(unsigned long elapsed) {
  // Play notes as they arrive
  while (rhythmNextNote < rhythmNoteCount) {
    RhythmNote n = rhythmReadNote(rhythmSongIdx, rhythmNextNote);
    if (n.time_ms <= elapsed) {
      // Don't override hit feedback sounds
      if (!rhythmToneOverride || millis() >= rhythmOverrideEnd) {
        rhythmToneOverride = false;
        if (n.freq_hz > 0) {
          tone(SPEAKER, n.freq_hz, 120);
        }
        rhythmToneEnd = millis() + 120;
      }
      rhythmNextNote++;
    } else {
      break;
    }
  }
}

void rhythmUpdateLEDs(unsigned long elapsed) {
  for (uint8_t lane = 0; lane < 5; lane++) {
    // Check for flash from hit
    if (rhythmHitFlash[lane] > 0 && millis() - rhythmHitFlash[lane] < 150) {
      rhythmSetLED(lane, 255);
      continue;
    }
    if (rhythmHitFlash[lane] > 0 && millis() - rhythmHitFlash[lane] >= 150) {
      rhythmHitFlash[lane] = 0;
      rhythmSetLED(lane, 0);
      continue;
    }

    // Find nearest upcoming note in this lane
    uint8_t brightness = 0;
    for (uint8_t i = rhythmNextHitNote; i < rhythmNoteCount; i++) {
      RhythmNote n = rhythmReadNote(rhythmSongIdx, i);
      if (n.lane != lane) continue;

      long timeUntil = (long)n.time_ms - (long)elapsed;
      if (timeUntil < -200) continue; // already missed
      if (timeUntil > 1200) break;    // too far away

      // Fade: 1200ms away = 20, 0ms = 255
      if (timeUntil <= 0) {
        brightness = 255;
      } else {
        brightness = (uint8_t)(255 - (timeUntil * 235L / 1200L));
        if (brightness < 20) brightness = 20;
      }
      break;
    }
    rhythmSetLED(lane, brightness);
  }
}

void rhythmCheckMisses(unsigned long elapsed) {
  // Check if any notes have gone past the hit window
  while (rhythmNextHitNote < rhythmNoteCount) {
    RhythmNote n = rhythmReadNote(rhythmSongIdx, rhythmNextHitNote);
    long diff = (long)elapsed - (long)n.time_ms;

    if (diff > 200) {
      // Missed!
      rhythmMisses++;
      rhythmCombo = 0;
      rhythmHitFlash[n.lane] = millis();
      rhythmHitType[n.lane] = 4; // miss
      rhythmNextHitNote++;
    } else {
      break;
    }
  }
}

void rhythmProcessHit(uint8_t lane, unsigned long elapsed) {
  // Find the closest note in this lane near current time
  int bestIdx = -1;
  long bestDiff = 999;

  for (uint8_t i = rhythmNextHitNote; i < rhythmNoteCount; i++) {
    RhythmNote n = rhythmReadNote(rhythmSongIdx, i);

    // Too far ahead
    if ((long)n.time_ms - (long)elapsed > 200) break;

    if (n.lane == lane) {
      long diff = abs((long)elapsed - (long)n.time_ms);
      if (diff < bestDiff) {
        bestDiff = diff;
        bestIdx = i;
      }
    }
  }

  if (bestIdx < 0 || bestDiff > 200) return; // no note to hit

  uint8_t mult = rhythmGetMultiplier();
  uint8_t hitType = 0;

  if (bestDiff <= 50) {
    // Perfect
    rhythmScore += 100UL * mult;
    rhythmPerfects++;
    rhythmCombo++;
    hitType = 1;
    // Sound override
    rhythmToneOverride = true;
    rhythmOverrideEnd = millis() + 30;
    tone(SPEAKER, 1600, 30);
  } else if (bestDiff <= 100) {
    // Great
    rhythmScore += 75UL * mult;
    rhythmGreats++;
    rhythmCombo++;
    hitType = 2;
    rhythmToneOverride = true;
    rhythmOverrideEnd = millis() + 30;
    tone(SPEAKER, 1200, 30);
  } else {
    // Good
    rhythmScore += 50UL * mult;
    rhythmGoods++;
    rhythmCombo++;
    hitType = 3;
    rhythmToneOverride = true;
    rhythmOverrideEnd = millis() + 30;
    tone(SPEAKER, 800, 30);
  }

  if (rhythmCombo > rhythmMaxCombo) rhythmMaxCombo = rhythmCombo;

  rhythmHitFlash[lane] = millis();
  rhythmHitType[lane] = hitType;

  // Mark this note as consumed: advance nextHitNote if it was the earliest
  // We need to skip over this note - mark by moving past
  // Since notes are sorted by time, we skip consumed ones
  if ((uint8_t)bestIdx == rhythmNextHitNote) {
    rhythmNextHitNote++;
    // Skip any other notes at same time that were already judged
    while (rhythmNextHitNote < rhythmNoteCount) {
      RhythmNote nn = rhythmReadNote(rhythmSongIdx, rhythmNextHitNote);
      if ((long)elapsed - (long)nn.time_ms > 200) {
        rhythmMisses++;
        rhythmCombo = 0;
        rhythmNextHitNote++;
      } else {
        break;
      }
    }
  }
}

void rhythmShowScore() {
  // Alternate between score and combo on p2Matrix
  unsigned long elapsed = millis() - rhythmStartTime;
  bool showCombo = ((elapsed / 2000) % 2) == 1;

  static char p2Buf[16];
  if (showCombo) {
    uint8_t mult = rhythmGetMultiplier();
    sprintf(p2Buf, "%dx %u", mult, rhythmCombo);
  } else {
    // Show score - use ltoa for unsigned long
    ltoa(rhythmScore, p2Buf, 10);
  }

  p2Matrix.displayClear();
  p2Matrix.displayText(p2Buf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
}

void startRhythmEnd() {
  allLEDsOff();
  noTone(SPEAKER);

  rhythmEndTime = millis();

  // Calculate grade
  uint16_t totalNotes = rhythmPerfects + rhythmGreats + rhythmGoods + rhythmMisses;
  uint16_t hitNotes = rhythmPerfects + rhythmGreats + rhythmGoods;
  uint8_t pct = totalNotes > 0 ? (uint8_t)((uint32_t)hitNotes * 100UL / totalNotes) : 0;

  char grade;
  if (pct >= 95) grade = 'S';
  else if (pct >= 85) grade = 'A';
  else if (pct >= 70) grade = 'B';
  else if (pct >= 50) grade = 'C';
  else grade = 'F';

  // Show score
  static char scoreBuf[16];
  ltoa(rhythmScore, scoreBuf, 10);

  static char gradeBuf[4];
  gradeBuf[0] = grade;
  gradeBuf[1] = '\0';

  p1Matrix.displayClear();
  p2Matrix.displayClear();
  p1Matrix.displayScroll(scoreBuf, PA_CENTER, PA_SCROLL_LEFT, 60);
  p2Matrix.displayText(gradeBuf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  // Play result sound
  if (grade != 'F') {
    tone(SPEAKER, NOTE_C5, 150);
  } else {
    tone(SPEAKER, 150, 300);
  }

  gameState = RHYTHM_END;
}

/*SETUP*/

void setup() {
  randomSeed(analogRead(A0));

  for (int i = 0; i < 5; i++) {
    pinMode(P1_BTN[i], INPUT_PULLUP);
    pinMode(P1_LED[i], OUTPUT);
  }
  for (int i = 0; i < 5; i++) {
    pinMode(P2_BTN[i], INPUT_PULLUP);
    pinMode(P2_LED[i], OUTPUT);
  }

  pinMode(START_BTN, INPUT_PULLUP);
  pinMode(SOLO_BTN, INPUT_PULLUP);
  pinMode(HERO_BTN, INPUT_PULLUP);
  pinMode(ULTRA_BTN, INPUT_PULLUP);
  pinMode(ULTRA_TRIG, OUTPUT);
  pinMode(ULTRA_ECHO, INPUT);
  digitalWrite(ULTRA_TRIG, LOW);
  pinMode(SPEAKER, OUTPUT);

  p1Matrix.begin();
  p2Matrix.begin();
  p1Matrix.setIntensity(4);
  p2Matrix.setIntensity(4);

  // === MATRIX ORIENTATION ===
  p1Matrix.setZoneEffect(0, true, PA_FLIP_UD);
  p1Matrix.setZoneEffect(0, true, PA_FLIP_LR);
  setPlayer2Normal();

  p1Matrix.setTextAlignment(PA_CENTER);
  p2Matrix.setTextAlignment(PA_CENTER);
}

void loop() {
  // Only run normal music engine outside rhythm modes
  if (gameState != RHYTHM_SELECT && gameState != RHYTHM_PLAYING && gameState != RHYTHM_END) {
    updateMusic();
  }
  p1Matrix.displayAnimate();
  p2Matrix.displayAnimate();

  /*WAITING MODE*/
  if (gameState == WAITING) {
    musicMode = MUSIC_HP;

    p1Matrix.displayClear();
    p2Matrix.displayClear();
    p1Matrix.displayText("0", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    p2Matrix.displayText("0", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

    // Check for rhythm game entry: SOLO + START pressed together
    if (digitalRead(SOLO_BTN) == LOW && digitalRead(START_BTN) == LOW) {
      delay(50); // Brief debounce for combo press
      if (digitalRead(SOLO_BTN) == LOW && digitalRead(START_BTN) == LOW) {
        startRhythmSelect();
        return;
      }
    }

    if (digitalRead(SOLO_BTN) == LOW) {
      gameMode = MODE_SOLO;
      setPlayer2Normal();
      soloScore = 0;
      cdBlinkCount = 0;
      cdOn = false;
      cdNext = millis();
      musicMode = MUSIC_NONE;
      gameState = COUNTDOWN;
    }

    if (digitalRead(START_BTN) == LOW) {
      gameMode = MODE_VS;
      setPlayer2Rotated();
      p1Score = 0;
      p2Score = 0;
      cdBlinkCount = 0;
      cdOn = false;
      cdNext = millis();
      musicMode = MUSIC_NONE;
      gameState = COUNTDOWN;
    }

    if (digitalRead(HERO_BTN) == LOW) {
      startHeroGame();
    }

    if (digitalRead(ULTRA_BTN) == LOW) {
      startUltraState();
    }
  }

  /*COUNTDOWN (shared)*/
  if (gameState == COUNTDOWN) {
    if (millis() >= cdNext) {
      cdOn = !cdOn;

      if (cdOn) {
        for (int i = 0; i < 5; i++) digitalWrite(P1_LED[i], HIGH);
        for (int i = 0; i < 5; i++) digitalWrite(P2_LED[i], HIGH);
        tone(SPEAKER, 1000, 200);
      } else {
        allLEDsOff();
        cdBlinkCount++;
      }

      cdNext = millis() + 500;
    }

    if (cdBlinkCount >= 3) {
      allLEDsOff();
      noTone(SPEAKER);
      musicMode = MUSIC_TETRIS;
      musicIndex = 0;

      if (gameMode == MODE_SOLO) {
        soloStartTime = millis();
        spawnSolo(1);
        gameState = SOLO_PLAYING;
      } else {
        spawnVS_P1();
        spawnVS_P2();
        gameState = VS_PLAYING;
      }
    }
  }

  /*SOLO MODE*/
  if (gameState == SOLO_PLAYING) {
    float timeLeft = 30.0 - (millis() - soloStartTime) / 1000.0;
    if (timeLeft < 0) timeLeft = 0;

    if (timeLeft <= 20 && soloCount < 2) spawnSolo(2);
    if (timeLeft <= 10 && soloCount < 3) spawnSolo(3);

    char buf[6];
    dtostrf(timeLeft, 4, 1, buf);

    char sBuf[8];
    itoa(soloScore, sBuf, 10);

    p1Matrix.displayClear();
    p2Matrix.displayClear();

    p1Matrix.displayText(buf,  PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    p2Matrix.displayText(sBuf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

    for (int i = 0; i < 10; i++) {
      bool pressed = (i < 5) ? !digitalRead(P1_BTN[i])
                             : !digitalRead(P2_BTN[i - 5]);

      if (pressed && millis() - lastPress[i] > DEBOUNCE) {
        lastPress[i] = millis();
        bool hit = false;

        for (int s = 0; s < soloCount; s++) {
          if (i == soloActive[s]) {
            soloScore++;
            replaceSolo(s);
            hit = true;
            break;
          }
        }

        if (!hit) soloScore = max(0, soloScore - 1);
      }
    }

    if (timeLeft <= 0) {
      if (soloScore >= 50) {
        cdBlinkCount = 0;
        cdOn = false;
        cdNext = millis();
        gameState = BONUS_COUNTDOWN;
      } else {
        victoryType = VICTORY_FAIL;

        musicMode  = MUSIC_WIN;
        musicIndex = 0;
        musicNext  = 0;

        soloVictoryStarted = false;
        gameMode  = MODE_SOLO;
        gameState = VICTORY;
      }
    }
  }

  /*BONUS COUNTDOWN*/
  if (gameState == BONUS_COUNTDOWN) {
    if (millis() >= cdNext) {
      cdOn = !cdOn;

      if (cdOn) {
        for (int i = 0; i < 5; i++) digitalWrite(P1_LED[i], HIGH);
        for (int i = 0; i < 5; i++) digitalWrite(P2_LED[i], HIGH);
        tone(SPEAKER, 1200, 200);
      } else {
        allLEDsOff();
        cdBlinkCount++;
      }

      cdNext = millis() + 500;
    }

    if (cdBlinkCount >= 3) {
      bonusStartTime = millis();
      spawnSolo(5);
      musicMode = MUSIC_TETRIS;
      gameState = BONUS;
    }
  }

  /*BONUS MODE*/
  if (gameState == BONUS) {
    float timeLeft = 10.0 - (millis() - bonusStartTime) / 1000.0;
    if (timeLeft < 0) timeLeft = 0;

    char buf[6];
    dtostrf(timeLeft, 4, 1, buf);
    p1Matrix.displayClear();
    p1Matrix.print(buf);

    p2Matrix.displayClear();
    p2Matrix.print(soloScore);

    for (int i = 0; i < 10; i++) {
      bool pressed = (i < 5) ? !digitalRead(P1_BTN[i])
                             : !digitalRead(P2_BTN[i - 5]);

      if (pressed && millis() - lastPress[i] > DEBOUNCE) {
        lastPress[i] = millis();
        bool hit = false;

        for (int s = 0; s < 5; s++) {
          if (i == soloActive[s]) {
            soloScore++;
            replaceSolo(s);
            hit = true;
            break;
          }
        }

        if (!hit) soloScore = max(0, soloScore - 1);
      }
    }

    if (timeLeft <= 0) {
      musicMode  = MUSIC_WIN;
      musicIndex = 0;
      musicNext  = 0;
      noTone(SPEAKER);

      soloVictoryStarted = false;
      winner = 1;
      gameState = VICTORY;
    }
  }

  /*VS MODE*/
  if (gameState == VS_PLAYING) {
    char b1[8];
    itoa(p1Score, b1, 10);
    char b2[8];
    itoa(p2Score, b2, 10);

    p1Matrix.displayClear();
    p2Matrix.displayClear();

    p1Matrix.displayText(b1, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    p2Matrix.displayText(b2, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

    // PLAYER 1
    for (int i = 0; i < 5; i++) {
      if (!digitalRead(P1_BTN[i]) && millis() - lastPress[i] > DEBOUNCE) {
        lastPress[i] = millis();
        if (i == p1Active) {
          p1Score++;
          spawnVS_P1();
        } else {
          p1Score = max(0, p1Score - 1);
        }
      }
    }

    // PLAYER 2
    for (int i = 0; i < 5; i++) {
      if (!digitalRead(P2_BTN[i]) && millis() - lastPress[i + 5] > DEBOUNCE) {
        lastPress[i + 5] = millis();
        if (i == p2Active) {
          p2Score++;
          spawnVS_P2();
        } else {
          p2Score = max(0, p2Score - 1);
        }
      }
    }

    if (p1Score >= 50 || p2Score >= 50) {
      winner = (p1Score >= 50) ? 1 : 2;

      musicMode = MUSIC_WIN;
      musicIndex = 0;

      vsVictoryStarted = false;
      gameState = VICTORY;
    }
  }

  /*HERO STATE*/
  if (gameState == HERO_STATE) {
    float timeLeft = heroRoundTimeMs() / 1000.0 - (millis() - heroRoundStart) / 1000.0;
    if (timeLeft < 0) timeLeft = 0;

    showHeroStatus(timeLeft);
    updateHeroInputs();

    if (heroRoundCleared()) {
      tone(SPEAKER, 1600, 180);

      if (heroRound >= HERO_TOTAL_ROUNDS) {
        startHeroEnd(true);
      } else {
        heroRound++;
        startHeroRound();
      }
    }
    else if (timeLeft <= 0) {
      tone(SPEAKER, 180, 400);
      startHeroEnd(false);
    }
  }

  /*HERO END*/
  if (gameState == HERO_END) {
    p1Matrix.displayAnimate();
    p2Matrix.displayAnimate();

    if (millis() - heroEndStart >= HERO_END_TIME) {
      allLEDsOff();
      noTone(SPEAKER);

      musicMode = MUSIC_HP;
      musicIndex = 0;
      musicNext = 0;

      gameMode = MODE_NONE;
      gameState = WAITING;
    }
  }


  /* ULTRA STATE */
  if (gameState == ULTRA_STATE) {
    showUltraDistance();

    if (millis() - ultraStartTime >= ULTRA_SHOW_TIME) {
      noTone(SPEAKER);
      gameState = WAITING;
    }
  }

  /* ===== BONUS END SCORE ANIMATION ===== */
  if (gameMode == MODE_SOLO && bonusEndAnimating) {
    p1Matrix.displayAnimate();
    p2Matrix.displayAnimate();

    unsigned long elapsed = millis() - bonusEndStart;

    // After 2.5s -> show numeric score
    if (elapsed > 2500 && elapsed < 5000) {
      p1Matrix.displayClear();
      p2Matrix.displayClear();
      p1Matrix.print(soloScore);
      p2Matrix.print(soloScore);
    }

    // After 5s -> reset game
    if (elapsed >= 5000) {
      bonusEndAnimating = false;

      allLEDsOff();

      musicMode = MUSIC_HP;
      musicIndex = 0;
      musicNext  = 0;

      gameMode  = MODE_NONE;
      gameState = WAITING;
    }

    return;
  }

  /* VICTORY (SOLO + VS SAFE)*/
  if (gameState == VICTORY) {

    // --- VS MODE ---
    if (gameMode == MODE_VS) {
      if (!vsVictoryStarted) {
        startVSVictoryAnimation();
      }

      p1Matrix.displayAnimate();
      p2Matrix.displayAnimate();

      unsigned long elapsed = millis() - vsVictoryStart;

      if (elapsed > 10000) {
        allLEDsOff();

        p1Score = 0;
        p2Score = 0;
        winner  = 0;

        musicMode = MUSIC_HP;
        musicIndex = 0;
        musicNext  = 0;

        gameMode  = MODE_NONE;
        gameState = WAITING;
      }
    }

    // --- SOLO MODE ---
    if (gameMode == MODE_SOLO) {
      if (!soloVictoryStarted) {
        soloVictoryStart = millis();
        soloVictoryStarted = true;

        p1Matrix.displayClear();
        p2Matrix.displayClear();

        if (victoryType == VICTORY_WIN) {
          p1Matrix.displayScroll("NICE!", PA_CENTER, PA_SCROLL_LEFT, 80);
          p2Matrix.displayScroll("BONUS!", PA_CENTER, PA_SCROLL_LEFT, 80);
        } else {
          p1Matrix.displayScroll("TIME UP", PA_CENTER, PA_SCROLL_LEFT, 80);
          p2Matrix.displayScroll("TRY AGAIN", PA_CENTER, PA_SCROLL_LEFT, 80);
        }
      }

      p1Matrix.displayAnimate();
      p2Matrix.displayAnimate();

      unsigned long elapsed = millis() - soloVictoryStart;

      if (elapsed > 2500 && elapsed < 5000) {
        p1Matrix.displayClear();
        p2Matrix.displayClear();
        p1Matrix.print(soloScore);
        p2Matrix.print(soloScore);
      }

      if (elapsed >= 5000) {
        allLEDsOff();

        soloScore = 0;
        winner = 0;
        victoryType = VICTORY_WIN;

        musicMode = MUSIC_HP;
        musicIndex = 0;
        musicNext  = 0;

        gameMode  = MODE_NONE;
        gameState = WAITING;
      }
    }
  }

  /* ======================= RHYTHM SELECT ======================= */
  if (gameState == RHYTHM_SELECT) {
    unsigned long now = millis();

    // Navigation: left/right buttons
    if (!digitalRead(P1_BTN[0]) && now - rhythmLastPress[0] > DEBOUNCE) {
      rhythmLastPress[0] = now;
      if (rhythmSongIdx == 0) rhythmSongIdx = NUM_SONGS - 1;
      else rhythmSongIdx--;
      rhythmSwitchSong();
    }

    if (!digitalRead(P1_BTN[4]) && now - rhythmLastPress[4] > DEBOUNCE) {
      rhythmLastPress[4] = now;
      rhythmSongIdx = (rhythmSongIdx + 1) % NUM_SONGS;
      rhythmSwitchSong();
    }

    // Select song with middle button
    if (!digitalRead(P1_BTN[2]) && now - rhythmLastPress[2] > DEBOUNCE) {
      rhythmLastPress[2] = now;
      noTone(SPEAKER);
      startRhythmPlaying();
      return;
    }

    // Back to WAITING with HERO_BTN
    if (digitalRead(HERO_BTN) == LOW) {
      noTone(SPEAKER);
      allLEDsOff();
      musicMode = MUSIC_HP;
      musicIndex = 0;
      musicNext = 0;
      gameState = WAITING;
      return;
    }

    // Auto-preview: play first ~8 notes after 500ms
    if (!rhythmPreviewPlaying && now - rhythmPreviewStart >= 500) {
      rhythmPreviewPlaying = true;
      rhythmPreviewIdx = 0;
      rhythmPreviewNext = now;
    }

    if (rhythmPreviewPlaying) {
      uint8_t previewMax = rhythmGetSongLen(rhythmSongIdx);
      if (previewMax > 10) previewMax = 10; // preview first 10 notes

      if (rhythmPreviewIdx < previewMax && now >= rhythmPreviewNext) {
        RhythmNote n = rhythmReadNote(rhythmSongIdx, rhythmPreviewIdx);
        if (n.freq_hz > 0) {
          tone(SPEAKER, n.freq_hz, 150);
        }
        rhythmPreviewIdx++;

        // Calculate delay to next note
        if (rhythmPreviewIdx < previewMax) {
          RhythmNote next = rhythmReadNote(rhythmSongIdx, rhythmPreviewIdx);
          unsigned long gap = next.time_ms - n.time_ms;
          if (gap < 100) gap = 100;
          if (gap > 500) gap = 500;
          rhythmPreviewNext = now + gap;
        }
      }

      if (rhythmPreviewIdx >= previewMax) {
        rhythmPreviewPlaying = false; // done previewing
      }
    }

    // LED indicators: pulse middle, dim left/right
    uint8_t pulse = (uint8_t)(128 + 127.0 * sin((float)(now - rhythmPulseTime) / 300.0));
    rhythmSetLED(2, pulse); // middle
    rhythmSetLED(0, 40);   // left hint
    rhythmSetLED(4, 40);   // right hint
    rhythmSetLED(1, 0);
    rhythmSetLED(3, 0);
  }

  /* ======================= RHYTHM PLAYING ======================= */
  if (gameState == RHYTHM_PLAYING) {
    unsigned long now = millis();
    unsigned long elapsed = now - rhythmStartTime;

    // Frame rate limiter
    if (now - rhythmLastFrame >= RHYTHM_FRAME_MS) {
      rhythmLastFrame = now;

      // Draw highway
      rhythmDrawHighway(elapsed);

      // Update LEDs
      rhythmUpdateLEDs(elapsed);

      // Show score on p2
      rhythmShowScore();
    }

    // Auto-play music
    rhythmAutoPlay(elapsed);

    // Check for missed notes
    rhythmCheckMisses(elapsed);

    // Check button presses
    for (uint8_t i = 0; i < 5; i++) {
      if (!digitalRead(P1_BTN[i]) && now - rhythmLastPress[i] > RHYTHM_DEBOUNCE) {
        rhythmLastPress[i] = now;
        rhythmProcessHit(i, elapsed);
      }
    }

    // Song over?
    if (elapsed >= rhythmSongLen_ms && rhythmNextHitNote >= rhythmNoteCount) {
      startRhythmEnd();
    }

    // Also allow HERO_BTN to quit mid-song
    if (digitalRead(HERO_BTN) == LOW) {
      startRhythmEnd();
    }
  }

  /* ======================= RHYTHM END ======================= */
  if (gameState == RHYTHM_END) {
    unsigned long elapsed = millis() - rhythmEndTime;

    // Phase 2: stats (3-6 seconds)
    if (elapsed >= 3000 && elapsed < 3100) {
      static char statBuf[16];
      sprintf(statBuf, "P:%u G:%u", rhythmPerfects, rhythmGreats);

      static char comboBuf[16];
      sprintf(comboBuf, "COMBO:%u", rhythmMaxCombo);

      p1Matrix.displayClear();
      p2Matrix.displayClear();
      p1Matrix.displayScroll(statBuf, PA_CENTER, PA_SCROLL_LEFT, 60);
      p2Matrix.displayText(comboBuf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

      // Victory jingle or sad tone
      uint16_t totalN = rhythmPerfects + rhythmGreats + rhythmGoods + rhythmMisses;
      uint16_t hitN = rhythmPerfects + rhythmGreats + rhythmGoods;
      uint8_t pct = totalN > 0 ? (uint8_t)((uint32_t)hitN * 100UL / totalN) : 0;
      if (pct >= 50) {
        tone(SPEAKER, NOTE_E5, 100);
      } else {
        tone(SPEAKER, 150, 200);
      }
    }

    // Phase 3: return to select (after 6 seconds)
    if (elapsed >= 6000) {
      allLEDsOff();
      noTone(SPEAKER);
      startRhythmSelect();
    }
  }
}
