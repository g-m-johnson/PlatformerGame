#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "PlayerFunctions.h"
#include "MainGame.h"
#include "EnemyFunctions.h"

EnemyState enemyState, resetEnemyState;

void CreateEnemies()
{
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	GameObject& obj_platform = Play::GetGameObject(vPlatforms.at(6));
	Play::SetSpriteOrigin("enemy", 62, 68);


	std::vector<int> vEnemies(1);
	for (int id : vEnemies)
	{
		id = Play::CreateGameObject(TYPE_ENEMY, { 0, 0 }, 40, "enemy");
		GameObject& obj_enemy = Play::GetGameObject(id);

		obj_enemy.velocity = { 2, 0 };
		obj_enemy.pos.y = obj_platform.pos.y - Play::GetSpriteHeight("enemy") / 1.5;
		int init_distance = Play::RandomRoll(Play::GetSpriteWidth("platform")
			- Play::GetSpriteWidth("enemy"));
		obj_enemy.pos.x = obj_platform.pos.x + (Play::GetSpriteWidth("enemy") / 2) + init_distance;
	}
}





void UpdateEnemies()
{
	GameObject& obj_enemy = Play::GetGameObjectByType(TYPE_ENEMY);
	Play::SetSprite(obj_enemy, "enemy_idle", 0.333f);
	Play::ColourSprite("enemy_idle", Play::cWhite);

	if (obj_enemy.velocity.x != 0)
	{
		if (obj_enemy.pos.x - obj_enemy.oldPos.x < 0)
		{
			enemyState.direction = true;
		}
		else
		{
			enemyState.direction = false;
		}
	}
	else
	{
		obj_enemy.velocity.x = 2;
	}

	switch (enemyState.state)
	{
	case STATE_WALK:
		UpdateEnemyMovement();
		break;

	case STATE_WOUNDED:
		UpdateEnemyMovement();

		if (gamePlayState.stopwatch - gamePlayState.init_time <= 1.)
		{
			Play::ColourSprite("enemy_idle", Play::cRed);
		}
		break;

	case STATE_DEAD:
		obj_enemy.velocity.x = 0;
		break;
	}

	if (enemyState.direction && enemyState.state != STATE_DEAD)
	{
		Play::DrawObject(obj_enemy);
	}
	if (!enemyState.direction && enemyState.state != STATE_DEAD)
	{
		DrawObjectYFlipped(obj_enemy);
	}
	else
	{
		Play::DrawObjectTransparent(obj_enemy, 0.0);
	}

	Play::UpdateGameObject(obj_enemy);

	
}





void UpdateEnemyMovement()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_enemy = Play::GetGameObjectByType(TYPE_ENEMY);
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	GameObject& obj_platform = Play::GetGameObject(vPlatforms.at(6));

	if (obj_enemy.pos.x <= (obj_platform.pos.x) ||
		obj_enemy.pos.x >= (obj_platform.pos.x + Play::GetSpriteWidth("spr_platform")))
	{
		obj_enemy.velocity.x = -(obj_enemy.velocity.x);
	}

	if (Play::IsColliding(obj_enemy, obj_player) && enemyState.state != STATE_DEAD 
		&& playerState.playerHP > 0 && !playerState.hurt)
	{
		playerState.playerHP -= 1;
		gamePlayState.damage_timer = gamePlayState.stopwatch;
	}

	std::vector<int> vAmmo = Play::CollectGameObjectIDsByType(TYPE_AMMO);
	for (int id : vAmmo)
	{
		GameObject& obj_ammo = Play::GetGameObject(id);
		if (Play::IsColliding(obj_ammo, obj_enemy) && enemyState.state != STATE_DEAD)
		{
			Play::DestroyGameObject(id);
			gamePlayState.init_time = gamePlayState.stopwatch;
			enemyState.enemyHP -= 25;
			enemyState.state = STATE_WOUNDED;
		}
	}

	if (enemyState.enemyHP <= 0)
	{
		enemyState.state = STATE_DEAD;
	}
}
