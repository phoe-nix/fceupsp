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

#define BUF_LEN 4096
#define CHUNK_LEN 734

u16 audio_buffer[BUF_LEN];

u64 read_count = 0;
u64 write_count = 0;
u16 *start = audio_buffer;
u16 *end = audio_buffer;
int chan = -1;

SceUID can_play;
SceUID new_thid;


int PSPAudioGetAvailableSamples() {
//	if(start == end)
//		return 0;
//
//	return (BUF_LEN + end - start) % BUF_LEN;
	u64 s = write_count - read_count;
	if(s < 0)
		return s * -1;
	else
		return s;
}

void PSPAudioAddSamples(int32 *samples, int32 count) {
	int i;

	for(i = 0; i < count; i++) {
		audio_buffer[(i + end - audio_buffer) % BUF_LEN] = (u16)samples[i];
	}

	end = audio_buffer + ((count + end - audio_buffer) % BUF_LEN);

	write_count += count;

	// Shift the start buffer to the right if we're inserting too much data.
//	if(count >= BUF_LEN) {
//		start = audio_buffer + ((end - audio_buffer + 1) % BUF_LEN);
//	}
}

void PSPAudioGetSamples(u16 *samples, int32 count) {
	int i;
	u16 last_sample = 0;
	u64 samples_available = PSPAudioGetAvailableSamples();

	for(i = 0; i < count; i++) {
		if(i < samples_available) {
			samples[i] = audio_buffer[(i + start - audio_buffer) % BUF_LEN];
			last_sample = samples[i];
		} else {
			samples[i] = last_sample;
		}
	}

	start = audio_buffer + ((count + start - audio_buffer) % BUF_LEN);

	read_count += count;
}

void PSPAudioPlayThread() {
	u16 s[CHUNK_LEN];

	sceKernelWaitSema(can_play, 1, 0);

	for(;;) {
//		if(sceKernelPollSema(can_play, 1) < 0)
//			continue;

		PSPAudioGetSamples(s, CHUNK_LEN);

		sceAudioOutputBlocking(chan, PSP_AUDIO_VOLUME_MAX, s);
		printf("Available samples: %d\n", PSPAudioGetAvailableSamples());
	}
}

void PSPAudioStop() {
	sceKernelSignalSema(can_play, 0);
}

void PSPAudioPlay() {
	sceKernelSignalSema(can_play, 1);
}


void PSPAudioInit() {
	memset(audio_buffer, 0, sizeof(u16) * BUF_LEN);

	chan = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_AUDIO_SAMPLE_ALIGN(CHUNK_LEN), PSP_AUDIO_FORMAT_MONO);

	can_play = sceKernelCreateSema("can_play", 0, 0, 1, 0);

	new_thid = sceKernelCreateThread("audio_thread", PSPAudioPlayThread, 0x12, 0x10000, 0, 0);

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
	sceKernelDeleteSema(can_play);
	sceAudioChRelease(chan);
}
