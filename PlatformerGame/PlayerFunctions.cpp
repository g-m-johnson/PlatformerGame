#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "PlayerFunctions.h"
#include "MainGame.h"
#include "EnemyFunctions.h"

PlayerState playerState, resetPlayerState;


//------------------------------------------------------------------------------
bool playerVisible = true;
void UpdatePlayer()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	std::vector<int> vAnchors = Play::CollectGameObjectIDsByType(TYPE_ANCHORPOINT);

	if (obj_player.pos.x >= (DISPLAY_WIDTH/2) && obj_player.pos.x <= (5300 - DISPLAY_WIDTH/2))
	{
		Play::SetCameraPosition({ obj_player.pos.x - (DISPLAY_WIDTH / 2), 0 });
	}
	if (obj_player.pos.x >= 5300 - (DISPLAY_WIDTH / 2))
	{
		Play::SetCameraPosition({ 5300 - DISPLAY_WIDTH, 0 });
	}
	if(obj_player.pos.x < DISPLAY_WIDTH/2)
	{
		Play::SetCameraPosition({ 0, 0 });
	}
	



	if (playerState.state != STATE_DEAD && 
		Play::IsLeavingDisplayArea(obj_player, Play::VERTICAL) && obj_player.pos.y > 700)
	{
		playerState.playerHP = 0;
	}
	if (playerState.playerHP == 0)
	{
		playerState.state = STATE_DEAD;
	}

	Point2f camPos = Play::GetCameraPosition();

	switch (playerState.state)
	{

	case STATE_WALK:
		Play::SetSprite(obj_player, "spr_walk", 0.2f);
		HandlePlayerControls();
		break;


	case STATE_IDLE:
		Play::SetSprite(obj_player, "scientist_idle", 0.2f);
		HandlePlayerControls();
		CheckForAiming();
		break;


	case STATE_JUMP:
		HandlePlayerControls();
		Play::SetSprite(obj_player, "jump", 0);


		for (int id : vAnchors)
		{
			GameObject& obj_anchor = Play::GetGameObject(id);

			if (Play::IsColliding(obj_player, obj_anchor) && Play::KeyPressed(VK_SPACE))
			{
				playerState.state = STATE_SWING;
				Play::PlayAudio("jump.mp3");
				gamePlayState.noteObjectId = id;
			}
		}
		break;


	case STATE_SWING:

		for (int id : vAnchors)
		{
			if (id != gamePlayState.noteObjectId)
			{
				DrawRopeSwing(id, 1, 0);
			}
		}

		SwingMechanic();
		break;


	case STATE_DEAD:
		PlayerDeath();
		break;


	case STATE_THROW:

		AimProjectile();

		if (Play::IsAnimationComplete(obj_player))
		{
			playerState.state = STATE_IDLE;
		}

		break;


	case STATE_LEAVING:
		obj_player.velocity = { 0, 0 };
		if (playerVisible)
		{
			Play::SetSprite(obj_player, "exit_level", 0.2f);
		}
		if (Play::IsAnimationComplete(obj_player))
		{
			playerVisible = false;
			Play::SetSprite(obj_player, "blank", 0);
			Play::DrawFontText("151", "LEVEL COMPLETE!", { camPos.x + DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 - 60}, Play::CENTRE);

			Play::DrawFontText("151", "PRESS SPACEBAR TO PLAY AGAIN", { camPos.x + DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + 60 }, Play::CENTRE);
			if (Play::KeyPressed(VK_SPACE))
			{
				GameReset();
			}
		}
		break;
	}


	if (playerState.direction)
	{
		DrawObjectYFlipped(obj_player);
	}
	else
	{
		Play::DrawObject(obj_player);
	}

	Play::UpdateGameObject(obj_player);
}



//------------------------------------------------------------------------------

void HandlePlayerControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	if (obj_player.velocity.x != 0)
	{
		if (obj_player.pos.x - obj_player.oldPos.x < 0)
		{
			playerState.direction = true;
		}
		else
		{
			playerState.direction = false;
		}
	}

	bool onSolidGround = PlayerAndPlatformCollision();
	if (onSolidGround == false)
	{
		obj_player.acceleration.y = 0.8f;
		playerState.state = STATE_JUMP;
	}
	if (onSolidGround == true)
	{
		playerState.state = STATE_WALK;
		obj_player.velocity.x = 0;
		if (Play::KeyPressed('W'))
		{
			obj_player.velocity.y = -17;
		}
	}

	if (Play::KeyDown('A'))
	{
		obj_player.velocity.x = -4;
	}
	if (Play::KeyDown('D'))
	{
		obj_player.velocity.x = 4;
	}

	if (obj_player.velocity.x == 0 && obj_player.velocity.y == 0)
	{
		playerState.state = STATE_IDLE;
	}

	
	if (gamePlayState.damage_timer >= 2. && 
		gamePlayState.stopwatch - gamePlayState.damage_timer <= 2.)
	{
		playerState.hurt = true;
	}
	else
	{
		playerState.hurt = false;
	}
	
}


//------------------------------------------------------------------------------
/*
* Some very simple AABB collision stuff
*/

bool PlayerAndPlatformCollision()
{
	std::vector <int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	bool onPlatform = false;
	int walk_width = Play::GetSpriteWidth("walk");
	for (int id : vPlatforms)
	{
		GameObject& obj_platform = Play::GetGameObject(id);
		float platform_xmin = obj_platform.pos.x;
		float platform_xmax = obj_platform.pos.x + 180;
		float platform_ymin = obj_platform.pos.y;
		float platform_ymax = obj_platform.pos.y + 50;

		GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
		//Need to account that the sprites 'feet' are a lot smaller than the sprite width
		float player_xmin = obj_player.pos.x - (Play::GetSpriteWidth("walk") / 5);
		float player_xmax = obj_player.pos.x + (Play::GetSpriteWidth("walk") / 5);
		float player_ymin = obj_player.pos.y - (Play::GetSpriteHeight("walk") / 2);
		float player_ymax = obj_player.pos.y + (Play::GetSpriteHeight("walk") / 2);


		if (platform_xmin < player_xmax && platform_xmax > player_xmin &&
			platform_ymin < player_ymax && platform_ymax > player_ymin)
		{
			if (obj_player.velocity.y >= 0)
			{
				obj_player.acceleration.y = 0;
				obj_player.velocity.y = 0;
				obj_player.pos.y = obj_platform.pos.y - (Play::GetSpriteHeight("walk") / 2) + 5;
				onPlatform = true;
			}
			else 
			{
				obj_player.acceleration.y = 0.5;
				obj_player.velocity.y = -obj_player.velocity.y;
			}
		}
	}
	return onPlatform;
}


//------------------------------------------------------------------------------
/*
* 
*/

