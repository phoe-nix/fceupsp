/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * sml.c - Simple Menu Library
 *
 * Copyright (c) 2009 bootsector@ig.com.br
 */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspge.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

u32 *sml_savescreen32(int x1, int y1, int x2, int y2) {
	int x, y, i, j;
	u32 color = 0xFFFFFFFF;

	u32* save_buffer = malloc(sizeof(u32) * (x2 - x1 + 1) * (y2 - y1 + 1) * 8 * 8);

	if(!save_buffer) {
		printf("Error allocating buffer\n");
		return NULL;
	}

	u32 *base_vram = (u32 *) (0x40000000 | (u32) sceGeEdramGetAddr());
	u32 *vram = base_vram;

	for(x = x1; x <= x2; x++) {
		for(y = y1; y <= y2; y++) {
			vram = (base_vram + x * 8) + (512 * y * 8);

			// 8x8 square
			for(i = 0; i < 8; i++) {
				for(j = 0; j < 8; j++) {
					*save_buffer++ = *(vram + j);
					*(vram + j) = color;
				}

				vram += 512; // PSP_LINE_SIZE
			}
		}
	}

	return save_buffer;
}

void sml_restorescreen32(int x1, int y1, int x2, int y2, u32 *save_buffer) {
	int x, y, i, j;

	if(!save_buffer) {
		return;
	}

	u32 *base_vram = (u32 *) (0x40000000 | (u32) sceGeEdramGetAddr());
	u32 *vram = base_vram;

	for(x = x1; x <= x2; x++) {
		for(y = y1; y <= y2; y++) {
			vram = (base_vram + x * 8) + (512 * y * 8);

			// 8x8 square
			for(i = 0; i < 8; i++) {
				for(j = 0; j < 8; j++) {
					*(vram + j) = *save_buffer++;
				}

				vram += 512; // PSP_LINE_SIZE
			}
		}
	}

	free(save_buffer);
}

