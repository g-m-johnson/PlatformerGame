#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH{ 1280 };
int DISPLAY_HEIGHT{ 720 };
int DISPLAY_SCALE{ 1 };






enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_PLAYER,
	TYPE_PLATFORM,
	TYPE_ANCHORPOINT,
	TYPE_AMMO,
	TYPE_ENEMY,
	TYPE_DEADENEMY,
};

enum PlayerState
{
	STATE_WALK = 0,
	STATE_IDLE,
	STATE_JUMP,
	STATE_SWING,
	STATE_DEAD,
};

enum EnemyState
{
	ENEMYSTATE_WALK = 0,
	ENEMYSTATE_WOUNDED,
	ENEMYSTATE_DEAD,
};

struct GameState
{
	int playerHP = 100;
	Point2f startingPoint = { 120, 184 };
	PlayerState playerState = STATE_WALK;
	EnemyState enemyState = ENEMYSTATE_WALK;
	bool direction = false; //true = left, false = right
};
GameState gameState;




// PROTOTYPES
void HandlePlayerControls();
void CreatePlatforms();
void UpdatePlatforms();
void TempCursorPos();
bool PlayerAndPlatformCollision();
void UpdatePlayer();
void UpdateEnemies();
void SwingMechanic();
void CreateAnchor();
void UpdateAnchor();
void DrawTarget();
void UpdateAmmo();
void UpdateEnemyMovementOnPlatform();
void CreateEnemies();
void DrawPlatforms();
void DrawObjectYFlipped(GameObject&);




// MAIN
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Background\\lab_background.png");
	Play::CreateGameObject(TYPE_PLAYER, gameState.startingPoint, 50, "player");
	Play::CentreAllSpriteOrigins();
	CreatePlatforms();
	CreateAnchor();
	CreateEnemies();
}

bool MainGameUpdate(float elapsedTime)
{
	Play::DrawBackground();
	DrawPlatforms();
	UpdatePlatforms();
	TempCursorPos();
	UpdateAnchor();
	UpdatePlayer();
	UpdateAmmo();
	UpdateEnemies();
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}





// FUNCTIONS
void HandlePlayerControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (obj_player.velocity.x != 0)
	{
		if (obj_player.pos.x - obj_player.oldPos.x < 0)
		{
			gameState.direction = true;
		}
		else
		{
			gameState.direction = false;
		}
	}

	bool onSolidGround = PlayerAndPlatformCollision();
	if (onSolidGround == false)
	{
		obj_player.acceleration.y = .8;
		gameState.playerState = STATE_JUMP;
	}
	if (onSolidGround == true)
	{
		gameState.playerState = STATE_WALK;
		obj_player.velocity.x = 0;
		if (Play::KeyPressed(VK_UP))
		{
			obj_player.velocity.y = -17;
		}
	}

	if (Play::KeyDown(VK_LEFT))
	{
		obj_player.velocity.x = -6;
	}
	if (Play::KeyDown(VK_RIGHT))
	{
		obj_player.velocity.x = 6;
	}

	if (obj_player.velocity.x == 0 && obj_player.velocity.y == 0)
	{
		gameState.playerState = STATE_IDLE;
	}
}

void CreatePlatforms()
{
	std::vector<std::vector<float>> platformPositions{
		{70., 360.},
		{335., 196.},
		{950., 427.},
		{1230., 427.},
		{1730., 680.},
		{1920., 680.},
		{1260., 680.}
	};
	std::vector <int> vPlatforms(platformPositions.size());

	int n = 0;
	for (int id : vPlatforms)
	{
		int id = Play::CreateGameObject(TYPE_PLATFORM, { 0, 0 }, 0, "platform_left_edge");
		GameObject& obj_platform = Play::GetGameObject(id);
		obj_platform.pos = { platformPositions.at(n).at(0), platformPositions.at(n).at(1) };
		n++;
	}
}

void UpdatePlatforms()
{
	std::vector <int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_platform);
		Play::DrawObject(obj_platform);
	}
}

void TempCursorPos()
{
	Point2D mousePos = Play::GetMousePos();
	Point2D camPos = Play::GetCameraPosition();
	Play::DrawFontText("64px", std::to_string(camPos.x + mousePos.x) + ", " + std::to_string(mousePos.y), mousePos);
}

bool PlayerAndPlatformCollision()
{
	std::vector <int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	bool onPlatform = false;
	float walk_width = Play::GetSpriteWidth("walk");
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);
		float platform_xmin = obj_platform.pos.x;
		float platform_xmax = obj_platform.pos.x + 180;
		float platform_ymin = obj_platform.pos.y;
		float platform_ymax = obj_platform.pos.y + 50;

		GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
		float player_xmin = obj_player.pos.x - (Play::GetSpriteWidth("walk") / 2);
		float player_xmax = obj_player.pos.x + (Play::GetSpriteWidth("walk") / 2);
		float player_ymin = obj_player.pos.y - (Play::GetSpriteHeight("walk") / 2);
		float player_ymax = obj_player.pos.y + (Play::GetSpriteHeight("walk") / 2);

		
		if (platform_xmin < player_xmax && platform_xmax > player_xmin &&
			platform_ymin < player_ymax && platform_ymax > player_ymin)
		{
			obj_player.acceleration.y = 0;
			obj_player.velocity.y = 0;
			obj_player.pos.y = obj_platform.pos.y - (Play::GetSpriteHeight("walk")/2) + 5;
			onPlatform = true;
			break;
		}
	}
	return onPlatform;
}

