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
};

enum PlayerState
{
	STATE_WALK = 0,
	STATE_JUMP,
	STATE_SWING,
	STATE_DEAD,
};

struct GameState
{
	Point2f startingPoint = { 120, 184 };
	PlayerState playerState = STATE_WALK;
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
void SwingMechanic();
void CreateAnchor();
void UpdateAnchor();
void DrawTarget();





// MAIN
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CreateGameObject(TYPE_PLAYER, gameState.startingPoint, 25, "player");
	Play::CentreSpriteOrigin("player");
	Play::CentreSpriteOrigin("anchor");
	CreatePlatforms();
	CreateAnchor();
}

bool MainGameUpdate(float elapsedTime)
{
	Play::ClearDrawingBuffer(Play::cYellow);
	UpdatePlatforms();
	TempCursorPos();
	UpdateAnchor();
	UpdatePlayer();
	Point2f camPos = Play::GetCameraPosition();
	Play::DrawFontText("64px", std::to_string(camPos.x) + ", " + std::to_string(camPos.y), {20, 20});
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

	if (obj_player.pos.x - obj_player.oldPos.x <= 0)
	{
		gameState.direction = true;
	}
	else
	{
		gameState.direction = false;
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

}

void CreatePlatforms()
{
	std::vector< std::vector<float>> platformPositions{
		{-250, 500},
		{70., 345.},
		{335., 196.},
		{950., 427.},
		{1230., 427.}
	};
	std::vector <int> vPlatforms(platformPositions.size());

	int n = 0;
	for (int id : vPlatforms)
	{
		int id = Play::CreateGameObject(TYPE_PLATFORM, { 0, 0 }, 0, "platform");
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
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);
		float platform_xmin = obj_platform.pos.x;
		float platform_xmax = obj_platform.pos.x + Play::GetSpriteWidth("platform");
		float platform_ymin = obj_platform.pos.y;
		float platform_ymax = obj_platform.pos.y + Play::GetSpriteHeight("platform");

		GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
		float player_xmin = obj_player.pos.x - (Play::GetSpriteWidth("player") / 2);
		float player_xmax = obj_player.pos.x + (Play::GetSpriteWidth("player") / 2);
		float player_ymin = obj_player.pos.y - (Play::GetSpriteHeight("player") / 2);
		float player_ymax = obj_player.pos.y + (Play::GetSpriteHeight("player") / 2);

		
		if (platform_xmin < player_xmax && platform_xmax > player_xmin &&
			platform_ymin < player_ymax && platform_ymax > player_ymin)
		{
			obj_player.acceleration.y = 0;
			obj_player.velocity.y = 0;
			obj_player.pos.y = obj_platform.pos.y - (Play::GetSpriteHeight("player")/2) + 5;
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
		if (gameState.direction == true)
		{
			obj_player.velocity.x = -4;
		}
		else
		{
			obj_player.velocity.x = 4;
		}
	}
}

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

	Play::DrawCircle(targetPoint, 10, Play::cBlack);
	Play::DrawLine({targetPoint.x + 13, targetPoint.y}, 
		{targetPoint.x - 13, targetPoint.y}, Play::cBlack);
	Play::DrawLine({ targetPoint.x, targetPoint.y + 13 },
		{ targetPoint.x, targetPoint.y - 13 }, Play::cBlack);


	if (Play::GetMouseButton(Play::LEFT))
	{
		int id = Play::CreateGameObject(TYPE_AMMO, obj_player.pos, 5, "ammo");
		GameObject& obj_ammo = Play::GetGameObject(id);
		Play::SetGameObjectDirection(obj_ammo, 8, theta);
	}
}

void UpdateAmmo()
{
	std::vector<int> vAmmo{ Play::CollectGameObjectIDsByType(TYPE_AMMO) };
	//for (int id:vAmmo)
}

void UpdatePlayer()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_anchor = Play::GetGameObjectByType(TYPE_ANCHORPOINT);

	if (obj_player.pos.x >= 800)
	{
		Play::SetCameraPosition({ obj_player.pos.x - (DISPLAY_WIDTH / 2), 0 });
	}

	if (Play::IsLeavingDisplayArea(obj_player, Play::VERTICAL) && obj_player.pos.y > 700)
	{
		gameState.playerState = STATE_DEAD;
	}

	switch (gameState.playerState)
	{
	case STATE_WALK:
		HandlePlayerControls();
		DrawTarget();
		break;

	case STATE_JUMP:
		HandlePlayerControls();

		if (Play::IsColliding(obj_player, obj_anchor) && Play::KeyPressed(VK_SPACE))
			gameState.playerState = STATE_SWING;
		break;

	case STATE_SWING:
		SwingMechanic();
		break;

	case STATE_DEAD:
		obj_player.pos = gameState.startingPoint;
		gameState.playerState = STATE_WALK;
		break;
	}

	Play::UpdateGameObject(obj_player);
	Play::DrawObject(obj_player);
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