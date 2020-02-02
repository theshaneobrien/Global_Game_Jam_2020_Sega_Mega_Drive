#define START_BUTTON 1
#define A_BUTTON 2
#define B_BUTTON 3
#define C_BUTTON 4

#define PLAYER_1 0
#define PLAYER_2 1

#include <genesis.h>
#include <resources.h>

//Screen
int desiredScreenWidth = 320;
int desiredScreenHeight = 224;

//Background MAps
Map *mapBackground;
Map *cloudBackground;

//Projectile vars
struct Projectile
{
	bool inPlay;
	Sprite *projectileSprite;
	fix16 posX;
	fix16 posY;
	int projectileSpeed;
	int projectileOwner;
	int direction;
};
int projectileHitSize = 8;
int projectileSpawnXOffset = 32;
int projectileSpawnYOffset = 32;
int projectileStartSpeed = 10;
struct Projectile projectiles[2];
int inPlayRaquetBalls = 0;

//Shield vars
struct Shield
{
	Sprite *shieldSprite;
	bool shieldActive;
	int shieldOwner;
	int posX;
	int posY;
};
int p1ShieldFrameCount = 0;
int p2ShieldFrameCount = 0;
int shieldFrameTime = 5;
int shieldOffset = 40;
int shieldHeight = 64;
int shieldWidth = 32;
struct Shield shields[2];

//Player
struct Player
{
	Sprite *playerSprite;
	fix16 posX;
	fix16 velX;
	fix16 posY;
	fix16 velY;
	int horizontalNormal;
	int verticalNormal;
	bool jumping;
	bool isMoving;
	int moveConstraintXLeft;
	int moveConstraintXRight;
	struct Shield playerShield;
};
struct Player players[2];

const int playerWidth = 64;
const int playerHeight = 64;
const int jumpForce = -3;
const fix16 jumpDistance = FIX16(0.75);

const int groundHeight = 200;

int scrollSpeed = 1;
int scrollAmount;
int frameCount = 0;

void init();
void setupPlayField();
void setupPlayers();
void gravity();
void playerJumping();
void playerWalking();
void setPlayerPosition();
void player1PosClamp();
void player2PosClamp();
//ShieldTimers
void p1ShieldTimer();
void p2ShieldTimer();
//ProjectileUpdates
int fireProjectile(int playerNum);
void projectileMovement();
void killProjectile(int projNum);
//Collision
void checkProjShieldCollision();

int countFrames();
void ScrollBackground();
void setupMusic();

//Button Functions
int buttonPressEvent(int playerNum, int button);

//Debug
Sprite *debug1;
Sprite *debug2;
Sprite *debug3;
Sprite *debug4;
void updateDebug();

static void myJoyHandler(u16 joy, u16 changed, u16 state);

int main()
{
	init();
	while (1)
	{
		countFrames();
		ScrollBackground();
		//Updates Sprites Position / Animation Frame
		SPR_update();
		gravity();
		setPlayerPosition();
		p1ShieldTimer();
		p2ShieldTimer();
		projectileMovement();
		checkProjShieldCollision();
		//Wait for the frame to finish rendering
		VDP_waitVSync();
		updateDebug();
	}
	return (0);
}

void init()
{
	//Input
	JOY_init();
	JOY_setEventHandler(&myJoyHandler);

	//Set up the resolution
	VDP_setScreenWidth320();
	VDP_setPlanSize(64, 32);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

	SPR_init(0, 0, 0);
	setupPlayField();
	setupPlayers();
	setupMusic();
}

void setupMusic()
{
	XGM_setLoopNumber(-1);
	XGM_startPlay(&music);
}

void setupPlayField()
{
	//Set up the map tilesets
	VDP_setPalette(PAL3, BGBuildings.palette->data);

	int currentIndex = TILE_USERINDEX;
	VDP_loadTileSet(BGBuildings.tileset, currentIndex, DMA);
	currentIndex += BGBuildings.tileset->numTile;
	VDP_loadTileSet(BGClouds.tileset, currentIndex, DMA);

	mapBackground = unpackMap(BGBuildings.map, NULL);
	//Background art is using Palette 3
	VDP_setMapEx(PLAN_A, mapBackground, TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, TILE_USERINDEX), 0, 0, 0, 0, 64, 28);

	cloudBackground = unpackMap(BGClouds.map, NULL);
	VDP_setMapEx(PLAN_B, cloudBackground, TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, currentIndex), 0, 0, 0, 0, 64, 28);
	//Set the background color
	//Manually sets a pallete colour to a hex code
	//First pallet is the background color
	//SHANE THIS IS USEFUL
	VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x00000));
}

