
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

// Song 1: Seven Nation Army (Easy) - iconic bass riff E E G E D C B
const RhythmNote PROGMEM song0_notes[] = {
  // Riff 1: E E G E D C B  (BPM ~120)
  {0, 2, NOTE_E4}, {500, 2, NOTE_E4}, {750, 3, NOTE_G4}, {1000, 2, NOTE_E4},
  {1500, 1, NOTE_D4}, {2000, 0, NOTE_C4}, {3000, 1, NOTE_B3},
  // Riff 2
  {4000, 2, NOTE_E4}, {4500, 2, NOTE_E4}, {4750, 3, NOTE_G4}, {5000, 2, NOTE_E4},
  {5500, 1, NOTE_D4}, {6000, 0, NOTE_C4}, {7000, 1, NOTE_B3},
  // Riff 3
  {8000, 2, NOTE_E4}, {8500, 2, NOTE_E4}, {8750, 3, NOTE_G4}, {9000, 2, NOTE_E4},
  {9500, 1, NOTE_D4}, {10000, 0, NOTE_C4}, {11000, 1, NOTE_B3},
  // Riff 4
  {12000, 2, NOTE_E4}, {12500, 2, NOTE_E4}, {12750, 3, NOTE_G4}, {13000, 2, NOTE_E4},
  {13500, 1, NOTE_D4}, {14000, 0, NOTE_C4}, {15000, 1, NOTE_B3},
  // Riff 5
  {16000, 2, NOTE_E4}, {16500, 2, NOTE_E4}, {16750, 3, NOTE_G4}, {17000, 2, NOTE_E4},
  {17500, 1, NOTE_D4}, {18000, 0, NOTE_C4}, {19000, 1, NOTE_B3},
  // Riff 6
  {20000, 2, NOTE_E4}, {20500, 2, NOTE_E4}, {20750, 3, NOTE_G4}, {21000, 2, NOTE_E4},
  {21500, 1, NOTE_D4}, {22000, 0, NOTE_C4}, {23000, 1, NOTE_B3},
};
#define SONG0_LEN 42

// Song 2: Sweet Child O Mine (Medium) - opening riff D D A G B G A G
const RhythmNote PROGMEM song1_notes[] = {
  // Pattern: D5 D4 A4 G4 B4 G4 A4 G4 (circular picking, 200ms apart)
  // Lanes: D4=0, G4=1, A4=2, B4=3, D5=4
  {0, 4, NOTE_D5}, {200, 0, NOTE_D4}, {400, 2, NOTE_A4}, {600, 1, NOTE_G4},
  {800, 3, NOTE_B4}, {1000, 1, NOTE_G4}, {1200, 2, NOTE_A4}, {1400, 1, NOTE_G4},
  // Repeat
  {1800, 4, NOTE_D5}, {2000, 0, NOTE_D4}, {2200, 2, NOTE_A4}, {2400, 1, NOTE_G4},
  {2600, 3, NOTE_B4}, {2800, 1, NOTE_G4}, {3000, 2, NOTE_A4}, {3200, 1, NOTE_G4},
  // Repeat
  {3600, 4, NOTE_D5}, {3800, 0, NOTE_D4}, {4000, 2, NOTE_A4}, {4200, 1, NOTE_G4},
  {4400, 3, NOTE_B4}, {4600, 1, NOTE_G4}, {4800, 2, NOTE_A4}, {5000, 1, NOTE_G4},
  // Repeat
  {5400, 4, NOTE_D5}, {5600, 0, NOTE_D4}, {5800, 2, NOTE_A4}, {6000, 1, NOTE_G4},
  {6200, 3, NOTE_B4}, {6400, 1, NOTE_G4}, {6600, 2, NOTE_A4}, {6800, 1, NOTE_G4},
  // E pattern variation: E5 E4 A4 G4 B4 G4 A4 G4
  {7200, 4, NOTE_E5}, {7400, 0, NOTE_E4}, {7600, 2, NOTE_A4}, {7800, 1, NOTE_G4},
  {8000, 3, NOTE_B4}, {8200, 1, NOTE_G4}, {8400, 2, NOTE_A4}, {8600, 1, NOTE_G4},
  // G pattern: G5 G4 B4 A4 D5 A4 B4 A4
  {9000, 4, NOTE_G5}, {9200, 1, NOTE_G4}, {9400, 3, NOTE_B4}, {9600, 2, NOTE_A4},
  {9800, 4, NOTE_D5}, {10000, 2, NOTE_A4}, {10200, 3, NOTE_B4}, {10400, 2, NOTE_A4},
  // Back to D pattern
  {10800, 4, NOTE_D5}, {11000, 0, NOTE_D4}, {11200, 2, NOTE_A4}, {11400, 1, NOTE_G4},
  {11600, 3, NOTE_B4}, {11800, 1, NOTE_G4}, {12000, 2, NOTE_A4}, {12200, 1, NOTE_G4},
  // Final repeat
  {12600, 4, NOTE_D5}, {13000, 0, NOTE_D4}, {13400, 2, NOTE_A4}, {13800, 1, NOTE_G4},
  {14200, 3, NOTE_B4}, {14600, 1, NOTE_G4}, {15000, 4, NOTE_D5},
};
#define SONG1_LEN 59

