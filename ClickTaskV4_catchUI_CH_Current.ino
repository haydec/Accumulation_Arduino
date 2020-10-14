/*
 Name:		ClickTaskV4_catchUI_CH_Current.ino
 Created:	10/14/2020 8:43:15 AM
 Author:	cilli
*/



/*
Name:		ClickTaskV4_catchUI_CH.ino
Created:	9/4/2020 12:54:45 PM
Author:	cilli
*/

#include <EEPROM.h>
#include <Event.h>
#include <Timer.h>
#include <LED.h>
#include <Wire.h>
#include <Servo.h>
#include <stdio.h> // Added by Cillian
#include <math.h>  // Added by Cillian

//#define DEBUG
/******************** PIN DEFINITIONS ******************************************/
#define ServoPin1       A5//connected to servo// modify this
#define ServoPin2       A6//connected to servo
#define ServoPin3       A4//connected to servo
//#define innerServoPin 48

#define RewardPinLeft  52//0 //connected to left pump
#define RewardPinRight 52//42 //conntect to right pump

#define StimOnTTL      38 //tells neuralynx that stimulus is beginning https://neuralynx.com/hardware/category/microdrives

#define CueTTL         4 //high if cue playing (M=1 on TDT)
#define StimTTL        5 //high if stim playing (M=2 on TDT)
#define ChoiceTTL      6 //high if choice 1 playing (M=4 on TDT)

#define YellowLaserPin 38

//IR Diode Pins
#define IR_LL   7   //Outer Left
#define IR_CL   16   //Inner Left
#define IR_C    17   //Center
#define IR_CR   15  //Inner Right
#define IR_RR   6  //Outer Right

//IR TRANSISTORS
#define TransPin_OL  13
#define TransPin_IL  18
#define TransPin_C   19
#define TransPin_IR  20
#define TransPin_OR  15

//IR leds used for pokes.
LED ledIR1 = LED(IR_LL);  //Outer Left
LED ledIR2 = LED(IR_CL);  //Inner Left
LED ledIR3 = LED(IR_C);  //Center
LED ledIR4 = LED(IR_CR);  //Inner Right
LED ledIR5 = LED(IR_RR);  //Outer Right

#define WhiteLED_R      8 //white led right
#define WhiteLED_L     11 //white led left

#define BlueLedPin     42
#define GreenLedPin    44
#define whiteLedPin    32

// Define Servo Positions // needs modification
#define ServoOpen         10//position at which Servo is open
#define ServoClosed       160 //position at which Servo is closed
#define ServoOpenBottom   10
#define ServoOpenTop      10
#define ServoClosedCenter 160


// Define time delays (in ms)
#define PunishDelay 10000  //corresponds to waiting period following in poke
#define MissDelay    600000  //corresponds to time that a mouse has to complete the trial
#define RewardDelay 10000  //corresponds to period that mouse has to collect reward following correct poke
#define BtwPipDelay 25 //corresponds to delay period between sounds played
#define PipDuration 50 //corresponds to length of sound played

// Declare LEDs
LED ledR = LED(WhiteLED_R);  //corresponds to white LED to right of center poke
LED ledL = LED(WhiteLED_L); //corresponds to white LED to left of center poke
LED blue_led = LED(BlueLedPin);
LED green_led = LED(GreenLedPin);
LED white_led = LED(whiteLedPin);


// Introduce other variables to be used by program
//these might be changed from session to session
int required = 100; //minimum holding time required during initiation, in ms
int jitterLength = 0;
int cueLength = 100;
int openDelay = 500; //delay between end of stimulus and door opening
int stimDuration = 10; // length of the stim presentation, ms
int modeRepeatChance = 100; // p that modality will repeat if mouse gets it wrong
//int sideRepeatChance  =  80; // probability that the side wil repeat if mouse gets it wrong, number between 1 and 100
bool CueFlag = true;

// yellow laser.............................................................
int yellowLaserOnTime = 0; // time after mouse triggers initiation to turn laser on, in ms
int yellowLaserDuration = required; // duration of the yellow laser in ms
int yellowCorrect = 2; // number of trials mouse has to get correct before a laser trial triggers

bool modalityBlock = false; // true=modality per block, false=use sequence for modality



//side sequence: indicates what side animal should go towards
// 3=left, 4=right

int side[] = { 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 3, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 3, 3, 4, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 3, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 3, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 3, 4, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3, 4, 3, 4, 3, 4, 4, 3, 4, 3, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 4, 3, 3 };

// Trial sequence
//int sequence[]= {side-1};

int numBlocks = 1;
int trialsPerBlock[] = { 400 }; // max # of trials (correct+in) in a block
int modalityPerBlock[] = { 6 }; // 0=aud no dist, 1=vis no dist, 2=aud dist, 3=vis dist, 6=use array sequence to control modality
int cueContextPerBlock[] = { 1 }; //0=green/uv led with dimmed blue/brown, 1=blue/brown noise, 2=blue noise/green led, 3=brown noise/uv, 4=G/HP for vis, UV/LP for aud, 5=HP/G for vis, LP/UV for aud
int accumulationPerBlock[] = { 2 }; //0=regular cue (1 pulse of cueLength duration), 1=fixed percentage, 2=easy to hard, 3=random percentage
double percentPerBlock[] = { 1 }; // percent of target cue
bool laserPerBlock[] = { false }; //true=laser will be used during block, false=no laser

double p = 1.0; // percentage correct that triggers next block, outdated
int nWindow = 1000; // number of trials used to calculate performance

//Increase with number of cues desired change in tandem with required
const int num_pips = 10; // number of cue pips played

// for accumulation case 2, easy to hard gradient
double pEasy = 1; // percentage of target cue that block starts off on
double pHard = 0.9; // percentage of target cue that block ends on

int n_max_in = 3; // number of ins before the wrong IR barrier is deactivated/right reward is pumped

// ******************************** Added By Cillian ***************************************//


int Reward_Type = 0; // added by Cillian for Reward_side function
const int RewardPeriod = 100; //Added by Cillian

//False = closed, True = Open
volatile bool PokeStateR = false;   //Added by Cillian
volatile bool PokeStateL = false;   //Added by Cillian
volatile bool ButtonPressed = false; //Added by Cillian

static float* U = new float[10]; 
static float* P = new float[10];
unsigned long* X = new unsigned long[10];

// Radnom0 Values for LCG (Ref Matlab for engineers by Stephen Chapman)
unsigned long  mod = 134456;
unsigned long  Seed = random(0, mod);
unsigned long a = 8121; 
unsigned long c = 28411;


