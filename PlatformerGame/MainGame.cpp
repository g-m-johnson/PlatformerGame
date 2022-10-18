#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "MainGame.h"
#include "PlayerFunctions.h"
#include "EnemyFunctions.h"


GamePlayState gamePlayState, resetGame;


// MAIN
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Background\\lab_background.png");
	Play::CreateGameObject(TYPE_PLAYER, playerState.startingPoint, 50, "player");
	Play::CentreAllSpriteOrigins();
	Play::StartAudioLoop("Machine-Madness");
	CreatePlatforms();
	CreateCollectables();
	CreateAnchor();
	CreateEnemies();
	CreateExitObjects();
}

bool MainGameUpdate(float elapsedTime)
{
	Play::DrawBackground();
	gamePlayState.stopwatch += elapsedTime;
	Play::DrawSprite("door", {160,278}, 0);
	DrawPlatforms();
	
	UpdateCollectables();
	UpdateEnemies();	
	UpdateRopeSwing();
	UpdateExitObjects();
	UpdatePlayer();
	UpdateAmmo();

	TempCursorPos();
	HandleUI();
	ControlScreen();
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}


// FUNCTIONS
void CreatePlatforms()
{
	std::vector<Point2D> platformPositions
	{
		{70, 360},
		{335, 196},
		{950, 427},
		{1230, 427},
		{1630, 680},
		{1920, 680},
		{1260, 680},
		{2100, 520},
		{2280, 360},
		{1575, 90},
		{2465, 360},
		{3110, 400},
		{3110, 665},
		{3750, 293}
	};
	std::vector <int> vPlatforms(platformPositions.size());

	int n = 0;
	for (int id : vPlatforms)
	{
		int id = Play::CreateGameObject(TYPE_PLATFORM, { 0, 0 }, 0, "platform_left_edge");
		GameObject& obj_platform = Play::GetGameObject(id);
		obj_platform.pos = platformPositions.at(n);
		n++;
	}
}

// Draws the coordinates of the mouse right next to the cursor
// For ease of placing platforms, etc.
void TempCursorPos()
{
	Point2D mousePos = Play::GetMousePos();
	Point2D camPos = Play::GetCameraPosition();
	Point2D textPos = { mousePos.x + camPos.x, mousePos.y + camPos.y };
	Play::DrawFontText("64px", std::to_string(camPos.x + mousePos.x) + ", " + std::to_string(mousePos.y), textPos);
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

	// Drawing target to screen
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

	// Ensuring that multiple bullets aren't released in one cursor press;
	// can only shoot if cursor has been released since last time.
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






void CreateAnchor()
{
	std::vector<Point2D> anchorPositions
	{
		{ 800, -10 },
		{2080, -10},
		{2914, -10},
		{3450, -10}
	};
	std::vector<int> vAnchors(anchorPositions.size());
	int n = 0;
	for (int id : vAnchors)
	{
		id = Play::CreateGameObject(TYPE_ANCHORPOINT, anchorPositions.at(n), 250, "anchor");
		n++;
	}
}


//Platform sprites come in separate sections. This function is just to draw them all put together.
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


//Draws object flipped in the Y axis
void DrawObjectYFlipped( GameObject& obj )
{
	Matrix2D flipMat = MatrixIdentity();
	flipMat.row[0].x = -1.0f;
	flipMat.row[2].x = obj.pos.x;
	flipMat.row[2].y = obj.pos.y;

	Play::DrawSpriteTransformed(obj.spriteId, flipMat, obj.frame);
}


void CreateCollectables()
{
	Play::SetSpriteOrigin("flask", 36, 73);
	std::vector<Point2D> flaskPositions
	{
		//{1215, 600},
		//{3200, 600}
	};
	std::vector<int> vFlasks(flaskPositions.size());
	int n = 0;
	for (int id : vFlasks)
	{
		id = Play::CreateGameObject(TYPE_FLASK, { 0, 0 }, 30, "flask");
		GameObject& obj_flask = Play::GetGameObject(id);
		obj_flask.pos = flaskPositions.at(n);
		n++;
	}

	std::vector<Point2D> healthPositions
	{
		{1600, 47}
	};
	std::vector<int> vHealth(healthPositions.size());
	int m = 0;
	for (int id : vHealth)
	{
		id = Play::CreateGameObject(TYPE_HEALTH, { 0, 0 }, 30, "medkit");
		GameObject& obj_health = Play::GetGameObject(id);
		obj_health.pos = healthPositions.at(m);
		m++;
	}
}

void UpdateCollectables()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	std::vector <int> vFlasks = Play::CollectGameObjectIDsByType(TYPE_FLASK);
	for (int id : vFlasks)
	{
		GameObject& obj_flask = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_flask);
		Play::DrawObject(obj_flask);

		if (Play::IsColliding(obj_player, obj_flask))
		{
			Play::DestroyGameObject(id);
			playerState.playerXP++;
		}
	}
	if (vFlasks.size() == 0)
	{
		playerState.exitActive = true;
	}

	//Health adds to player HP
	std::vector <int> vHealth = Play::CollectGameObjectIDsByType(TYPE_HEALTH);
	for (int id : vHealth)
	{
		GameObject& obj_health = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_health);
		Play::DrawObject(obj_health);

		if (Play::IsColliding(obj_player, obj_health) && playerState.playerHP <= 100)
		{
			Play::DestroyGameObject(id);
			if (playerState.playerHP <= 2)
			{
				playerState.playerHP += 1;
			}
			else
			{
				playerState.playerHP = 3;
			}
		}
	}
}

