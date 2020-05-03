/*
 * I'm not hard coder, so my code is... You know what I mean, but it works...
 * 
 * I use 3 menu name : T (for top), C (for center), B (for bottom)
 * 
 * Top : just print the name of the current sequencer
 * Center : print the must common function : Modulo, Number of step, Phase and speed multiplier
 * Bottom : print the more specific function : scale, velocity range, randomness...
 */

#include <MIDI.h>
#include <Bounce.h>
#include "Trig.h"

#define TFT_MISO  12
#define TFT_MOSI  11  //a12
#define TFT_SCK   13  //a13
#define TFT_DC   9 
#define TFT_CS   10  
#define TFT_RST  8

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define BACKGROUND 0x0000
#define FORGROUND 0xFFFF

#define NBDETRIG 16
#define NBDESEQ 12

#include <Adafruit_GFX.h>    // Core graphics library
#include <ST7789_t3.h> // Hardware-specific library
#include <SPI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
const int channel = 1;

ST7789_t3 tft = ST7789_t3(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

float p = 3.1415926;

int memButton1;
int memEncState1;
int memEncA1;

int memButton2;
int memEncState2;
int memEncA2;

int memButton3;
int memEncState3;
int memEncA3;

int height = 160;
int width = 128;

float rayGen = 52;
float rayTrig = 5;

int time = 0;
float prevTime = 0.;
float speed = 0.001;

int curSeq = 1;
int nextSeq = 1;
bool needUpdateGenScreen = false;
int waitTime = 0;

void drawCentreString(const String &buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    tft.setCursor(x - w / 2, y);
    tft.print(buf);
}

struct DataMenuC {
  String name;
  int val;
  int min;
  int max;
};

struct DataMenuB {
  String name;
  int v1;
  int v2;
  int minV1;
  int maxV1;
  int minV2;
  int maxV2;
  String nameList[12];
};

struct Seq {
  
  String type;
  String name;
  int midiChannel;

  bool active;
  int curStep;
  int curMidiNote;
  
  int curMenuC;   
  int curMenuHoriB; 
  int curMenuVertB;

  int nextPhaz;

  float aleaData[10][NBDETRIG];
  Trig trig[NBDETRIG];
  
  struct DataMenuC dataMenuC[4];
  struct DataMenuB dataMenuB[2];
     
};

//Mode gate :
// 0 = oneStep
// 1 = toNextStep (note off only befor the next step)
// 2 = both depending of randomness... TODO

/*
 * les seq de pitch doivent toujours être avant les seq gate associé.
 * Un gate ira toujours chercher dans le seq précédent et si c'est un pitch il s'associe immédiatement
 * Cela permet d'actualiser les pitch avant les gates
 */

 //Octave, 5/8, 4/5/8, Penta1, Penta2,Maj, minHarmo, Arabo, -7, M7

String noteName[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
String modeName[12] = {"8ve", "5+8", "4+5+8", "Pent1", "Pent2", "Maj", "MinH", "Arab", "-7", "M7"};
int menuBWidth[3] = {30,75, 105};


// Init all the sequencer. This part is not optimize at all....

struct Seq seqArray [NBDESEQ] =
{
  {
  "gate",
  "BASS DRUM",
  1,
  false,
  0,
  36,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "gate",
  "SNARE",
  2,
  false,
  0,
  36,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "gate",
  "HH",
  3,
  false,
  0,
  36,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "gate",
  "PERC",
  4,
  false,
  0,
  36,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "pitch",
  "Pitch VOICE 1",
  5,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Pitch", 60, 0, 20, 100, 0, 50},
    {"Alea", 0, 1, 0, 9, 0, 9}}
  },
  {
  "gate",
  "Gate VOICE 1",
  5,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "pitch",
  "Pitch VOICE 2",
  6,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Pitch", 60, 0, 20, 100, 0, 50},
    {"Alea", 0, 1, 0, 9, 0, 9}}
  },
  {
  "gate",
  "Gate VOICE 2",
  6,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "pitch",
  "Pitch VOICE 3",
  7,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Pitch", 60, 0, 20, 100, 0, 50},
    {"Alea", 0, 1, 0, 9, 0, 9}}
  },
  {
  "gate",
  "Gate VOICE 3",
  7,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  },
  {
  "pitch",
  "Pitch VOICE 4",
  8,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Pitch", 60, 0, 20, 100, 0, 50},
    {"Alea", 0, 1, 0, 9, 0, 9}}
  },
  {
  "gate",
  "Gate VOICE 4",
  8,
  false,
  0,
  60,
  0,
  0,
  0,
  0,
  { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
  { Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0),
    Trig(0, 0), Trig(0, 0), Trig(0, 0), Trig(0, 0)},
  { {"MOD", 4, 1, NBDETRIG},
    {"PHAZ", 0, 0, NBDETRIG*4},
    {"STEP", 6, 1, NBDETRIG},
    {"SPEED", 1, 0, 11 }},
  { {"Velocity", 80, 100, 0, 120, 10, 127},
    {"Gate", 0, 1, 0, 2, 0, 9}}
  }
};

