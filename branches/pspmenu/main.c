/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Basic ELF template
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: main.c 1888 2006-05-01 08:47:04Z tyranid $
 * $HeadURL$
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

#define ITEMS_COUNT 5

/* Define the module info section */
PSP_MODULE_INFO("template", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/* Exit callback */
int exit_callback()
{
  sceKernelExitGame();

  return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
  int cbid;

  cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
  sceKernelRegisterExitCallback(cbid);

  sceKernelSleepThreadCB();

  return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
  int thid = 0;

  thid = sceKernelCreateThread("update_thread", CallbackThread,
			       0x11, 0xFA0, 0, 0);
  if(thid >= 0)
    {
      sceKernelStartThread(thid, 0, 0);
    }

  return thid;
}

//-------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	SetupCallbacks();

	pspDebugScreenInit();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	char options[ITEMS_COUNT][10] = {"Abacate", "Melancia", "Abobora", "Pessego", "Alface"};
	int x1, y1, x2, y2;
	SceCtrlData pad;
	int old_buttons = 0;

	x1 = 0; y1 = 0;
	x2 = 10; y2 = 2;

	int how_many_items_can_show = y2 - y1 + 1;
	int first_viewable_item = 0; // last one is: first_viewable_item + how_many_items_can_show - 1
	int last_viewable_item = first_viewable_item + how_many_items_can_show - 1;
	int curr_item = 0; // Currently selected item
	int start_item = curr_item % how_many_items_can_show;

	int max_item_width = x2 - x1 + 1;

	int x, y;
	int item;

	for(;;) {
		// Show currently viewable items
		y = y1;
		for(item = first_viewable_item; item <= (last_viewable_item >= ITEMS_COUNT)?ITEMS_COUNT-1:last_viewable_item; item++) {
			pspDebugScreenSetXY(x1, y);

			if(item == curr_item) {
				pspDebugScreenSetBackColor(0xFFFFFFFF);
				pspDebugScreenSetTextColor(0x00000000);
			} else {
				pspDebugScreenSetBackColor(0x00000000);
				pspDebugScreenSetTextColor(0xFFFFFFFF);
			}

			pspDebugScreenPrintf("%s", options[item]);

			y++;
		}

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons != old_buttons){
			if (pad.Buttons & PSP_CTRL_UP) {
				if(curr_item > 0) {
					curr_item--;

				}
			}

			if (pad.Buttons & PSP_CTRL_DOWN) {
				if(curr_item < ITEMS_COUNT-1) {
					curr_item++;
				}
			}

			old_buttons = pad.Buttons;
		}

	}

	return 0;
}