void SwingMechanic()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_anchor = Play::GetGameObject(gamePlayState.noteObjectId);
	//get index of this point
	//loop through all other rope swings and draw them stationary

	int ropeLength = obj_anchor.radius + obj_player.radius + 10;
	float dx = obj_anchor.pos.x - obj_player.pos.x;
	float dy = obj_player.pos.y - obj_anchor.pos.y;
	float theta = atan(dy / dx);
	float phi = atan(dx / dy);

	if (!Play::IsColliding(obj_player, obj_anchor))
	{
		DrawRopeSwing(gamePlayState.noteObjectId, 2, phi);
		obj_player.velocity = { 0, 0 };
		obj_player.acceleration = { 0, 0 };
		if (dy < 100)
		{
			playerState.direction = !playerState.direction;
		}

		if (playerState.direction == false)
		{
			theta += 0.02f;
		}
		else
		{
			theta -= 0.02f;
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
	else
	{
		DrawRopeSwing(gamePlayState.noteObjectId, 1, phi);
	}


	if (Play::KeyPressed(VK_SPACE))
	{
		playerState.state = STATE_JUMP;
		if (playerState.direction)
		{
			obj_player.velocity.x = -6;
		}
		else
		{
			obj_player.velocity.x = 6;
		}
	}
}



//------------------------------------------------------------------------------
/*
* DEFENSE FUNCTIONS
*/

void CheckForAiming()
{
	if (Play::GetMouseButton(Play::LEFT))
	{
		playerState.state = STATE_THROW;
	}
}

bool aiming = false;
void AimProjectile()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D mousePos = Play::GetMousePos();
	Point2D camPos = Play::GetCameraPosition();
	float dx = (camPos.x + mousePos.x) - obj_player.pos.x;
	float dy = mousePos.y - obj_player.pos.y;
	float theta = atan((dy) / (dx));

	if (playerState.direction)
	{
		if ((camPos.x + mousePos.x) > obj_player.pos.x)
		{
			if (mousePos.y > obj_player.pos.y)
				theta = PLAY_PI / 2;
			else
				theta = 3 * PLAY_PI / 2;
		}
	}
	else
	{
		if ((camPos.x + mousePos.x) < obj_player.pos.x)
		{
			if (mousePos.y > obj_player.pos.y)
				theta = 3 * PLAY_PI / 2;
			else
				theta = PLAY_PI / 2;
		}
	}


	// I got very confused about angles here
	Point2D targetPoint = { obj_player.pos.x + (100 * cos(theta)),
			obj_player.pos.y + (100 * sin(theta)) };
	
	if ((mousePos.x + camPos.x) < obj_player.pos.x)
	{
		targetPoint = { obj_player.pos.x - (100 * cos(theta)),
			obj_player.pos.y - (100 * sin(theta)) };
	}
	


	// Target only appears when left mouse button is pressed
	if (Play::GetMouseButton(Play::LEFT))
	{
		// Drawing target to screen
		Play::DrawCircle(targetPoint, 10, Play::cWhite);
		Play::DrawLine({ targetPoint.x + 13, targetPoint.y },
			{ targetPoint.x - 13, targetPoint.y }, Play::cWhite);
		Play::DrawLine({ targetPoint.x, targetPoint.y + 13 },
			{ targetPoint.x, targetPoint.y - 13 }, Play::cWhite);

		obj_player.frame = 1;
		Play::SetSprite(obj_player, "throw", 0);

		aiming = true;
	}

	// Ammo thrown after left mouse button is released
	if (!Play::GetMouseButton(Play::LEFT) && aiming == true)
	{
		int id = Play::CreateGameObject(TYPE_AMMO, obj_player.pos, 5, "round_bottle_red");
		GameObject& obj_ammo = Play::GetGameObject(id);
		obj_ammo.rotSpeed = 0.1f;

		Play::SetSprite(obj_player, "throw", 0.33f);

		if (targetPoint.x >= obj_player.pos.x)
		{
			Play::SetGameObjectDirection(obj_ammo, 8, theta + PLAY_PI / 2);
		}
		else
		{
			Play::SetGameObjectDirection(obj_ammo, 8, theta - PLAY_PI / 2);
		}
		aiming = false;
	}

}

void UpdateAmmo()
{
	std::vector<int> vAmmo = Play::CollectGameObjectIDsByType(TYPE_AMMO);
	for (int id : vAmmo)
	{
		GameObject& obj_ammo = Play::GetGameObject(id);

		Play::UpdateGameObject(obj_ammo);
		Play::DrawObjectRotated(obj_ammo);

		if (!Play::IsVisible(obj_ammo))
		{
			Play::DestroyGameObject(id);
		}
	}
}



//------------------------------------------------------------------------------


void PlayerDeath()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D camPos = Play::GetCameraPosition();
	obj_player.velocity.x = 0;
	Play::SetSprite(obj_player, "die", 0.2f);

	if (Play::IsAnimationComplete(obj_player))
	{
		obj_player.frame = 5;
		obj_player.animSpeed = 0.0f;
		Play::DrawFontText("132", "PRESS SPACEBAR TO RESTART",
			{ (camPos.x + DISPLAY_WIDTH / 2), DISPLAY_HEIGHT / 2 }, Play::CENTRE);

		if (Play::KeyPressed(VK_SPACE))
		{
			GameReset();
		}
	}

}