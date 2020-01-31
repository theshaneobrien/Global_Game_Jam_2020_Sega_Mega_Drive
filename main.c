#include <genesis.h>
#include <resources.h>

//Background MAps
Map *mapBackground;


void init();
void setupPlayField();

static void myJoyHandler(u16 joy, u16 changed, u16 state);

int main()
{
	init();
	while (1)
	{
		//Updates Sprites Position / Animation Frame
		SPR_update();
		//Wait for the frame to finish rendering
		VDP_waitVSync();
		
	}
	return(0);
	
}

void init()
{
	//Input
	JOY_init();
	JOY_setEventHandler(&myJoyHandler);

	//Set up the resolution
	VDP_setScreenWidth320();
	VDP_setPlanSize(64, 32);
	
	//Set the background color
	//Manually sets a pallete colour to a hex code
	//First pallet is the background color
	//SHANE THIS IS USEFUL
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x6dc2ca));

	SPR_init(0, 0, 0);
	setupPlayField();
}

void setupPlayField()
{
	//Set up the map tilesets
	VDP_setPalette(PAL3, devBG.palette->data);
	VDP_loadTileSet(devBG.tileset, 1, DMA);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	mapBackground = unpackMap(devBG.map, NULL);

	VDP_setMapEx(PLAN_A, mapBackground, TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, 1), 0, 0, 0, 0, 63, 28);
	//Set the background color
	//Manually sets a pallete colour to a hex code
	//First pallet is the background color
	//SHANE THIS IS USEFUL
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x6dc2ca));
}


static void myJoyHandler(u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			
		}
		//Set the player velocity if the left or right dpad are pressed;
		//Set velocity to 0 if no direction is pressed
		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if(state & BUTTON_C)
		{

		}

		if (state & BUTTON_RIGHT)
		{
			
		}
		else if (state & BUTTON_LEFT)
		{
			
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT))
			{
				
			}
		}
	}
}