// Song 3: Enter Sandman (Medium) - E-E-G-Bb-G-E riff
const RhythmNote PROGMEM song2_notes[] = {
  // Lanes: B3/CS4=0, D4=1, E4=2, G4=3, AS4/E5=4
  // Main riff: E-E-G-Bb-G-E, D-C#-E
  {0, 2, NOTE_E4}, {300, 2, NOTE_E4}, {600, 2, NOTE_E4},
  {900, 3, NOTE_G4}, {1100, 4, NOTE_AS4}, {1300, 3, NOTE_G4},
  {1500, 2, NOTE_E4}, {1800, 2, NOTE_E4},
  {2100, 1, NOTE_D4}, {2300, 0, NOTE_CS4}, {2600, 2, NOTE_E4},
  // Repeat
  {3200, 2, NOTE_E4}, {3500, 2, NOTE_E4}, {3800, 2, NOTE_E4},
  {4100, 3, NOTE_G4}, {4300, 4, NOTE_AS4}, {4500, 3, NOTE_G4},
  {4700, 2, NOTE_E4}, {5000, 2, NOTE_E4},
  {5300, 1, NOTE_D4}, {5500, 0, NOTE_CS4}, {5800, 2, NOTE_E4},
  // Bridge
  {6400, 0, NOTE_B3}, {6700, 1, NOTE_D4}, {7000, 2, NOTE_E4}, {7300, 3, NOTE_G4},
  {7600, 2, NOTE_E4}, {7900, 1, NOTE_D4}, {8200, 0, NOTE_B3},
  // Heavy part
  {8800, 4, NOTE_E5}, {9100, 3, NOTE_G4}, {9400, 4, NOTE_AS4}, {9700, 3, NOTE_G4},
  {10000, 2, NOTE_E4}, {10300, 1, NOTE_D4}, {10600, 0, NOTE_CS4}, {10900, 2, NOTE_E4},
  // Final riff
  {11500, 2, NOTE_E4}, {11800, 2, NOTE_E4},
  {12100, 3, NOTE_G4}, {12300, 4, NOTE_AS4}, {12500, 3, NOTE_G4},
  {12800, 2, NOTE_E4}, {13100, 1, NOTE_D4}, {13400, 0, NOTE_CS4},
  {13700, 2, NOTE_E4}, {14200, 2, NOTE_E4},
};
#define SONG2_LEN 45

// Song 4: Crazy Train (Med-Hard) - F#-A-F#-B-F#-A-F#-E alternating
const RhythmNote PROGMEM song3_notes[] = {
  // Lanes: D4=0, E4=1, FS4=2, A4=3, B4=4
  // Pattern: F#-A-F#-B-F#-A-F#-E at ~150ms
  {0, 2, NOTE_FS4}, {150, 3, NOTE_A4}, {300, 2, NOTE_FS4}, {450, 4, NOTE_B4},
  {600, 2, NOTE_FS4}, {750, 3, NOTE_A4}, {900, 2, NOTE_FS4}, {1050, 1, NOTE_E4},
  // Repeat
  {1350, 2, NOTE_FS4}, {1500, 3, NOTE_A4}, {1650, 2, NOTE_FS4}, {1800, 4, NOTE_B4},
  {1950, 2, NOTE_FS4}, {2100, 3, NOTE_A4}, {2250, 2, NOTE_FS4}, {2400, 1, NOTE_E4},
  // Repeat
  {2700, 2, NOTE_FS4}, {2850, 3, NOTE_A4}, {3000, 2, NOTE_FS4}, {3150, 4, NOTE_B4},
  {3300, 2, NOTE_FS4}, {3450, 3, NOTE_A4}, {3600, 2, NOTE_FS4}, {3750, 1, NOTE_E4},
  // Verse riff - ascending
  {4200, 0, NOTE_D4}, {4400, 1, NOTE_E4}, {4600, 2, NOTE_FS4}, {4800, 3, NOTE_A4},
  {5000, 4, NOTE_B4}, {5200, 3, NOTE_A4}, {5400, 2, NOTE_FS4}, {5600, 1, NOTE_E4},
  // Bridge
  {6000, 4, NOTE_D5}, {6200, 4, NOTE_CS5}, {6400, 3, NOTE_A4},
  {6600, 1, NOTE_E4}, {6800, 2, NOTE_FS4}, {7000, 3, NOTE_A4},
  {7200, 4, NOTE_CS5}, {7400, 4, NOTE_D5},
  // Back to main riff
  {7800, 2, NOTE_FS4}, {7950, 3, NOTE_A4}, {8100, 2, NOTE_FS4}, {8250, 4, NOTE_B4},
  {8400, 2, NOTE_FS4}, {8550, 3, NOTE_A4}, {8700, 2, NOTE_FS4}, {8850, 1, NOTE_E4},
  // Repeat
  {9150, 2, NOTE_FS4}, {9300, 3, NOTE_A4}, {9450, 2, NOTE_FS4}, {9600, 4, NOTE_B4},
  {9750, 2, NOTE_FS4}, {9900, 3, NOTE_A4}, {10050, 2, NOTE_FS4}, {10200, 1, NOTE_E4},
  // Final run
  {10600, 0, NOTE_D4}, {10750, 1, NOTE_E4}, {10900, 2, NOTE_FS4},
  {11050, 3, NOTE_A4}, {11200, 4, NOTE_B4}, {11400, 4, NOTE_D5},
  {11600, 4, NOTE_CS5}, {11800, 3, NOTE_A4}, {12000, 2, NOTE_FS4},
  {12200, 1, NOTE_E4}, {12400, 2, NOTE_FS4}, {12600, 3, NOTE_A4},
};
#define SONG3_LEN 64

