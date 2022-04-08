#include <AccelStepper.h>

static char command[20];
static bool is_running;
static bool is_flowing;
static int cur_pos;

static unsigned long ms_zero, ms_print, ms_start_flow;
static unsigned long ms_trans, ms_max;

static float tau;
      
static enum {
  STATE_IDLE,
  STATE_MANUAL,
  STATE_EXP
} state = STATE_IDLE;

AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 9, 10, 11, false);

void show_prompt() {
  // Clear command
  command[0] = '\0';

  // Draw command prompt
  Serial.print("\r\n>");
}

void init_stepper() {
  // Reset current position
  cur_pos = 0;
  stepper.setCurrentPosition(10 * cur_pos);

  // Enable stepper
  stepper.enableOutputs();

  // Test stepper
  stepper.runToNewPosition(10 * (cur_pos + 1));
  stepper.runToNewPosition(10 * cur_pos);
}

void handle_command(char *str) {
  const char s[2] = " ";
  char *tokens[5];
  int n = 0;

  // Get first token
  tokens[n] = strtok(str, s);
  
  // Get rest of tokens
  while(tokens[n++] != NULL) {
    tokens[n] = strtok(NULL, s);
  }

  // Process tokens
  if (!strncmp(tokens[0], "m", 1)) {
    // Start in inactive mode
    is_running = false;
    
    // Initialize stepper motor
    init_stepper();

    // Display status
    Serial.println("Press space to start timer...");
    
    // Start manual triggering
    state = STATE_MANUAL;
  } else if (!strncmp(tokens[0], "e", 1)) {
    // Save time constant
    if (n > 1) {
      tau = atof(tokens[1]);
    } else {
      tau = 15000;
    }

    // Save maximum sample time
    if (n > 2) {
      ms_max = atol(tokens[2]);
    } else {
      ms_max = -1;
    }

    // Start in inactive mode
    is_running = is_flowing = false;
    
    // Initialize stepper motor
    init_stepper();

    // Display status
    Serial.println("Press space to start timer...");
    
    // Start exponential triggering
    state = STATE_EXP;
  } else {
    // Display error
    Serial.println("Unknown command");

    // Show command prompt
    show_prompt();
  }
}

void handle_input(char ch) {
  if (state == STATE_IDLE) {
    if (ch == 13) {  // Carriage return
      // Print carriage return
      Serial.println("");
  
      // Process command
      handle_command(command);
    } else {
      // Print character
      Serial.print(ch);
  
      // Add to command
      strncat(command, &ch, 1);
    }
  } else if (state == STATE_MANUAL) {
    if (ch == 27) {  // Escape
      unsigned long ms_start, ms_end;
      
      // Get end time
      ms_start = ms_end = millis();

      // Disable stepper
      stepper.disableOutputs();

      // Print time
      Serial.print(ms_start - ms_zero);
      Serial.print(",");
      Serial.print(ms_end - ms_zero);
      Serial.print("\r\n");

      // Go to idle state
      state = STATE_IDLE;

      // Show command prompt
      show_prompt();
    } else if (ch == ' ') {
      unsigned long ms_start, ms_end;
      
      if (is_running) {
        // Increment position
        ++cur_pos;
      
        // Go to new position
        ms_start = millis();
        stepper.runToNewPosition(10 * cur_pos);
        ms_end = millis();
      } else {
        // Initialize timers
        ms_start = ms_end = ms_zero = millis();
        ms_print = 0;

        // Display status
        Serial.println("Press space to advance to next tube...");
        Serial.println("Press escape to finish...");

        // Change to active mode
        is_running = true;
      }

      // Remember transition time
      ms_trans = (ms_start + ms_end) / 2;
      
      // Print time
      Serial.print(ms_start - ms_zero);
      Serial.print(",");
      Serial.print(ms_end - ms_zero);
      Serial.print("\r\n");
    }
  } else if (state == STATE_EXP) {
    if (ch == 27) {  // Escape
      unsigned long ms_start, ms_end;
      
      // Get end time
      ms_start = ms_end = millis();

      // Disable stepper
      stepper.disableOutputs();

      // Print time
      Serial.print(ms_start - ms_zero);
      Serial.print(",");
      Serial.print(ms_end - ms_zero);
      Serial.print("\r\n");

      // Go to idle state
      state = STATE_IDLE;

      // Show command prompt
      show_prompt();
    } else if (ch == ' ') {
      unsigned long ms_start, ms_end;
      
      if (is_flowing) {
        // Increment position
        ++cur_pos;
      
        // Go to new position
        ms_start = millis();
        stepper.runToNewPosition(10 * cur_pos);
        ms_end = millis();

        // Display status
        Serial.println("Changed to manual mode...");
        Serial.println("Press space to advance to next tube...");

        // Go to manual state
        state = STATE_MANUAL;
      } else if (is_running) {
        // Initialize timers
        ms_start = ms_end = ms_start_flow = millis();

        // Display status
        Serial.println("Press space to change to manual mode...");
        Serial.println("Press escape to finish...");

        // Change mode to flowing
        is_flowing = true;
      } else {
        // Initialize timers
        ms_zero = millis();
        ms_print = 0;

        // Display status
        Serial.println("Press space when flow starts...");

        // Change mode to running
        is_running = true;
      }

      if (is_flowing) {
        // Remember transition time
        ms_trans = (ms_start + ms_end) / 2;

        // Print time
        Serial.print(ms_start - ms_zero);
        Serial.print(",");
        Serial.print(ms_end - ms_zero);
        Serial.print("\r\n");
      }
    }
  }
}

