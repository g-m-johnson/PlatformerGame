#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "PlayerFunctions.h"
#include "MainGame.h"
#include "EnemyFunctions.h"

PlayerState playerState, resetPlayerState;

void UpdatePlayer()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_anchor = Play::GetGameObjectByType(TYPE_ANCHORPOINT);

	if (obj_player.pos.x >= (DISPLAY_WIDTH/2))
	{
		Play::SetCameraPosition({ obj_player.pos.x - (DISPLAY_WIDTH / 2), 0 });
	}
	else
	{
		Play::SetCameraPosition({ 0, 0 });
	}

	if (playerState.state != STATE_DEAD && 
		Play::IsLeavingDisplayArea(obj_player, Play::VERTICAL) 
		&& obj_player.pos.y > 700)
	{
		playerState.playerHP = 0;
	}
	if (playerState.playerHP == 0)
	{
		playerState.state = STATE_DEAD;
	}

	switch (playerState.state)
	{
	case STATE_WALK:
		Play::SetSprite(obj_player, "shotgun_walk", 0.33f);
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
			playerState.state = STATE_SWING;
		break;

	case STATE_SWING:
		SwingMechanic();
		break;

	case STATE_DEAD:
		obj_player.velocity.x = 0;
		Play::ColourSprite("walk", Play::cRed);
		Play::ColourSprite("scientist_idle", Play::cRed);	
		Point2f camPos = Play::GetCameraPosition();
		Play::DrawFontText("132", "PRESS SPACEBAR TO RESTART", 
			{(camPos.x + DISPLAY_WIDTH/2), DISPLAY_HEIGHT/2}, Play::CENTRE);
		
		if (Play::KeyPressed(VK_SPACE))
		{
			playerState = resetPlayerState;
			obj_player.pos = playerState.startingPoint;
			Play::SetCameraPosition({ 0, 0 });

			gamePlayState = resetGame;
			enemyState = resetEnemyState;
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
		obj_player.acceleration.y = .8;
		playerState.state = STATE_JUMP;
	}
	if (onSolidGround == true)
	{
		playerState.state = STATE_WALK;
		obj_player.velocity.x = 0;
		if (Play::KeyPressed(VK_UP))
		{
			obj_player.velocity.y = -17;
		}
	}

	if (Play::KeyDown(VK_LEFT))
	{
		obj_player.velocity.x = -7;
	}
	if (Play::KeyDown(VK_RIGHT))
	{
		obj_player.velocity.x = 7;
	}

	if (obj_player.velocity.x == 0 && obj_player.velocity.y == 0)
	{
		playerState.state = STATE_IDLE;
	}


	if (gamePlayState.damage_timer >= 2. && gamePlayState.stopwatch - gamePlayState.damage_timer <= 2.)
	{
		Play::ColourSprite("walk", Play::cRed);
		Play::ColourSprite("scientist_idle", Play::cRed);
		playerState.hurt = true;
	}
	else
	{
		Play::ColourSprite("walk", Play::cWhite);
		Play::ColourSprite("scientist_idle", Play::cWhite);
		playerState.hurt = false;
	}
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
		//Need to account that the sprites 'feet' are a lot smaller than the sprite
		float player_xmin = obj_player.pos.x - (Play::GetSpriteWidth("walk") / 5);
		float player_xmax = obj_player.pos.x + (Play::GetSpriteWidth("walk") / 5);
		float player_ymin = obj_player.pos.y - (Play::GetSpriteHeight("walk") / 2);
		float player_ymax = obj_player.pos.y + (Play::GetSpriteHeight("walk") / 2);


		if (platform_xmin < player_xmax && platform_xmax > player_xmin &&
			platform_ymin < player_ymax && platform_ymax > player_ymin)
		{
			obj_player.acceleration.y = 0;
			obj_player.velocity.y = 0;
			obj_player.pos.y = obj_platform.pos.y - (Play::GetSpriteHeight("walk") / 2) + 5;
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
			playerState.direction = !playerState.direction;
		}

		if (playerState.direction == false)
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
		playerState.state = STATE_JUMP;
		if (playerState.direction)
		{
			obj_player.velocity.x = -4;
		}
		else
		{
			obj_player.velocity.x = 4;
		}
	}
}