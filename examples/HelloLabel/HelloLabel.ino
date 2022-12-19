// HelloLabel.ino
//
// Create a single label that says "Hello world" and display it.

#include<Arduino.h>
#include<uiwidgets.h>

TFT_eSPI lcd; // Connect to the hardware LCD from TFT_eSPI library.
Screen screen(lcd); // Create a Screen that manages widgets on top of the lcd.

// The string to display. This must either be a global constant, or an allocated
// buffer whose lifetime is as long as that of the label that uses it (or longer).
const char *message = "Hello, world!";
StrLabel helloLabel(message); // a label widget that displays a string.

void setup() {
  // Bind message to screen
  screen.setWidget(&helloLabel);

  // Set some style
  message.setBackground(TFT_BLUE);
  message.setColor(TFT_YELLOW);

  // Optional: Can also add a border to the message
  // message.setBorder(BORDER_ROUNDED);
  // message.setBorder(BORDER_RECT);
  // message.setBorder(BORDER_BOTTOM);  // etc...

  // If you compiled additional fonts into TFT_eSPI, you can select them by number here
  //message.setFont(2);

  screen.render(); //  Draw the screen.
}

void loop() {
  delay(10);
}
