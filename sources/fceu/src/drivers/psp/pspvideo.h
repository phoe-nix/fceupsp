#ifndef PSPVIDEO_H_
#define PSPVIDEO_H_

// Function Prototypes
void PSPVideoInit();
void PSPVideoRenderFrame(uint8 *XBuf);
void PSPVideoOverrideNESClut();
void PSPVideoToggleScreen();


#endif /* PSPVIDEO_H_ */