// Init the scales
// Octave, 5/8, 4/5/8, Penta1, Penta2,Maj, minHarmo, Arabo, -7, M7
int adaptPitch[10][12] = {
  {0,-1,-2,-3,-4,-5, 6, 5, 4, 3, 2, 1},
  {0,-1,-2,-3, 3, 2, 1, 0, 4, 3, 2, 1},
  {0,-1,-2,-3, 1, 0,-1, 0,-1,-2, 2, 1},
  {0,-1, 1, 0, 1, 0, 1, 0,-1, 1, 0, 1},
  {0,-1, 0,-1, 1, 0, 1, 0,-1, 0,-1, 1},
  {0,-1, 0,-1, 0, 0,-1, 0,-1, 0,-1, 0},
  {0,-1, 0, 0,-1, 0, 1, 0, 0,-1, 0,-1},
  {0, 0,-1, 1, 0, 0, 1, 0, 0,-1, 1, 0},
  {0,-1,-2, 0,-1,-2, 1, 0,-1, 1, 0,-1},
  {0,-1,-2, 1, 0,-1, 1, 0,-1, 2, 1, 0}
};

// Init the time multipler
int speedArray[12] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64};


void setup() {

  Serial.begin(9600);
  MIDI.begin();
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(BACKGROUND);

  // For each sequencer
  for(int s = 0 ; s < NBDESEQ ; s++){

    // Init the place of each trigger in the screen
    for(int i = 0 ; i < NBDETRIG ; i++){
      float x = cos(p*2/NBDETRIG * i) * rayGen + width/2;
      float y = sin(p*2/NBDETRIG * i) * rayGen + height/2;
      seqArray[s].trig[i].x = x;
      seqArray[s].trig[i].y = y;
      // Init the modulo
      if( i % seqArray[curSeq].dataMenuC[0].val  == 0){
        seqArray[s].trig[i].on = true;
      }
    }
    
    // Create every random data
    for(int i = 0 ; i < seqArray[s].dataMenuB[1].maxV2 ; i++){
      // Pour chaque trig
      for(int y = 0 ; y < NBDETRIG ; y++){
        if(i ==0){
          seqArray[s].aleaData[i][y] = (float)(y)/(float)NBDETRIG;
        } else {
          seqArray[s].aleaData[i][y] = random(0,100)/100.;
        }
        
      }
    }
  }

  // Init Encoder 1
  pinMode(21, INPUT_PULLUP); //Button
  pinMode(19, INPUT_PULLUP); //A
  pinMode(20, INPUT_PULLUP); //B

  memButton1 = digitalRead(21);
  memEncA1 =  digitalRead(19);

  // Init Encoder 2
  pinMode(16, INPUT_PULLUP); //Button
  pinMode(18, INPUT_PULLUP); //A
  pinMode(17, INPUT_PULLUP); //B
  
  memButton2 = digitalRead(17);
  memEncA2 =  digitalRead(16);

  // Init Encoder 3
  pinMode(4, INPUT_PULLUP); //Button
  pinMode(6, INPUT_PULLUP); //A
  pinMode(5, INPUT_PULLUP); //B
  
  memButton3 = digitalRead(4);
  memEncA3 =  digitalRead(6);

  // Init the screen
  updateGenScreen();
  
}

