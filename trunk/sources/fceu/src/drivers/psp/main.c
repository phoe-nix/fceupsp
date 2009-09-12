#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <psppower.h>
#include <pspgu.h>


#include "../../types.h"
#include "../../driver.h"
#include "../../fceu.h"
#include "pspvideo.h"
#include "pspinput.h"
#include "vram.h"


/* Define the module info section */
PSP_MODULE_INFO("fceu-psp", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Sets new HEAP size for PRX (10MB)
PSP_HEAP_SIZE_KB(10 * 1024);

FCEUGI *CurGame = NULL;
void DoFun();
//extern unsigned int __attribute__((aligned(16))) clut256[];
//extern uint8 *XBuf;
int SetupCallbacks(void);

int main(int argc, char *argv[])
{
	SetupCallbacks();

	sceKernelDcacheWritebackAll();

	scePowerSetClockFrequency(333, 333, 166);

	//XBuf = getStaticVramBuffer(512,272,3);

    if(!(FCEUI_Initialize())) {
		printf("FCEUltra did not initialize.\n");
		return(0);
	}

	FCEUI_SetVidSystem(0); // 0 - NTSC
	FCEUI_SetGameGenie(0);
	FCEUI_DisableSpriteLimitation(1);
	FCEUI_SetSoundVolume(0);
	FCEUI_SetSoundQuality(0);
	FCEUI_SetLowPass(0);
	FCEUI_Sound(0);

    FCEUGI *tmp;

    if((tmp=FCEUI_LoadGame("ms0:/nesrom2.nes"))) {
        printf("Game Loaded!\n");
        CurGame=tmp;
    }
    else {
        printf("Didn't load Game!\n");
    }

    PSPInputInitPads();

    PSPVideoOverrideNESClut();

    PSPVideoInit();


	while(CurGame) {//FCEUI_CloseGame turns this false
        DoFun();
    }

	sceGuTerm();

	sceKernelExitGame();

	return 0;
}

void FCEUD_Update(uint8 *XBuf, int32 *tmpsnd, int32 ssize)
{
	PSPVideoRenderFrame(XBuf);
	PSPInputReadPad();
	//printf("FCEUD_Update\n");

//	OutputSound(tmpsnd, ssize);

//	if (Get_NESInput()) {
//		FCEUI_CloseGame();
//		CurGame=0;
//	}
}

void DoFun()
{
    uint8 *gfx;
    int32 *sound;
    int32 ssize;

    FCEUI_Emulate(&gfx, &sound, &ssize, 0);
    FCEUD_Update(gfx, sound, ssize);
}

/* Definitions for some platform-dependant FCEU functions */
void FCEUD_PrintError(char *s) {
	printf("Error: %s", s);
}

FILE *FCEUD_UTF8fopen(const char *fn, const char *mode) {
	return(fopen(fn, mode));
}

void FCEUD_GetPalette(uint8 index, uint8 *r, uint8 *g, uint8 *b) {
//	unsigned int* clut = (unsigned int*)(((unsigned int)clut256)|0x40000000);
//
//	*r = clut[index] & 0xFF;
//	*g = clut[index] & 0xFF00 >> 8;
//	*b = clut[index] & 0xFF0000 >> 16;
}

void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b) {
//	unsigned int* clut = (unsigned int*)(((unsigned int)clut256)|0x40000000);
//
//	unsigned int color = ((b<<16)|(g<<8)|(r<<0));
//
//	clut[index]     = color;
//	clut[index+64]  = color;
//	clut[index+128] = color;
//	clut[index+192] = color;
}

void FCEUD_Message(char *s) {
	printf("Message: %s", s);
}

int FCEUD_SendData(void *data, uint32 len) {
	printf("FCEUD_SendData not implemented");
	return(0);
}

void FCEUD_NetworkClose(void) {
	printf("FCEUD_NetworkClose not implemented");
}


/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
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

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