float LowProb = 0.1;  //Inputs for LCG
float HighProb = 0.9; //Inputs for LCG
int N = 10; // number of elements generated between high and low probability 



float pseudo_uniform(float Low, float High, int N) {

    /*Linear congruential generator*/
    //https://en.wikipedia.org/wiki/Linear_congruential_generator

    X[0] = (a * X[9] + c) % mod;
    U[0] = ((float)X[9]) / mod;
  

    for (int i = 1; i < N; i++) {

        X[i] = (a * X[i - 1] + c) % mod;
        U[i] = ((float)X[i]) / mod;
    }

    /*Convert to desired probability range*/
    for (int i = 0; i < N; i++) {
        P[i] = Low + (High - Low) * U[i];
    }
    randomSeed(analogRead(0));
    int index = random(0, N); // NOT UNIFORM for Low N



    return P[index];
}













// ******************************** Added By Cillian ***************************************//





/*********************************** REST OF CODE ******************************************/
Timer t;
Timer h;
int tids[35];

int delayDuration = required - stimDuration; // this timer is set in on_cue_enter

Servo servo1;
Servo servo2;
Servo servo3;

double pCue;
void training_fxn(int); //Used to interpret input to control pokes during training

//array sizes
const size_t nSide = (size_t)sizeof(side) / sizeof(side[0]);

// serLogTime prints timestamp to log
void serLogTime() {
#ifndef DEBUG
    unsigned long logTime = millis();
    // Serial.print("TIME");
    Serial.print(logTime);
    Serial.print("\n");
#endif
}

void serDebug(String str) {
#ifdef DEBUG
    Serial.print(millis());
    Serial.print(" ");
    Serial.print(str);
    Serial.print("\n");
#endif
}

// serLog prints messages to log
void serLog(String str, bool debugVar = false) {
#ifndef DEBUG
    Serial.print("LOG ");
    Serial.print(str);
    Serial.print("\n");

    Serial.print("TIME");
    Serial.print(millis());
    Serial.print("\n");
#endif
    if (debugVar) serDebug(str);
}


namespace Machine {
    enum State { ST_IDLE, ST_TRG, ST_CUE, ST_DELAY, ST_STIM, ST_RESPONSE, ST_REWARD, ST_PUNISH, ST_MISS, nStates };
    enum Event { EVT_CORR_ACTION, EVT_INC_ACTION, EVT_REMOVED, EVT_DELAY_START, EVT_DELAY_END, EVT_MOUSE_INIT, EVT_MISS_TIMER, EVT_STIM_TIMER, EVT_CUE_TIMER, EVT_TRG, EVT_PIP, nEvents };

    const size_t nElements = nStates * nEvents;
    int table[nElements];

    // "FUNCTION POINTERS" to enter, exit, update functions where index indicates function for particular state
    // function pointers point to code, NOT data
    // function pointer allows for cleaner code which is more maintainable
    // No need to write write large number of control flow statements, simply point!




    void (*STenter[nStates])();  // Declare 9 Function pointers (36 bits) with no aruguments and returns no value 
    void (*STexit[nStates])();   // Declare 9 Function pointers (36 bits) with no aruguments and returns no value 
    void (*STupdate[nStates])(); // Declare 9 Function pointers (36 bits) with no aruguments and returns no value 

    volatile Event evt; //
    volatile State st; //

    String state_to_str(State c) {
        switch (c) {
        case ST_IDLE: return "ST_IDLE";
        case ST_TRG: return "ST_TRG";
        case ST_CUE: return "ST_CUE";
        case ST_DELAY: return "ST_DELAY";
        case ST_STIM: return "ST_STIM";
        case ST_RESPONSE: return "ST_RESPONSE";
        case ST_REWARD: return "ST_REWARD";
        case ST_PUNISH: return "ST_PUNISH";
        case ST_MISS: return "ST_MISS";
        default: return "lol";
        }
    }

    String event_to_str(Event e) {
        switch (e) {
        case EVT_CORR_ACTION: return "EVT_CORR_ACTION";
        case EVT_INC_ACTION: return "EVT_INC_ACTION";
        case EVT_REMOVED: return "EVT_REMOVED";
        case EVT_DELAY_START: return "EVT_DELAY_START";
        case EVT_DELAY_END: return "EVT_DELAY_END";
        case EVT_MOUSE_INIT: return "EVT_MOUSE_INIT";
        case EVT_MISS_TIMER: return "EVT_MISS_TIMER";
        case EVT_STIM_TIMER: return "EVT_STIM_TIMER";
        case EVT_CUE_TIMER: return "EVT_CUE_TIMER";
        case EVT_TRG: return "EVT_TRG";
        default: return "lol";
        }
    }

    void add(State state, Event evt, int next_state) {
        table[(int)evt + nEvents * (int)state] = (int)next_state;
    }

    State get(State state, Event evt) {
        return (State)table[(int)evt + nEvents * (int)state];
    }

    void update() {
        if (Machine::st == Machine::ST_IDLE && Machine::evt == (Event)-1) //if the machine state is idle and and the event enum value is -1 (casting -1 as enum type). See line 280
            (*Machine::STenter[(int)Machine::st])(); // Invoke Function (on_idle_enter || on_trg_enter || on_cue_enter || on_stim_enter || on_response_enter || on_punish_enter || on_miss_enter || on_reward_enter) using FUNCTION POINTER

        if (Machine::evt == (Event)-1) {
            if (Machine::STupdate[(int)Machine::st] != nullptr)
                (*Machine::STupdate[(int)Machine::st])();// Invoke Function (on_response_update) using FUNCTION POINTER


            return;
        }

        // DEBUG TRANSITIONS x
        String debugStr = event_to_str(Machine::evt) + " -- " + state_to_str(Machine::st) + " to ";
        if (Machine::STexit[(int)Machine::st] != nullptr)
            (*Machine::STexit[(int)Machine::st])(); //Invoke Function(on_idle_exit || on_trg_exit || on_cue_exit || on_stim_exit || on_response_exit || on_punish_exit || on_miss_exit || on_reward_exit) using FUNCTION POINTER

        Machine::st = get(Machine::st, Machine::evt);;

        debugStr += state_to_str(Machine::st);
        serDebug(debugStr);

        if (Machine::STenter[(int)Machine::st] != nullptr)
            (*Machine::STenter[(int)Machine::st])();

        Machine::evt = (Event)-1; //reset Machine::Event after done handling it
    }


}