void loop() {

  // update current time
  float curTime = prevTime + (millis()-time) * 0.001;

  
  for(int s = 0 ; s < NBDESEQ ; s++){

    // For each sequencer get the speed multiplier
    int tempModulo = speedArray[seqArray[s].dataMenuC[3].val];
    
    // For each sequencer, if you move to the next step
    if(fmod(curTime*(float)tempModulo, 1.0) < fmod(prevTime*(float)tempModulo,1.0) && seqArray[s].active)
    { 
      // Si tu es un seqGate et que tu est en mode OneStep et que ton pas précédent était on alors
      // If this sequencer is a gate one and the mode is 0, and the previous note was on, send note off
      if(seqArray[s].dataMenuB[1].v1 == 0
         && seqArray[s].trig[seqArray[s].curStep].on
         && seqArray[s].type == "gate"){

        MIDI.sendNoteOff(seqArray[s].curMidiNote, 0, seqArray[s].midiChannel); 
        
      }

      // get the number of this step according to the phase.
      int tempPhazCurStep = (seqArray[s].curStep + seqArray[s].dataMenuC[1].val) % NBDETRIG;
      
      //If your the seq show in the screen, erase the last clock circle;
      if(s == curSeq){
        tft.drawCircle(seqArray[s].trig[tempPhazCurStep].x, seqArray[s].trig[tempPhazCurStep].y, 7, BACKGROUND); 
      }

      //Get the new phase modifier as the current one;
      seqArray[s].dataMenuC[1].val = seqArray[s].nextPhaz;
      // Update the current trig number
      seqArray[s].curStep = max(0, (seqArray[s].curStep + 1) % seqArray[s].dataMenuC[2].val);
      // get the number of this step according to the new phase.
      tempPhazCurStep = (seqArray[s].curStep + seqArray[s].dataMenuC[1].val) % NBDETRIG;

      // If you're a gate Seq :
      if(seqArray[s].type == "gate" 
         && seqArray[s].trig[tempPhazCurStep].on){

        // Send note off to the old note
        MIDI.sendNoteOff(seqArray[s].curMidiNote, 0, seqArray[s].midiChannel);
        // Get the new note to the pitch sequencer (if you've one...)
        if(seqArray[max(0,s-1)].type == "pitch"){
          seqArray[s].curMidiNote = seqArray[s-1].curMidiNote;
        }
        // Get the current velocity
        int tempVel = (int)
            abs(
              seqArray[s].aleaData[seqArray[s].dataMenuB[1].v2][seqArray[s].curStep]
              *
              (seqArray[s].dataMenuB[0].v2 - seqArray[s].dataMenuB[0].v1) + seqArray[s].dataMenuB[0].v1
            );
         // Send midi note !
         MIDI.sendNoteOn(seqArray[s].curMidiNote, tempVel, seqArray[s].midiChannel);
        
      }

      // If you're a gate Seq :
      if(seqArray[s].type == "pitch" 
         && seqArray[s].trig[tempPhazCurStep].on){

        // Actualize your pitch
        int tempPitch = (int)
          ((seqArray[s].aleaData[seqArray[s].dataMenuB[1].v2][tempPhazCurStep] - 0.5)
          * seqArray[s].dataMenuB[0].v2
          + seqArray[s].dataMenuB[0].v1);

        // get the good scale !
        tempPitch += 
          adaptPitch[seqArray[s].dataMenuB[1].v1]
            [
            max(0, 
            (tempPitch - seqArray[s].dataMenuB[0].v1%12)%12)];
        
        
        seqArray[s].curMidiNote = tempPitch;
          
      }

      //If your the seq show in the screen, draw the new clock circle;
      if(s == curSeq){
        tft.drawCircle(seqArray[s].trig[tempPhazCurStep].x, seqArray[s].trig[tempPhazCurStep].y, 7, BLUE); 
      }
    }
  }
  

  prevTime = curTime;

   ///// Encoder 1 - Menu Center//////  
  if(digitalRead(21) < memButton1) {
    genClickMenuC();
  }

  if(memEncA1 != digitalRead(19)){
    
    if(digitalRead(19) != digitalRead(20)){
      genTourneMenuC(1);
    } else {
      genTourneMenuC(-1);
    } 
  }

  memButton1 = digitalRead(21);
  memEncA1 =  digitalRead(19);

  ///// Encoder 2 - Menu Bottom//////  
  if(digitalRead(16) < memButton2) {
    genClickMenuB();
  }

  if(memEncA2 != digitalRead(18)){
    if(digitalRead(18) != digitalRead(17)){
      genTourneMenuB(1);
    } else {
      genTourneMenuB(-1);
    } 
  }

  memButton2 = digitalRead(16);
  memEncA2 =  digitalRead(18);

  ///// Encoder 3 - Menu Top//////  
  if(digitalRead(4) < memButton3) {
    genClickMenuT();
  }

  if(memEncA3 != digitalRead(6)){
    
    needUpdateGenScreen = true;
    waitTime = millis();
    if(digitalRead(6) != digitalRead(5)){
      genTourneMenuT(1);
    } else {
      genTourneMenuT(-1);
    } 
  }

  // This is a timer, because when I turn the encoder 3, I need to refresh all the screen. I change my sequencer.
  // Here, the programm wait 500ms before update the screen.
  // So, if you want to go from sequencer 1 to 8 the programm wait and print only the 8, not the 2, the 3, the 4...
  // That's very important to keep the clock.
  if(waitTime < millis() - 500 && needUpdateGenScreen){  
    if(curSeq!=nextSeq){
      updateGenScreen();
    }
    needUpdateGenScreen = false;
  }

  memButton3 = digitalRead(4);
  memEncA3 =  digitalRead(6);

  time = millis();
  delay(10);
}


