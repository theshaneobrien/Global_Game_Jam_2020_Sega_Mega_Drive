#define START_BUTTON 1
#define A_BUTTON 2
#define B_BUTTON 3
#define C_BUTTON 4

#define PLAYER_1 0
#define PLAYER_2 1

#define ANIM_IDLE 3
#define ANIM_WALKING 2
#define ANIM_JUMPING 1
#define ANIM_DEFLECT 0

#include <genesis.h>
#include <resources.h>

//Screen
int desiredScreenWidth = 320;
int desiredScreenHeight = 224;

//GameplayState
bool hitFreeze = FALSE;
int hitCounter = 0;
int hitFreezeDuration = 10;

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
	int speed;
	int currentOwner;
	int originalOwner;
	int direction;
	int hitCount;
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
int shieldHeight = 64;
int shieldWidth = 32;
struct Shield shields[2];

struct Hitbox
{
	int posX;
	int posY;
	int width;
	int height;
	int offset;
};

//Player
struct Player
{
	Sprite *playerSprite;
	fix16 posX;
	fix16 velX;
	fix16 posY;
	fix16 velY;
	bool jumping;
	bool grounded;
	bool isMovingRight;
	bool isMovingLeft;
	bool hasActiveProjectile;
	int horizontalNormal;
	int verticalNormal;
	int moveConstraintXLeft;
	int moveConstraintXRight;
	int shieldOffset;
	int score;
	struct Shield playerShield;
	struct Hitbox hitbox;
};
struct Player players[2];
const int playerWidth = 64;
const int playerHeight = 64;
const int jumpForce = -3;
const fix16 jumpDistance = FIX16(0.75);
const int walkingFrameTime = 32;
int p1WalkingCount = 0;
int p2WalkingCount = 0;

const int groundHeight = 208;

int scrollSpeed = 1;
int scrollAmount;
int frameCount = 0;

void init();
void setupPlayField();
void setupPlayers();
void hitFreezeCount();
void gravity();
void playerJumping();
void playerWalking();
void p1WalkingCounter();
void p2WalkingCounter();
void setPlayerPosition();
void playerPosClamp();
//ShieldTimers
void shieldTimer();
//ProjectileUpdates
int fireProjectile(int playerNum);
void projectileMovement();
void killProjectile(int projNum);
//Collision
void checkProjShieldCollision();
void shieldCollision(int projNum, int shieldHitNum);
void checkProjPlayerCollision();
void playerCollision(int projNum, int playerHitNum);