namespace sPoked {
    enum { OL = 0, IL = 1, IR = 2, OR = 3, None = 4 };
    volatile int port;
    String to_str_long(int s) {
        switch (s) {
        case OL:
            return "Outer left";
        case IL:
            return "Center left";
        case IR:
            return "Center right";
        case OR:
            return "Outer right";
        default:
            return "none";
        }
    }

    String to_str_short(int s) {
        switch (s) {
        case OL:
            return "OL";
        case IL:
            return "IL";
        case IR:
            return "IR";
        case OR:
            return "OR";
        default:
            return "none";
        }
    }

    int to_enum_short(String p) {
        if (p == "OL") return sPoked::OL;
        else if (p == "IL") return sPoked::IL;
        else if (p == "IR") return sPoked::IR;
        else if (p == "OR") return sPoked::OR;
        else return sPoked::None;
    }

}

namespace target {
    enum Side { right = 4, left = 3 };

    String to_str(int s) {
        switch (s) {
        case right: return "right";
        case left: return "left";
        default: return "lol";
        }
    }

    bool isCorrectPoke(int poked, int side) {
        if (side == target::Side::right && poked >= 2) return true;
        else if (side == target::Side::left && poked <= 1) return true;
        else return false;
    }


}





enum Noise { silence = 11, blue_left = 12, brown_left = 3, blue_right = 4, blue = 0, brown = 1, brown_right = 13, white = 10 };


int stimToRewardPin(int p) {
    switch (p) {
    case sPoked::IL: return RewardPinLeft;
    case sPoked::OL: return RewardPinLeft;
    case sPoked::IR: return RewardPinRight;
    case sPoked::OR: return RewardPinRight;
    }
    return 0;
}


class Counter {
public:
    int n;
    int Cmax;
    int Cmin;
    bool (*trigger) (); //function pointer that should return a bool that says whether to trigger or not

    /// Intialize possible calls of objects of counter class
    Counter() : n(0), Cmax(32767), Cmin(0) {}

    Counter(int Cmax, int Cmin) : n(0), Cmax(Cmax), Cmin(Cmin) {}

    Counter(int Cmax, int Cmin, bool (*trigger) ()) : n(0), Cmax(Cmax), Cmin(Cmin), trigger(trigger) {}

    Counter(bool (*trigger) ()) : n(0), Cmax(32767), Cmin(0), trigger(trigger) {}

    void increment() {
        ++n;

        if (n >= Cmax)
            reset();
    }

    void reset() {
        this->n = Cmin;
    }

    void setCount(int n) {
        this->n = n;
    }

};

template <typename T>
class Sequence {
public:
    size_t len;
    T* ary; // pointer variable of Type T
    int count;
    T current;

    Sequence() {
        count = 0;
        this->current = 0; //This sequence current value is zero
    }

    Sequence(T* ary, size_t len) {
        this->ary = ary; // Use "This pointer" to assign array value pointer variable value
        this->len = len;
        count = 0;
        this->current = ary[count];
    }

    T next() {
        ++count;
        int ind = count % len;
        this->current = ary[ind];
        return current;
    }

    T randNext() {
        this->current = ary[random(len)];
        return current;
    }

    void reset() {
        this->count = 0;
        this->current = ary[this->count];
    }
    //  Added Template return type and returned the value
    T setCurrent(T n) {
        ++count;// Iterate Count so that when .next() is called, it is at the next index of the side array
        this->current = n;
        return current;
    }
};

// Initialize sequences & counters
// ex: cue, side, laser trial, correct counter
int cue[num_pips];

//Sequence<int> _side (side, nSide);
Sequence<int> _trial(side, nSide); // can be changed to separate size and feature 

Sequence<int> _block;
Sequence<int> _cue(cue, num_pips);

Counter laserTrial([]() -> bool {
    if (laserTrial.n > yellowCorrect) return random(0, 2);
    else if (laserTrial.n == yellowCorrect) return true;
    else return false;
    //return true;
    });


Counter trialCounter;
Counter PunishCounterSuc; // Added by Cillian

struct TINFO {

    /*


    */

    /*                       0             1          2           3          4       5         6              7      */
    String fieldNames[8] = { "initTS", "trialtype", "response", "required", "side", "poked", "distractor", "autoinit" };
    signed long fieldValues[8] = { 999,      -1,         2,           -1,       -1,      -1,           -1,          0 };
    int numFields = 8;

    void print() {
        String s = "::";
        for (int i = 0; i < numFields; i++) {
            s += String('*') + String(fieldValues[i]);
        }
        Serial.print(s); Serial.print('\n');
    }

    void populate() {

        //Cillian Block

        /*if (sideOverRide == 3 || sideOverRide == 4 )
            fieldValues[4] = (int)_trial.setCurrent(sideOverRide); // Side
        else*/
        fieldValues[4] = (int)_trial.current; // Side
    //Cillian Block

        fieldValues[3] = required;
        fieldValues[1] = _trial.current % 2 == 0; //Blue or Brown Cue

        fieldValues[5] = (int)sPoked::port;
        fieldValues[6] = _trial.current > 1;
    }

    void setResponse(int c) {
        fieldValues[2] = c;
    }

    void setInitiationTS(unsigned long m) {
        fieldValues[0] = m;
    }

    void setAutoInit(int c) {
        fieldValues[7] = c;
    }


    int getResponse() {
        return fieldValues[2];
    }
};

TINFO ctrialinfo;

void play(int);

void genCueSequence(int* cue, int n, double p) {
    int targetCue = _trial.current % 2; //Blue or Brown Cue
    int nontargetCue = !targetCue;
    // Serial.println("HERE");
    // Serial.println(targetCue);
    // Serial.println(nontargetCue);

    for (int i = 0; i < round(p * n); i++) {
        cue[i] = targetCue;
    }

    if (p >= 1) return;

    for (int i = round(p * n); i < n; i++) {
        cue[i] = nontargetCue;
    }

    // shuffle~
    int r; int temp;
    for (int i = 0; i < n; i++) {
        r = random(n);

        temp = cue[i];
        cue[i] = cue[r];
        cue[r] = temp;
    }

    // for (int i = 0; i < num_pips; i++) {
    //   Serial.print(cue[i]); Serial.print(", ");
    // }

}


