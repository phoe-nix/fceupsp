#ifndef PSPAUDIO_H_
#define PSPAUDIO_H_

/* Function Prototypes */
void PSPAudioAddSamples(u16 *samples, int32 count);
void PSPAudioGetSamples(u16 *samples, int32 count);
void PSPAudioPlayThread();
void PSPAudioStop();
void PSPAudioPlay();
void PSPAudioInit();
void PSPAudioFinish();
void PSPAudioReset();
int PSPAudioGetAvailableSamples();

#endif /* PSPAUDIO_H_ */