// When I click on the encoder for Menu Center
void genClickMenuC() {
  
  // Supprime l'ancien texte
  tft.setTextColor(BACKGROUND);
  genAfficheMenuC();

  // Change le courant
  seqArray[curSeq].curMenuC = (seqArray[curSeq].curMenuC + 1) % 4; // Avec le nombre de menu C
  
  // Affiche le nouveau texte
  tft.setTextColor(FORGROUND);
  genAfficheMenuC();

}

// When I turn the encoder for Menu Center
void genTourneMenuC(int _val) {

  //Serial.println(_sens);
  switch(seqArray[curSeq].curMenuC){
    case 0 :
      genAfficheActiveTrig(false);
      seqArray[curSeq].dataMenuC[0].val = max(seqArray[curSeq].dataMenuC[0].min, min(seqArray[curSeq].dataMenuC[0].max, seqArray[curSeq].dataMenuC[0].val + _val));
      genAfficheActiveTrig(true);
      break;
    case 1 :
      seqArray[curSeq].nextPhaz =  max(seqArray[curSeq].dataMenuC[1].min, min(seqArray[curSeq].dataMenuC[1].max, seqArray[curSeq].nextPhaz + _val));
      genModifPhaz(_val);//genActualizeNbStep();
      break;
    case 2 :
      seqArray[curSeq].dataMenuC[2].val = max(seqArray[curSeq].dataMenuC[2].min, min(seqArray[curSeq].dataMenuC[2].max, seqArray[curSeq].dataMenuC[2].val + _val));
      //genActualizeNbStep();
      genModif1Trig(_val);
      break;
    case 3 :
      tft.setTextColor(BACKGROUND);
      genAfficheMenuC();   
      seqArray[curSeq].dataMenuC[3].val = max(seqArray[curSeq].dataMenuC[3].min, min(seqArray[curSeq].dataMenuC[3].max, seqArray[curSeq].dataMenuC[3].val + _val));   
      tft.setTextColor(FORGROUND);
      genAfficheMenuC();
      break;
  }  
}

// When I click on the encoder for Menu Bottom
void genClickMenuB() {
  genAfficheOneMenuB(seqArray[curSeq].curMenuHoriB, RED);
  seqArray[curSeq].curMenuHoriB = (seqArray[curSeq].curMenuHoriB + 1) % 3;
  genAfficheOneMenuB(seqArray[curSeq].curMenuHoriB, FORGROUND);
}

// When I turn the encoder for Menu Bottom
void genTourneMenuB(int val) {
 
  switch (seqArray[curSeq].curMenuHoriB) {
    case 0 : 
      genInitMenuB(false);
      seqArray[curSeq].curMenuVertB = max(0, min(1, seqArray[curSeq].curMenuVertB + val));
      genInitMenuB(true);
      break;
    case 1 : 
      genAfficheOneMenuB(seqArray[curSeq].curMenuHoriB, BACKGROUND);
      seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].v1 = 
          min(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].maxV1,
          max(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].minV1, 
              seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].v1 + val));
      genAfficheOneMenuB(seqArray[curSeq].curMenuHoriB, FORGROUND);
      break;
    case 2 :
      genAfficheOneMenuB(seqArray[curSeq].curMenuHoriB, BACKGROUND);
      seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].v2 = 
          min(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].maxV2,
          max(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].minV2, 
              seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].v2 + val));
      genAfficheOneMenuB(seqArray[curSeq].curMenuHoriB, FORGROUND);
      break;     
  }
}

