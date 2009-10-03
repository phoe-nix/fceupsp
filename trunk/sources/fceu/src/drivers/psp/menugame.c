#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>

#include "../../types.h"
#include "../../driver.h"
#include "../../fceu.h"
#include "menugame.h"
#include "sml.h"

int save_state_slot = 0;
extern int endgame;

int select_save() {
	char options[10][2] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	u32 *screen;
	int s;

	screen = sml_savescreen32(38, 13, 40, 17);

	sml_drawbox(38, 13, 40, 17, ' ', ' ', 0xFFFF0000, 0xFFFF0000, 0x00000000, 0x00000000);
	s = sml_menubox(39, 14, 39, 16, &options[0][0], 10, 2, save_state_slot, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF);

	if(s < 0)
		s = save_state_slot;

	sml_restorescreen32(38, 13, 40, 17, screen);

	return s;
}

void menugame() {
	char options[6][18] = {"Save State Slot >",
						   "Save State       ",
						   "Restore State    ",
						   "Reset Game       ",
						   "Exit Game        ",
						   "Close Menu       "};

	u32 *screen;
	int s;

    pspDebugScreenInit();

	pspDebugScreenClear();

	screen = sml_savescreen32(19, 13, 37, 20);

	for(;;) {
		sml_drawbox(19, 13, 37, 20, ' ', ' ', 0xFFFF0000, 0xFFFF0000, 0x00000000, 0x00000000);
		s = sml_menubox(20, 14, 36, 19, &options[0][0], 6, 18, 0, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF);

		if(s == 0) {
			save_state_slot = select_save();
			FCEUI_SelectState(save_state_slot);
		}
		else if(s == 1) {
			FCEUI_SaveState(NULL);
			break;
		}
		else if(s == 2) {
			FCEUI_LoadState(NULL);
			break;
		}
		else if(s == 3) {
			FCEUI_ResetNES();
			break;
		}
		else if(s == 4) {
			endgame = 1;
			break;
		}
		else {
			break;
		}
	}

	sml_restorescreen32(19, 13, 37, 20, screen);
}