//Random Utils
int countFrames();
void scrollBackground();
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
		hitFreezeCount();
		if(hitFreeze == FALSE)
		{
			countFrames();
			scrollBackground();
			//Updates Sprites Position / Animation Frame
			SPR_update();
			gravity();
			setPlayerPosition();
			shieldTimer();
			projectileMovement();
			p1WalkingCounter();
			p2WalkingCounter();
			checkProjShieldCollision();
			checkProjPlayerCollision();
		}
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

	//Set the players intial position and constraints
	players[0].posX = intToFix16(0);
	players[0].posY = intToFix16(groundHeight - playerHeight);
	players[0].moveConstraintXLeft = 0;players[0].shieldOffset;
	players[0].moveConstraintXRight = (screenWidth / 2) - playerWidth;
	players[0].shieldOffset = playerWidth-16;
	players[0].grounded == TRUE;
	//Shield
	players[0].playerShield.shieldOwner = PLAYER_1;
	players[0].playerShield.posX = fix16ToInt(players[0].posX) + players[0].shieldOffset + 10;
	players[0].playerShield.posY = players[0].posY;
	players[0].playerShield.shieldSprite = SPR_addSprite(&shieldSprite, players[0].playerShield.posX, players[0].playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setVisibility(players[0].playerShield.shieldSprite, HIDDEN);
	//Hitbox
	players[0].hitbox.posX = fix16ToInt(players[0].posX);
	players[0].hitbox.posY = fix16ToInt(players[0].posY);
	players[0].hitbox.width = 32;
	players[0].hitbox.height = 64;
	players[0].hitbox.offset = -8;
	//Score
	players[0].score = 0;

	//Set the players intial position and constraints
	players[1].posX = intToFix16(256);
	players[1].posY = intToFix16(groundHeight - playerHeight);
	players[1].moveConstraintXLeft = screenWidth / 2;
	players[1].moveConstraintXRight = 256;
	players[1].shieldOffset = (-playerWidth / 2) + 32;
	players[1].grounded == TRUE;
	//Shield
	players[1].playerShield.shieldOwner = PLAYER_2;
	players[1].playerShield.posX = fix16ToInt(players[1].posX) + players[1].shieldOffset;
	players[1].playerShield.posY = players[1].posY;
	players[1].playerShield.shieldSprite = SPR_addSprite(&shieldSprite, players[1].playerShield.posX, players[1].playerShield.posY, TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_setVisibility(players[1].playerShield.shieldSprite, HIDDEN);
	//Hitbox
	players[1].hitbox.posX = fix16ToInt(players[1].posX)+playerWidth;
	players[1].hitbox.posY = fix16ToInt(players[1].posY);
	players[1].hitbox.width = 32;
	players[1].hitbox.height = 64;
	players[1].hitbox.offset = (-playerWidth / 2) + 64;
	//Score
	players[1].score = 0;

	//Insert the player sprites at the above positions
	players[0].playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(players[0].posX), fix16ToInt(players[0].posY), TILE_ATTR(PAL1, 0, FALSE, FALSE));
	SPR_setAnim(players[0].playerSprite, ANIM_IDLE);
	players[1].playerSprite = SPR_addSprite(&player1Sprite, fix16ToInt(players[1].posX), fix16ToInt(players[1].posY), TILE_ATTR(PAL1, 0, FALSE, TRUE));
	SPR_setAnim(players[1].playerSprite, ANIM_IDLE);
	SPR_update();

	//set up projectiles
	for (int projNumber = 0; projNumber < 2; projNumber++)
	{
		projectiles[projNumber].projectileSprite = SPR_addSprite(&projectileSprite, 0, 0, TILE_ATTR(PAL1, 0, FALSE, FALSE));
		SPR_setVisibility(projectiles[projNumber].projectileSprite, HIDDEN);
	}

	shields[0] = players[0].playerShield;
	shields[1] = players[1].playerShield;

	//debug
	// debug1 = SPR_addSprite(&debug, players[1].hitbox.posY + players[1].hitbox.width, players[1].hitbox.posY + players[1].hitbox.height, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	// debug2 = SPR_addSprite(&debug, players[1].hitbox.posY, players[1].hitbox.posY + players[1].hitbox.height, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	// debug3 = SPR_addSprite(&debug, players[1].hitbox.posY + players[1].hitbox.width, players[1].hitbox.posY + players[1].hitbox.height, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	// debug4 = SPR_addSprite(&debug, players[1].hitbox.posY, players[1].hitbox.posY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
}

void hitFreezeCount()
{
	if(hitFreeze == TRUE)
	{
		hitCounter++;
		if(hitCounter > hitFreezeDuration)
		{
			hitCounter = 0;
			hitFreeze = FALSE;
		}
	}
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
	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		//Apply Velocity, need to use fix16Add to add two "floats" together
		players[playerNum].posY = fix16Add(players[playerNum].posY, players[playerNum].velY);
		players[playerNum].posX = fix16Add(players[playerNum].posX, players[playerNum].velX);
		if (fix16ToInt(players[playerNum].posY) + playerHeight >= groundHeight)
		{
			players[playerNum].jumping = FALSE;
			players[playerNum].velY = intToFix16(0);
			players[playerNum].posY = intToFix16(groundHeight - playerHeight);
			players[playerNum].velX = intToFix16(0);
			if (players[playerNum].jumping == FALSE && players[playerNum].isMovingRight == FALSE && players[playerNum].isMovingLeft == FALSE && players[playerNum].grounded == FALSE)
			{
				//SPR_setAnim(players[playerNum].playerSprite, ANIM_IDLE);
				players[playerNum].grounded = TRUE;
			}
		}
		else
		{
			players[playerNum].velY = fix16Add(players[playerNum].velY, 6);
			playerPosClamp();
		}
	}
}

