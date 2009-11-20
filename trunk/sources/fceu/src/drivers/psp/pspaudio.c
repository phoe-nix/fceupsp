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

#define BUF_LEN 10240
#define CHUNK_LEN 512

u16 audio_buffer[BUF_LEN];

int start = 0;
int end = 0;
int chan = -1;
int added_some_data = 0;

//SceUID can_play;
int can_play = 0;
SceUID new_thid;


int PSPAudioGetAvailableSamples() {
//	if(start == end)
//		return 0;

	return (BUF_LEN + end - start) % BUF_LEN;
}

void PSPAudioAddSamples(u16 *samples, int32 count) {
	int i, real_i;
	int overwritten = 0;

	for(i = 0; i < count; i++) {
		real_i = (i + end) % BUF_LEN;
		audio_buffer[real_i] = samples[i];
		if(added_some_data && (real_i == start)) {
			//start = (real_i + 1) % BUF_LEN;
			overwritten = 1;
			printf("Buffer overwritten\n");
		}

	}

	end = (count + end) % BUF_LEN;
	if(overwritten)
		start = (end + 1) % BUF_LEN;

	added_some_data = 1;
}

void PSPAudioGetSamples(u16 *samples, int32 count) {
	int i;
	u16 last_sample = 0;
	int samples_available;
	memset(samples, 0, sizeof(u16) * count);

	if((samples_available = PSPAudioGetAvailableSamples()) < count)
		//                return;
		printf("No samples available. Req.: %d Avail.: %d\n", count, samples_available);

	for(i = 0; i < count; i++) {
		if(i < samples_available) {
			samples[i] = audio_buffer[(i + start) % BUF_LEN];
			last_sample = 0;
		} else {
			samples[i] = last_sample;
		}
	}

	start = (count + start) % BUF_LEN;
}


void PSPAudioPlayThread() {
	u16 s[CHUNK_LEN];

	sceKernelWaitSema(can_play, 1, 0);

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
	//sceKernelSignalSema(can_play, -1);
	can_play = 0;
}

void PSPAudioPlay() {
	//sceKernelSignalSema(can_play, 1);
//	sceKernelWakeupThread(new_thid);
	can_play = 1;
}

void PSPAudioReset() {
	memset(audio_buffer, 0, sizeof(u16) * BUF_LEN);
	start = 0;
	end = 0;
	added_some_data = 0;
}

void PSPAudioInit() {
	memset(audio_buffer, 0, sizeof(u16) * BUF_LEN);

	chan = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_AUDIO_SAMPLE_ALIGN(CHUNK_LEN), PSP_AUDIO_FORMAT_MONO);

	//can_play = sceKernelCreateSema("can_play", 0, 0, 1, 0);
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
	//sceKernelDeleteSema(can_play);
	sceAudioChRelease(chan);
}
