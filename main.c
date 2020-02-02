#define START_BUTTON 1
#define A_BUTTON 2
#define B_BUTTON 3
#define C_BUTTON 4

#define PLAYER_1 1
#define PLAYER_2 2

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
	bool projectileAlive;
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
struct Projectile projectile1;
struct Projectile projectile2;
struct Projectile projectiles[2];

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

struct Player player1;
struct Player player2;
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
void projectileMovement();
void p2ProjectileLife();
//Collision
void checkProjShieldCollision();

int countFrames();
void ScrollBackground();
void setupMusic();

//Button Functions
int p1ButtonPressEvent(int button);
int p2ButtonPressEvent(int button);

//Debug
Sprite *debug1;
Sprite *debug2;
Sprite *debug3;
Sprite *debug4;

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
		p2ProjectileLife();
		checkProjShieldCollision();
		//Wait for the frame to finish rendering
		VDP_waitVSync();
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

	players[0] = player1;
	players[1] = player2;

	//Set the players intial position and constraints
	player1.posX = intToFix16(0);
	player1.posY = intToFix16(groundHeight - playerHeight);
	player1.moveConstraintXLeft = 0;
	player1.moveConstraintXRight = (screenWidth / 2) - playerWidth;
	//Shield
	player1.playerShield.shieldOwner = PLAYER_1;
	player1.playerShield.posX = fix16ToInt(player1.posX) + shieldOffset;
	player1.playerShield.posY = player1.posY;
	player1.playerShield.shieldSprite = SPR_addSprite(&shieldSprite, player1.playerShield.posX, player1.playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(player1.playerShield.shieldSprite, HIDDEN);
	

	//Set the players intial position and constraints
	player2.posX = intToFix16(256);
	player2.posY = intToFix16(groundHeight - playerHeight);
	player2.moveConstraintXLeft = screenWidth / 2;
	player2.moveConstraintXRight = 256;
	//Shield
	player2.playerShield.shieldOwner = PLAYER_2;
	player2.playerShield.posX = fix16ToInt(player2.posX) - 0;
	player2.playerShield.posY = player2.posY;
	player2.playerShield.shieldSprite = SPR_addSprite(&shieldSprite, player2.playerShield.posX, player2.playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_setVisibility(player2.playerShield.shieldSprite, HIDDEN);

	//Insert the player sprites at the above positions
	player1.playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(player1.posX), fix16ToInt(player1.posY), TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setAnim(player1.playerSprite, 1);
	player2.playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(player2.posX), fix16ToInt(player2.posY), TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_update();

	//set up projectiles
	projectiles[0] = projectile1;
	projectiles[0].projectileOwner = PLAYER_1;
	projectiles[0].direction = 1;
	projectiles[0].projectileAlive = FALSE;
	projectiles[0].projectileSpeed = 0;
	projectiles[0].posX = player1.posX + intToFix16(projectileSpawnXOffset);
	projectiles[0].posY = intToFix16(fix16ToInt(player1.posY) + projectileSpawnYOffset);
	projectiles[0].projectileSprite = SPR_addSprite(&projectileSprite, fix16ToInt(player1.posX) + projectileSpawnXOffset, projectiles[0].posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(projectiles[0].projectileSprite, HIDDEN);

	projectiles[1] = projectile2;
	projectiles[1].projectileOwner = PLAYER_1;
	projectiles[1].direction = 1;
	projectiles[1].projectileAlive = FALSE;
	projectiles[1].projectileSpeed = 0;
	projectiles[1].posX = player1.posX + intToFix16(projectileSpawnXOffset);
	projectiles[1].posY = intToFix16(fix16ToInt(player1.posY) + projectileSpawnYOffset);
	projectiles[1].projectileSprite = SPR_addSprite(&projectileSprite, fix16ToInt(player1.posX) + projectileSpawnXOffset, projectiles[1].posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(projectiles[1].projectileSprite, HIDDEN);

	//set up shields
	shields[0] = player1.playerShield;
	shields[1] = player2.playerShield;

	//debug
	debug1 = SPR_addSprite(&debug, player2.playerShield.posX + shieldWidth, player2.playerShield.posY + shieldHeight, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	debug2 = SPR_addSprite(&debug, player2.playerShield.posX, player2.playerShield.posY + shieldHeight, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	debug3 = SPR_addSprite(&debug, player2.playerShield.posX + shieldWidth, player2.playerShield.posY + shieldHeight, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	debug4 = SPR_addSprite(&debug, player2.playerShield.posX, player2.playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
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
	player1.posY = fix16Add(player1.posY, player1.velY);
	player1.posX = fix16Add(player1.posX, player1.velX);

	player2.posY = fix16Add(player2.posY, player2.velY);
	player2.posX = fix16Add(player2.posX, player2.velX);

	//Check if player is on floor
	if (fix16ToInt(player1.posY) + playerHeight >= groundHeight)
	{
		player1.jumping = FALSE;
		player1.velY = FIX16(0);
		player1.posY = intToFix16(groundHeight - playerHeight);
		player1.velX = intToFix16(0);
	}
	else
	{
		player1.velY = fix16Add(player1.velY, 6);
		player1PosClamp();
	}

	if (fix16ToInt(player2.posY) + playerHeight >= groundHeight)
	{
		player2.jumping = FALSE;
		player2.velY = FIX16(0);
		player2.posY = intToFix16(groundHeight - playerHeight);
		player2.velX = intToFix16(0);
	}
	else
	{
		player2.velY = fix16Add(player2.velY, 6);
		player2PosClamp();
	}
}

void playerJumping(int player, int direction)
{
	int jumpDirection = direction;
	if (player == PLAYER_1 && player1.jumping != TRUE)
	{
		player1.jumping = TRUE;
		player1.velY = FIX16(jumpForce);
		player1.velX = fix16Mul(intToFix16(direction), jumpDistance);
	}

	if (player == PLAYER_2 && player2.jumping != TRUE)
	{
		player2.jumping = TRUE;
		player2.velY = FIX16(jumpForce);
		player2.velX = fix16Mul(intToFix16(direction), jumpDistance);
	}
}

void playerWalking()
{

	if (player1.horizontalNormal == 1)
	{
		if (player1.posX < intToFix16(player1.moveConstraintXRight))
		{
			player1.posX += intToFix16(playerWidth / 2);
		}
	}
	else if (player1.horizontalNormal == -1)
	{
		if (player1.posX > intToFix16(player1.moveConstraintXLeft))
		{
			player1.posX -= intToFix16(playerWidth / 2);
		}
	}

	if (player2.horizontalNormal == 1)
	{
		if (player2.posX < intToFix16(player2.moveConstraintXRight))
		{
			player2.posX += intToFix16(playerWidth / 2);
		}
	}
	else if (player2.horizontalNormal == -1)
	{
		if (player2.posX > intToFix16(player2.moveConstraintXLeft))
		{
			player2.posX -= intToFix16(playerWidth / 2);
		}
	}
}

void setPlayerPosition()
{
	//Debug
	SPR_setPosition(debug1, player2.playerShield.posX -4, player2.playerShield.posY - 4);
	SPR_setPosition(debug2, player2.playerShield.posX + 12, player2.playerShield.posY - 4);
	SPR_setPosition(debug3, player2.playerShield.posX + 12, player2.playerShield.posY + shieldHeight - 4);
	SPR_setPosition(debug4, player2.playerShield.posX -4, player2.playerShield.posY + shieldHeight - 4);

	//Players
	SPR_setPosition(player1.playerSprite, fix16ToInt(player1.posX), fix16ToInt(player1.posY));
	SPR_setPosition(player1.playerShield.shieldSprite, fix16ToInt(player1.posX) + shieldOffset, fix16ToInt(player1.posY));
	player1.playerShield.posX = fix16ToInt(player1.posX) + shieldOffset;
	player1.playerShield.posY = player1.posY;

	SPR_setPosition(player2.playerSprite, fix16ToInt(player2.posX), fix16ToInt(player2.posY));
	SPR_setPosition(player2.playerShield.shieldSprite, fix16ToInt(player2.posX) - 0, fix16ToInt(player2.posY));
	player2.playerShield.posX = fix16ToInt(player2.posX) - 0;
	player2.playerShield.posY = fix16ToInt(player2.posY);
}

void player1PosClamp()
{
	if (player1.posX < intToFix16(player1.moveConstraintXLeft))
	{
		player1.posX = intToFix16(0);
		player1.velX = intToFix16(0);
	}

	if (player1.posX > intToFix16(player1.moveConstraintXRight))
	{
		player1.posX = intToFix16(player1.moveConstraintXRight);
		player1.velX = intToFix16(0);
	}
}

void player2PosClamp()
{
	if (player2.posX < intToFix16(player2.moveConstraintXLeft))
	{
		player2.posX = intToFix16(player2.moveConstraintXLeft);
		player2.velX = intToFix16(0);
	}

	if (player2.posX > intToFix16(player2.moveConstraintXRight))
	{
		player2.posX = intToFix16(player2.moveConstraintXRight);
		player2.velX = intToFix16(0);
	}
}

//ShieldTimers
void p1ShieldTimer()
{
	if (player1.playerShield.shieldActive)
	{
		SPR_setVisibility(player1.playerShield.shieldSprite, VISIBLE);
		SPR_setAnim(player1.playerShield.shieldSprite, 0);
		SPR_update();
		//Start counting frames
		p1ShieldFrameCount++;
		if (p1ShieldFrameCount > shieldFrameTime)
		{
			SPR_setVisibility(player1.playerShield.shieldSprite, HIDDEN);
			//SPR_setAnim(player1.shieldSprite, 0);
			player1.playerShield.shieldActive = FALSE;
			p1ShieldFrameCount = 0;
		}
	}
}

void p2ShieldTimer()
{
	if (player2.playerShield.shieldActive)
	{
		SPR_setVisibility(player2.playerShield.shieldSprite, VISIBLE);

		//Start counting frames
		//p2ShieldFrameCount++;
		if (p2ShieldFrameCount > shieldFrameTime)
		{
			SPR_setVisibility(player2.playerShield.shieldSprite, HIDDEN);
			player2.playerShield.shieldActive = FALSE;
			p2ShieldFrameCount = 0;
		}
	}
}

//Projectile
void projectileMovement()
{
	for (int i = 0; i < 2; i++)
	{
		if (projectiles[i].projectileAlive == TRUE)
		{
			projectiles[i].projectileSpeed = projectileStartSpeed;
			projectiles[i].posX = fix16Add(projectiles[i].posX, intToFix16(projectiles[i].projectileSpeed * projectiles[i].direction));
			//player1.playerProjectile.posY = fix16Add(player1.posX, player1.velX);
			SPR_setPosition(projectiles[i].projectileSprite, fix16ToInt(projectiles[i].posX), fix16ToInt(projectiles[i].posY));
			if (projectiles[i].posX > intToFix16(screenWidth))
			{
				SPR_setVisibility(projectiles[i].projectileSprite, HIDDEN);
				projectiles[i].posX = player1.posX + intToFix16(projectileSpawnXOffset);
				projectiles[i].projectileAlive == FALSE;
			}
		}
	}
	
	
}

void p2ProjectileLife()
{
	// if (player2.playerProjectile.projectileAlive == TRUE)
	// {
	// 	player2.playerProjectile.projectileSpeed = projectileStartSpeed;
	// 	player2.playerProjectile.posX = fix16Sub(player2.playerProjectile.posX, intToFix16(player2.playerProjectile.projectileSpeed));
	// 	//player1.playerProjectile.posY = fix16Add(player1.posX, player1.velX);
	// 	SPR_setPosition(player2.playerProjectile.projectileSprite, fix16ToInt(player2.playerProjectile.posX), fix16ToInt(player2.playerProjectile.posY));
	// 	if (player2.playerProjectile.posX < 0)
	// 	{
	// 		player2.playerProjectile.posX = player2.posX + intToFix16(projectileSpawnXOffset);
	// 		//player2.playerProjectile.projectileAlive == FALSE;
	// 		//SPR_setVisibility(player2.playerProjectile.projectileSprite, HIDDEN);
	// 	}
	// }
}

//Collision
void checkProjShieldCollision()
{
	if (projectiles[0].projectileAlive && player2.playerShield.shieldActive)
	{
		if (projectiles[0].posX < intToFix16(player2.playerShield.posX + shieldWidth) && projectiles[0].posX + intToFix16(projectileHitSize) > intToFix16(player2.playerShield.posX))
		{
			if (projectiles[0].posY < intToFix16(player2.playerShield.posY + shieldHeight) && projectiles[0].posY + intToFix16(projectileHitSize) >= intToFix16(player2.playerShield.posY))
			{
				//player1.playerProjectile.direction = -1;
				SPR_setVisibility(projectiles[0].projectileSprite, HIDDEN);
			}
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
int p1ButtonPressEvent(int button)
{
	if (button == A_BUTTON)
	{
		playerJumping(PLAYER_1, player1.horizontalNormal);
	}
	else if (button == B_BUTTON)
	{
		projectiles[0].projectileAlive = FALSE;
		projectiles[0].posY = intToFix16(fix16ToInt(player1.posY) + projectileSpawnYOffset);
		projectiles[0].posX = player1.posX + intToFix16(projectileSpawnXOffset);
		projectiles[0].projectileAlive = TRUE;
		SPR_setVisibility(projectiles[0].projectileSprite, VISIBLE);
	}
	else if (button == C_BUTTON)
	{
		player1.playerShield.shieldActive = TRUE;
	}
	return (0);
}

int p2ButtonPressEvent(int button)
{
	if (button == A_BUTTON)
	{
		playerJumping(PLAYER_2, player2.horizontalNormal);
	}
	else if (button == B_BUTTON)
	{
		projectiles[1].projectileAlive = FALSE;
		projectiles[1].posY = intToFix16(fix16ToInt(player2.posY) + projectileSpawnYOffset);
		projectiles[1].posX = player2.posX + intToFix16(projectileSpawnXOffset);
		projectiles[1].projectileAlive = TRUE;
		SPR_setVisibility(projectiles[1].projectileSprite, VISIBLE);
	}
	else if (button == C_BUTTON)
	{
		player2.playerShield.shieldActive = TRUE;
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
			p1ButtonPressEvent(START_BUTTON);
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if (state & BUTTON_A)
		{
			p1ButtonPressEvent(A_BUTTON);
		}

		if (state & BUTTON_B)
		{
			p1ButtonPressEvent(B_BUTTON);
		}

		if (state & BUTTON_C)
		{
			p1ButtonPressEvent(C_BUTTON);
		}

		if (state & BUTTON_RIGHT)
		{
			player1.horizontalNormal = 1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				player1.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_LEFT)
		{
			player1.horizontalNormal = -1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				player1.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			player1.verticalNormal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				player1.verticalNormal = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			player1.verticalNormal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				player1.verticalNormal = 0;
			}
		}
	}

	if (joy == JOY_2)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			p2ButtonPressEvent(START_BUTTON);
		}

		//State = This will be 1 if the button is currently pressed and 0 if it isn’t.
		if (state & BUTTON_A)
		{
			p2ButtonPressEvent(A_BUTTON);
		}

		if (state & BUTTON_B)
		{
			p2ButtonPressEvent(B_BUTTON);
		}

		if (state & BUTTON_C)
		{
			p2ButtonPressEvent(C_BUTTON);
		}

		if (state & BUTTON_RIGHT)
		{
			player2.horizontalNormal = 1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_RIGHT)
			{
				player2.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_LEFT)
		{
			player2.horizontalNormal = -1;
			playerWalking();
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_LEFT)
			{
				player2.horizontalNormal = 0;
			}
		}

		if (state & BUTTON_UP)
		{
			player2.verticalNormal = 1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_UP)
			{
				player2.verticalNormal = 0;
			}
		}

		if (state & BUTTON_DOWN)
		{
			player2.verticalNormal = -1;
		}
		else
		{
			//changed = This tells us whether the state of a button has changed over the last frame.
			//If the current state is different from the state in the previous frame, this will be 1 (otherwise 0).
			if (changed & BUTTON_DOWN)
			{
				player2.verticalNormal = 0;
			}
		}
	}
}
