#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

#include "../../types.h"
#include "../../driver.h"
#include "../../fceu.h"

int NESButtons;
SceCtrlData pad;

void PSPInputInitPads() {
    void *NESPads;
	int attrib = 0;

	FCEUI_DisableFourScore(1);

	NESPads = &NESButtons;
	FCEUI_SetInput(0, SI_GAMEPAD, NESPads, attrib);
	FCEUI_SetInput(1, SI_GAMEPAD, NESPads, attrib);

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

void PSPInputReadPad() {
	static int old_buttons = 0;
	int buttons = 0;
	sceCtrlReadBufferPositive(&pad, 1);

	//if (pad.Buttons != 0 && pad.Buttons != old_buttons){
	if (pad.Buttons != old_buttons){
		if (pad.Buttons & PSP_CTRL_SQUARE){
			buttons |= JOY_B;
			printf("Square pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_TRIANGLE){
			printf("Triangle pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_CIRCLE){
			printf("Cicle pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_CROSS){
			buttons |= JOY_A;
			printf("Cross pressed \n");
		}

		if (pad.Buttons & PSP_CTRL_UP){
			buttons |= JOY_UP;
			printf("Up pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_DOWN){
			buttons |= JOY_DOWN;
			printf("Down pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_LEFT){
			buttons |= JOY_LEFT;
			printf("Left pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_RIGHT){
			buttons |= JOY_RIGHT;
			printf("Right pressed \n");
		}

		if (pad.Buttons & PSP_CTRL_START){
			buttons |= JOY_START;
			printf("Start pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_SELECT){
			buttons |= JOY_SELECT;
			printf("Select pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_LTRIGGER){
			printf("L-trigger pressed \n");
		}
		if (pad.Buttons & PSP_CTRL_RTRIGGER){
			printf("R-trigger pressed \n");
		}

		old_buttons = pad.Buttons;
		NESButtons = buttons;
	}
}
