#include <genesis.h>
#include <resources.h>

//Background MAps
Map *mapBackground;

//Player
struct Player{
	Sprite *playerSprite;
	fix16 posX;
	fix16 velX;
	fix16 posY;
	fix16 VelY;
};

struct Player player1;
struct Player player2;

int playerWidth = 32;
int playerHeight = 38;
bool jumping = FALSE;

//Controller Vectors
int p1Horizontal = 0;
int p1Vertical = 0;
int p2Horizontal = 0;
int p2Vertical = 0;

void init();
void setupPlayField();
void setupPlayers();

void gravity();

//Button Functions
int p1PressedA();
int p1PressedB();
int p1PressedC();
int p1PressedStart();
int p2PressedA();
int p2PressedB();
int p2PressedC();
int p2PressedStart();

static void myJoyHandler(u16 joy, u16 changed, u16 state);

int main()
{
	init();
	while (1)
	{
		//Updates Sprites Position / Animation Frame
		SPR_update();
		gravity();
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
	setupPlayers();
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
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x00000));
}

void setupPlayers()
{
	VDP_setPalette(PAL2, player1Sprite.palette->data);
	player1.playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(player1.posX), player1.posY, TILE_ATTR(PAL2, 0, FALSE, FALSE));
	SPR_update();
}

void gravity()
{
	//Apply Velocity, need to use fix16Add to add two "floats" together
	player1.posY = fix16Add(player1.posY, player1.VelY);

	//Apply gravity
	if (jumping == TRUE)
	{
		
	}

	//Check if player is on floor
	if (fix16ToInt(player1.posY) + playerHeight >= 300)
	{
		jumping = FALSE;
	 	player1.VelY = FIX16(0);
		player1.posY = intToFix16(testGroundCollision(0) - playerHeight);
	 }
	 else
	 {
		 player1.VelY  = fix16Add(player1.VelY, 6);
	 }
}


//Input Stuff
int p1PressedA()
{
	return(0);
}

int p1PressedB()
{
	return(0);
}

int p1PressedC()
{
	return(0);
}

int p1PressedStart()
{
	return(0);
}

int p2PressedA()
{
	return(0);
}

int p2PressedB()
{
	return(0);
}

int p2PressedC()
{
	return(0);
}

int p2PressedStart()
{
	return(0);
}

static void myJoyHandler(u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			p1PressedC();
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if(state & BUTTON_A)
		{
			p1PressedC();
		}

		if(state & BUTTON_B)
		{
			p1PressedC();
		}
		
		if(state & BUTTON_C)
		{
			p1PressedC();
		}

		if (state & BUTTON_RIGHT)
		{
			p1Horizontal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				p1Horizontal = 0;
			}
		}
		
		if (state & BUTTON_LEFT)
		{
			p1Horizontal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				p1Horizontal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			p1Vertical = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				p1Vertical = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			p1Vertical = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				p1Vertical = 0;
			}
		}
	}

	if (joy == JOY_2)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			p2PressedStart();
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if(state & BUTTON_A)
		{
			p2PressedA();
		}

		if(state & BUTTON_B)
		{
			p2PressedB();
		}
		
		if(state & BUTTON_C)
		{
			p2PressedC();
		}

		if (state & BUTTON_RIGHT)
		{
			p2Horizontal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				p2Horizontal = 0;
			}
		}
		
		if (state & BUTTON_LEFT)
		{
			p2Horizontal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				p2Horizontal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			p2Vertical = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				p2Vertical = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			p2Vertical = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				p2Vertical = 0;
			}
		}
	}
}