void play_appropriate_cue(int modVal, int cueVal) {

    // if (cueVal == 6) {
    //   cueVal = (random(0,2)) ? 0 : 1;
    // }

    if (modVal == 0) {
        if (!CueFlag || cueVal == 0) blue_led.on(); white_led.off();
        switch (cueVal) { // explicit is better than implicit
        case 0:
            serDebug("cue low blue & uv");
            play(blue_left); blue_led.on(); white_led.off();
            break;
        case 1:
            serDebug("cue blue");
            play(blue);
            break;
        case 2:
            serDebug("cue blue");
            play(blue);
            break;
        case 3:
            serDebug("cue uv");
            blue_led.on(); white_led.off();
            break;
        }
    }
    else if (modVal == 1) {
        if (!CueFlag || cueVal == 0) green_led.on(); white_led.off();
        switch (cueVal) { // explicit is better than implicit
        case 0:
            serDebug("cue low brown & green");
            play(brown_left); green_led.on(); white_led.off();

            break;
        case 1:
            serDebug("cue brown");
            play(brown);
            break;
        case 2:
            serDebug("cue green");
            green_led.on();
            white_led.off();
            break;
        case 3:
            serDebug("cue brown");
            play(brown);
            break;
        }
    }
}





int* window;
int* blocks;

void initWindow() {
    window = (int*)malloc(sizeof(int) * nWindow);
    for (int i = 0; i < nWindow; i++) {
        window[i] = 0;
    }
}

double getWindowPercentage() {
    int sum = 0;
    for (int i = 0; i < nWindow; i++) {
        sum += window[i];
    }

    return (double)sum / (double)nWindow;
}

void updateWindow(int n) {
    window[trialCounter.n % nWindow] = n;
}

void initBlocks() {
    serDebug("init blocks");
    _block.ary = (int*)malloc(sizeof(int) * numBlocks);
    for (int i = 0; i < numBlocks; i++) {
        _block.ary[i] = i;
        serDebug(String(i));
    }

    _block.len = numBlocks;
}



Counter inCount([]() -> bool {
    if (inCount.n >= n_max_in) {
        return true;
    }
    return false;
    });

void play(int n) {
    switch (n) {
    case silence:
        serDebug("playing silence");
        digitalWrite(CueTTL, LOW);
        digitalWrite(StimTTL, LOW);
        digitalWrite(ChoiceTTL, LOW);
        break;
    case blue_left:
        serDebug("playing blue left");
        digitalWrite(CueTTL, LOW);
        digitalWrite(StimTTL, HIGH); // Edited 28-09-2020 by Cillian
        digitalWrite(ChoiceTTL, LOW);// Edited 28-09-2020 by Cillian
        break;
    case brown_left:
        serDebug("playing brown left");
        digitalWrite(CueTTL, HIGH);   // Edited 28-09-2020 by Cillian
        digitalWrite(StimTTL, LOW);   // Edited 28-09-2020 by Cillian
        digitalWrite(ChoiceTTL, HIGH);// Edited 28-09-2020 by Cillian
        break;
    case blue_right:
        serDebug("playing blue right");
        digitalWrite(CueTTL, LOW);
        digitalWrite(StimTTL, HIGH);
        digitalWrite(ChoiceTTL, LOW); // Edited 28-09-2020 by Cillian
        break;
    case blue:
        serDebug("playing blue");
        digitalWrite(CueTTL, LOW);  // Edited 28-09-2020 by Cillian
        digitalWrite(StimTTL, HIGH);// Edited 28-09-2020 by Cillian
        digitalWrite(ChoiceTTL, LOW);
        break;
    case brown:
        serDebug("playing brown");
        digitalWrite(CueTTL, HIGH);
        digitalWrite(StimTTL, LOW);
        digitalWrite(ChoiceTTL, HIGH);
        break;
    case brown_right:
        serDebug("playing brown right");
        digitalWrite(CueTTL, HIGH);
        digitalWrite(StimTTL, LOW);    // Edited 28-09-2020 by Cillian
        digitalWrite(ChoiceTTL, HIGH); // Edited 28-09-2020 by Cillian
        break;
    case white:
        serDebug("playing white");
        digitalWrite(CueTTL, HIGH);
        digitalWrite(StimTTL, HIGH);
        digitalWrite(ChoiceTTL, HIGH);
        break;
    }
}

String int_to_noise(int n) {
    switch (n) {
    case brown: return "brown";
    case blue: return "blue";
    case brown_left: return "low brown";
    case blue_left: return "low blue";
    default: return "lol";
    }
}

void openpokesL() {
    serDebug("opening pokes left");

    servo1.attach(ServoPin1); //re-attach servo to pin
    servo2.attach(ServoPin2); //re-attach servo to pin

    delay(50);
    servo1.write(ServoOpen);
    delay(100);

    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
}

void closepokesL() {
    serDebug("closing poke left (L)");

    servo1.attach(ServoPin1); //re-attach servo to pin
    servo2.attach(ServoPin2); //re-attach servo to pin

    delay(50);
    for (int i = 0; i <= ServoClosed; i += 10) {
        servo1.write(ServoOpen + i);
        delay(100);
    }
    servo1.write(ServoClosed);
    delay(100);

    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
}

void openpokesLR() {
    serLog("open side pokes");

    servo1.attach(ServoPin1); //re-attach servo to pin
    servo2.attach(ServoPin2); //re-attach servo to pin

    delay(50);
    //  for (int i = 0; i >= ServoOpen; i += 50) {
    //    servo1.write(ServoClosed - i);
    //    servo2.write(ServoClosed - i);
    //    delay(50);
    //  }

    servo1.write(ServoOpen);
    servo2.write(ServoOpen);
    delay(150);

    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
}

void closepokesLR() {
    serDebug("closing poke left");

    servo1.attach(ServoPin1); //re-attach servo to pin
    servo2.attach(ServoPin2); //re-attach servo to pin

    delay(50);

    for (int i = 0; i <= ServoClosed; i += 10) {
        servo1.write(ServoOpen + i);
        servo2.write(ServoOpen + i);
        delay(150);
    }
    servo1.write(ServoClosed);
    servo2.write(ServoClosed);
    delay(100);

    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
}

void openpokesR() {

    servo1.attach(ServoPin1); //re-attach servo to pin
    servo2.attach(ServoPin2); //re-attach servo to pin

    delay(50);
    serDebug("opening pokes right");
    servo2.write(ServoOpen);
    delay(100);

    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
}

