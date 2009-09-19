#include "file_browser.h"

#include <pspctrl.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <pspdisplay.h>
#include <pspkernel.h>

#define PAGE_SIZE 30

// recacheGameDir() by Anonymous tipster
// game directory display
struct GameEntry
{
   char *name;
};

int gameEntriesTotal = 0;
int gameDirSelected = 0;

char current_dir[200] = "ms0:/PSP/GAME/";

#define MAX_ENTRIES 1000

struct GameEntry gameEntry[MAX_ENTRIES];

int is_file( const char *filename)
{
	SceIoStat stats;
	sceIoGetstat( filename, &stats);

	if ( stats.st_mode & FIO_S_IFDIR)
		return 0;
	else
		return 1;
}

void recacheGameDir()
{
	struct SceIoDirent dir;
	memset( &dir, 0, sizeof(SceIoDirent));
	pspDebugScreenSetXY( 0, 6);

	static int dfd;
	dfd = sceIoDopen(current_dir);

	if(dfd > 0)
	{
		int f = 0;
		for( f = 0; f < MAX_ENTRIES; f++)
		{
			if( gameEntry[f].name)
			{
				free( gameEntry[f].name);
			}
			gameEntry[f].name = NULL;
		}
		int count = 0;
		count = 0;

		while( sceIoDread( dfd, &dir) > 0)
		{
			char full_name[200];
			static char * name;
			name = (char*)memalign( 16, 300);
			sprintf( name,"%s" , dir.d_name);
			static int s = 0;
			s = strlen(name);

			sprintf(full_name, "%s%s", current_dir, name);
			printf("%s\n", full_name);

			if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
				if(is_file(full_name) && (strstr(strupr(name), strupr(".nes")) == NULL))
					continue;
			}

			gameEntry[count].name = name;
			count++;

			if(count > MAX_ENTRIES - 1)
			{
				count = MAX_ENTRIES - 1;
			}

			gameEntriesTotal = count;
			if(gameEntriesTotal < 0)
			{
				gameEntriesTotal = 0;
			}
		}
	}
	sceIoDclose(dfd);
}