void playerJumping(int player, int direction)
{
	int jumpDirection = direction;
	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		if(playerNum == player)
		{
			if (players[playerNum].jumping != TRUE)
			{
				players[playerNum].velY = intToFix16(jumpForce);
				players[playerNum].velX = fix16Mul(intToFix16(direction), jumpDistance);
				SPR_setAnim(players[playerNum].playerSprite, ANIM_JUMPING);
				players[playerNum].jumping = TRUE;
			}
		}
	}
}

void playerWalking()
{
	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		if (players[playerNum].horizontalNormal == 1)
		{
			players[playerNum].isMovingRight = TRUE;
		}
		else if (players[playerNum].horizontalNormal == -1)
		{
			players[playerNum].isMovingLeft = TRUE;
		}
	}
}

void p1WalkingCounter()
{
	if (players[0].isMovingRight == TRUE)
	{
		SPR_setAnim(players[0].playerSprite, ANIM_WALKING);
		p1WalkingCount++;
		if (players[0].jumping == FALSE)
		{
			if (players[0].posX < intToFix16(players[0].moveConstraintXRight))
			{
				players[0].posX += FIX16(1);
			}
			if (p1WalkingCount > walkingFrameTime)
			{
				p1WalkingCount = 0;
				SPR_setAnim(players[0].playerSprite, ANIM_IDLE);
				players[0].isMovingRight = FALSE;
			}
		}
	}
	else if (players[0].isMovingLeft == TRUE)
	{
		SPR_setAnim(players[0].playerSprite, ANIM_WALKING);
		p1WalkingCount++;
		if (players[0].jumping == FALSE)
		{
			if (players[0].posX > intToFix16(players[0].moveConstraintXLeft))
			{
				players[0].posX -= FIX16(1);
			}
			if (p1WalkingCount > walkingFrameTime)
			{
				p1WalkingCount = 0;
				SPR_setAnim(players[0].playerSprite, ANIM_IDLE);
				players[0].isMovingLeft = FALSE;
			}
		}
	}
}

void p2WalkingCounter()
{
	if(players[1].jumping == FALSE)
	{
		if (players[1].isMovingRight == TRUE)
		{
			SPR_setAnim(players[1].playerSprite, ANIM_WALKING);
			p2WalkingCount++;
			if (players[1].jumping == FALSE)
			{
				if (players[1].posX < intToFix16(players[1].moveConstraintXRight))
				{
					players[1].posX += FIX16(1);
				}
				if (p2WalkingCount > walkingFrameTime)
				{
					p2WalkingCount = 0;
					SPR_setAnim(players[1].playerSprite, ANIM_IDLE);
					players[1].isMovingRight = FALSE;
				}
			}
		}
		else if (players[1].isMovingLeft == TRUE)
		{
			SPR_setAnim(players[1].playerSprite, ANIM_WALKING);
			p2WalkingCount++;
			if (players[0].jumping == FALSE)
			{
				if (players[1].posX > intToFix16(players[1].moveConstraintXLeft))
				{
					players[1].posX -= FIX16(1);
				}
				if (p2WalkingCount > walkingFrameTime)
				{
					p2WalkingCount = 0;
					SPR_setAnim(players[1].playerSprite, ANIM_IDLE);
					players[1].isMovingLeft = FALSE;
				}
			}
		}
	}
}