void sml_drawbox(int x1, int y1, int x2, int y2, char border_char, char fill_char, u32 border_text_color, u32 border_back_color, u32 fill_text_color, u32 fill_back_color) {
	int x, y;

	for(x = x1; x <= x2; x++) {
		for(y = y1; y <= y2; y++) {

			pspDebugScreenSetXY(x, y);

			/* Borders */
			if(x == x1 || x == x2 || y == y1 || y == y2) {

				pspDebugScreenSetBackColor(border_back_color);
				pspDebugScreenSetTextColor(border_text_color);

				pspDebugScreenPrintf("%c", border_char);
			} else {
				/* Fill */
				pspDebugScreenSetBackColor(fill_back_color);
				pspDebugScreenSetTextColor(fill_text_color);
				pspDebugScreenPrintf("%c", fill_char);
			}
		}
	}

	/* Set default colors back */
	pspDebugScreenSetBackColor(0x00000000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
}

int sml_menubox(int x1, int y1, int x2, int y2, char *options, int option_count, int option_max_width, int initial_selected_item, u32 normal_item_text_color, u32 normal_item_back_color, u32 selected_item_text_color, u32 selected_item_back_color) {
	/* Used to format a printf mask so we can print option_max_width chars from options*/
	char print_mask[20];

	/* Pad reading variables*/
	SceCtrlData pad;
	int old_pad = 0;
	int pad_data = 0;
	long rpt_time = 0;

	/* How many options we can print (lines) inside the menu */
	int how_many_items_can_show = y2 - y1 + 1;

	/* Both below are moved according to the user navigations through the options */
	int first_viewable_item = initial_selected_item;
	int last_viewable_item = first_viewable_item + how_many_items_can_show - 1;

	/* Currently selected item */
	int curr_item = initial_selected_item;

	/* Construct the printf mask for delimited options output */
	int max_item_width = x2 - x1 + 1;
	sprintf(print_mask, "%%.%ds", max_item_width);

	/* Menu navigation/printing variables */
	int y;
	int item;

	/* Return value (the option index or -1 if cancelled by the user */
	int retval = -1;

	/* Menu navigation main loop */
	for(;;) {
		/* Clear region */
		sml_drawbox(x1, y1, x2, y2, ' ', ' ', normal_item_text_color, normal_item_back_color, normal_item_text_color, normal_item_back_color);

		/* Show currently viewable items */
		y = y1;

		for(item = first_viewable_item; item <= ((last_viewable_item >= option_count)?option_count-1:last_viewable_item); item++) {
			/* "Line feed" */
			pspDebugScreenSetXY(x1, y);

			/* Highlights the selected item. Doesn't highlight the other ones */
			if(item == curr_item) {
				pspDebugScreenSetBackColor(selected_item_back_color);
				pspDebugScreenSetTextColor(selected_item_text_color);
			} else {
				pspDebugScreenSetBackColor(normal_item_back_color);
				pspDebugScreenSetTextColor(normal_item_text_color);
			}

			/* Print the menu option */
			pspDebugScreenPrintf(print_mask, (options + item * option_max_width));

			y++;
		}

		/* Checks for user selection. Created this label (Ugh!) to avoid menu flickering */
read_pad:

		sceCtrlReadBufferPositive(&pad, 1);
		pad_data = pad.Buttons;

		/* Didn't read anything. Retry it */
		if(pad_data == 0) {
			old_pad = 0;
			goto read_pad;
		}

		if(clock() > rpt_time || old_pad != pad_data) {

			if(old_pad != pad_data) {
				rpt_time = clock() + 400 * 1000; // 400ms
				old_pad = pad_data;
			} else {
				rpt_time = clock() + 40 * 1000; // 40ms
			}

			if (pad_data & PSP_CTRL_UP) {
				if(curr_item > 0) {
					curr_item--;
					if(curr_item < first_viewable_item || curr_item > last_viewable_item) {
						first_viewable_item--;
						last_viewable_item = first_viewable_item + how_many_items_can_show - 1;
					}
				}
//				else {
//					curr_item = option_count - 1;
//					first_viewable_item = option_count - 1;
//					last_viewable_item = option_count - 1;
//				}
			}

			if (pad_data & PSP_CTRL_DOWN) {
				if(curr_item < option_count-1) {
					curr_item++;
					if(curr_item < first_viewable_item || curr_item > last_viewable_item) {
						first_viewable_item++;
						last_viewable_item = first_viewable_item + how_many_items_can_show - 1;
					}
				}
//				else {
//					curr_item = 0;
//					first_viewable_item = 0;
//					last_viewable_item = first_viewable_item + how_many_items_can_show - 1;
//				}
			}

			if (pad_data & PSP_CTRL_CROSS) {
				retval = curr_item;
				break;
			}

			if (pad_data & PSP_CTRL_CIRCLE) {
				break;
			}

		}

	}

	// Set default colors back
	pspDebugScreenSetBackColor(0x00000000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);

	return retval;
}

int sml_confirmationbox(char *message) {
	int message_x = (68 - strlen(message)) / 2;
	int retval = -1;
	char options[2][4] = {"No ", "Yes"};

	u32 *buffer = sml_savescreen32(message_x - 1, 14, message_x + strlen(message), 18);

	if(!buffer) {
		return -1;
	}

	// Draw external border
	sml_drawbox(message_x - 1, 14, message_x + strlen(message), 18, ' ', ' ', 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000);

	// Prints the message
	pspDebugScreenSetXY(message_x, 15);
	pspDebugScreenPrintf("%s", message);

	// Draw menu
	retval = sml_menubox(32, 16, 34, 17, &options[0][0], 2, 4, 0, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF);

	sml_restorescreen32(message_x - 1, 14, message_x + strlen(message), 18, buffer);

	return retval;
}

void sml_messagebox(char *message) {
	int message_x = (68 - strlen(message)) / 2;
	char options[1][3] = {"OK"};

	u32 *buffer = sml_savescreen32(message_x - 1, 14, message_x + strlen(message), 17);

	if(!buffer) {
		return;
	}

	// Draw external border
	sml_drawbox(message_x - 1, 14, message_x + strlen(message), 17, ' ', ' ', 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000);

	// Prints the message
	pspDebugScreenSetXY(message_x, 15);
	pspDebugScreenPrintf("%s", message);

	// Draw menu
	sml_menubox(32, 16, 33, 16, &options[0][0], 1, 3, 0, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF);

	sml_restorescreen32(message_x - 1, 14, message_x + strlen(message), 17, buffer);
}
