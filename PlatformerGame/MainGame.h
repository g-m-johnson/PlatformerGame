#pragma once

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720
#define DISPLAY_SCALE 1

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_PLAYER,
	TYPE_PLATFORM,
	TYPE_ANCHORPOINT,
	TYPE_AMMO,
	TYPE_ENEMY,
	TYPE_MOLECULE,
	TYPE_HEALTH,
	TYPE_DOOR,
	TYPE_COMPUTER,
	TYPE_CHUNKS,
};

enum CharacterState
{
	STATE_WALK = 0,
	STATE_IDLE,
	STATE_JUMP,
	STATE_SWING,
	STATE_WOUNDED,
	STATE_DEAD,
	STATE_THROW,
	STATE_LEAVING,
};

struct GamePlayState
{
	float stopwatch = 0;
	float init_time = 0;
	float damage_timer = 0;
	int noteObjectId = 0;
};
extern GamePlayState gamePlayState, resetGame;

struct PlayerState
{
	int playerHP = 3;
	int playerXP = 0;
	bool hurt = false;
	Point2f startingPoint = { 160, 315 };
	CharacterState state = STATE_WALK;
	bool direction = false; //true = left, false = right
	bool exitActive = false;
};
extern PlayerState playerState, resetPlayerState;

struct EnemyState
{
	int enemyHP = 50;
	std::vector <int> platformNumbers {6, 13};
	CharacterState state = STATE_WALK;
	bool direction = false;
};
extern EnemyState enemyState, resetEnemyState;

//------------------------------------------------------------------------------

// Draws health bar in top left corner
void HandleUI();

// Draws the user input controls to screen when 'I' is pressed
void ControlScreen();

// Draws the coordinates of the cursor on screen
void TempCursorPos();



// Creates platforms given a list of positions
void CreatePlatforms();

// Pieces platform sprites together 
void DrawPlatforms();



// Creates the anchor that the player can swing from
void CreateAnchor();

// Updates swing when it is not in use by the player
void UpdateRopeSwing();

// Puts sprites together for rope swing
void DrawRopeSwing(int id, int choice, float angle);



void CreateCollectables();

void UpdateCollectables();



void CreateExitObjects();

void UpdateExitObjects();



// Draws a sprite as a reflection in the Y axis
void DrawObjectYFlipped(GameObject&);



void GameReset();



//------------------------------------------------------------------------------