void setPlayerPosition()
{
	//Players
	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		//Debug
		if(frameCount % 2 == 0)
		{
			// SPR_setPosition(debug1, players[0].hitbox.posX + players[0].hitbox.width, players[0].hitbox.posY);
			// SPR_setPosition(debug2, players[0].hitbox.posX, players[0].hitbox.posY + players[0].hitbox.height);
			// SPR_setPosition(debug3, players[0].hitbox.posX + players[0].hitbox.width, players[0].hitbox.posY + players[1].hitbox.height);
			// SPR_setPosition(debug4, players[0].hitbox.posX, players[0].hitbox.posY);
		}
		else
		{
			// SPR_setPosition(debug1, players[1].hitbox.posX + players[1].hitbox.width, players[1].hitbox.posY);
			// SPR_setPosition(debug2, players[1].hitbox.posX, players[1].hitbox.posY + players[1].hitbox.height);
			// SPR_setPosition(debug3, players[1].hitbox.posX + players[1].hitbox.width, players[1].hitbox.posY + players[1].hitbox.height);
			// SPR_setPosition(debug4, players[1].hitbox.posX, players[1].hitbox.posY);
		}
		

		players[playerNum].playerShield.posX = fix16ToInt(players[playerNum].posX) + players[playerNum].shieldOffset;
		players[playerNum].playerShield.posY = fix16ToInt(players[playerNum].posY);

		players[playerNum].hitbox.posX = fix16ToInt(players[playerNum].posX) + players[playerNum].hitbox.offset;
		players[playerNum].hitbox.posY = fix16ToInt(players[playerNum].posY);

		SPR_setPosition(players[playerNum].playerSprite, fix16ToInt(players[playerNum].posX), fix16ToInt(players[playerNum].posY));
		SPR_setPosition(players[playerNum].playerShield.shieldSprite, players[playerNum].playerShield.posX, fix16ToInt(players[playerNum].posY));
	}
}

void playerPosClamp()
{
	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		if (players[playerNum].posX < intToFix16(players[playerNum].moveConstraintXLeft))
		{
			players[playerNum].posX = intToFix16(players[playerNum].moveConstraintXLeft);
			players[playerNum].velX = intToFix16(0);
		}

		if (players[playerNum].posX > intToFix16(players[playerNum].moveConstraintXRight))
		{
			players[playerNum].posX = intToFix16(players[playerNum].moveConstraintXRight);
			players[playerNum].velX = intToFix16(0);
		}
	}
}

//ShieldTimers
void shieldTimer()
{
	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		if (players[playerNum].playerShield.shieldActive)
		{
			SPR_setAnimAndFrame(players[playerNum].playerSprite, ANIM_DEFLECT, 1);
			SPR_setVisibility(players[playerNum].playerShield.shieldSprite, VISIBLE);
			SPR_setAnim(players[playerNum].playerShield.shieldSprite, 0);
			SPR_update();
			//Start counting frames
			p1ShieldFrameCount++;
			if (p1ShieldFrameCount > shieldFrameTime)
			{
				SPR_setVisibility(players[playerNum].playerShield.shieldSprite, HIDDEN);
				SPR_setAnim(players[playerNum].playerSprite, ANIM_IDLE);
				players[playerNum].playerShield.shieldActive = FALSE;
				p1ShieldFrameCount = 0;
			}
		}
	}
}

void startShield(int playerNum)
{
	players[playerNum].playerShield.shieldActive = TRUE;
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
			projectiles[projNum].currentOwner = playerNum;
			projectiles[projNum].originalOwner = playerNum;
			players[playerNum].hasActiveProjectile = TRUE;
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
			projectiles[projNum].speed = projectileStartSpeed;
			projectiles[projNum].posX = fix16Add(projectiles[projNum].posX, intToFix16(projectiles[projNum].speed * projectiles[projNum].direction));
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
	projectiles[projNum].hitCount = 0;
	inPlayRaquetBalls--;
	players[projectiles[projNum].currentOwner].hasActiveProjectile = FALSE;
}