void closepokesR() {
    serDebug("closing poke left");

    servo1.attach(ServoPin1); //re-attach servo to pin
    servo2.attach(ServoPin2); //re-attach servo to pin

    delay(50);
    for (int i = 0; i <= ServoClosed; i += 10) {
        servo2.write(ServoOpen + i);
        delay(150);
    }
    servo2.write(ServoClosed);
    delay(100);

    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
}

void openpokesCT() {

    servo3.attach(ServoPin3); //re-attach servo to pin

    delay(50);

    serDebug("opening pokes top");
    servo3.write(ServoOpenTop);
    delay(100);

    servo3.detach(); //detach servo to pin
}

void closepokesCT() {

    servo3.attach(ServoPin3); //re-attach servo to pin

    delay(50);

    serDebug("closing poke left");
    for (int i = 0; i <= ServoClosedCenter; i += 5) {
        servo3.write(ServoOpenTop + i);
        delay(150);
    }
    servo3.write(ServoClosedCenter);
    delay(100);

    servo3.detach(); //detach servo to pin
}

void openpokesCB() {

    servo3.attach(ServoPin3); //re-attach servo to pin

    delay(50);
    serDebug("opening pokes bottom");
    servo3.write(ServoOpenBottom);
    delay(100);

    servo3.detach(); //detach servo to pin
}

void closepokesCB() {

    servo3.attach(ServoPin3); //re-attach servo to pin

    serDebug("closing poke left");
    delay(50);
    for (int i = 0; i >= ServoClosedCenter; i -= 5) {
        servo3.write(ServoOpenBottom + i);
        delay(150);
    }
    servo3.write(ServoClosedCenter);
    delay(100);

    servo3.detach(); //detach servo to pin
}

void stopTimers() {
    for (auto tid : tids) t.stop(tid);
    for (auto tid : tids) h.stop(tid);
}

//Machine::STub
void on_idle_enter() {
    serDebug("=====================================================");

    if (ctrialinfo.getResponse() == 1) { //if previous trial was correct, make sure that both IR barriers get activated again~
        serDebug("activated barriers");
        EIFR |= (1 << INTF0); EIFR |= (1 << INTF1);  //EIFR |= (1 << INTF2); EIFR |= (1 << INTF3); EIFR |= (1 << INTF4); EIFR |= (1 << INTF5); // clears any waiting interrupts, impt for user-controlled activation/deactivation of side IR barriers
        attachInterrupt(digitalPinToInterrupt(TransPin_IL), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::IL : sPoked::None;
            }, RISING); // Button Press Interupt is a lambda expression which rises the desired poke if the previous trail was correct. 
        attachInterrupt(digitalPinToInterrupt(TransPin_OL), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::OL : sPoked::None;
            }, RISING);

        attachInterrupt(digitalPinToInterrupt(TransPin_IR), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::IR : sPoked::None;
            }, RISING);
        attachInterrupt(digitalPinToInterrupt(TransPin_OR), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::OR : sPoked::None;
            }, RISING);
    }

    digitalWrite(YellowLaserPin, LOW);

    ctrialinfo = TINFO();

    //reset/prep new trial
    sPoked::port = sPoked::None;
    Machine::evt = Machine::EVT_TRG; // Update Event which determine the next State
    stopTimers();

    EIFR |= (1 << INTF0); EIFR |= (1 << INTF1); //EIFR |= (1 << INTF2); EIFR |= (1 << INTF3); EIFR |= (1 << INTF4); EIFR |= (1 << INTF5); // clears any waiting interrupts, impt for user-controlled activation/deactivation of side IR barriers
}

//Machine::STub
void on_idle_exit() {
}

//Machine::STub
void on_trg_enter() {
    serLog("trial available " + target::to_str(_trial.current));

    play(white);
    //white_led.on();

    attachInterrupt(digitalPinToInterrupt(TransPin_C), []() {
        Machine::evt = Machine::EVT_MOUSE_INIT;
        }, RISING);


#ifdef DEBUG
    t.after(500, []() {
        Machine::evt = Machine::EVT_MOUSE_INIT;
        });
#endif
}

//Machine::STub_trial
void on_trg_exit() {
    ctrialinfo.setInitiationTS(millis());
    attachInterrupt(digitalPinToInterrupt(TransPin_C), []() {
        if (Machine::st < Machine::ST_RESPONSE) {
            serDebug("removed");
            serLog(F("removed"));

            play(silence);
            Machine::evt = Machine::EVT_REMOVED;

            green_led.off();
            blue_led.off();

            stopTimers();
        }
        }, FALLING);

    if (laserTrial.trigger() && laserPerBlock[_block.current]) {
        tids[5] = t.after(yellowLaserOnTime, []() {
            digitalWrite(YellowLaserPin, HIGH);
            serLog("yellow laser on");
            });

        tids[6] = t.after(yellowLaserOnTime + yellowLaserDuration, []() {
            digitalWrite(YellowLaserPin, LOW);
            serLog("yellow laser off");
            });
    }
}