void setupPlayers()
{
	//Sprites are using Palette 1
	VDP_setPalette(PAL1, player1Sprite.palette->data);

	players[0] = players[0];
	players[1] = players[1];

	//Set the players intial position and constraints
	players[0].posX = intToFix16(0);
	players[0].posY = intToFix16(groundHeight - playerHeight);
	players[0].moveConstraintXLeft = 0;
	players[0].moveConstraintXRight = (screenWidth / 2) - playerWidth;
	//Shield
	players[0].playerShield.shieldOwner = PLAYER_1;
	players[0].playerShield.posX = fix16ToInt(players[0].posX) + shieldOffset;
	players[0].playerShield.posY = players[0].posY;
	players[0].playerShield.shieldSprite = SPR_addSprite(&shieldSprite, players[0].playerShield.posX, players[0].playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(players[0].playerShield.shieldSprite, HIDDEN);
	

	//Set the players intial position and constraints
	players[1].posX = intToFix16(256);
	players[1].posY = intToFix16(groundHeight - playerHeight);
	players[1].moveConstraintXLeft = screenWidth / 2;
	players[1].moveConstraintXRight = 256;
	//Shield
	players[1].playerShield.shieldOwner = PLAYER_2;
	players[1].playerShield.posX = fix16ToInt(players[1].posX) - 0;
	players[1].playerShield.posY = players[1].posY;
	players[1].playerShield.shieldSprite = SPR_addSprite(&shieldSprite, players[1].playerShield.posX, players[1].playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_setVisibility(players[1].playerShield.shieldSprite, HIDDEN);

	//Insert the player sprites at the above positions
	players[0].playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(players[0].posX), fix16ToInt(players[0].posY), TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setAnim(players[0].playerSprite, 1);
	players[1].playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(players[1].posX), fix16ToInt(players[1].posY), TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_update();

	//set up projectiles
	projectiles[0].projectileOwner = PLAYER_1;
	projectiles[0].direction = 1;
	projectiles[0].inPlay = FALSE;
	projectiles[0].projectileSpeed = 0;
	projectiles[0].projectileSprite = SPR_addSprite(&projectileSprite, fix16ToInt(players[0].posX) + projectileSpawnXOffset, projectiles[0].posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(projectiles[0].projectileSprite, HIDDEN);

	projectiles[1].projectileOwner = PLAYER_2;
	projectiles[1].direction = 1;
	projectiles[1].inPlay = FALSE;
	projectiles[1].projectileSpeed = 0;
	projectiles[1].projectileSprite = SPR_addSprite(&projectileSprite, fix16ToInt(players[1].posX) + projectileSpawnXOffset, projectiles[1].posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(projectiles[1].projectileSprite, HIDDEN);

	//set up shields
	shields[0] = players[0].playerShield;
	shields[1] = players[1].playerShield;

	//debug
	debug1 = SPR_addSprite(&debug, players[1].playerShield.posX + shieldWidth, players[1].playerShield.posY + shieldHeight, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	debug2 = SPR_addSprite(&debug, players[1].playerShield.posX, players[1].playerShield.posY + shieldHeight, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	debug3 = SPR_addSprite(&debug, players[1].playerShield.posX + shieldWidth, players[1].playerShield.posY + shieldHeight, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	debug4 = SPR_addSprite(&debug, players[1].playerShield.posX, players[1].playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
}

int countFrames()
{
	frameCount++;
	if (frameCount > 60)
	{
		frameCount = 0;
	}
	return frameCount;
}

void gravity()
{
	//Apply Velocity, need to use fix16Add to add two "floats" together
	players[0].posY = fix16Add(players[0].posY, players[0].velY);
	players[0].posX = fix16Add(players[0].posX, players[0].velX);

	players[1].posY = fix16Add(players[1].posY, players[1].velY);
	players[1].posX = fix16Add(players[1].posX, players[1].velX);

	//Check if player is on floor
	if (fix16ToInt(players[0].posY) + playerHeight >= groundHeight)
	{
		players[0].jumping = FALSE;
		players[0].velY = intToFix16(0);
		players[0].posY = intToFix16(groundHeight - playerHeight);
		players[0].velX = intToFix16(0);
	}
	else
	{
		players[0].velY = fix16Add(players[0].velY, 6);
		player1PosClamp();
	}

	if (fix16ToInt(players[1].posY) + playerHeight >= groundHeight)
	{
		players[1].jumping = FALSE;
		players[1].velY = intToFix16(0);
		players[1].posY = intToFix16(groundHeight - playerHeight);
		players[1].velX = intToFix16(0);
	}
	else
	{
		players[1].velY = fix16Add(players[1].velY, 6);
		player2PosClamp();
	}
}

void playerJumping(int player, int direction)
{
	int jumpDirection = direction;
	if (player == PLAYER_1 && players[0].jumping != TRUE)
	{
		players[0].jumping = TRUE;
		players[0].velY = intToFix16(jumpForce);
		players[0].velX = fix16Mul(intToFix16(direction), jumpDistance);
	}

	if (player == PLAYER_2 && players[1].jumping != TRUE)
	{
		players[1].jumping = TRUE;
		players[1].velY = intToFix16(jumpForce);
		players[1].velX = fix16Mul(intToFix16(direction), jumpDistance);
	}
}

void playerWalking()
{

	if (players[0].horizontalNormal == 1)
	{
		if (players[0].posX < intToFix16(players[0].moveConstraintXRight))
		{
			players[0].posX += intToFix16(playerWidth / 2);
		}
	}
	else if (players[0].horizontalNormal == -1)
	{
		if (players[0].posX > intToFix16(players[0].moveConstraintXLeft))
		{
			players[0].posX -= intToFix16(playerWidth / 2);
		}
	}

	if (players[1].horizontalNormal == 1)
	{
		if (players[1].posX < intToFix16(players[1].moveConstraintXRight))
		{
			players[1].posX += intToFix16(playerWidth / 2);
		}
	}
	else if (players[1].horizontalNormal == -1)
	{
		if (players[1].posX > intToFix16(players[1].moveConstraintXLeft))
		{
			players[1].posX -= intToFix16(playerWidth / 2);
		}
	}
}

void setPlayerPosition()
{
	//Debug
	SPR_setPosition(debug1, players[1].playerShield.posX -4, players[1].playerShield.posY - 4);
	SPR_setPosition(debug2, players[1].playerShield.posX + 12, players[1].playerShield.posY - 4);
	SPR_setPosition(debug3, players[1].playerShield.posX + 12, players[1].playerShield.posY + shieldHeight - 4);
	SPR_setPosition(debug4, players[1].playerShield.posX -4, players[1].playerShield.posY + shieldHeight - 4);

	//Players
	SPR_setPosition(players[0].playerSprite, fix16ToInt(players[0].posX), fix16ToInt(players[0].posY));
	SPR_setPosition(players[0].playerShield.shieldSprite, fix16ToInt(players[0].posX) + shieldOffset, fix16ToInt(players[0].posY));
	players[0].playerShield.posX = fix16ToInt(players[0].posX) + shieldOffset;
	players[0].playerShield.posY = players[0].posY;

	SPR_setPosition(players[1].playerSprite, fix16ToInt(players[1].posX), fix16ToInt(players[1].posY));
	SPR_setPosition(players[1].playerShield.shieldSprite, fix16ToInt(players[1].posX) - 0, fix16ToInt(players[1].posY));
	players[1].playerShield.posX = fix16ToInt(players[1].posX) - 0;
	players[1].playerShield.posY = fix16ToInt(players[1].posY);
}

void player1PosClamp()
{
	if (players[0].posX < intToFix16(players[0].moveConstraintXLeft))
	{
		players[0].posX = intToFix16(0);
		players[0].velX = intToFix16(0);
	}

	if (players[0].posX > intToFix16(players[0].moveConstraintXRight))
	{
		players[0].posX = intToFix16(players[0].moveConstraintXRight);
		players[0].velX = intToFix16(0);
	}
}

void player2PosClamp()
{
	if (players[1].posX < intToFix16(players[1].moveConstraintXLeft))
	{
		players[1].posX = intToFix16(players[1].moveConstraintXLeft);
		players[1].velX = intToFix16(0);
	}

	if (players[1].posX > intToFix16(players[1].moveConstraintXRight))
	{
		players[1].posX = intToFix16(players[1].moveConstraintXRight);
		players[1].velX = intToFix16(0);
	}
}

//ShieldTimers
void p1ShieldTimer()
{
	if (players[0].playerShield.shieldActive)
	{
		SPR_setVisibility(players[0].playerShield.shieldSprite, VISIBLE);
		SPR_setAnim(players[0].playerShield.shieldSprite, 0);
		SPR_update();
		//Start counting frames
		p1ShieldFrameCount++;
		if (p1ShieldFrameCount > shieldFrameTime)
		{
			SPR_setVisibility(players[0].playerShield.shieldSprite, HIDDEN);
			//SPR_setAnim(players[0].shieldSprite, 0);
			players[0].playerShield.shieldActive = FALSE;
			p1ShieldFrameCount = 0;
		}
	}
}

void p2ShieldTimer()
{
	if (players[1].playerShield.shieldActive)
	{
		SPR_setVisibility(players[1].playerShield.shieldSprite, VISIBLE);

		//Start counting frames
		//p2ShieldFrameCount++;
		if (p2ShieldFrameCount > shieldFrameTime)
		{
			SPR_setVisibility(players[1].playerShield.shieldSprite, HIDDEN);
			players[1].playerShield.shieldActive = FALSE;
			p2ShieldFrameCount = 0;
		}
	}
}

//Projectiles
int fireProjectile(int playerNum)
{
	for (int projNum = 0; projNum < 2; projNum++)
	{
		if(inPlayRaquetBalls < 2 && projectiles[projNum].inPlay == FALSE)
		{
			projectiles[projNum].inPlay = FALSE;
			projectiles[projNum].posY = players[playerNum].posY +intToFix16(projectileSpawnYOffset);
			projectiles[projNum].posX = players[playerNum].posX + intToFix16(projectileSpawnXOffset);
			projectiles[projNum].projectileOwner = playerNum;
			if(playerNum == PLAYER_2)
			{
				projectiles[projNum].direction = -1;
			}else
			{
				projectiles[projNum].direction = 1;
			}
			SPR_setVisibility(projectiles[projNum].projectileSprite, VISIBLE);
			projectiles[projNum].inPlay = TRUE;
			inPlayRaquetBalls++;
			return 1;
		}
	}
	return 0;
}

void projectileMovement()
{
	for (int projNum = 0; projNum < 2; projNum++)
	{
		if (projectiles[projNum].inPlay == TRUE)
		{
			projectiles[projNum].projectileSpeed = projectileStartSpeed;
			projectiles[projNum].posX = fix16Add(projectiles[projNum].posX, intToFix16(projectiles[projNum].projectileSpeed * projectiles[projNum].direction));
			SPR_setPosition(projectiles[projNum].projectileSprite, fix16ToInt(projectiles[projNum].posX), fix16ToInt(projectiles[projNum].posY));
			if (projectiles[projNum].posX > intToFix16(screenWidth) || projectiles[projNum].posX < intToFix16(0))
			{
				killProjectile(projNum);
			}
		}
		
	}
}

void killProjectile(int projNum)
{
	projectiles[projNum].inPlay = FALSE;
	SPR_setVisibility(projectiles[projNum].projectileSprite, HIDDEN);
	projectiles[projNum].posX = intToFix16(screenWidth / 2);
	inPlayRaquetBalls--;
}

//Collision
void checkProjShieldCollision()
{
	for (int projNum = 0; projNum < 2; projNum++)
	{
		if (projectiles[projNum].inPlay && players[1].playerShield.shieldActive)
		{
			if (projectiles[projNum].posX < intToFix16(players[1].playerShield.posX + shieldWidth) && projectiles[projNum].posX + intToFix16(projectileHitSize) > intToFix16(players[1].playerShield.posX))
			{
				if (projectiles[projNum].posY < intToFix16(players[1].playerShield.posY + shieldHeight) && projectiles[projNum].posY + intToFix16(projectileHitSize) >= intToFix16(players[1].playerShield.posY))
				{
					//players[0].playerProjectile.direction = -1;
					SPR_setVisibility(projectiles[projNum].projectileSprite, HIDDEN);
				}
			}
		}
		for (int shieldNum = 0; shieldNum < 2; shieldNum++)
		{
			/* code */
		}
		
	}
}

//Background Effects
void ScrollBackground()
{
	if (frameCount % 6 == 0)
	{
		VDP_setHorizontalScroll(PLAN_B, scrollAmount -= scrollSpeed);
	}
}

//Input Stuff
int buttonPressEvent(int playerNum, int button)
{
	if (button == A_BUTTON)
	{
		playerJumping(playerNum, players[playerNum].horizontalNormal);
	}
	else if (button == B_BUTTON)
	{
		fireProjectile(playerNum);
	}
	else if (button == C_BUTTON)
	{
		players[playerNum].playerShield.shieldActive = TRUE;
	}
	return (0);
}

static void myJoyHandler(u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			buttonPressEvent(PLAYER_1, START_BUTTON);
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if (state & BUTTON_A)
		{
			buttonPressEvent(PLAYER_1, A_BUTTON);
		}

		if (state & BUTTON_B)
		{
			buttonPressEvent(PLAYER_1, B_BUTTON);
		}

		if (state & BUTTON_C)
		{
			buttonPressEvent(PLAYER_1, C_BUTTON);
		}

		if (state & BUTTON_RIGHT)
		{
			players[0].horizontalNormal = 1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				players[0].horizontalNormal = 0;
			}
		}

		if (state & BUTTON_LEFT)
		{
			players[0].horizontalNormal = -1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				players[0].horizontalNormal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			players[0].verticalNormal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				players[0].verticalNormal = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			players[0].verticalNormal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				players[0].verticalNormal = 0;
			}
		}
	}

	if (joy == JOY_2)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			buttonPressEvent(PLAYER_2, START_BUTTON);
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if (state & BUTTON_A)
		{
			buttonPressEvent(PLAYER_2, A_BUTTON);
		}

		if (state & BUTTON_B)
		{
			buttonPressEvent(PLAYER_2, B_BUTTON);
		}

		if (state & BUTTON_C)
		{
			buttonPressEvent(PLAYER_2, C_BUTTON);
		}

		if (state & BUTTON_RIGHT)
		{
			players[1].horizontalNormal = 1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				players[1].horizontalNormal = 0;
			}
		}

		if (state & BUTTON_LEFT)
		{
			players[1].horizontalNormal = -1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				players[1].horizontalNormal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			players[1].verticalNormal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				players[1].verticalNormal = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			players[1].verticalNormal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				players[1].verticalNormal = 0;
			}
		}
	}
}

char strPosX[16] = "0";
char str_x1[16] = "0";
char str_x2[16] = "0";
char strPosY[16] = "0";
char str_y1[16] = "0";
char str_y2[16] = "0";

void updateDebug()
{

	int debugInfo1 = projectiles[0].inPlay;
	int debugInfo2 = projectiles[1].inPlay;
	sprintf(strPosY, "%d", inPlayRaquetBalls);
	sprintf(str_y1, "%d", debugInfo2);
	sprintf(str_y2, "%d", debugInfo1);

	VDP_clearTextBG(PLAN_A, 16, 6, 4);
	VDP_clearTextBG(PLAN_A, 16, 8, 4);
	VDP_clearTextBG(PLAN_A, 16, 10, 4);

	VDP_drawTextBG(PLAN_A, "debugInfo0:", 1, 6);
	VDP_drawTextBG(PLAN_A, strPosY, 16, 6);
	VDP_drawTextBG(PLAN_A, "debugInfo1:", 1, 8);
	VDP_drawTextBG(PLAN_A, str_y1, 16, 8);
	VDP_drawTextBG(PLAN_A, "debugInfo2:", 1, 10);
	VDP_drawTextBG(PLAN_A, str_y2, 16, 10);
}
