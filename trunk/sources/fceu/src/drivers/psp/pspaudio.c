#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <pspaudiolib.h>
#include <pspaudio.h>
#include <string.h>

#include "../../types.h"
#include "../../driver.h"
#include "../../fceu.h"

#include "pspaudio.h"
#include "sfifo.h"

#define BUF_LEN 10240
#define CHUNK_LEN 512

sfifo_t sound_fifo;
int chan = -1;

int can_play = 0;
SceUID new_thid;

void PSPAudioAddSamples(u16 *samples, int32 count) {
	u8 *s = (u8 *)samples;
	sfifo_write(&sound_fifo, s, count<<1);
}

void PSPAudioGetSamples(u16 *samples, int32 count) {
	u8 *s = (u8 *)samples;

	memset(samples, 0, sizeof(u16) * count);

	sfifo_read(&sound_fifo, s, count<<1);
}


void PSPAudioPlayThread() {
	u16 s[CHUNK_LEN];

	for(;;) {
		if(can_play) {
			PSPAudioGetSamples(s, CHUNK_LEN);

			sceAudioOutputBlocking(chan, PSP_AUDIO_VOLUME_MAX, s);
			//printf("Available samples: %d\n", PSPAudioGetAvailableSamples());
		} else {
			sceKernelDelayThread(0);
		}
	}
}

void PSPAudioStop() {
	can_play = 0;
}

void PSPAudioPlay() {
	can_play = 1;
}

void PSPAudioReset() {
	sfifo_flush(&sound_fifo);
}

void PSPAudioInit() {
	sfifo_init(&sound_fifo, BUF_LEN * 2);

	chan = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_AUDIO_SAMPLE_ALIGN(CHUNK_LEN), PSP_AUDIO_FORMAT_MONO);

	can_play = 0;

	new_thid = sceKernelCreateThread("audio_thread",(SceKernelThreadEntry) PSPAudioPlayThread, 0x12, 0x10000, 0, 0);

	if(new_thid) {
		printf("Starting audio thread\n");
		sceKernelStartThread(new_thid, 0, NULL);
	} else {
		printf("Failed creating audio thread\n");
	}
}

void PSPAudioFinish() {
	PSPAudioStop();

	sceKernelTerminateThread(new_thid);
	sceKernelWaitThreadEnd(new_thid,NULL);
	sceKernelDeleteThread(new_thid);
	sceAudioChRelease(chan);
	sfifo_close(&sound_fifo);
}