void setup() {
  // Configure stepper
  stepper.setMaxSpeed(200);
  stepper.setAcceleration(1000);
  stepper.setEnablePin(12);

  // Set up serial
  Serial.begin(9600);
  
  // Show command prompt
  show_prompt();
}

void loop() {
  while (Serial.available()) {
    char ch = Serial.read();
    handle_input(ch);
  }

  if ((state == STATE_EXP) && is_flowing) {
    // Get current time
    unsigned long ms = millis();

    const long N = 20;
  
    int next_pos = N - (int) ceil(N * exp(-(float) (ms - ms_start_flow) / tau));
    if (next_pos > cur_pos) {
      // Increment position
      cur_pos = next_pos;
    
      // Go to new position
      unsigned long ms_start = millis();
      stepper.runToNewPosition(10 * cur_pos);
      unsigned long ms_end = millis();
    
      // Remember transition time
      ms_trans = (ms_start + ms_end) / 2;

      // Print time
      Serial.print(ms_start - ms_zero);
      Serial.print(",");
      Serial.print(ms_end - ms_zero);
      Serial.print("\r\n");
    } else if ((ms - ms_trans > ms_max) && is_flowing) {
      // Display status
      Serial.println("Changed to manual mode...");
      Serial.println("Press space to advance to next tube...");

      // Go to manual state
      state = STATE_MANUAL;
    }
  }

  if ((state == STATE_MANUAL) && is_running) {
    // Get current time
    unsigned long ms = millis();

    if (ms - ms_trans > ms_max) {
      // Increment position
      ++cur_pos;
    
      // Go to new position
      unsigned long ms_start = millis();
      stepper.runToNewPosition(10 * cur_pos);
      unsigned long ms_end = millis();
    
      // Remember transition time
      ms_trans = (ms_start + ms_end) / 2;

      // Print time
      Serial.print(ms_start - ms_zero);
      Serial.print(",");
      Serial.print(ms_end - ms_zero);
      Serial.print("\r\n");
    }
  }

  if ((state != STATE_IDLE) && is_running) {
    // Get current time
    unsigned long ms = millis();

    if ((ms - ms_zero) / 100 != ms_print / 100)
    {
      // Calculate time for display
      ms_print = ((ms - ms_zero) / 100) * 100;

      // Print current time
      Serial.print(ms_print);
      Serial.print("\r");
    }
  }
}