// When I click on the encoder for Menu Top
void genClickMenuT(){
  if(seqArray[curSeq].active){
    seqArray[curSeq].active = false;
  } else {
    seqArray[curSeq].active = true;
  }
}

// When I turn the encoder for Menu Top
void genTourneMenuT(int _val) {

  int tempCurSeq = max(0, min(NBDESEQ-1, (nextSeq + _val)));

  if(nextSeq!=tempCurSeq){
    tft.setTextColor(BACKGROUND);

    //EraseMenuT
    drawCentreString(seqArray[nextSeq].name, width/2, 10);

    nextSeq = tempCurSeq;

    tft.setTextColor(BLUE);
    //PrintMenuT
    drawCentreString(seqArray[nextSeq].name, width/2, 10);
       
  }
}

// Update the all screen.
void updateGenScreen(){
  tft.setTextColor(BACKGROUND);

  //InitMenuB
  genInitMenuB(false);
  //Efface les actives Trig
  genSimpleAfficheTrig(false);
  //EffaceMenuC 
  genAfficheMenuC();

  int tempPhazCurStep = (seqArray[curSeq].curStep + seqArray[curSeq].dataMenuC[1].val) % NBDETRIG;
  //EffaceCounter
  tft.drawCircle(seqArray[curSeq].trig[tempPhazCurStep].x, seqArray[curSeq].trig[tempPhazCurStep].y, 7, BACKGROUND); 

  curSeq = nextSeq;

  tempPhazCurStep = (seqArray[curSeq].curStep + seqArray[curSeq].dataMenuC[1].val) % NBDETRIG;

  tft.setTextColor(FORGROUND);

  //EffaceMenuC 
  genAfficheMenuC();
  //Affiche Menu Top dans la bonne couleur
  drawCentreString(seqArray[nextSeq].name, width/2, 10);
  //InitMenuB
  genInitMenuB(true);
  //Efface les actives Trig
  genSimpleAfficheTrig(true);
   
  //EffaceCounter
  tft.drawCircle(seqArray[curSeq].trig[tempPhazCurStep].x, seqArray[curSeq].trig[tempPhazCurStep].y, 7, BLUE); 
  genActualizeNbStep();
}

// Actualize the number of step, on and off according to the phase
void genActualizeNbStep(){
  for (int i = 0 ; i < NBDETRIG ; i++) {
    int updateI = (i-seqArray[curSeq].nextPhaz+seqArray[curSeq].dataMenuC[1].max) % NBDETRIG;
    if(updateI < seqArray[curSeq].dataMenuC[2].val){
      tft.drawCircle(seqArray[curSeq].trig[i].x, seqArray[curSeq].trig[i].y, 10, FORGROUND); 
    } else {
      tft.drawCircle(seqArray[curSeq].trig[i].x, seqArray[curSeq].trig[i].y, 10, RED);  
    }    
  }
}

// Actualize only the first and the last step according to the number of step and phase.
// This is an optimize version of "getActualizeNbStep()"
void genModif1Trig(int val){

  int updateI = (((seqArray[curSeq].dataMenuC[2].val-1)+seqArray[curSeq].nextPhaz+seqArray[curSeq].dataMenuC[1].max) % NBDETRIG);
  if (val == 1) {
    //int updateI = (seqArray[curSeq].dataMenuC[2].val-seqArray[curSeq].nextPhaz+seqArray[curSeq].dataMenuC[1].max) % NBDETRIG;
    //if(updateI < seqArray[curSeq].dataMenuC[2].val){
    tft.drawCircle(seqArray[curSeq].trig[updateI].x, seqArray[curSeq].trig[updateI].y, 10, FORGROUND); 
  } else {
    tft.drawCircle(seqArray[curSeq].trig[updateI+1].x, seqArray[curSeq].trig[updateI+1].y, 10, RED);  
  }     
}