void on_cue_enter() {
    int stimVal;
    if (_trial.current % 2 == 0) stimVal = 0;
    else stimVal = 1;


    switch (cueContextPerBlock[_block.current]) {
    case 0:
        serLog(F("cue uv/green"), true); break;
    case 1:
        serLog(F("cue HP/LP"), true); break;
    case 2:
        serLog("cue HP/green", true); break;
    case 3:
        serLog("cue LP/uv", true); break;
    case 4:
        if (stimVal == 0) {
            serLog("cue LP/uv", true); break;
        }
        else {
            serLog("cue HP/green", true); break;
        }
    case 5:
        if (stimVal == 0) {
            serLog("cue LP/uv", true); break;
        }
        else {
            serLog("cue HP/green", true); break;
        }
    }


    String st;
    if (accumulationPerBlock[_block.current] == 0) {
        //    if (cueContextPerBlock[_block.current] == 0 ||
        //        (cueContextPerBlock[_block.current] == 3 && stimVal == 0) ||
        //        (cueContextPerBlock[_block.current] == 2 && stimVal == 1) ) {
        //      serLog("dim trial");
        //    }

        play_appropriate_cue(stimVal, cueContextPerBlock[_block.current]);



        tids[0] = t.after(cueLength, []() {
            serLog("sound stop");

            digitalWrite(CueTTL, LOW); //ensure no audio cue
            digitalWrite(StimTTL, LOW);
            digitalWrite(ChoiceTTL, LOW);

            blue_led.off();
            green_led.off();

            Machine::evt = Machine::EVT_DELAY_START;

            tids[1] = h.after(delayDuration, []() {
                Machine::evt = Machine::EVT_DELAY_END;
                });
            });
    }
    else if (accumulationPerBlock[_block.current] > 0) {


        st = "cue seq " + String(pCue) + " ";
        for (int i = 0; i < num_pips; i++) {
            st += String(_cue.ary[i]);
        }
        serLog(st);
        serDebug(st);

        _cue.reset();

        if (cueContextPerBlock[_block.current] < 4)
            play_appropriate_cue(_cue.current, cueContextPerBlock[_block.current]);
        else if (cueContextPerBlock[_block.current] >= 4) {
            if (_trial.current % 2 == 0) {
                play_appropriate_cue(_cue.current, 3);
            }
            else {
                play_appropriate_cue(_cue.current, 2);
            }
        }

        _cue.next();

        tids[31] = t.after(PipDuration, []() {

            serDebug("end pip");

            play(silence); green_led.off(); blue_led.off();
            tids[32] = h.every(PipDuration + BtwPipDelay, []() {
                serDebug("end pip");
                play(silence); green_led.off(); blue_led.off();
                }, num_pips - 1);
            });
        // pulse Timer
        tids[30] = t.every(PipDuration + BtwPipDelay, []() {
            if (cueContextPerBlock[_block.current] < 4)
                play_appropriate_cue(_cue.current, cueContextPerBlock[_block.current]);
            else if (cueContextPerBlock[_block.current] >= 4) {
                if (_trial.current % 2 == 0) {
                    play_appropriate_cue(_cue.current, 3);
                }
                else {
                    play_appropriate_cue(_cue.current, 2);
                }

            }
            _cue.next();
            }, num_pips - 1);

        tids[29] = t.after((num_pips * PipDuration) + (((num_pips - 1) * BtwPipDelay)), []() {
            Machine::evt = Machine::EVT_DELAY_START;

            tids[1] = h.after(delayDuration, []() {
                Machine::evt = Machine::EVT_DELAY_END;
                });
            });
    }
}

void on_cue_exit() {
    play(silence);
    green_led.off();
    blue_led.off();
}

void on_stim_enter() {
    serLog(F("autostart"));

    digitalWrite(StimOnTTL, HIGH); //place flag telling neuralynx about stim start time
    delay(5);
    digitalWrite(StimOnTTL, LOW); //stop telling neuralynx

    String trialString = "stim ";
    trialString += target::to_str(_trial.current);
    trialString += " on";
    serLog(trialString);


    serDebug(trialString);

    trialString = "";

    tids[2] = t.after(stimDuration, []() {
        Machine::evt = Machine::EVT_STIM_TIMER;
        });
}

void on_stim_exit() {
    play(silence);
    serLog(F("end stimulus"));


    ledL.off(); ledR.off();
}

void on_response_enter() {
    stopTimers();

    detachInterrupt(digitalPinToInterrupt(TransPin_C));
    play(silence);



    // Added by Cillian Hayde Block Begin//////////////////////

        // This Code Block takes input from MATLAB

    if (PokeStateL == true || PokeStateR == true) {  // This Condtional Statement tracks whether the button Open Pokes (Left/Right) has been pressed in Matlab

        ButtonPressed = true;
        //serLog("Button Pressed true");
    }

    else {

        ButtonPressed = false;
    }
    serLog(String(ButtonPressed));



    if (PokeStateL == true) {
        openpokesL();
        serLog("Left Poked Open by MATLAB");
        PokeStateL = false;
    }
    else if (PokeStateR == true) {

        openpokesR();
        serLog("Right Poked Open by MATLAB");
        PokeStateR = false;
    }

    // This Code Block takes input from MATLAB//////////

    ////Automation Block////////////////////////////////
    if (ButtonPressed == true) {
        serLog("Button Pressed: Ignore Automation Block");
    }
    else {
        if (PunishCounterSuc.n >= 7 && _trial.current == target::Side::left) {
            serLog("Seven Failures, Left Poke is Opened");
            openpokesL();
            closepokesR();
        }
        else if (PunishCounterSuc.n >= 7 && _trial.current == target::Side::right) {

            serLog("Seven Failures, Right Poke is Opened");
            openpokesR();
            //closepokesL();
        }
        else {

            serLog("Standard: Open all Pokes");
            openpokesLR(); // Originally Here
            //openpokesR(); // Originally Here
        }
    }
    ////Automation Block////////////////////////////////

    tids[3] = t.after(MissDelay, []() {
        Machine::evt = Machine::EVT_MISS_TIMER;
        });

#ifdef DEBUG
    t.after(50, []() {
        sPoked::port = random(0, 4);
        });
#endif

}

//Added by Cillian Block End////////////////////////////////




//Machine::STub
void on_response_update() {
    if (sPoked::port == sPoked::None)
        return;

    serLog("poked " + sPoked::to_str_long(sPoked::port));


    serDebug("poked " + sPoked::to_str_long(sPoked::port));
    if (target::isCorrectPoke(sPoked::port, _trial.current)) {
        Machine::evt = Machine::EVT_CORR_ACTION;
        updateWindow(1);
        ctrialinfo.setResponse(1);

        PunishCounterSuc.reset(); // Added by Cillian


    }
    else {
        Machine::evt = Machine::EVT_INC_ACTION;
        updateWindow(0);
        ctrialinfo.setResponse(0);
        PunishCounterSuc.increment(); //Added by Cillian

    }

    trialCounter.increment();
}
//Machine::STub
void on_response_exit() {
    stopTimers();

    ctrialinfo.populate();
    ctrialinfo.print();



    if (Machine::evt == Machine::EVT_CORR_ACTION || modeRepeatChance < random(100)) _trial.next();

    if ((getWindowPercentage() >= p && trialCounter.n >= nWindow) || trialCounter.n >= trialsPerBlock[_block.current]) {
        _block.next();
        trialCounter.reset();
        initWindow();
    }

    if (_trial.current == 6) {
        _trial.current = side[_trial.count % nSide];
    }



}

