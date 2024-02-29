/*
  ----------------------------------------------------------------------------

  This file is part of the Sanworks Bpod_FormatMicroSD repository
  Copyright (C) Sanworks LLC, Rochester, New York, USA

  ----------------------------------------------------------------------------

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 3.

  This program is distributed  WITHOUT ANY WARRANTY and without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// This firmware allows the user to detect and format the microSD card in any Bpod module
// In MATLAB, run BpodFormatMicroSD('COM3'); % Where COM3 is the correct port for the device.
//
// Attribution: This code was derived from the SdFormatter example sketch in Bill Greiman's SDFat library:
// https://github.com/greiman/SdFat/tree/master/examples/SdFormatter
//
// Usage: Send 'F' (ASCII 70) to the USB serial port to begin formatting. The device will return:
// 1. sd_initOK (0 if init failed, 1 if not)
// 2. eraseOK (0 if erase failed, 1 if not)
// 3. A human-readable character string indicating the final status of the format operation.

#include "SdFat.h"

FatFormatter fatFormatter;
SdCardFactory cardFactory;
SdCard* m_card = nullptr;
uint32_t const ERASE_SIZE = 262144L;
uint8_t  sectorBuffer[512];
uint8_t msg = 0;
uint8_t sd_initOK = 0;
uint8_t sd_eraseOK = 0;
uint32_t sd_cardSectorCount = 0;

void setup() {
  m_card = cardFactory.newCard(SdioConfig(FIFO_SDIO));
  sd_initOK = 1;
  if (!m_card || m_card->errorCode()) {
    sd_initOK = 0;
  }
}

void loop() {
  if (Serial.available()) {
    msg = Serial.read();
    switch(msg) {
      case 'F': // Erase and format microSD card
        Serial.write(sd_initOK);
        if (sd_initOK){
          sd_eraseOK = eraseCard();
          if (!m_card->readSector(0, sectorBuffer)) {
            sd_eraseOK = 0;
          }
          Serial.write(sd_eraseOK);
          if (sd_eraseOK) {
            fatFormatter.format(m_card, sectorBuffer, &Serial);
          }
        } else {
          Serial.write(0);
        }
      break;
    }
  }
}

uint8_t eraseCard() {
  uint32_t firstBlock = 0;
  uint32_t lastBlock;
  uint8_t eraseOK = 1;
  sd_cardSectorCount = m_card->sectorCount();
  do {
    lastBlock = firstBlock + ERASE_SIZE - 1;
    if (lastBlock >= sd_cardSectorCount) {
      lastBlock = sd_cardSectorCount - 1;
    }
    if (!m_card->erase(firstBlock, lastBlock)) {
      eraseOK = 0;
    }
    firstBlock += ERASE_SIZE;
  } while (firstBlock < sd_cardSectorCount);
  return eraseOK;
}