void SwingMechanic()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_anchor = Play::GetGameObjectByType(TYPE_ANCHORPOINT);

	Play::DrawLine(obj_player.pos, obj_anchor.pos, Play::cWhite);
	float ropeLength = obj_anchor.radius + obj_player.radius + 10;
	float dx = obj_anchor.pos.x - obj_player.pos.x;
	float dy = obj_player.pos.y - obj_anchor.pos.y;
	float theta = atan(dy / dx);
	

	if (!Play::IsColliding(obj_player, obj_anchor))
	{
		obj_player.velocity = { 0, 0 };
		obj_player.acceleration = { 0, 0 };
		if (dy < 100)
		{
			gameState.direction = !gameState.direction;
		}

		if (gameState.direction == false)
		{
			theta += 0.04;
		}
		else
		{
			theta -= 0.04;
		}

		if (obj_player.pos.x <= obj_anchor.pos.x)
		{
			obj_player.pos.x = obj_anchor.pos.x - ropeLength * cos(theta);
			obj_player.pos.y = obj_anchor.pos.y + ropeLength * sin(theta);
		}
		else
		{
			obj_player.pos.x = obj_anchor.pos.x + ropeLength * cos(theta);
			obj_player.pos.y = obj_anchor.pos.y - ropeLength * sin(theta);
		}

	}


	if (Play::KeyPressed(VK_SPACE))
	{
		gameState.playerState = STATE_JUMP;
		if (gameState.direction)
		{
			obj_player.velocity.x = -4;
		}
		else
		{
			obj_player.velocity.x = 4;
		}
	}
}

bool cursorReleased = true;
void DrawTarget()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D mousePos = Play::GetMousePos();
	Point2D camPos = Play::GetCameraPosition();
	float dx = (camPos.x + mousePos.x) - obj_player.pos.x;
	float dy = mousePos.y - obj_player.pos.y;
	float theta = atan(dy / dx);

	Point2D targetPoint = { obj_player.pos.x + (100 * cos(theta)),
			obj_player.pos.y + (100 * sin(theta)) };
	if ((mousePos.x + camPos.x) < obj_player.pos.x)
	{
		targetPoint = { obj_player.pos.x - (100 * cos(theta)),
			obj_player.pos.y - (100 * sin(theta)) };
	}

	Play::DrawCircle(targetPoint, 10, Play::cWhite);
	Play::DrawLine({targetPoint.x + 13, targetPoint.y}, 
		{targetPoint.x - 13, targetPoint.y}, Play::cWhite);
	Play::DrawLine({ targetPoint.x, targetPoint.y + 13 },
		{ targetPoint.x, targetPoint.y - 13 }, Play::cWhite);


	if (Play::GetMouseButton(Play::LEFT) && cursorReleased == true)
	{
		int id = Play::CreateGameObject(TYPE_AMMO, obj_player.pos, 5, "ammo");
		GameObject& obj_ammo = Play::GetGameObject(id);

		if (targetPoint.x >= obj_player.pos.x)
		{
			Play::SetGameObjectDirection(obj_ammo, 8, theta + PLAY_PI / 2);
		}
		else
		{
			Play::SetGameObjectDirection(obj_ammo, 8, theta - PLAY_PI/2);
		}
		cursorReleased = false;		
	}
	if (Play::GetMouseButton(Play::LEFT) == false)
	{
		cursorReleased = true;
	}
}

void UpdateAmmo()
{
	std::vector<int> vAmmo = Play::CollectGameObjectIDsByType(TYPE_AMMO);
	for (int id : vAmmo)
	{
		GameObject& obj_ammo = Play::GetGameObject(id);
		
		Play::UpdateGameObject(obj_ammo);
		Play::DrawObject(obj_ammo);

		if (!Play::IsVisible(obj_ammo))
		{
			Play::DestroyGameObject(id);
		}
	}
}

