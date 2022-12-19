// RowColLayout.ino
//
// Demonstrate how to lay out multiple items using rows and cols

#include<Arduino.h>
#include<uiwidgets.h>

TFT_eSPI lcd; // Connect to the hardware LCD from TFT_eSPI library.
Screen screen(lcd); // Create a Screen that manages widgets on top of the lcd.

Rows rows(3); // container for our top-level rows

// first row
Cols firstRow(3); // columns within 1st row.
IntLabel label1(1);
IntLabel label2(2);
IntLabel label3(3);

// second row
Cols secondRow(3); // columns within 2nd row.
IntLabel label4(1);
IntLabel label5(1);
IntLabel label6(6);

// third row
Cols thirdRow(3); // columns within 3rd row.
IntLabel label7(7);
IntLabel label8(8);
IntLabel label9(9);


void setup() {
  // Bind widgets in a hierarchy and configure layout and style of each.
  screen.setWidget(&rows);

  rows.setRow(0, &firstRow, 60); // 60 px tall first row
  firstRow.setColumn(0, &label1, 40); // a set of columns that are 40, 80, and 40 cols wide
  firstRow.setColumn(1, &label2, 80);
  firstRow.setColumn(2, &label3, 40);
  label2.setBackground(TFT_BLUE); // show bounding box of label2 vs label1 & 3

  rows.setRow(1, &secondRow, 50); // 50 px tall second row
  secondRow.setColumn(0, &label4, EQUAL); // columns in this row are equal-width
  secondRow.setColumn(1, &label5, EQUAL);
  secondRow.setColumn(2, &label6, EQUAL);
  label5.setBackground(TFT_WHITE); // show bounding box of label 5 vs 4 and 6
  label5.setColor(TFT_RED)

  rows.setRow(2, &thirdRow, EQUAL); // final row gets all remaining height (sole vertical "EQUAL")
  thirdRow.setColumn(0, &label7, EQUAL); // columns in this row are equal-width
  thirdRow.setColumn(1, &label8, EQUAL);
  thirdRow.setColumn(2, &label9, EQUAL);
  label8.setBorder(BORDER_RECT); // Distinguish extent of 7, 8, 9 labels with borders

  screen.render(); //  Draw the screen.
}

void loop() {
  delay(10);
}