// Song 5: Thunderstruck (Hard) - B alternating rapidly with lower notes
const RhythmNote PROGMEM song4_notes[] = {
  // Lanes: D4=0, E4=1, FS4=2, A4=3, B4=4
  // B-A-B-E-B-A-B-E pattern at ~150ms
  {0, 4, NOTE_B4}, {150, 3, NOTE_A4}, {300, 4, NOTE_B4}, {450, 1, NOTE_E4},
  {600, 4, NOTE_B4}, {750, 3, NOTE_A4}, {900, 4, NOTE_B4}, {1050, 1, NOTE_E4},
  // B-A-B-FS-B-A-B-FS
  {1200, 4, NOTE_B4}, {1350, 3, NOTE_A4}, {1500, 4, NOTE_B4}, {1650, 2, NOTE_FS4},
  {1800, 4, NOTE_B4}, {1950, 3, NOTE_A4}, {2100, 4, NOTE_B4}, {2250, 2, NOTE_FS4},
  // B-A-B-E-B-A-B-D
  {2400, 4, NOTE_B4}, {2550, 3, NOTE_A4}, {2700, 4, NOTE_B4}, {2850, 1, NOTE_E4},
  {3000, 4, NOTE_B4}, {3150, 3, NOTE_A4}, {3300, 4, NOTE_B4}, {3450, 0, NOTE_D4},
  // B-E-B-FS-B-A-B-FS
  {3600, 4, NOTE_B4}, {3750, 1, NOTE_E4}, {3900, 4, NOTE_B4}, {4050, 2, NOTE_FS4},
  {4200, 4, NOTE_B4}, {4350, 3, NOTE_A4}, {4500, 4, NOTE_B4}, {4650, 2, NOTE_FS4},
  // Power chord break
  {5000, 1, NOTE_E4}, {5200, 1, NOTE_E4}, {5400, 3, NOTE_A4}, {5600, 3, NOTE_A4},
  {5800, 4, NOTE_B4}, {6000, 4, NOTE_B4}, {6200, 3, NOTE_A4}, {6400, 1, NOTE_E4},
  // Resume fast picking
  {6800, 4, NOTE_B4}, {6950, 3, NOTE_A4}, {7100, 4, NOTE_B4}, {7250, 1, NOTE_E4},
  {7400, 4, NOTE_B4}, {7550, 3, NOTE_A4}, {7700, 4, NOTE_B4}, {7850, 1, NOTE_E4},
  {8000, 4, NOTE_B4}, {8150, 3, NOTE_A4}, {8300, 4, NOTE_B4}, {8450, 2, NOTE_FS4},
  {8600, 4, NOTE_B4}, {8750, 3, NOTE_A4}, {8900, 4, NOTE_B4}, {9050, 0, NOTE_D4},
  // Climax ascending
  {9400, 1, NOTE_E4}, {9550, 2, NOTE_FS4}, {9700, 3, NOTE_A4}, {9850, 4, NOTE_B4},
  {10000, 4, NOTE_D5}, {10150, 4, NOTE_E5}, {10300, 4, NOTE_D5}, {10450, 3, NOTE_B4},
  {10600, 3, NOTE_A4}, {10750, 1, NOTE_E4},
  // End: back to pattern
  {11100, 4, NOTE_B4}, {11250, 3, NOTE_A4}, {11400, 4, NOTE_B4}, {11550, 1, NOTE_E4},
  {11700, 4, NOTE_B4}, {11850, 3, NOTE_A4}, {12000, 4, NOTE_B4}, {12150, 0, NOTE_D4},
  {12300, 1, NOTE_E4}, {12500, 4, NOTE_B4},
};
#define SONG4_LEN 74