void HandleUI()
{
	Point2D camPos = Play::GetCameraPosition();
	std::vector<Point2D> vHearts
	{
		{camPos.x + 120, camPos.y + 50},
		{camPos.x + 170, camPos.y + 50},
		{camPos.x + 220, camPos.y + 50}
	};

	Play::DrawFontText("64", "LIVES: ", { camPos.x + 20, camPos.y + 50 });

	for (int i = 0; i < playerState.playerHP; i++)
	{
		Play::DrawSprite("heart", vHearts.at(i), 0);
	}
}

void UpdateRopeSwing()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	std::vector<int> vAnchors = Play::CollectGameObjectIDsByType(TYPE_ANCHORPOINT);
	for (int id : vAnchors)
	{
		if (playerState.state != STATE_SWING)
		{
			DrawRopeSwing(id, 1, 0);
		}
	}
}

void ControlScreen()
{
	if (Play::KeyDown('I'))
	{
		Play::DrawRect({ Play::GetCameraPosition().x + 100, 100 }, { Play::GetCameraPosition().x + 1180, 620 }, Play::cBlack, true);
		Play::DrawFontText("132", "CONTROLS:", { Play::GetCameraPosition().x + 640, 200 }, Play::CENTRE);
		Play::DrawFontText("64", "A/D TO WALK", { Play::GetCameraPosition().x + 640, 300 }, Play::CENTRE);
		Play::DrawFontText("64", "W TO JUMP", { Play::GetCameraPosition().x + 640, 400 }, Play::CENTRE);
		Play::DrawFontText("64", "SPACEBAR WHEN NEAR ROPE TO SWING", { Play::GetCameraPosition().x + 640, 500 }, Play::CENTRE);
	}
}


void CreateExitObjects()
{
	Play::CreateGameObject(TYPE_DOOR, { 2555, 278 }, 20, "door");

	int id = Play::CreateGameObject(TYPE_COMPUTER, { 2370, 300 }, 50, "computer");
	GameObject& obj_computer = Play::GetGameObject(id);
	Play::SetSprite(obj_computer, "computer", 0.1f);
}

bool hasCollided = false;
void UpdateExitObjects()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_door = Play::GetGameObjectByType(TYPE_DOOR);
	GameObject& obj_computer = Play::GetGameObjectByType(TYPE_COMPUTER);
	Play::UpdateGameObject(obj_door);
	Play::DrawObject(obj_door);
	Play::UpdateGameObject(obj_computer);
	Play::DrawObject(obj_computer);

	if (playerState.exitActive && Play::IsColliding(obj_computer, obj_player) 
		&& !hasCollided)
	{
		Play::SetSprite(obj_door, "door", 0.2f);
		hasCollided = true;
	}

	if (hasCollided)
	{
		obj_player.radius = 20;
		if (Play::IsAnimationComplete(obj_door))
		{
			obj_door.frame = 7;
			Play::SetSprite(obj_door, "door", 0);
		}

		if (Play::IsAnimationComplete(obj_door) && 
			Play::IsColliding(obj_player, obj_door))
		{
			playerState.state = STATE_LEAVING;
		}
	}
}
//------------------------------------------------------------------------------