//Collision
void checkProjShieldCollision()
{
	for (int projNum = 0; projNum < 2; projNum++)
	{
		for (int shieldHitNum = 0; shieldHitNum < 2; shieldHitNum++)
		{
			if (projectiles[projNum].inPlay == TRUE && players[shieldHitNum].playerShield.shieldActive == TRUE)
			{
				if (projectiles[projNum].posX < intToFix16(players[shieldHitNum].playerShield.posX + shieldWidth - 6) && projectiles[projNum].posX + intToFix16(projectileHitSize) > intToFix16(players[shieldHitNum].playerShield.posX - 6))
				{
					if (projectiles[projNum].posY < intToFix16(players[shieldHitNum].playerShield.posY + shieldHeight) && projectiles[projNum].posY + intToFix16(projectileHitSize) >= intToFix16(players[shieldHitNum].playerShield.posY))
					{
						if (projectiles[projNum].currentOwner != players[shieldHitNum].playerShield.shieldOwner)
						{
							shieldCollision(projNum, shieldHitNum);
						}
					}
				}
			}
		}
	}
}

void shieldCollision(int projNum, int shieldHitNum)
{
	hitFreeze = TRUE;
	projectiles[projNum].currentOwner = shieldHitNum;
	players[projectiles[projNum].originalOwner].hasActiveProjectile = FALSE;
	projectiles[projNum].direction = projectiles[projNum].direction * -1;
	projectiles[projNum].hitCount++;
	if (projectiles[projNum].hitCount > 3)
	{
		killProjectile(projNum);
	}
}

void checkProjPlayerCollision()
{
	for (int projNum = 0; projNum < 2; projNum++)
	{
		for (int playerHitNum = 0; playerHitNum < 2; playerHitNum++)
		{
			if (projectiles[projNum].inPlay == TRUE)
			{
				if (projectiles[projNum].posX < intToFix16(players[playerHitNum].hitbox.posX + players[playerHitNum].hitbox.width) && projectiles[projNum].posX + intToFix16(projectileHitSize) > intToFix16(players[playerHitNum].hitbox.posX))
				{
					if (projectiles[projNum].posY < intToFix16(players[playerHitNum].hitbox.posY + players[playerHitNum].hitbox.height) && projectiles[projNum].posY + intToFix16(projectileHitSize) >= intToFix16(players[playerHitNum].hitbox.posY))
					{
						if (projectiles[projNum].currentOwner != playerHitNum)
						{
							playerCollision(projNum, playerHitNum);
						}
					}
				}
			}
		}
	}
}

void playerCollision(int projNum, int playerHitNum)
{
	hitFreeze = TRUE;
	if(playerHitNum == 0)
	{
		players[1].score++;
	}else
	{
		players[0].score++;
	}
	
	killProjectile(projNum);
}

//Background Effects
void scrollBackground()
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
		if(players[playerNum].hasActiveProjectile == FALSE)
		{
			fireProjectile(playerNum);			
		}
	}
	else if (button == C_BUTTON)
	{
		startShield(playerNum);
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
			if (players[0].isMovingLeft == FALSE)
			{
				players[0].horizontalNormal = 1;
				playerWalking();
			}
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
			if (players[0].isMovingRight == FALSE)
			{
				players[0].horizontalNormal = -1;
				playerWalking();
			}
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

	int debugInfo1 = players[0].score;
	int debugInfo2 = players[1].score;
	sprintf(strPosY, "%d", inPlayRaquetBalls);
	sprintf(str_y1, "%d", debugInfo1);
	sprintf(str_y2, "%d", debugInfo2);

	VDP_clearTextBG(PLAN_A, 16, 6, 4);
	VDP_clearTextBG(PLAN_A, 16, 8, 4);
	VDP_clearTextBG(PLAN_A, 16, 10, 4);

	// VDP_drawTextBG(PLAN_A, "debugInfo0:", 1, 6);
	// VDP_drawTextBG(PLAN_A, strPosY, 16, 6);
	// VDP_drawTextBG(PLAN_A, "debugInfo1:", 1, 8);
	// VDP_drawTextBG(PLAN_A, str_y1, 16, 8);
	// VDP_drawTextBG(PLAN_A, "debugInfo2:", 1, 10);
	// VDP_drawTextBG(PLAN_A, str_y2, 16, 10);
}