// Song 6: Through the Fire and Flames (Extreme) - fast runs 80-100ms
const RhythmNote PROGMEM song5_notes[] = {
  // Lanes: C5=0, D5=1, E5=2, F5/G5=3, A5/B5=4
  // Fast ascending run
  {0, 0, NOTE_C5}, {90, 1, NOTE_D5}, {180, 2, NOTE_E5}, {270, 3, NOTE_F5},
  {360, 3, NOTE_G5}, {450, 4, NOTE_A5}, {540, 4, NOTE_B5}, {630, 4, NOTE_A5},
  // Fast descending
  {720, 3, NOTE_G5}, {810, 3, NOTE_F5}, {900, 2, NOTE_E5}, {990, 1, NOTE_D5},
  {1080, 0, NOTE_C5}, {1170, 1, NOTE_D5}, {1260, 2, NOTE_E5}, {1350, 3, NOTE_G5},
  // Second run ascending
  {1500, 1, NOTE_D5}, {1590, 2, NOTE_E5}, {1680, 3, NOTE_F5}, {1770, 3, NOTE_G5},
  {1860, 4, NOTE_A5}, {1950, 4, NOTE_B5}, {2040, 4, NOTE_A5}, {2130, 3, NOTE_G5},
  // Descending
  {2220, 2, NOTE_E5}, {2310, 1, NOTE_D5}, {2400, 0, NOTE_C5}, {2490, 1, NOTE_D5},
  {2580, 2, NOTE_E5}, {2670, 3, NOTE_G5}, {2760, 4, NOTE_B5}, {2850, 4, NOTE_A5},
  // Power section (slower)
  {3100, 2, NOTE_E4}, {3200, 2, NOTE_E4}, {3300, 3, NOTE_G4}, {3400, 4, NOTE_A4},
  {3500, 4, NOTE_B4}, {3600, 3, NOTE_A4}, {3700, 2, NOTE_G4}, {3800, 1, NOTE_E4},
  {3900, 0, NOTE_D4}, {4000, 1, NOTE_E4}, {4100, 2, NOTE_G4}, {4200, 3, NOTE_A4},
  {4300, 4, NOTE_B4}, {4400, 4, NOTE_D5}, {4500, 3, NOTE_B4}, {4600, 2, NOTE_A4},
  // Solo - crazy fast
  {4900, 4, NOTE_E5}, {4980, 3, NOTE_D5}, {5060, 0, NOTE_C5}, {5140, 3, NOTE_B4},
  {5220, 2, NOTE_A4}, {5300, 3, NOTE_B4}, {5380, 0, NOTE_C5}, {5460, 3, NOTE_D5},
  {5540, 4, NOTE_E5}, {5620, 3, NOTE_F5}, {5700, 4, NOTE_E5}, {5780, 3, NOTE_D5},
  {5860, 0, NOTE_C5}, {5940, 2, NOTE_B4}, {6020, 0, NOTE_C5}, {6100, 3, NOTE_D5},
  // Sweeping arpeggios
  {6300, 0, NOTE_C4}, {6380, 1, NOTE_E4}, {6460, 2, NOTE_G4}, {6540, 0, NOTE_C5},
  {6620, 4, NOTE_E5}, {6700, 0, NOTE_C5}, {6780, 2, NOTE_G4}, {6860, 1, NOTE_E4},
  {6960, 1, NOTE_D4}, {7040, 2, NOTE_FS4}, {7120, 3, NOTE_A4}, {7200, 1, NOTE_D5},
  {7280, 4, NOTE_FS5}, {7360, 1, NOTE_D5}, {7440, 3, NOTE_A4}, {7520, 2, NOTE_FS4},
  // Final blitz
  {7700, 2, NOTE_E5}, {7780, 3, NOTE_F5}, {7860, 3, NOTE_G5}, {7940, 4, NOTE_A5},
  {8020, 3, NOTE_G5}, {8100, 2, NOTE_E5}, {8180, 1, NOTE_D5}, {8260, 0, NOTE_C5},
  {8400, 2, NOTE_E5}, {8500, 3, NOTE_G5}, {8600, 2, NOTE_E5}, {8700, 0, NOTE_C5},
};
#define SONG5_LEN 84

