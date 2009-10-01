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
#include <string.h>

#include "sml.h"
#include "filebrowser.h"

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

	/* Get the full path to EBOOT.PBP. */
	char psp_full_path[1024 + 1];
	char *psp_eboot_path;

	strncpy(psp_full_path, argv[0], sizeof(psp_full_path) - 1);
	psp_full_path[sizeof(psp_full_path) - 1] = '\0';

	psp_eboot_path = strrchr(psp_full_path, '/');
	if(psp_eboot_path != NULL) {
		*(psp_eboot_path+1) = '\0';
	}

	pspDebugScreenSetXY(0, 0);
	pspDebugScreenSetTextColor(0xFF00FFFF);
	pspDebugScreenSetBackColor(0xFFFF0000);
	pspDebugScreenPrintf("%-68.68s", " FCEU-PSP - FCEUltra for PSP (Alpha)");
	pspDebugScreenSetXY(0, 1);
	pspDebugScreenPrintf("(C)2009 - bootsector@ig.com.br");
	sml_drawbox(0, 1, 67, 31, ' ', ' ', 0xFFFF0000, 0xFFFF0000, 0x00000000, 0x00000000);
	pspDebugScreenSetXY(0, 1);
	pspDebugScreenSetTextColor(0xFF00FFFF);
	pspDebugScreenSetBackColor(0xFFFF0000);
	pspDebugScreenPrintf(" (C)2009 - bootsector@ig.com.br");
	pspDebugScreenSetXY(0, 32);
	pspDebugScreenSetTextColor(0xFF00FFFF);
	pspDebugScreenSetBackColor(0xFFFF0000);
	pspDebugScreenPrintf(" X-Select O-Exit");
	//char *filename = filebrowser(psp_full_path);
	char *filename = filebrowser("ms0:/");

	if(filename)
		printf("%s\n", filename);


	return 0;
}
