/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2012 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2012 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdlib.h>

#ifndef __PSL1GHT__
#include <sdk_version.h>
#endif

#ifdef __PSL1GHT__
#include <io/pad.h>
#else
#include <cell/pad.h>
#endif

#ifdef HAVE_MOUSE
#ifdef __PSL1GHT__
#include <io/mouse.h>
#else
#include <cell/mouse.h>
#endif
#endif

#include <sys/memory.h>

#ifdef HAVE_OSKUTIL
#ifdef __PSL1GHT__
#include <sysutil/osk.h>
#else
#include <sysutil/sysutil_oskdialog.h>
#endif
#endif

#ifdef HAVE_SYSUTILS
#include <sysutil/sysutil_common.h>
#endif

#include "sdk_defines.h"

#include "ps3_input.h"
#include "../driver.h"
#include "../console/retroarch_console.h"
#include "../libretro.h"
#include "../general.h"

/*============================================================
	PS3 MOUSE
============================================================ */

#ifdef HAVE_MOUSE

#ifndef __PSL1GHT__
#define MAX_MICE 7
#endif

static void ps3_mouse_input_deinit(void)
{
   cellMouseEnd();
}

static uint32_t ps3_mouse_input_mice_connected(void)
{
   CellMouseInfo mouse_info;
   cellMouseGetInfo(&mouse_info);
   return mouse_info.now_connect;
}

CellMouseData ps3_mouse_input_poll_device(uint32_t id)
{
   CellMouseData mouse_data;

   // Get new pad data
   cellMouseGetData(id, &mouse_data);

   return mouse_data;
}

#endif

/*============================================================
	PS3 PAD
============================================================ */

#define MAP(x) (x & 0xFF)

const struct platform_bind platform_keys[] = {
   { CTRL_CIRCLE_MASK, "Circle button" },
   { CTRL_CROSS_MASK, "Cross button" },
   { CTRL_TRIANGLE_MASK, "Triangle button" },
   { CTRL_SQUARE_MASK, "Square button" },
   { CTRL_UP_MASK, "D-Pad Up" },
   { CTRL_DOWN_MASK, "D-Pad Down" },
   { CTRL_LEFT_MASK, "D-Pad Left" },
   { CTRL_RIGHT_MASK, "D-Pad Right" },
   { CTRL_SELECT_MASK, "Select button" },
   { CTRL_START_MASK, "Start button" },
   { CTRL_L1_MASK, "L1 button" },
   { CTRL_L2_MASK, "L2 button" },
   { CTRL_L3_MASK, "L3 button" },
   { CTRL_R1_MASK, "R1 button" },
   { CTRL_R2_MASK, "R2 button" },
   { CTRL_R3_MASK, "R3 button" },
   { CTRL_LSTICK_LEFT_MASK, "LStick Left" },
   { CTRL_LSTICK_RIGHT_MASK, "LStick Right" },
   { CTRL_LSTICK_UP_MASK, "LStick Up" },
   { CTRL_LSTICK_DOWN_MASK, "LStick Down" },
   { CTRL_LEFT_MASK | CTRL_LSTICK_LEFT_MASK, "LStick D-Pad Left" },
   { CTRL_RIGHT_MASK | CTRL_LSTICK_RIGHT_MASK, "LStick D-Pad Right" },
   { CTRL_UP_MASK | CTRL_LSTICK_UP_MASK, "LStick D-Pad Up" },
   { CTRL_DOWN_MASK | CTRL_LSTICK_DOWN_MASK, "LStick D-Pad Down" },
   { CTRL_RSTICK_LEFT_MASK, "RStick Left" },
   { CTRL_RSTICK_RIGHT_MASK, "RStick Right" },
   { CTRL_RSTICK_UP_MASK, "RStick Up" },
   { CTRL_RSTICK_DOWN_MASK, "RStick Down" },
   { CTRL_LEFT_MASK | CTRL_RSTICK_LEFT_MASK, "RStick D-Pad Left" },
   { CTRL_RIGHT_MASK | CTRL_RSTICK_RIGHT_MASK, "RStick D-Pad Right" },
   { CTRL_UP_MASK | CTRL_RSTICK_UP_MASK, "RStick D-Pad Up" },
   { CTRL_DOWN_MASK | CTRL_RSTICK_DOWN_MASK, "RStick D-Pad Down" },
};