// Song 7: Free Bird (Hard) - slow intro then fast solo
const RhythmNote PROGMEM song6_notes[] = {
  // Lanes: D5=0, E5=1, G5=2, A5=3, B5/D6=4
  // Opening gentle: G-A-B-A-G melody (slow, 500ms)
  {0, 2, NOTE_G4}, {500, 3, NOTE_A4}, {1000, 4, NOTE_B4}, {1500, 3, NOTE_A4},
  {2000, 2, NOTE_G4}, {2500, 1, NOTE_E4}, {3000, 2, NOTE_G4}, {3500, 3, NOTE_A4},
  // Solo begins - picking up speed (300ms)
  {4200, 2, NOTE_G5}, {4500, 3, NOTE_A5}, {4800, 4, NOTE_B5}, {5100, 3, NOTE_A5},
  {5400, 2, NOTE_G5}, {5700, 1, NOTE_E5}, {6000, 2, NOTE_G5}, {6300, 4, NOTE_B5},
  // Fast pentatonic runs (150ms)
  {6700, 0, NOTE_D5}, {6850, 1, NOTE_E5}, {7000, 2, NOTE_G5}, {7150, 3, NOTE_A5},
  {7300, 4, NOTE_B5}, {7450, 3, NOTE_A5}, {7600, 2, NOTE_G5}, {7750, 1, NOTE_E5},
  {7900, 0, NOTE_D5}, {8050, 1, NOTE_E5}, {8200, 2, NOTE_G5}, {8350, 4, NOTE_B5},
  // Double-time (130ms)
  {8600, 4, NOTE_B5}, {8730, 3, NOTE_A5}, {8860, 2, NOTE_G5}, {8990, 1, NOTE_E5},
  {9120, 0, NOTE_D5}, {9250, 1, NOTE_E5}, {9380, 2, NOTE_G5}, {9510, 3, NOTE_A5},
  {9640, 4, NOTE_B5}, {9770, 4, NOTE_D6}, {9900, 4, NOTE_B5}, {10030, 3, NOTE_A5},
  {10160, 2, NOTE_G5}, {10290, 1, NOTE_E5}, {10420, 2, NOTE_G5}, {10550, 3, NOTE_A5},
  // Climax runs
  {10800, 1, NOTE_E5}, {10930, 2, NOTE_G5}, {11060, 3, NOTE_A5}, {11190, 4, NOTE_B5},
  {11320, 4, NOTE_D6}, {11450, 4, NOTE_B5}, {11580, 3, NOTE_A5}, {11710, 2, NOTE_G5},
  {11840, 1, NOTE_E5}, {11970, 2, NOTE_G5}, {12100, 3, NOTE_A5}, {12230, 4, NOTE_B5},
  {12360, 4, NOTE_D6}, {12490, 4, NOTE_E6}, {12620, 4, NOTE_D6}, {12750, 4, NOTE_B5},
  // Ending - slowing down
  {13000, 2, NOTE_G5}, {13200, 1, NOTE_E5}, {13400, 0, NOTE_D5},
  {13700, 2, NOTE_G4}, {14000, 3, NOTE_A4}, {14300, 4, NOTE_B4},
  {14700, 2, NOTE_G4},
};
#define SONG6_LEN 63

// Song 8: Pirates of the Caribbean - He's a Pirate (Med-Hard)
const RhythmNote PROGMEM song7_notes[] = {
  // Lanes: D4=0, E4/F4=1, G4/A4=2, B4/C5=3, D5/E5/F5/G5=4
  // Iconic melody: A-C-D-D-D-E-F-F-F-F-G-E-E-D-C-D (BPM ~160, ~190ms per 8th)
  {0, 2, NOTE_A4}, {190, 3, NOTE_C5}, {380, 4, NOTE_D5}, {500, 4, NOTE_D5},
  {620, 4, NOTE_D5}, {810, 4, NOTE_E5}, {1000, 4, NOTE_F5}, {1120, 4, NOTE_F5},
  {1240, 4, NOTE_F5}, {1360, 4, NOTE_F5}, {1550, 4, NOTE_G5}, {1740, 4, NOTE_E5},
  {1930, 4, NOTE_E5}, {2120, 4, NOTE_D5}, {2310, 3, NOTE_C5}, {2500, 4, NOTE_D5},
  // Second phrase
  {2880, 2, NOTE_A4}, {3070, 3, NOTE_C5}, {3260, 4, NOTE_D5}, {3380, 4, NOTE_D5},
  {3500, 4, NOTE_D5}, {3690, 4, NOTE_E5}, {3880, 4, NOTE_F5}, {4000, 4, NOTE_F5},
  {4120, 4, NOTE_F5}, {4240, 4, NOTE_F5}, {4430, 4, NOTE_G5}, {4620, 4, NOTE_E5},
  {4810, 4, NOTE_E5}, {5000, 4, NOTE_D5}, {5190, 3, NOTE_C5}, {5380, 4, NOTE_D5},
  // Rising section: D-E-F-G-A
  {5760, 0, NOTE_D4}, {5950, 1, NOTE_E4}, {6140, 1, NOTE_F4}, {6330, 2, NOTE_G4},
  {6520, 2, NOTE_A4}, {6710, 2, NOTE_A4}, {6900, 2, NOTE_G4}, {7090, 1, NOTE_F4},
  {7280, 1, NOTE_E4}, {7470, 0, NOTE_D4}, {7660, 1, NOTE_E4}, {7850, 0, NOTE_D4},
  // Repeat main theme
  {8230, 2, NOTE_A4}, {8420, 3, NOTE_C5}, {8610, 4, NOTE_D5}, {8730, 4, NOTE_D5},
  {8850, 4, NOTE_E5}, {9040, 4, NOTE_F5}, {9230, 4, NOTE_G5}, {9420, 4, NOTE_E5},
  {9610, 4, NOTE_D5}, {9800, 3, NOTE_C5}, {9990, 4, NOTE_D5},
  // Dramatic ascending run
  {10380, 0, NOTE_D4}, {10570, 1, NOTE_E4}, {10760, 2, NOTE_G4}, {10950, 2, NOTE_A4},
  {11140, 3, NOTE_B4}, {11330, 3, NOTE_C5}, {11520, 4, NOTE_D5},
  {11710, 3, NOTE_C5}, {11900, 2, NOTE_A4}, {12090, 2, NOTE_G4}, {12280, 2, NOTE_A4},
  // Final phrase
  {12660, 2, NOTE_A4}, {12850, 3, NOTE_C5}, {13040, 4, NOTE_D5},
  {13230, 3, NOTE_C5}, {13420, 2, NOTE_A4}, {13610, 2, NOTE_G4},
  {13800, 0, NOTE_D4}, {14000, 2, NOTE_A4},
};
#define SONG7_LEN 70