void UpdatePlayer()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_anchor = Play::GetGameObjectByType(TYPE_ANCHORPOINT);
	obj_player.scale = 100.;
	if (obj_player.pos.x >= 800)
	{
		Play::SetCameraPosition({ obj_player.pos.x - (DISPLAY_WIDTH / 2), 0 });
	}

	if (Play::IsLeavingDisplayArea(obj_player, Play::VERTICAL) && obj_player.pos.y > 700)
	{
		gameState.playerHP = 0;
	}
	if (gameState.playerHP <= 0)
	{
		gameState.playerState = STATE_DEAD;
	}

	switch (gameState.playerState)
	{
	case STATE_WALK:
		Play::SetSprite(obj_player, "shotgun_walk", 0.1f);
		HandlePlayerControls();
		DrawTarget();
		break;

	case STATE_IDLE:
		Play::SetSprite(obj_player, "scientist_idle", 0.33f);
		HandlePlayerControls();
		DrawTarget();
		break;

	case STATE_JUMP:
		HandlePlayerControls();
		Play::SetSprite(obj_player, "jump", 0);
		if (Play::IsColliding(obj_player, obj_anchor) && Play::KeyPressed(VK_SPACE))
			gameState.playerState = STATE_SWING;
		break;

	case STATE_SWING:
		SwingMechanic();
		break;

	case STATE_DEAD:
		obj_player.pos = gameState.startingPoint;
		Play::SetCameraPosition({ 0, 0 });
		gameState.playerState = STATE_JUMP;
		obj_player.velocity.x = 0;
		gameState.playerHP = 100;
		break;
	}

	if (gameState.direction)
	{
		DrawObjectYFlipped(obj_player);
	}
	else
	{
		Play::DrawObject(obj_player);
	}

	Play::UpdateGameObject(obj_player);
}

void UpdateEnemies()
{
	GameObject& obj_enemy = Play::GetGameObjectByType(TYPE_ENEMY);
	switch (gameState.enemyState)
	{
	case ENEMYSTATE_WALK:
		UpdateEnemyMovementOnPlatform();
		Play::SetSprite(obj_enemy, "enemy_idle", 0.333f);
		break;

	case ENEMYSTATE_WOUNDED:

		break;

	case ENEMYSTATE_DEAD:

		break;
	}
}


void CreateAnchor()
{
	Point2D AnchorPos = { 800, 20 };
	Play::CreateGameObject(TYPE_ANCHORPOINT, AnchorPos, 250, "anchor");
}

void UpdateAnchor()
{
	GameObject& obj_anchor = Play::GetGameObjectByType(TYPE_ANCHORPOINT);
	Play::UpdateGameObject(obj_anchor);
	Play::DrawObject(obj_anchor);
}

void CreateEnemies()
{
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	GameObject& obj_platform = Play::GetGameObject(vPlatforms.at(6));

	std::vector<int> vEnemies (1);
	for (int id : vEnemies)
	{
		id = Play::CreateGameObject(TYPE_ENEMY, { 0, 0 }, 25, "enemy");
		GameObject& obj_enemy = Play::GetGameObject(id);

		obj_enemy.velocity = { 2, 0 };
		obj_enemy.pos.y = obj_platform.pos.y - Play::GetSpriteHeight("enemy")/2;
		int init_distance = Play::RandomRoll(Play::GetSpriteWidth("platform")
			- Play::GetSpriteWidth("enemy"));
		obj_enemy.pos.x = obj_platform.pos.x + (Play::GetSpriteWidth("enemy") / 2) + init_distance;
	}

}

void UpdateEnemyMovementOnPlatform()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_enemy = Play::GetGameObjectByType(TYPE_ENEMY);
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	GameObject& obj_platform = Play::GetGameObject(vPlatforms.at(6));
	
	if (obj_enemy.pos.x <= (obj_platform.pos.x ) ||
		obj_enemy.pos.x >= (obj_platform.pos.x + Play::GetSpriteWidth("spr_platform")))
	{
		obj_enemy.velocity.x = -(obj_enemy.velocity.x);
	}

	if (Play::IsColliding(obj_enemy, obj_player))
	{
		gameState.playerHP -= 25;
	}

	Play::UpdateGameObject(obj_enemy);
	Play::DrawObject(obj_enemy);

	std::vector<int> vAmmo = Play::CollectGameObjectIDsByType(TYPE_AMMO);
	for (int id : vAmmo)
	{
		GameObject& obj_ammo = Play::GetGameObject(id);
		if (Play::IsColliding(obj_ammo, obj_enemy))
		{
			Play::DestroyGameObject(id);
			obj_enemy.type = TYPE_DEADENEMY;
		}
	}
}

void DrawPlatforms()
{
	Play::SetSpriteOrigin("platform_left", 0, 0);
	Play::SetSpriteOrigin("platform_inner", 0, 0);
	Play::SetSpriteOrigin("platform_right", 0, 0);
	std::vector<int> vPlatform = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	int Sprite1Width = Play::GetSpriteWidth("platform_left_edge");
	int Sprite2Width = Play::GetSpriteWidth("platform_inner_repeating");

	for (int id : vPlatform)
	{
		GameObject& obj_platform = Play::GetGameObject(id);
		Play::DrawSprite("platform_left_edge", obj_platform.pos, 0);
		Play::DrawSprite("platform_inner_repeating",
			{ obj_platform.pos.x + Sprite1Width, obj_platform.pos.y}, 0);
		Play::DrawSprite("platform_right_edge", 
			{ obj_platform.pos.x + Sprite1Width + Sprite2Width, obj_platform.pos.y }, 0);
	}
}

void DrawObjectYFlipped( GameObject& obj )
{
	Matrix2D flipMat = MatrixIdentity();
	flipMat.row[0].x = -1.0f;
	flipMat.row[2].x = obj.pos.x;
	flipMat.row[2].y = obj.pos.y;

	Play::DrawSpriteTransformed(obj.spriteId, flipMat, obj.frame);
}