#include "i2c.h"
#include "HT16K33.h"

HT16K33::HT16K33(uint8_t _addr) {
  i2c_addr=_addr;
  uint8_t buffer[16] = {};
}

void HT16K33::init() {
  i2cInit();
  clearDisplay();
  sendCommand(0x21);
  sendCommand(0x81);
  setBrightness(INITIAL_BRIGHTNESS);
}

void HT16K33::sendCommand(uint8_t c) {
  writeRegister(i2c_addr, c);
}

void HT16K33::setBrightness(uint8_t b) {
  brightness=b;
  sendCommand(0xE0 | brightness);
}

void HT16K33::increaseBrightness() {
  if (brightness<MAX_BRIGHTNESS) {
    brightness++;
    setBrightness(brightness);
  }
}

void HT16K33::decreaseBrightness() {
  if (brightness>MIN_BRIGHTNESS) {
    brightness--;
    setBrightness(brightness);
  }
}

uint16_t* HT16K33::transposeMatrix(uint8_t* matrix) {
  uint16_t transposedMatrix[8];
  for (uint8_t i=0;i<8;i++) {      // each outer loop generates one uint16_t for the horizontalDisplayBuffer
    uint16_t val=0;
    for (uint8_t j=0;j<16;j++)  {  // each inner loop looks at one uint8_t in the verticalDisplayBuffer
      uint8_t bitVal=(matrix[j]>>i)&1;
      uint8_t shiftVal=j;
      val+=(bitVal<<shiftVal);
    }
    transposedMatrix[i]=val;
  }
  return transposedMatrix;
}

void HT16K33::storeToBuffer(uint8_t* matrix) {
  for (uint8_t i=0;i<16;i++) buffer[i]=matrix[i];
}

void HT16K33::writeToDisplay(uint8_t* matrix) {
  writeRegisters(i2c_addr, DISP_REGISTER, 8, transposeMatrix(matrix));
}

void HT16K33::refreshDisplay() {
  writeToDisplay(buffer);
}

void HT16K33::clearDisplay() {
  uint8_t zero[16] = {0};
  writeToDisplay(zero);
}

void HT16K33::readButtons() {
  previousButtonState = currentButtonState;
  currentButtonState = readRegister(i2c_addr, KEYS_REGISTER);
  
  for (uint8_t i=0;i<8;i++) {
    if ( !((previousButtonState>>i)&1) && ((currentButtonState>>i)&1) ) buttonFirstPress|=_BV(i);
    else buttonFirstPress&=~_BV(i);
    
    if ( ((previousButtonState>>i)&1) && ((currentButtonState>>i)&1) ) {
      if (buttonHoldTime[i]<0xFFFF) buttonHoldTime[i]+=1;
    }
    else buttonHoldTime[i]=0;
  }
}

bool HT16K33::getButtonFirstPress(uint8_t i) {
  return (buttonFirstPress>>i)&1;
}

uint16_t HT16K33::getButtonHoldTime(uint8_t i) {
  return buttonHoldTime[i];
}

bool HT16K33::allowToMove(uint8_t i, uint16_t time, uint8_t rate) {
  // if first press, or hold time >time (time/50=sec) and %rate=0 (to reduce the rate) 
  return (getButtonFirstPress(i) || (getButtonHoldTime(i)>time && getButtonHoldTime(i)%rate==0));
}