const unsigned int platform_keys_size = sizeof(platform_keys);

static uint64_t state[MAX_PADS];
static unsigned pads_connected;
#ifdef HAVE_MOUSE
static unsigned mice_connected;
#endif

uint64_t cell_pad_input_poll_device(uint32_t id)
{
   CellPadData pad_data;
   static uint64_t ret[MAX_PADS];

   // Get new pad data
   cellPadGetData(id, &pad_data);

   if (pad_data.len != 0)
   {
      ret[id] = 0;

      // Build the return value.
      ret[id] |= (uint64_t)MAP(pad_data.button[LOWER_BUTTONS]);
      ret[id] |= (uint64_t)MAP(pad_data.button[HIGHER_BUTTONS]) << 8;
      ret[id] |= (uint64_t)MAP(pad_data.button[RSTICK_X]) << 32;
      ret[id] |= (uint64_t)MAP(pad_data.button[RSTICK_Y]) << 40;
      ret[id] |= (uint64_t)MAP(pad_data.button[LSTICK_X]) << 16;
      ret[id] |= (uint64_t)MAP(pad_data.button[LSTICK_Y]) << 24;

      ret[id] |= (uint64_t)(PRESSED_LEFT_LSTICK(ret[id])) << LSTICK_LEFT_SHIFT;
      ret[id] |= (uint64_t)(PRESSED_RIGHT_LSTICK(ret[id])) << LSTICK_RIGHT_SHIFT;
      ret[id] |= (uint64_t)(PRESSED_UP_LSTICK(ret[id])) << LSTICK_UP_SHIFT;
      ret[id] |= (uint64_t)(PRESSED_DOWN_LSTICK(ret[id])) << LSTICK_DOWN_SHIFT;

      ret[id] |= (uint64_t)(PRESSED_LEFT_RSTICK(ret[id])) << RSTICK_LEFT_SHIFT;
      ret[id] |= (uint64_t)(PRESSED_RIGHT_RSTICK(ret[id])) << RSTICK_RIGHT_SHIFT;
      ret[id] |= (uint64_t)(PRESSED_UP_RSTICK(ret[id])) << RSTICK_UP_SHIFT;
      ret[id] |= (uint64_t)(PRESSED_DOWN_RSTICK(ret[id])) << RSTICK_DOWN_SHIFT;
   }

   return ret[id];
}
static void ps3_input_poll(void *data)
{
   CellPadInfo2 pad_info;
   (void)data;

   for (unsigned i = 0; i < MAX_PADS; i++)
      state[i] = cell_pad_input_poll_device(i);

   cellPadGetInfo2(&pad_info);
   pads_connected = pad_info.now_connect; 
#ifdef HAVE_MOUSE
   mice_connected = ps3_mouse_input_mice_connected();
#endif
}

#ifdef HAVE_MOUSE

static int16_t ps3_mouse_device_state(void *data, unsigned player, unsigned id)
{
   CellMouseData mouse_state = ps3_mouse_input_poll_device(player);

   switch (id)
   {
      case RETRO_DEVICE_ID_MOUSE_LEFT:
         return (!mice_connected ? 0 : mouse_state.buttons & CELL_MOUSE_BUTTON_1);
      case RETRO_DEVICE_ID_MOUSE_RIGHT:
         return (!mice_connected ? 0 : mouse_state.buttons & CELL_MOUSE_BUTTON_2);
      case RETRO_DEVICE_ID_MOUSE_X:
         return (!mice_connected ? 0 : mouse_state.x_axis);
      case RETRO_DEVICE_ID_MOUSE_Y:
         return (!mice_connected ? 0 : mouse_state.y_axis);
      default:
         return 0;
   }
}

