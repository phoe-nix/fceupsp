/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * sml.h - Simple Menu Library
 *
 * Copyright (c) 2009 bootsector@ig.com.br
 */

#ifndef SML_H_
#define SML_H_

/* Function Prototypes */
u32 *sml_savescreen32(int x1, int y1, int x2, int y2);
void sml_restorescreen32(int x1, int y1, int x2, int y2, u32 *save_buffer);
void sml_drawbox(int x1, int y1, int x2, int y2, char border_char, char fill_char, u32 border_text_color, u32 border_back_color, u32 fill_text_color, u32 fill_back_color);
int sml_menubox(int x1, int y1, int x2, int y2, char *options, int option_count, int option_max_width, int initial_selected_item, u32 normal_item_text_color, u32 normal_item_back_color, u32 selected_item_text_color, u32 selected_item_back_color);
int sml_confirmationbox(char *message);
void sml_messagebox(char *message);


#endif /* SML_H_ */
