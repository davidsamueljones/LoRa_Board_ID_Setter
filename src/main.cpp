#include <EEPROM.h>
#include <Arduino.h>

#include "breakout.h"

// EEPROM locations of relevant bytes
#define IDX_IDENTIFIER_BYTE_1 (0)
#define IDX_IDENTIFIER_BYTE_2 (1)
#define IDX_BOARD_ID          (2)

// Unique pattern (byte 1) for checking existence of Board ID
#define IDENTIFIER_BYTE_1 0x5E
// Unique pattern (byte 2) for checking existence of Board ID
#define IDENTIFIER_BYTE_2 0x1F

// Bit for identifying a master datalogger
#define DL_MASTER_ID_FLAG         (0x80)
// Bit for identifying a slave datalogger
#define DL_SLAVE_ID_FLAG          (0x40)

#ifndef BOARD_ID
#define BOARD_ID (DL_SLAVE_ID_FLAG | 0x01)
#endif

static bool board_id_set = false;

void setup() {
  // Initialise breakout board
  breakout_init();
  breakout_set_led(BO_LED_3, true); // Waiting for serial
  breakout_set_led(BO_LED_1, true); // Waiting for write command
  breakout_set_led(BO_LED_2, true); // Waiting for write to finish

  Serial.begin(115200);
  delay(100);
  // Warning: Not valid for all possible microcontrollers, ideal behaviour will
  // mean bootup will be delayed until serial monitor open
  while (breakout_get_switch_state() == sw_state_bot && !Serial) {}
  breakout_set_led(BO_LED_3, false);
  if (breakout_get_switch_state() != sw_state_mid) {
    Serial.printf("Return switch to middle to continue...\n");
  }
  // Wait for switch to return to middle
  while (breakout_get_switch_state() != sw_state_mid) {}
  Serial.printf("New ID: 0x%02X\n", BOARD_ID);
  
  Serial.printf("Checking if set identifier exists...\n");
  uint8_t identifier_byte_1 = EEPROM.read(IDX_IDENTIFIER_BYTE_1);
  uint8_t identifier_byte_2 = EEPROM.read(IDX_IDENTIFIER_BYTE_2);
  bool existing_set = identifier_byte_1 == IDENTIFIER_BYTE_1 &&
                      identifier_byte_2 == IDENTIFIER_BYTE_2;
  if (existing_set) {
    Serial.printf("Set identifier found!\n");
    uint8_t board_id = EEPROM.read(IDX_BOARD_ID);
    Serial.printf("Current ID: 0x%02X\n", board_id);
    if (board_id == BOARD_ID) {
      Serial.printf("New ID matches current ID, giving up...\n");
      board_id_set = true;
      return; 
    }
  } else {
    Serial.printf("Set identifier not found!\n");
  }
  Serial.printf("Put switch to upper position to write ID...\n");
  while (breakout_get_switch_state() != sw_state_top) {}
  breakout_set_led(BO_LED_1, false);
  Serial.printf("Writing new ID...\n");
  EEPROM.write(IDX_BOARD_ID, BOARD_ID);
  Serial.printf("Verifying ID write...\n");
  uint8_t board_id = EEPROM.read(IDX_BOARD_ID);
  if (board_id == BOARD_ID) {
    Serial.printf("ID Set: SUCCESSFUL\n");
    if (!existing_set) {
      Serial.printf("Writing identifier bytes...\n");
      EEPROM.write(IDX_IDENTIFIER_BYTE_1, IDENTIFIER_BYTE_1);
      EEPROM.write(IDX_IDENTIFIER_BYTE_2, IDENTIFIER_BYTE_2);
    }
    Serial.printf("Finished!\n");
    board_id_set = true;
  } else {
    Serial.printf("ID Set: FAILED\n");
  }
  breakout_set_led(BO_LED_3, false);
}

void loop() {
  breakout_set_led(BO_LED_1, false);
  breakout_set_led(BO_LED_2, false);
  breakout_set_led(BO_LED_3, false);
  delay(1500);
  if (board_id_set) {
    static uint8_t board_id = EEPROM.read(IDX_BOARD_ID);
    Serial.printf("Board ID: 0x%02X\n", board_id);
    breakout_set_led(BO_LED_1, true);
  } else {
    Serial.printf("Board ID not set!\n");
    breakout_set_led(BO_LED_2, true);
  }
  delay(500);
}