#endif

static int16_t ps3_input_state(void *data, const struct retro_keybind **binds,
      unsigned port, unsigned device,
      unsigned index, unsigned id)
{
   (void)data;

   unsigned player = port;
   uint64_t button = binds[player][id].joykey;
   int16_t retval = 0;

   if(player < pads_connected)
   {
      switch (device)
      {
         case RETRO_DEVICE_JOYPAD:
            retval = CTRL_MASK(state[player], button) ? 1 : 0;
            break;
#ifdef HAVE_MOUSE
         case RETRO_DEVICE_MOUSE:
	    retval = ps3_mouse_device_state(data, player, id);
            break;
#endif
      }
    }

   return retval;
}

/*============================================================
	ON-SCREEN KEYBOARD UTILITY
============================================================ */

#ifdef HAVE_OSKUTIL

#define OSK_IN_USE 1

void oskutil_init(oskutil_params *params, unsigned int containersize)
{
   params->flags = 0;
   params->is_running = false;
   if(containersize)
      params->osk_memorycontainer =  containersize; 
   else
      params->osk_memorycontainer =  1024*1024*7;
}

static bool oskutil_enable_key_layout (void)
{
   int ret = pOskSetKeyLayoutOption(CELL_OSKDIALOG_10KEY_PANEL | \
      CELL_OSKDIALOG_FULLKEY_PANEL);
   if (ret < 0)
      return (false);
   else
      return (true);
}

static void oskutil_create_activation_parameters(oskutil_params *params)
{
   params->dialogParam.controlPoint.x = 0.0;
   params->dialogParam.controlPoint.y = 0.0;

   int32_t LayoutMode = CELL_OSKDIALOG_LAYOUTMODE_X_ALIGN_CENTER | CELL_OSKDIALOG_LAYOUTMODE_Y_ALIGN_TOP;
   pOskSetLayoutMode(LayoutMode);

   params->dialogParam.osk_allowed_panels = 
   CELL_OSKDIALOG_PANELMODE_ALPHABET |
   CELL_OSKDIALOG_PANELMODE_NUMERAL | 
   CELL_OSKDIALOG_PANELMODE_NUMERAL_FULL_WIDTH |
   CELL_OSKDIALOG_PANELMODE_ENGLISH;

   params->dialogParam.firstViewPanel = CELL_OSKDIALOG_PANELMODE_ENGLISH;
   params->dialogParam.osk_prohibit_flags = 0;
}

void oskutil_write_message(oskutil_params *params, const wchar_t* msg)
{
   params->inputFieldInfo.osk_inputfield_message = (uint16_t*)msg;
}

void oskutil_write_initial_message(oskutil_params *params, const wchar_t* msg)
{
   params->inputFieldInfo.osk_inputfield_starttext = (uint16_t*)msg;
}

bool oskutil_start(oskutil_params *params) 
{
   memset(params->osk_text_buffer, 0, sizeof(*params->osk_text_buffer));
   memset(params->osk_text_buffer_char, 0, 256);

   params->text_can_be_fetched = false;

   if (params->flags & OSK_IN_USE)
      return (true);

   int ret = sys_memory_container_create(&params->containerid, params->osk_memorycontainer);

   if(ret < 0)
      return (false);

   params->inputFieldInfo.osk_inputfield_max_length = CELL_OSKDIALOG_STRING_SIZE;	

   oskutil_create_activation_parameters(params);

   if(!oskutil_enable_key_layout())
      return (false);

   ret = pOskLoadAsync(params->containerid, &params->dialogParam, &params->inputFieldInfo);
   if(ret < 0)
      return (false);

   params->flags |= OSK_IN_USE;
   params->is_running = true;

   return (true);
}

void oskutil_close(oskutil_params *params)
{
   pOskAbort();
}

