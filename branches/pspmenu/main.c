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
#include <stdio.h>

#include "sml.h"

/* Define the module info section */
PSP_MODULE_INFO("template", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Sets new HEAP size for PRX (10MB)
PSP_HEAP_SIZE_KB(10 * 1024);

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

	char options[5][20] = {"Abacate    ", "Melancia   ", "Abobora    ", "Pessego    ", "Alface     "};

	sml_drawbox(0, 0, 12, 4, ' ', ' ', 0x77777777, 0x77777777, 0x00000000, 0x00000000);
	int option = sml_menubox(1, 1, 11, 3, &options[0][0], 5, 20, 0, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF);

	pspDebugScreenSetXY(0, 30);
	pspDebugScreenPrintf("%d", option);

	return 0;
}
