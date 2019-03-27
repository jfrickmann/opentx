/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <opentx.h>

#define RECEIVER_OPTIONS_2ND_COLUMN 80

extern uint8_t g_moduleIdx;

void onTxOptionsUpdateConfirm(const char * result)
{
  if (result == STR_OK) {
    reusableBuffer.hardwareAndSettings.moduleSettings.state = PXX2_SETTINGS_WRITE;
    reusableBuffer.hardwareAndSettings.moduleSettings.dirty = 2;
    reusableBuffer.hardwareAndSettings.moduleSettings.timeout = 0;
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_MODULE_SETTINGS;
  }
  else {
    popMenu();
  }
}

enum {
  ITEM_MODULE_SETTINGS_RF_PROTOCOL,
  ITEM_MODULE_SETTINGS_EXTERNAL_ANTENNA,
  ITEM_MODULE_SETTINGS_POWER,
  ITEM_MODULE_SETTINGS_COUNT
};

/* Options order:
 * - RF Protocol
 * - External antenna
 * - Power
 */
const uint8_t moduleOptions[] = {
  0b00000000, // None
  0b11111011, // XJT
  0b11111011, // IXJT
  0b11111011, // IXJT-PRO
  0b11111011, // IXJT-S
  0b11111100, // R9M
  0b11111100, // R9MLite
  0b11111100, // R9MLite-PRO
};

#define IF_MODULE_OPTIONS(option, count) uint8_t((moduleOptions[modelId] & (1 << option)) ? count : HIDDEN_ROW)

void menuModelModuleOptions(event_t event)
{
  uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].information.modelID;

  SUBMENU_NOTITLE(ITEM_MODULE_SETTINGS_COUNT, {
    IF_MODULE_OPTIONS(0, 0),
    IF_MODULE_OPTIONS(1, 0),
    IF_MODULE_OPTIONS(2, 0),
  });

  if (event == EVT_ENTRY) {
#if defined(SIMU)
    reusableBuffer.hardwareAndSettings.moduleSettings.state = PXX2_SETTINGS_OK;
#else
    // no need to initialize reusableBuffer.hardwareAndSettings.moduleSettings.state to PXX2_HARDWARE_INFO
    reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].current = PXX2_HW_INFO_TX_ID;
    reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].maximum = PXX2_HW_INFO_TX_ID;
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_GET_HARDWARE_INFO;
#endif
  }

  if (reusableBuffer.hardwareAndSettings.moduleSettings.state == PXX2_HARDWARE_INFO && moduleSettings[g_moduleIdx].mode == MODULE_MODE_NORMAL) {
    reusableBuffer.hardwareAndSettings.moduleSettings.state = PXX2_SETTINGS_READ;
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_MODULE_SETTINGS;
  }

  if (menuEvent) {
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_NORMAL;
    if (reusableBuffer.hardwareAndSettings.moduleSettings.dirty) {
      abortPopMenu();
      POPUP_CONFIRMATION("Update TX options?", onTxOptionsUpdateConfirm);
    }
    else {
      return;
    }
  }

  if (event == EVT_KEY_LONG(KEY_ENTER) && reusableBuffer.hardwareAndSettings.moduleSettings.dirty) {
    killEvents(event);
    reusableBuffer.hardwareAndSettings.moduleSettings.state = PXX2_SETTINGS_WRITE;
    reusableBuffer.hardwareAndSettings.moduleSettings.dirty = 0;
    reusableBuffer.hardwareAndSettings.moduleSettings.timeout = 0;
    moduleSettings[g_moduleIdx].mode = MODULE_MODE_MODULE_SETTINGS;
  }

  if (reusableBuffer.hardwareAndSettings.moduleSettings.dirty == 2 && reusableBuffer.hardwareAndSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
    popMenu();
  }

  if (menuVerticalPosition == 0) {
    while (mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
      ++menuVerticalPosition;
    }
  }

  int8_t sub = menuVerticalPosition;
  lcdDrawTextAlignedLeft(0, "Module options");
  lcdInvertLine(0);

  if (reusableBuffer.hardwareAndSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
    for (uint8_t k=0; k<LCD_LINES-1; k++) {
      coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
      uint8_t i = k + menuVerticalOffset;
      for (int j=0; j<=i; ++j) {
        if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
          ++i;
        }
      }
      LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

      switch (i) {
        case ITEM_MODULE_SETTINGS_RF_PROTOCOL:
          lcdDrawText(0, y, "RF Protocol");
          lcdDrawTextAtIndex(RECEIVER_OPTIONS_2ND_COLUMN, y, STR_XJT_PROTOCOLS, reusableBuffer.hardwareAndSettings.moduleSettings.rfProtocol + 1, attr);
          if (attr) {
            reusableBuffer.hardwareAndSettings.moduleSettings.rfProtocol = checkIncDec(event, reusableBuffer.hardwareAndSettings.moduleSettings.rfProtocol, RF_PROTO_X16, RF_PROTO_LAST, 0, nullptr);
            if (checkIncDec_Ret) {
              reusableBuffer.hardwareAndSettings.moduleSettings.dirty = true;
            }
          }
          break;

        case ITEM_MODULE_SETTINGS_EXTERNAL_ANTENNA:
          reusableBuffer.hardwareAndSettings.moduleSettings.externalAntenna = editCheckBox(reusableBuffer.hardwareAndSettings.moduleSettings.externalAntenna, RECEIVER_OPTIONS_2ND_COLUMN, y, "Ext. antenna", attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.moduleSettings.dirty = true;
          }
          break;

        case ITEM_MODULE_SETTINGS_POWER:
          lcdDrawText(0, y, "Power");
          lcdDrawNumber(RECEIVER_OPTIONS_2ND_COLUMN, y, reusableBuffer.hardwareAndSettings.moduleSettings.txPower, attr);
          lcdDrawText(lcdNextPos, y, "dBm");
          if (attr) {
            reusableBuffer.hardwareAndSettings.moduleSettings.txPower = checkIncDec(event, reusableBuffer.hardwareAndSettings.moduleSettings.txPower, -127, 127);
            if (checkIncDec_Ret) {
              reusableBuffer.hardwareAndSettings.moduleSettings.dirty = true;
            }
          }
          break;
      }
    }
  }
  else {
    lcdDrawText(4 * FW, 4 * FH, "Waiting for TX...");
  }
}