void oskutil_finished(oskutil_params *params)
{
   int num;

   params->outputInfo.osk_callback_return_param = CELL_OSKDIALOG_INPUT_FIELD_RESULT_OK;
   params->outputInfo.osk_callback_num_chars = 256;
   params->outputInfo.osk_callback_return_string = (uint16_t *)params->osk_text_buffer;

   pOskUnloadAsync(&params->outputInfo);

   switch (params->outputInfo.osk_callback_return_param)
   {
      case CELL_OSKDIALOG_INPUT_FIELD_RESULT_OK:
         num = wcstombs(params->osk_text_buffer_char, params->osk_text_buffer, 256);
         params->osk_text_buffer_char[num]=0;
         params->text_can_be_fetched = true;
         break;
      case CELL_OSKDIALOG_INPUT_FIELD_RESULT_CANCELED:
      case CELL_OSKDIALOG_INPUT_FIELD_RESULT_ABORT:
      case CELL_OSKDIALOG_INPUT_FIELD_RESULT_NO_INPUT_TEXT:
      default:
         params->osk_text_buffer_char[0]=0;
         params->text_can_be_fetched = false;
         break;
   }

   params->flags &= ~OSK_IN_USE;
}

void oskutil_unload(oskutil_params *params)
{
   sys_memory_container_destroy(params->containerid);
   params->is_running = false;
}

#endif

/*============================================================
	RetroArch PS3 INPUT DRIVER 
============================================================ */

static void ps3_free_input(void *data)
{
   (void)data;
   //cellPadEnd();
}

static void* ps3_input_initialize(void)
{
   cellPadInit(MAX_PADS);
#ifdef HAVE_MOUSE
   cellMouseInit(MAX_MICE);
#endif
   for(unsigned i = 0; i < MAX_PADS; i++)
   	ps3_input_map_dpad_to_stick(g_settings.input.dpad_emulation[i], i);
   return (void*)-1;
}

void ps3_input_map_dpad_to_stick(uint32_t map_dpad_enum, uint32_t controller_id)
{
   switch(map_dpad_enum)
   {
      case DPAD_EMULATION_NONE:
         g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_UP].joykey	= platform_keys[PS3_DEVICE_ID_JOYPAD_UP].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_DOWN].joykey	= platform_keys[PS3_DEVICE_ID_JOYPAD_DOWN].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_LEFT].joykey	= platform_keys[PS3_DEVICE_ID_JOYPAD_LEFT].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_RIGHT].joykey	= platform_keys[PS3_DEVICE_ID_JOYPAD_RIGHT].joykey;
	 break;
      case DPAD_EMULATION_LSTICK:
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_UP].joykey	= platform_keys[PS3_DEVICE_ID_LSTICK_UP_DPAD].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_DOWN].joykey	= platform_keys[PS3_DEVICE_ID_LSTICK_DOWN_DPAD].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_LEFT].joykey	= platform_keys[PS3_DEVICE_ID_LSTICK_LEFT_DPAD].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_RIGHT].joykey	= platform_keys[PS3_DEVICE_ID_LSTICK_RIGHT_DPAD].joykey;
	 break;
      case DPAD_EMULATION_RSTICK:
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_UP].joykey	= platform_keys[PS3_DEVICE_ID_RSTICK_UP_DPAD].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_DOWN].joykey	= platform_keys[PS3_DEVICE_ID_RSTICK_DOWN_DPAD].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_LEFT].joykey	= platform_keys[PS3_DEVICE_ID_RSTICK_LEFT_DPAD].joykey;
	 g_settings.input.binds[controller_id][RETRO_DEVICE_ID_JOYPAD_RIGHT].joykey	= platform_keys[PS3_DEVICE_ID_RSTICK_RIGHT_DPAD].joykey;
	 break;
   }
}

