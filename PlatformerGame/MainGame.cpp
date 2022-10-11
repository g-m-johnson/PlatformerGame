#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "MainGame.h"
#include "PlayerFunctions.h"
#include "EnemyFunctions.h"


GamePlayState gamePlayState, resetGame;
PlayerState playerState, resetPlayerState;

// MAIN
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Background\\lab_background.png");
	Play::CreateGameObject(TYPE_PLAYER, playerState.startingPoint, 50, "player");
	Play::CentreAllSpriteOrigins();
	CreatePlatforms();
	CreateAnchor();
	CreateEnemies();
}

bool MainGameUpdate(float elapsedTime)
{
	Play::DrawBackground();
	gamePlayState.stopwatch += elapsedTime;
	DrawPlatforms();
	UpdatePlatforms();
	//TempCursorPos();
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

