// Use the MD_MAX72XX library to Print some text on the display
//
// Demonstrates the use of the library to print text.
//
// User can enter text on the serial monitor and this will display as a
// message on the display.

#include <IRremote.h>
#include <MD_MAX72xx.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

#define BTN               A0
#define IR_RECEIVE_PIN    A1

//IR recievier object
IRrecv IrReceiver(IR_RECEIVE_PIN);

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Text parameters
#define CHAR_SPACING  2 // pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE  75


char message[BUF_SIZE] = "00:00";
bool newMessageAvailable = true;


void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t   curLen = 0;
  uint16_t  showLen = 0;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 2;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: {// Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        const char c = *pMsg++;
        if(c == '1') {
          cBuf[0] = B01000100;
          cBuf[1] = B01000010;
          cBuf[2] = B01111111;
          cBuf[3] = B01000000;
          cBuf[4] = B01000000;
          showLen = 5;
        } else if(c == ':') {
          cBuf[0] = B00110110;
          cBuf[1] = B00110110;
          showLen = 2;
        } else {
          showLen = mx.getChar(c, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        }
        
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying
      }

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:	// display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen) {
          state = 0;
        }
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}


void setup()
{
  pinMode(BTN, INPUT);

  // Start the receiver
  IrReceiver.enableIRIn();

  mx.begin();
  mx.control(0, MAX_DEVICES-1, MD_MAX72XX::INTENSITY, 0x0);

  Serial.begin(115200);
  Serial.println("\n[MD_MAX72XX Message Display]\nType a message for the display\nEnd message line with a newline");
  printText(0, MAX_DEVICES-1, " ");
  delay(500);
}

uint8_t minutes = 0;
uint8_t seconds = 0;

void loop()
{
//  IrReceiver.disableIRIn();
  
  sprintf(message, "%02d:%02d", minutes, seconds++);
  if(seconds >= 60) {
    seconds = 0;
    if(++minutes >= 60) {
      minutes = 0;
    }
  }
  printText(0, MAX_DEVICES-1, message);

//  IrReceiver.enableIRIn();
  
  for(int i=0; i<20; i++) {
    if(!digitalRead(BTN)) {
      seconds = 0;
      minutes = 0;
      break;
    }
    
//    if(IrReceiver.decode()) {
//      Serial.println(IrReceiver.results.value, HEX);
//      IrReceiver.resume();
//    }

    delay(49);
  }
  
  delay(10);
}