// Song 9: Jaws Theme (Easy) - 2-note terror
const RhythmNote PROGMEM song8_notes[] = {
  // The iconic E-F pattern, starts slow, gets faster
  {0, 0, NOTE_E3}, {1000, 1, NOTE_F3},
  {2000, 0, NOTE_E3}, {2900, 1, NOTE_F3},
  {3700, 0, NOTE_E3}, {4400, 1, NOTE_F3},
  {5000, 0, NOTE_E3}, {5500, 1, NOTE_F3},
  // Getting faster
  {6000, 0, NOTE_E3}, {6400, 1, NOTE_F3},
  {6800, 0, NOTE_E3}, {7150, 1, NOTE_F3},
  {7500, 0, NOTE_E3}, {7800, 1, NOTE_F3},
  {8100, 0, NOTE_E3}, {8350, 1, NOTE_F3},
  // Fast now - shark approaching
  {8600, 0, NOTE_E3}, {8800, 1, NOTE_F3},
  {9000, 0, NOTE_E3}, {9200, 1, NOTE_F3},
  {9400, 0, NOTE_E3}, {9550, 1, NOTE_F3},
  {9700, 0, NOTE_E3}, {9850, 1, NOTE_F3},
  // Frantic
  {10000, 0, NOTE_E3}, {10120, 1, NOTE_F3},
  {10240, 0, NOTE_E3}, {10360, 1, NOTE_F3},
  {10480, 0, NOTE_E3}, {10600, 1, NOTE_F3},
  {10720, 0, NOTE_E3}, {10840, 1, NOTE_F3},
  // ATTACK! Big ascending stabs
  {11100, 2, NOTE_G4}, {11250, 3, NOTE_A4}, {11400, 4, NOTE_B4},
  {11600, 4, NOTE_C5}, {11800, 4, NOTE_D5}, {12000, 4, NOTE_E5},
  // Calm after the storm
  {12500, 0, NOTE_E3}, {13000, 1, NOTE_F3},
  {13500, 0, NOTE_E3}, {14000, 1, NOTE_F3},
  {14500, 0, NOTE_E3},
};
#define SONG8_LEN 43

// Song 10: Never Gonna Give You Up - Rick Astley (Medium)
const RhythmNote PROGMEM song9_notes[] = {
  // Lanes: D4=0, E4=1, G4=2, A4=3, B4=4
  // Synth riff: D-E-G-E-B-B-A (rest) D-E-G-E-A-A-G-E
  {0, 0, NOTE_D4}, {200, 1, NOTE_E4}, {400, 2, NOTE_G4}, {600, 1, NOTE_E4},
  {800, 4, NOTE_B4}, {1000, 4, NOTE_B4}, {1400, 3, NOTE_A4},
  // Second part of riff
  {1800, 0, NOTE_D4}, {2000, 1, NOTE_E4}, {2200, 2, NOTE_G4}, {2400, 1, NOTE_E4},
  {2600, 3, NOTE_A4}, {2800, 3, NOTE_A4}, {3200, 2, NOTE_G4}, {3400, 1, NOTE_E4},
  // Chorus: "Never gonna give you up" G-A-G-E-D-D-E-G
  {3800, 2, NOTE_G4}, {4000, 3, NOTE_A4}, {4200, 2, NOTE_G4}, {4400, 1, NOTE_E4},
  {4600, 0, NOTE_D4}, {4900, 0, NOTE_D4}, {5200, 1, NOTE_E4}, {5400, 2, NOTE_G4},
  // "Never gonna let you down" G-A-B-A-G-E-D
  {5800, 2, NOTE_G4}, {6000, 3, NOTE_A4}, {6200, 4, NOTE_B4}, {6400, 3, NOTE_A4},
  {6600, 2, NOTE_G4}, {6900, 1, NOTE_E4}, {7200, 0, NOTE_D4},
  // "Never gonna run around" D-E-G-E-A-G-E-D-E-G-A
  {7600, 0, NOTE_D4}, {7800, 1, NOTE_E4}, {8000, 2, NOTE_G4}, {8200, 1, NOTE_E4},
  {8400, 3, NOTE_A4}, {8600, 2, NOTE_G4}, {8800, 1, NOTE_E4}, {9000, 0, NOTE_D4},
  {9200, 1, NOTE_E4}, {9400, 2, NOTE_G4}, {9600, 3, NOTE_A4},
  // Riff repeat
  {10000, 0, NOTE_D4}, {10200, 1, NOTE_E4}, {10400, 2, NOTE_G4}, {10600, 1, NOTE_E4},
  {10800, 4, NOTE_B4}, {11000, 4, NOTE_B4}, {11400, 3, NOTE_A4},
  // Chorus repeat
  {11800, 2, NOTE_G4}, {12000, 3, NOTE_A4}, {12200, 2, NOTE_G4}, {12400, 1, NOTE_E4},
  {12600, 0, NOTE_D4}, {12900, 0, NOTE_D4}, {13200, 1, NOTE_E4}, {13400, 2, NOTE_G4},
  {13600, 2, NOTE_G4}, {13800, 3, NOTE_A4}, {14000, 4, NOTE_B4}, {14200, 3, NOTE_A4},
  {14400, 2, NOTE_G4}, {14700, 1, NOTE_E4}, {15000, 0, NOTE_D4},
};
#define SONG9_LEN 58