static bool ps3_key_pressed(void *data, int key)
{
   (void)data;
#ifdef HAVE_OPENGL
   gl_t *gl = driver.video_data;
#endif

   switch (key)
   {
      case RARCH_FAST_FORWARD_HOLD_KEY:
         return CTRL_RSTICK_DOWN(state[0]) && CTRL_R2(~state[0]);
      case RARCH_LOAD_STATE_KEY:
         return (CTRL_RSTICK_UP(state[0]) && CTRL_R2(state[0]));
      case RARCH_SAVE_STATE_KEY:
         return (CTRL_RSTICK_DOWN(state[0]) && CTRL_R2(state[0]));
      case RARCH_STATE_SLOT_PLUS:
         return (CTRL_RSTICK_RIGHT(state[0]) && CTRL_R2(state[0]));
      case RARCH_STATE_SLOT_MINUS:
         return (CTRL_RSTICK_LEFT(state[0]) && CTRL_R2(state[0]));
      case RARCH_FRAMEADVANCE:
      	if(g_console.frame_advance_enable)
	{
		g_console.menu_enable = true;
		g_console.ingame_menu_enable = true;
		g_console.mode_switch = MODE_MENU;
	}
	 return false;
      case RARCH_REWIND:
         return CTRL_RSTICK_UP(state[0]) && CTRL_R2(~state[0]);
      case RARCH_QUIT_KEY:
#ifdef HAVE_OPENGL
	 if(IS_TIMER_EXPIRED(gl))
	 {
            uint32_t r3_pressed = CTRL_R3(state[0]);
	    uint32_t l3_pressed = CTRL_L3(state[0]);
	    bool retval = false;
	    g_console.menu_enable = (r3_pressed && l3_pressed && IS_TIMER_EXPIRED(gl));
	    g_console.ingame_menu_enable = r3_pressed && !l3_pressed;

	    if(g_console.menu_enable || (g_console.ingame_menu_enable && !g_console.menu_enable))
	    {
               g_console.mode_switch = MODE_MENU;
	       SET_TIMER_EXPIRATION(gl, 30);
	       retval = g_console.menu_enable;
	    }

	    retval = g_console.ingame_menu_enable ? g_console.ingame_menu_enable : g_console.menu_enable;
	    return retval;
	 }
#endif
      default:
         return false;
   }
}

static void ps3_set_default_keybind_lut(void)
{
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_B]		= platform_keys[PS3_DEVICE_ID_JOYPAD_CROSS].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_Y]		= platform_keys[PS3_DEVICE_ID_JOYPAD_SQUARE].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_SELECT]	= platform_keys[PS3_DEVICE_ID_JOYPAD_SELECT].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_START]	= platform_keys[PS3_DEVICE_ID_JOYPAD_START].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_UP]		= platform_keys[PS3_DEVICE_ID_JOYPAD_UP].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_DOWN]	= platform_keys[PS3_DEVICE_ID_JOYPAD_DOWN].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_LEFT]	= platform_keys[PS3_DEVICE_ID_JOYPAD_LEFT].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_RIGHT]	= platform_keys[PS3_DEVICE_ID_JOYPAD_RIGHT].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_A]		= platform_keys[PS3_DEVICE_ID_JOYPAD_CIRCLE].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_X]		= platform_keys[PS3_DEVICE_ID_JOYPAD_TRIANGLE].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_L]		= platform_keys[PS3_DEVICE_ID_JOYPAD_L1].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_R]		= platform_keys[PS3_DEVICE_ID_JOYPAD_R1].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_R2]		= platform_keys[PS3_DEVICE_ID_JOYPAD_R2].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_R3]		= platform_keys[PS3_DEVICE_ID_JOYPAD_R3].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_L2]		= platform_keys[PS3_DEVICE_ID_JOYPAD_L2].joykey;
   rarch_default_keybind_lut[RETRO_DEVICE_ID_JOYPAD_L3]		= platform_keys[PS3_DEVICE_ID_JOYPAD_L3].joykey;
}

const input_driver_t input_ps3 = {
   .init = ps3_input_initialize,
   .poll = ps3_input_poll,
   .input_state = ps3_input_state,
   .key_pressed = ps3_key_pressed,
   .free = ps3_free_input,
   .set_default_keybind_lut = ps3_set_default_keybind_lut,
   .ident = "ps3",
};

