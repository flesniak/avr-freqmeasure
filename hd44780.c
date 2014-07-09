#include "hd44780.h"

#include <util/delay.h>

void commit() {
  CONTROL_PORT |= (1 << CONTROL_E);
  CONTROL_PORT &= ~(1 << CONTROL_E);
}

uchar readByte() {
  _delay_us(400);
  CONTROL_PORT |= (1 << CONTROL_E);
  _delay_us(400);
  uchar d = DATA_PORT;
  CONTROL_PORT &= ~(1 << CONTROL_E);
  return d;
}

bool isBusy() {
  //return (readByte() & 0b10000000);
  _delay_us(40); //still need delay, dont know why.
  uchar d = DATA_PORT;
  return( d & 0b10000000 );
}

void writeByte(uchar byte) {
  //_delay_us(40);
  while( isBusy() );
  CONTROL_PORT &= ~(1 << CONTROL_RW); //set WRITE
  DATA_DDR = 0xFF; //outputs
  DATA_PORT = byte;
  commit();
  DATA_PORT = 0;
  DATA_DDR = 0;
  CONTROL_PORT |= (1 << CONTROL_RW); //set READ
}

void init() {
  //library treats these DDR and CONTROL_PORT values as default, these shall not be changed from outside!
  DATA_DDR = 0;
  DATA_PORT = 0x00;
  CONTROL_DDR |= (1 << CONTROL_RS) | (1 << CONTROL_RW) | (1 << CONTROL_E); //every CONTROL pin is output
  CONTROL_PORT &= ~((1 << CONTROL_RS) | (0 << CONTROL_RW) | (1 << CONTROL_E)); //disable CONTROL pins, RW=1 is default
}

void clearDisplay() {
  writeByte(0x01);
  _delay_ms(2); //takes ~1.52ms
}

void setCursorStart() {
  writeByte(0x02);
}

void setMode1(bool cursorIncrement, bool displayShifting) {
  writeByte( 0x04 | (cursorIncrement << 1) | displayShifting );
}

void setMode2(bool displayOn, bool cursorOn, bool cursorFlash) {
  writeByte( 0x08 | (displayOn << 2) | (cursorOn << 1) | cursorFlash );
}

void setMovement(bool moveContent, bool moveRight) {
  writeByte( 0x10 | (moveContent << 3) | (moveRight << 2) );
}

void setMode3(bool eightBit, bool multiLine, bool bigFont) {
  writeByte( 0x20 | (eightBit << 4) | (multiLine << 3) | (bigFont << 2) );
}

void setCGRAMAddress(uchar address) {
  writeByte( 0x40 | (address & 0x3F) );
}

void setDDRAMAddress(uchar address) {
  writeByte( 0x80 | (address & 0x7F) );
}

displayStatus_t getStatus() {
  displayStatus_t d;
  d.address = readByte();
  d.busy = (d.address >> 7);
  d.address &= 0x7F;
  return d;
}

void writeRAM(uchar data) {
  CONTROL_PORT |= (1 << CONTROL_RS);
  writeByte(data);
  CONTROL_PORT &= ~(1 << CONTROL_RS);
}

uchar readRAM() {
  CONTROL_PORT |= (1 << CONTROL_RS);
  uchar d = readByte();
  CONTROL_PORT &= ~(1 << CONTROL_RS);
  return d;
}