// Song 11: Imperial March - Darth Vader (Medium)
const RhythmNote PROGMEM song10_notes[] = {
  // Lanes: DS4=0, FS4=1, G4=2, AS4=3, D5/DS5=4
  // DUN DUN DUN dun-da-DUN dun-da-DUN
  {0, 2, NOTE_G4}, {500, 2, NOTE_G4}, {1000, 2, NOTE_G4},
  {1500, 0, NOTE_DS4}, {1750, 3, NOTE_AS4},
  {2000, 2, NOTE_G4}, {2500, 0, NOTE_DS4}, {2750, 3, NOTE_AS4},
  {3000, 2, NOTE_G4},
  // Second phrase higher: D5-D5-D5-Eb5-Bb4-F#4-Eb4-Bb4-G4
  {4000, 4, NOTE_D5}, {4500, 4, NOTE_D5}, {5000, 4, NOTE_D5},
  {5500, 4, NOTE_DS5}, {5750, 3, NOTE_AS4},
  {6000, 1, NOTE_FS4}, {6500, 0, NOTE_DS4}, {6750, 3, NOTE_AS4},
  {7000, 2, NOTE_G4},
  // Repeat main theme
  {8000, 2, NOTE_G4}, {8500, 2, NOTE_G4}, {9000, 2, NOTE_G4},
  {9500, 0, NOTE_DS4}, {9750, 3, NOTE_AS4},
  {10000, 2, NOTE_G4}, {10500, 0, NOTE_DS4}, {10750, 3, NOTE_AS4},
  {11000, 2, NOTE_G4},
  // Second phrase repeat
  {12000, 4, NOTE_D5}, {12500, 4, NOTE_D5}, {13000, 4, NOTE_D5},
  {13500, 4, NOTE_DS5}, {13750, 3, NOTE_AS4},
  {14000, 1, NOTE_FS4}, {14500, 0, NOTE_DS4}, {14750, 3, NOTE_AS4},
  {15000, 2, NOTE_G4},
  // Final DUN DUN DUN
  {16000, 2, NOTE_G4}, {16500, 2, NOTE_G4}, {17000, 2, NOTE_G4},
  {17500, 0, NOTE_DS4}, {17750, 3, NOTE_AS4},
  {18000, 2, NOTE_G4},
};
#define SONG10_LEN 42

// Song 12: Pink Panther Theme (Medium) - chromatic sneak
const RhythmNote PROGMEM song11_notes[] = {
  // Lanes: D4=0, DS4/E4=1, G4/GS4=2, A4/AS4=3, C5/D5/DS5=4
  // Iconic motif: (rest) Eb-E-(pause)-G-Ab-(pause)-Eb-E-G-Ab-A-Bb-A-Ab-G-E
  // With swing/dotted feel
  {500, 1, NOTE_DS4}, {850, 1, NOTE_E4},
  {1500, 2, NOTE_G4}, {1850, 2, NOTE_GS4},
  {2500, 1, NOTE_DS4}, {2850, 1, NOTE_E4},
  {3200, 2, NOTE_G4}, {3550, 2, NOTE_GS4},
  // Ascending chromatic phrase
  {4000, 3, NOTE_A4}, {4200, 3, NOTE_AS4}, {4400, 3, NOTE_A4},
  {4600, 2, NOTE_GS4}, {4800, 2, NOTE_G4}, {5200, 1, NOTE_E4},
  // Repeat main motif
  {5800, 1, NOTE_DS4}, {6150, 1, NOTE_E4},
  {6800, 2, NOTE_G4}, {7150, 2, NOTE_GS4},
  {7800, 1, NOTE_DS4}, {8150, 1, NOTE_E4},
  {8500, 2, NOTE_G4}, {8850, 2, NOTE_GS4},
  // High section
  {9300, 4, NOTE_C5}, {9500, 4, NOTE_D5}, {9700, 4, NOTE_DS5},
  {10000, 4, NOTE_C5}, {10300, 2, NOTE_G4}, {10600, 1, NOTE_E4},
  // Sneaky walk down
  {11100, 3, NOTE_A4}, {11300, 2, NOTE_GS4}, {11500, 2, NOTE_G4},
  {11800, 1, NOTE_E4}, {12100, 1, NOTE_DS4}, {12400, 0, NOTE_D4},
  // Return to main theme
  {12900, 1, NOTE_DS4}, {13250, 1, NOTE_E4},
  {13900, 2, NOTE_G4}, {14250, 2, NOTE_GS4},
  {14600, 3, NOTE_A4}, {14800, 3, NOTE_AS4}, {15000, 3, NOTE_A4},
  {15200, 2, NOTE_GS4},
  // Ending
  {15600, 2, NOTE_G4}, {15900, 1, NOTE_E4}, {16300, 1, NOTE_DS4},
  {16800, 1, NOTE_E4},
};
#define SONG11_LEN 44

