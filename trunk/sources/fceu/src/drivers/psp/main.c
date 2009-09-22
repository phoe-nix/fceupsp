#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <psppower.h>
#include <pspgu.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include <string.h>

#include "../../types.h"
#include "../../driver.h"
#include "../../fceu.h"
#include "pspvideo.h"
#include "pspinput.h"
#include "vram.h"
#include "file_browser.h"

#define SOUND_ENABLED

/* Define the module info section */
PSP_MODULE_INFO("fceu-psp", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Sets new HEAP size for PRX (10MB)
PSP_HEAP_SIZE_KB(10 * 1024);

FCEUGI *CurGame = NULL;
int endgame = 0;
void DoFun();
int SetupCallbacks(void);

void PSPSoundOutput(int32 *tmpsnd, int32 ssize);
int chan;
int32 *tmpsnd_;
int32 ssize_;

void audioCallback(void* buf, unsigned int length, void *userdata) {
//	const float sampleLength = 1.0f / sampleRate;
//	const float scaleFactor = SHRT_MAX - 1.0f;
//	static float freq0 = 440.0f;
//	sample_t* ubuf = (sample_t*) buf;
//
//	int i;
//
//	if (frequency != freq0) {
//		time *= (freq0 / frequency);
//	}
//	for (i = 0; i < length; i++) {
//		short s = (short) (scaleFactor * currentFunction(2.0f * PI * frequency
//				* time));
//		ubuf[i].l = s;
//		ubuf[i].r = s;
//		time += sampleLength;
//	}
//	if (time * frequency > 1.0f) {
//		double d;
//		time = modf(time * frequency, &d) / frequency;
//	}
//	freq0 = frequency;
//	memcpy(buf, sound_data, length);
}

int main(int argc, char *argv[])
{
	SetupCallbacks();

	sceKernelDcacheWritebackAll();

	scePowerSetClockFrequency(333, 333, 166);

    PSPVideoInit();

#ifdef SOUND_ENABLED
    chan = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_AUDIO_SAMPLE_ALIGN(367), PSP_AUDIO_FORMAT_MONO);
//	pspAudioInit();
//	pspAudioSetChannelCallback(0, audioCallback, NULL);

#endif

    if(!(FCEUI_Initialize())) {
		printf("FCEUltra did not initialize.\n");
		return(0);
	}

	FCEUI_SetVidSystem(0); // 0 - NTSC
	FCEUI_SetGameGenie(0);
	FCEUI_DisableSpriteLimitation(1);
	FCEUI_SetSoundVolume(100);
	FCEUI_SetSoundQuality(0);
	FCEUI_SetLowPass(0);
#ifdef SOUND_ENABLED
	FCEUI_Sound(22050);
#else
	FCEUI_Sound(0);
#endif

    FCEUGI *tmp;

    for(;;) {
    	pspDebugScreenInit();

    	if((tmp=FCEUI_LoadGame(file_browser("ms0:/")))) {
    		printf("Game Loaded!\n");
    		CurGame=tmp;
    	}
    	else {
    		printf("Didn't load Game!\n");
    		continue;
    	}

    	PSPInputInitPads();

    	PSPVideoOverrideNESClut();

//    	SceUID new_thid = sceKernelCreateThread("audio_thread", PSPSoundOutput, 0x06, 0x10000, 0, 0);
//    	if(new_thid) {
//    		printf("Starting sound thread...\n");
//    		sceKernelStartThread(new_thid, 0, NULL);
//    	}

    	while(CurGame) {//FCEUI_CloseGame turns this false
    		DoFun();
    	}

    	//sceKernelTerminateDeleteThread(new_thid);
    }

	sceGuTerm();

	sceKernelExitGame();

	return 0;
}

void FCEUD_Update(uint8 *XBuf, int32 *tmpsnd, int32 ssize)
{
	PSPVideoRenderFrame(XBuf);
	PSPInputReadPad();

	if(endgame) {
		FCEUI_CloseGame();
		CurGame=0;
		endgame = 0;
	}

#ifdef SOUND_ENABLED
//	tmpsnd_ = tmpsnd;
//	ssize_ = ssize;
	PSPSoundOutput(tmpsnd, ssize);
//	printf("%d\n", ssize);
#endif

}

void PSPSoundOutput(int32 *tmpsnd, int32 ssize) {
    int i, j = 0;
    s16 ssound[ssize * 2];

//	int i;
	u16 sample;
//	u32 *dst = (u32 *)mixbuffer;

	for( i = 0; i < ssize * 2; i+=2 )
	{
		sample = tmpsnd[j++];
		ssound[i] = sample;
		ssound[i+1] = sample;
	}

    //memset(ssound, 0, 1024);

	//memset(sound_data, 0, 4096);

//    for (i = 0, j = 0; j < ssize_; i += 2, j++) {
//        ssound[i]=tmpsnd[j] & 0xFFFF;
//        ssound[i+1]=tmpsnd[j] & 0xFFFF;
//    }
    //printf("ssize<<1: %d\n", ssize<<1);
    //sceAudioSetChannelDataLen(chan, ssize<<1);
	sceAudioOutputBlocking(chan, PSP_AUDIO_VOLUME_MAX, ssound);
//	printf("%d", ssize);
	//return 0;
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

