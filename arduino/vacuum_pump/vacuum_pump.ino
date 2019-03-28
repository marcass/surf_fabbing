#define RELAY 3
#define SENS A1

float set_vac = -0.85;
float on_thresh = set_vac + 0.03;
int offset = 118; // zero pressure calibrate
int fullScale = 922; // max pressure calibrate

//set up non-tunable control variables
unsigned long RUN_THRESH = 1000; //1min in milliseconds, minimum time for heating/cooling elements to run
bool can_turn_off = true; //bool for short cycle timer
unsigned long start_time = 0; //variable for timing heating/cooling duration
unsigned long MAX_RUN_TIME = 1200000; //20 min
unsigned long RELAX_TIME = 60000; //1min 

//system states
const int STATE_ERROR = -1;
const int STATE_RELAX = 0;
const int STATE_IDLE = 1;
const int STATE_VAC = 3;
int state = STATE_IDLE; //initialise in idle

//set up temp measurement variables
float vacuum;
unsigned long last_vac_request = 0;
bool waiting_for_conversion = false;
unsigned long MEAS_INTERVAL = 500; //take temperature measurement every  half s

void setup() {
  Serial.begin(9600);
  //initialise relay
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
}

void perform_action(String action) {
  //start timer so don't short cycle
  start_time = millis();
  if(action == "vac") {
    can_turn_off = false;
    digitalWrite(RELAY, HIGH);
  } else if (action == "disable") {
    Serial.println("...Switching off");
    digitalWrite(RELAY, LOW);
  }
}

void proc_idle() {
  if (vacuum > set_vac) {
    state = STATE_VAC;
    perform_action("vac");
  }
}

void proc_vac() {
  // check if minimum runtime has elapsed
  if ((millis() - start_time) > RUN_THRESH) {
    can_turn_off = true;
  }
  // check if conditions are right to turn heater off 
  if ((vacuum < set_vac) && can_turn_off) {
    state = STATE_IDLE;
    perform_action("disable");
  }
  // check if heater has been on for too long
  if ((millis() - start_time) > MAX_RUN_TIME) {
    #if (DEBUG) 
      Serial.println("Maximum heat time exceeded, entering STATE_RELAX");
    #endif
    state = STATE_RELAX;
    perform_action("disable");
  }
}

void proc_relax() {
  if (millis() - start_time > RELAX_TIME) {
    Serial.println("Relax time exceeded, entering STATE_IDLE");
    state = STATE_IDLE;
  }
}

void error_handler() {
  if (millis() - start_time > RUN_THRESH) {
    can_turn_off = true;
    perform_action("disable"); 
  }
}

void loop() {
  if ((millis() - last_vac_request) > MEAS_INTERVAL) {
    last_vac_request = millis();
    vacuum = (analogRead(SENS) - offset) * 11.6376 / (fullScale - offset);
    Serial.print("Pressure is  ");
    Serial.print(vacuum);
    Serial.print("  bar, ");
    Serial.print("Analogue read is: ");
    Serial.print(analogRead(SENS));
    Serial.print(": State is: ");
    Serial.println(state);
  }    
  // Manage states- can't heat while cooling and vice versa
  switch (state) {
    case STATE_IDLE:
      proc_idle();
      break;
    case STATE_VAC:
      proc_vac();
      break;
    case STATE_RELAX:
      proc_relax();
      break;
    case STATE_ERROR:
      error_handler();
      break;    
  }
}