void on_reward_enter() {

    const int RewardPeriod = 100; //Added by Cillian
    inCount.reset();
    laserTrial.increment();

    openpokesCB();
    delay(15);
    closepokesLR();
    serLog("reward delivered");


    if (inCount.trigger()) {//Used to interpret input to control pokes during training for next trial
        training_fxn(3);
        training_fxn(4);
    }

    byte rewardPin = stimToRewardPin(sPoked::port);
    tids[4] = t.pulseImmediate(rewardPin, RewardPeriod, HIGH);
    tids[5] = t.after(RewardDelay, []() {
        Machine::evt = Machine::EVT_REMOVED;
        });

    if (accumulationPerBlock[_block.current] == 1) {
        pCue = percentPerBlock[_block.current];
        genCueSequence(_cue.ary, num_pips, percentPerBlock[_block.current]);
    }
    else if (accumulationPerBlock[_block.current] == 2) {
        pCue = min(pHard, pEasy + trialCounter.n * ((pHard - pEasy) / trialsPerBlock[_block.current]));
        genCueSequence(_cue.ary, num_pips, pCue);
    }
    else if (accumulationPerBlock[_block.current] == 3) {
        pCue = pseudo_uniform(LowProb, HighProb, N);
        serLog("Probability generated is  " + String(pCue));
        genCueSequence(_cue.ary, num_pips, pCue);
    }
}

void on_reward_exit() {
    closepokesCB();
}

void on_punish_enter() {
    inCount.increment();
    closepokesLR();
    laserTrial.reset();

    tids[6] = t.after(PunishDelay, [] { Machine::evt = Machine::EVT_REMOVED; });
}


void on_punish_exit() {
}

void on_miss_enter() {



    serLog(F("missed your shot!"));


    closepokesLR();

    tids[7] = t.after(MissDelay, [] { Machine::evt = Machine::EVT_REMOVED; });

}

void on_miss_exit() {
}

int UserInput = 0;



void training_fxn(int n) {//Used to interpret input to control pokes during training, case list



    switch (n) {
    case 0: // deactivate left barrier

        serDebug("deactivate left barrier");
        detachInterrupt(digitalPinToInterrupt(TransPin_OL));
        detachInterrupt(digitalPinToInterrupt(TransPin_IL));
        break;
    case 1: // deactivate right barrier
        // if added by Cillian
        if (sPoked::port == target::Side::left) {
            serDebug("deactivate right barrier");
            detachInterrupt(digitalPinToInterrupt(TransPin_OR));
            detachInterrupt(digitalPinToInterrupt(TransPin_IR));
        }

        break;
    case 3: // active left barrier
        serDebug("activate left barrier");
        attachInterrupt(digitalPinToInterrupt(TransPin_IL), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::IL : sPoked::None;
            }, RISING);
        attachInterrupt(digitalPinToInterrupt(TransPin_OL), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::OL : sPoked::None;
            }, RISING);
        break;
    case 4: // activate right barrier
        serDebug("activate right barrier");
        attachInterrupt(digitalPinToInterrupt(TransPin_IR), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::IR : sPoked::None;
            }, RISING);
        attachInterrupt(digitalPinToInterrupt(TransPin_OR), []() {
            sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::OR : sPoked::None;
            }, RISING);
        break;
    case 2: // send reward to left side (3)

        serLog(" Single Reward");
        Reward_Type = 1;
        serLogTime();
        break;
    case 5: //  send reward right side (4)
        serLog(" Double Reward");
        Reward_Type = 2;
        serLogTime();
        break;

    case 6: 
        serLog("Trial Available - OverRide Left");
        serDebug("OverRide Left");
        _trial.setCurrent(3);
        serLogTime();
        break;
    case 7:
        serLog("Trial Available - OverRide Right");
        serDebug("OverRide Right");
        _trial.setCurrent(4);
        serLogTime();

        break;

    case 8:
        //      PrevPokeStateL = PokeStateL;
        serLog("Open Poke Left");
        serDebug("Open Poke Left");
        PokeStateL = true;
        serLogTime();
        break;
        /*
            case 9:
                PrevPokeStateL = PokeStateL;
                serLog("Close Poke Left");
                serDebug("Close Poke Left");
                PokeStateL = false;
                serLogTime();
                break;
        */
    case 10:
        //     PrevPokeStateR = PokeStateR;
        serLog("Over Poke Right");
        serDebug("Over Poke Right");
        PokeStateR = true;
        serLogTime();
        break;
        /*
            case 11:
                PrevPokeStateR = PokeStateR;
                serLog("Close Poke Right");
                serDebug("Close Poke Right");
                PokeStateR = false;
                serLogTime();
                break;
        */
    }
}

void check_keys() {//Input from MATLAB
    if (Serial.available() <= 0)
        return;

    int incomingByte = Serial.read();
    serDebug("received " + String(incomingByte));
    serLog("received " + String(incomingByte));
    training_fxn(incomingByte);

}

void Reward_Side() {

    /*Function added by Cillian*/
    if ((Reward_Type == 1) && Machine::evt == Machine::EVT_CORR_ACTION) {
    
        serLog("Single Reward Delivered by MATLAB Button");
        tids[13] = t.pulseImmediate(RewardPinLeft, RewardPeriod, HIGH);
        serLogTime();
        Reward_Type = 0;

    }
    
    else if ((Reward_Type == 2 ) && Machine::evt == Machine::EVT_CORR_ACTION) {

        serLog("Double Reward Delivered by MATLAB Button");
        tids[13] = t.pulseImmediate(RewardPinLeft, RewardPeriod, HIGH);
        tids[13] = t.pulseImmediate(RewardPinLeft, RewardPeriod, LOW); // Added by Cillian 
        tids[13] = t.pulseImmediate(RewardPinLeft, RewardPeriod, HIGH);// Added by Cillian
        serLogTime();
        Reward_Type = 0;
    }
    else {
        serLog("No Reward Delivered by MATLAB Button");
    }
        
}


void initSequence() {
    int totalSize = 0;

    for (int i = 0; i < numBlocks; i++) {
        totalSize += trialsPerBlock[i];
    }

    _trial.len = totalSize;
    _trial.ary = (int*)malloc(sizeof(int) * totalSize);

    int ind = 0;
    for (int i = 0; i < numBlocks; i++) {
        for (int j = 0; j < trialsPerBlock[i]; j++) {
            _trial.ary[ind] = modalityPerBlock[i];
            ind++;
        }
    }

}