// Song metadata
const uint8_t NUM_SONGS = 12;

const uint8_t songLengths[] PROGMEM = {
  SONG0_LEN, SONG1_LEN, SONG2_LEN, SONG3_LEN,
  SONG4_LEN, SONG5_LEN, SONG6_LEN, SONG7_LEN,
  SONG8_LEN, SONG9_LEN, SONG10_LEN, SONG11_LEN
};

// Pointers to song arrays (stored in a regular array of PROGMEM pointers)
const RhythmNote* const songData[] PROGMEM = {
  song0_notes, song1_notes, song2_notes, song3_notes,
  song4_notes, song5_notes, song6_notes, song7_notes,
  song8_notes, song9_notes, song10_notes, song11_notes
};

// Song names in PROGMEM
const char sName0[] PROGMEM = "7 Nation Army";
const char sName1[] PROGMEM = "Sweet Child";
const char sName2[] PROGMEM = "Enter Sandman";
const char sName3[] PROGMEM = "Crazy Train";
const char sName4[] PROGMEM = "Thunderstruck";
const char sName5[] PROGMEM = "Fire & Flames";
const char sName6[] PROGMEM = "Free Bird";
const char sName7[] PROGMEM = "Pirates";
const char sName8[] PROGMEM = "Jaws";
const char sName9[] PROGMEM = "Rickroll";
const char sName10[] PROGMEM = "Imperial March";
const char sName11[] PROGMEM = "Pink Panther";

const char* const songNames[] PROGMEM = {
  sName0, sName1, sName2, sName3, sName4, sName5, sName6,
  sName7, sName8, sName9, sName10, sName11
};

// Difficulty strings in PROGMEM
const char sDiff0[] PROGMEM = "Easy";
const char sDiff1[] PROGMEM = "Medium";
const char sDiff2[] PROGMEM = "Medium";
const char sDiff3[] PROGMEM = "Med-Hard";
const char sDiff4[] PROGMEM = "Hard";
const char sDiff5[] PROGMEM = "Extreme";
const char sDiff6[] PROGMEM = "Hard";
const char sDiff7[] PROGMEM = "Med-Hard";
const char sDiff8[] PROGMEM = "Easy";
const char sDiff9[] PROGMEM = "Medium";
const char sDiff10[] PROGMEM = "Medium";
const char sDiff11[] PROGMEM = "Medium";

const char* const songDiffs[] PROGMEM = {
  sDiff0, sDiff1, sDiff2, sDiff3, sDiff4, sDiff5, sDiff6,
  sDiff7, sDiff8, sDiff9, sDiff10, sDiff11
};

/* ======================= RHYTHM GAME STATE ======================= */

const unsigned long RHYTHM_DEBOUNCE = 80;
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
const unsigned long HIGHWAY_WINDOW_MS = 4000; // notes visible 4s ahead

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
      if (timeUntil < -300) continue; // already missed
      if (timeUntil > 1500) break;    // too far away

      // Fade: 1500ms away = 20, 0ms = 255
      if (timeUntil <= 0) {
        brightness = 255;
      } else {
        brightness = (uint8_t)(255 - (timeUntil * 235L / 1500L));
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

    if (diff > 300) {
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
    if ((long)n.time_ms - (long)elapsed > 300) break;

    if (n.lane == lane) {
      long diff = abs((long)elapsed - (long)n.time_ms);
      if (diff < bestDiff) {
        bestDiff = diff;
        bestIdx = i;
      }
    }
  }

  if (bestIdx < 0 || bestDiff > 300) return; // no note to hit

  uint8_t mult = rhythmGetMultiplier();
  uint8_t hitType = 0;

  if (bestDiff <= 80) {
    // Perfect
    rhythmScore += 100UL * mult;
    rhythmPerfects++;
    rhythmCombo++;
    hitType = 1;
    // Sound override
    rhythmToneOverride = true;
    rhythmOverrideEnd = millis() + 30;
    tone(SPEAKER, 1600, 30);
  } else if (bestDiff <= 160) {
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
      if ((long)elapsed - (long)nn.time_ms > 300) {
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