// Modify the phase
void genModifPhaz(int val){
  int nextMax = (((seqArray[curSeq].dataMenuC[2].val-1)+seqArray[curSeq].nextPhaz+seqArray[curSeq].dataMenuC[1].max) % NBDETRIG);
  int nextMin = (seqArray[curSeq].nextPhaz) % NBDETRIG;
  if (val == 1) {
    //int updateI = (seqArray[curSeq].dataMenuC[2].val-seqArray[curSeq].nextPhaz+seqArray[curSeq].dataMenuC[1].max) % NBDETRIG;
    //if(updateI < seqArray[curSeq].dataMenuC[2].val){
    tft.drawCircle(seqArray[curSeq].trig[nextMax].x, seqArray[curSeq].trig[nextMax].y, 10, FORGROUND); 
    if(nextMin==0){
      tft.drawCircle(seqArray[curSeq].trig[NBDETRIG-1].x, seqArray[curSeq].trig[NBDETRIG-1].y, 10, RED);
    } else {
      tft.drawCircle(seqArray[curSeq].trig[nextMin-1].x, seqArray[curSeq].trig[nextMin-1].y, 10, RED);
    }
    
  } else {
    tft.drawCircle(seqArray[curSeq].trig[(nextMax+1)%NBDETRIG].x, seqArray[curSeq].trig[(nextMax+1)%NBDETRIG].y, 10, RED);
    tft.drawCircle(seqArray[curSeq].trig[nextMin].x, seqArray[curSeq].trig[nextMin].y, 10, FORGROUND);  
  }  
}


void genSimpleAfficheTrig(bool visible) {
  for(int i = 0 ; i < NBDETRIG ; i++){

    if( i % seqArray[curSeq].dataMenuC[0].val  == 0 && visible){
      tft.fillCircle(seqArray[curSeq].trig[i].x, seqArray[curSeq].trig[i].y, 4, FORGROUND);
    } else if( i % seqArray[curSeq].dataMenuC[0].val == 0){
      tft.fillCircle(seqArray[curSeq].trig[i].x, seqArray[curSeq].trig[i].y, 4, BACKGROUND);
    }
  }
}

void genAfficheActiveTrig(bool visible) {
  for(int i = 0 ; i < NBDETRIG ; i++){

    if( i % seqArray[curSeq].dataMenuC[0].val  == 0 && visible){
      tft.fillCircle(seqArray[curSeq].trig[i].x, seqArray[curSeq].trig[i].y, 4, FORGROUND);
      seqArray[curSeq].trig[i].on = true;
    } else if( i % seqArray[curSeq].dataMenuC[0].val == 0){
      tft.fillCircle(seqArray[curSeq].trig[i].x, seqArray[curSeq].trig[i].y, 4, BACKGROUND);
      seqArray[curSeq].trig[i].on = false;
    }
  }
}

void genAfficheOneMenuB(int id, uint16_t color) {

  tft.setTextColor(color);
  int v1;
  
  switch (id) {
    case 0 : drawCentreString(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].name , menuBWidth[0], height-10); break;
    case 1 : 
      v1 = seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].v1;
      if(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].name =="Pitch"){
        
        drawCentreString(noteName[v1%12] + v1/12 , menuBWidth[1], height-10); 

      } else if (seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].name =="Alea"){
        
        drawCentreString(modeName[v1], menuBWidth[1], height-10); 
        
      } else {
        
        drawCentreString(v1 , menuBWidth[1], height-10);
        
      };
      break;
    case 2 : drawCentreString(seqArray[curSeq].dataMenuB[seqArray[curSeq].curMenuVertB].v2 , menuBWidth[2], height-10); break;
  }  
}

void genAfficheMenuC(){

  drawCentreString(seqArray[curSeq].dataMenuC[seqArray[curSeq].curMenuC].name, width/2, height/2-5); 
  if(seqArray[curSeq].dataMenuC[seqArray[curSeq].curMenuC].name == "SPEED"){
    drawCentreString(speedArray[seqArray[curSeq].dataMenuC[seqArray[curSeq].curMenuC].val], width/2, height/2+5);
  }
}

void genInitMenuB(bool visible) { 
  if(visible){
    seqArray[curSeq].curMenuHoriB = 0;
    genAfficheOneMenuB(0, FORGROUND);
    genAfficheOneMenuB(1, RED);
    genAfficheOneMenuB(2, RED);
  } else {
    genAfficheOneMenuB(0, BACKGROUND);
    genAfficheOneMenuB(1, BACKGROUND);
    genAfficheOneMenuB(2, BACKGROUND);
  }   
}
