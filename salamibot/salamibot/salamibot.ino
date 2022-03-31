static char command[20] = "";

enum {
  STATE_IDLE,
  STATE_MANUAL,
  STATE_EXP
} state = STATE_IDLE;

void show_prompt() {
  // Clear command
  command[0] = '\0';

  // Draw command prompt
  Serial.print("\r\n>");
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
    // Display status
    Serial.println("Ready...");
    
    // Start manual triggering
    state = STATE_MANUAL;
  } else if (!strncmp(tokens[0], "e", 1)) {
    // Display status
    Serial.println("Ready...");
    
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
      // Go to idle state
      state = STATE_IDLE;

      // Show command prompt
      show_prompt();
    }
  } else if (state == STATE_EXP) {
    if (ch == 27) {  // Escape
      // Go to idle state
      state = STATE_IDLE;

      // Show command prompt
      show_prompt();
    }
  }
}

void setup() {
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
}