void setup_cont() {
    Serial.begin(115200); // baud rate 115200 (Cillian)
    randomSeed(analogRead(0));
    X[9] = Seed; // Seed for pseudo unifrom function (LCG)



#ifdef DEBUG
    Serial.print("DEBUG SESSION\n");
#endif

    initWindow();
    initBlocks();
    if (modalityBlock) initSequence();

    servo1.attach(ServoPin1); //attach servo to pin
    servo1.write(ServoClosed); //set to closed

    servo2.attach(ServoPin2); //attach servo to pin
    servo2.write(ServoClosed); //set to closed

    servo3.attach(ServoPin3); //attach servo to pin
    servo3.write(ServoClosedCenter); //set to closed

    delay(100);
    servo1.detach(); //detach servo to pin
    servo2.detach(); //detach servo to pin
    servo3.detach(); //detach servo to pin

    pinMode(StimOnTTL, OUTPUT); //prepare TTLs

    digitalWrite(TransPin_OL, HIGH);//outer left
    digitalWrite(TransPin_IL, HIGH);//center left
    digitalWrite(TransPin_C, HIGH);//center
    digitalWrite(TransPin_IR, HIGH);//center right
    digitalWrite(TransPin_OR, HIGH);//outer right

    ledIR2.on();
    ledIR3.on();
    ledIR4.on();
    ledIR5.on();
    ledIR1.on();

    ledR.off();
    ledL.off();

    pinMode(RewardPinLeft, OUTPUT); //configure reward pins to output milk
    pinMode(RewardPinRight, OUTPUT);

    pinMode(CueTTL, OUTPUT);
    pinMode(StimTTL, OUTPUT);
    pinMode(ChoiceTTL, OUTPUT);

    blue_led.off();
    green_led.off();
    white_led.on();
    attachInterrupt(digitalPinToInterrupt(TransPin_OR), []() {
        sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::OR : sPoked::None;
        }, RISING);

    attachInterrupt(digitalPinToInterrupt(TransPin_OL), []() {
        sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::OL : sPoked::None;
        }, RISING);

    attachInterrupt(digitalPinToInterrupt(TransPin_IR), []() {
        sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::IR : sPoked::None;
        }, RISING);

    attachInterrupt(digitalPinToInterrupt(TransPin_IL), []() {
        sPoked::port = (Machine::st == Machine::ST_RESPONSE) ? sPoked::IL : sPoked::None;
        }, RISING);

    attachInterrupt(digitalPinToInterrupt(TransPin_C), []() {
        Machine::evt = Machine::EVT_MOUSE_INIT;
        }, RISING);

    if ((num_pips * PipDuration) + (((num_pips - 1) * BtwPipDelay)) > required) {
        serLog("  Warning: Required is not long enough for accumulation");
    }

    // state machine set up

    for (unsigned int i = 0; i < Machine::nElements; i++)
        Machine::table[i] = (unsigned int)Machine::ST_IDLE;

    for (int i = 0; i < Machine::nStates; i++) {
        Machine::STenter[i] = nullptr;
        Machine::STexit[i] = nullptr;
        Machine::STupdate[i] = nullptr;
    }

    //state_in, evt that triggers it, and the next state
    //state_in, evt that triggers a transition, and the next state
    Machine::add(Machine::ST_IDLE, Machine::EVT_TRG, (int)Machine::ST_TRG);
    Machine::add(Machine::ST_TRG, Machine::EVT_MOUSE_INIT, (int)Machine::ST_CUE);
    Machine::add(Machine::ST_CUE, Machine::EVT_DELAY_START, (int)Machine::ST_DELAY);
    Machine::add(Machine::ST_DELAY, Machine::EVT_DELAY_END, (int)Machine::ST_STIM);
    Machine::add(Machine::ST_DELAY, Machine::EVT_PIP, (int)Machine::ST_CUE);
    Machine::add(Machine::ST_STIM, Machine::EVT_STIM_TIMER, (int)Machine::ST_RESPONSE);

    //unchanged transitions
    Machine::add(Machine::ST_RESPONSE, Machine::EVT_CORR_ACTION, (int)Machine::ST_REWARD);
    Machine::add(Machine::ST_RESPONSE, Machine::EVT_INC_ACTION, (int)Machine::ST_PUNISH);
    Machine::add(Machine::ST_RESPONSE, Machine::EVT_MISS_TIMER, (int)Machine::ST_MISS);

    // Machine:: enter/update/exit functions
    Machine::STenter[(int)Machine::ST_IDLE] = &on_idle_enter; // assign function to function pointer
    Machine::STenter[(int)Machine::ST_TRG] = &on_trg_enter;
    Machine::STenter[(int)Machine::ST_CUE] = &on_cue_enter;
    Machine::STenter[(int)Machine::ST_STIM] = &on_stim_enter;
    Machine::STenter[(int)Machine::ST_RESPONSE] = &on_response_enter;
    Machine::STenter[(int)Machine::ST_PUNISH] = &on_punish_enter;
    Machine::STenter[(int)Machine::ST_MISS] = &on_miss_enter;
    Machine::STenter[(int)Machine::ST_REWARD] = &on_reward_enter;

    Machine::STexit[(int)Machine::ST_IDLE] = &on_idle_exit;
    Machine::STexit[(int)Machine::ST_TRG] = &on_trg_exit;
    Machine::STexit[(int)Machine::ST_CUE] = &on_cue_exit;
    Machine::STexit[(int)Machine::ST_STIM] = &on_stim_exit;
    Machine::STexit[(int)Machine::ST_RESPONSE] = &on_response_exit;
    Machine::STexit[(int)Machine::ST_PUNISH] = &on_punish_exit;
    Machine::STexit[(int)Machine::ST_MISS] = &on_miss_exit;
    Machine::STexit[(int)Machine::ST_REWARD] = &on_reward_exit;

    Machine::STupdate[(int)Machine::ST_RESPONSE] = &on_response_update;

    Machine::st = Machine::ST_IDLE;
    Machine::evt = (Machine::Event) - 1;

    if (accumulationPerBlock[_block.current] == 1) {
        genCueSequence(_cue.ary, num_pips, percentPerBlock[_block.current]);
        pCue = percentPerBlock[_block.current];
    }
    else if (accumulationPerBlock[_block.current] == 2) {
        pCue = min(pHard, pEasy + trialCounter.n * ((pHard - pEasy) / trialsPerBlock[_block.current]));
        genCueSequence(_cue.ary, num_pips, pCue);
    }
    else if (accumulationPerBlock[_block.current] == 3) {
        pCue = pseudo_uniform(LowProb,  HighProb,  N);
        serLog("Probability generated is  " + String(pCue));
        genCueSequence(_cue.ary, num_pips, pCue);
    }
    _trial.reset();
    if (_trial.current == 6) {
        _trial.current = side[_trial.count % nSide];
    }
}

//#ifdef DEBUG
void setup() {
    setup_cont();
    delay(3 * 1000);
}
void loop() {
    check_keys();
    Reward_Side();
    t.update(); h.update();
    Machine::update();
}
//#endif