char* file_browser( const char* initial_dir)
{
	char* selection = NULL;

	pspDebugScreenClear();
	int browsing = 1;
	int current_selection = 0;

	int current_page = 0;
	int last_page = 0;

	SceCtrlData current_pad;
	SceCtrlData last_pad;

	sceCtrlPeekBufferPositive(&last_pad, 1);

	if( initial_dir != NULL)
	{
		strcpy(current_dir, initial_dir);
	}

	recacheGameDir();

	while(browsing)
	{
		pspDebugScreenSetXY(0,0);

		int min = 0;
	    int i = 0;

	    current_page = current_selection / PAGE_SIZE;

	    if( current_page != last_page)
	    {
		    pspDebugScreenClear();
	    }

		min = PAGE_SIZE * current_page;;

		pspDebugScreenPrintf("FCEUltra for PSP by bootsector (Alpha)\n");
		pspDebugScreenPrintf("Current Directory: %s\nPage Number: %d\n\n", current_dir, current_page);
	    for( i = min; ((i < gameEntriesTotal) && (i < (min + PAGE_SIZE))); i++)
	    {
		    if( i != current_selection)
		    {
		    	pspDebugScreenPrintf("%d: %s\n", i, gameEntry[i].name);
	    	}
	    	else
	    	{
		    	pspDebugScreenPrintf("%d: %s <-\n", i, gameEntry[i].name);
	    	}
	    }
	    last_page = current_page;

	    sceCtrlPeekBufferPositive(&current_pad, 1);

	    if(last_pad.Buttons != current_pad.Buttons)
	    {
		    last_pad = current_pad;

		    if( (current_pad.Buttons & PSP_CTRL_DOWN) && (current_pad.Buttons & PSP_CTRL_SQUARE))
		    {
			    if( gameEntriesTotal > PAGE_SIZE)
			    {
			    	current_selection += PAGE_SIZE;

			    	if( current_selection > gameEntriesTotal)
			    	{
					    current_selection = gameEntriesTotal-1;
			    	}
		    	}
		    }
		    else if( current_pad.Buttons & PSP_CTRL_DOWN)
		    {
			    current_selection++;
			    if( current_selection >= gameEntriesTotal)
			    {
				    current_selection = 0;
			    }
		    }

		    if( (current_pad.Buttons & PSP_CTRL_UP) && (current_pad.Buttons & PSP_CTRL_SQUARE))
		    {
			    if( gameEntriesTotal > PAGE_SIZE)
			    {
			    	current_selection -= PAGE_SIZE;

			    	if( current_selection < 0)
			    	{
				    	current_selection = 0;
			    	}
		    	}
		    }
		    else if( current_pad.Buttons & PSP_CTRL_UP)
		    {
			    current_selection--;
			    if( current_selection < 0)
			    {
				    current_selection = gameEntriesTotal - 1;
			    }
		    }

		    if( current_pad.Buttons & PSP_CTRL_CIRCLE)
		    {
			    char temp[200];

			    int i = 0;
		    	int max = strlen( current_dir);
		    	int dir_no = 0;
		    	while( (i < max))
		    	{
			    	if( current_dir[i] == '/')
			    	{
				    	dir_no++;
			    	}
			    	i++;
		    	}
		    	if( dir_no > 1)
		    	{
			    	char * temp2 = strtok( current_dir, "/");
			    	strcpy( temp, temp2);
				    strcat( temp, "/");
			    	i = 2;
			    	while( (i < dir_no))
			    	{
				    	temp2 = strtok( NULL, "/");
				    	strcat( temp, temp2);
				    	strcat( temp, "/");
				    	i++;
			    	}
			    	strcpy( current_dir, temp);
		    	}
		    	pspDebugScreenClear();
		    	recacheGameDir();
			   	current_selection = 0;

		    }

		    if( current_pad.Buttons & PSP_CTRL_CROSS)
		    {
			    char temp[200];
			    strcpy( temp, current_dir);
			    if( strcmp( ".", gameEntry[current_selection].name) != 0 && strcmp( "..", gameEntry[current_selection].name) != 0)
			    {
			    	strcat( temp, gameEntry[current_selection].name);
			    	if( is_file(temp))
			    	{
					    selection = (char*)malloc( sizeof(temp));
					    strcpy( selection, temp);
					    browsing = 0;
			   		}
			    	else
			    	{
				   		current_selection = 0;
				    	strcpy( current_dir, temp);
				    	strcat( current_dir, "/");
				    	pspDebugScreenClear();
				    	recacheGameDir();
				    	current_selection = 0;
			    	}
		    	}
		    	else if( strcmp( ".", gameEntry[current_selection].name) == 0)
		    	{
			    	sprintf( current_dir, "%s", "ms0:/");
			    	pspDebugScreenClear();
			    	recacheGameDir();
			    	current_selection = 0;
		    	}
		    	else if( strcmp( "..", gameEntry[current_selection].name) == 0)
		    	{
			    	int i = 0;
			    	int max = strlen( current_dir);
			    	int dir_no = 0;
			    	while( (i < max))
			    	{
				    	if( current_dir[i] == '/')
				    	{
					    	dir_no++;
				    	}
				    	i++;
			    	}
			    	if( dir_no > 1)
			    	{
				    	char * temp2 = strtok( current_dir, "/");
				    	strcpy( temp, temp2);
					    strcat( temp, "/");
				    	i = 2;
				    	while( (i < dir_no))
				    	{
					    	temp2 = strtok( NULL, "/");
					    	strcat( temp, temp2);
					    	strcat( temp, "/");
					    	i++;
				    	}
				    	strcpy( current_dir, temp);
			    	}
			    	pspDebugScreenClear();
			    	recacheGameDir();
			    	current_selection = 0;
		    	}
		    }
	    }
	    sceDisplayWaitVblankStart();
    }
    pspDebugScreenClear();

    return selection;
}
