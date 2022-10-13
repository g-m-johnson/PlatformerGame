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
	TYPE_FLASK,
	TYPE_HEALTH,
};

enum CharacterState
{
	STATE_WALK = 0,
	STATE_IDLE,
	STATE_JUMP,
	STATE_SWING,
	STATE_WOUNDED,
	STATE_DEAD,
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
	Point2f startingPoint = { 120, 184 };
	CharacterState state = STATE_WALK;
	bool direction = false; //true = left, false = right
};
extern PlayerState playerState, resetPlayerState;

struct EnemyState
{
	int enemyHP = 50;
	int platformNumber = 0;
	CharacterState state = STATE_WALK;
	bool direction = false;
};
extern EnemyState enemyState, resetEnemyState;


// Draws the coordinates of the cursor on screen
void TempCursorPos();
// Creates platforms given a list of positions
void CreatePlatforms();
void CreateAnchor();
void UpdateRopeSwing();
// Draws a target a set distance away from the origin of the sprite
void DrawTarget();
void UpdateAmmo();
// Pieces platform sprites together 
void DrawPlatforms();
// Draws a sprite as a reflection in the Y axis
void DrawObjectYFlipped(GameObject&);
void CreateCollectables();
void UpdateCollectables();
void HandleUI();