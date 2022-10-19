#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "PlayerFunctions.h"
#include "MainGame.h"
#include "EnemyFunctions.h"

EnemyState enemyState, resetEnemyState;

void CreateEnemies()
{
	std::vector<int> vPlatforms = Play::CollectGameObjectIDsByType(TYPE_PLATFORM);
	Play::SetSpriteOrigin("enemy", 62, 68);

	int n = 0;
	std::vector<int> vEnemies(enemyState.platformNumbers.size());
	for (int id : vEnemies)
	{
		id = Play::CreateGameObject(TYPE_ENEMY, { 0, 0 }, 40, "enemy");
		GameObject& obj_enemy = Play::GetGameObject(id);
		GameObject& obj_platform = Play::GetGameObject(vPlatforms.at(enemyState.platformNumbers.at(n)));
		obj_enemy.associatedPlatformId = obj_platform.GetId();

		obj_enemy.velocity = { 2, 0 };
		obj_enemy.pos.y = obj_platform.pos.y - Play::GetSpriteHeight("enemy") / 1.5f;
		int init_distance = Play::RandomRoll(Play::GetSpriteWidth("platform")
			- Play::GetSpriteWidth("enemy"));
		obj_enemy.pos.x = obj_platform.pos.x + (Play::GetSpriteWidth("enemy") / 2) + init_distance;

		n++;
	}
}





void UpdateEnemies()
{
	std::vector<int> vEnemies = Play::CollectGameObjectIDsByType(TYPE_ENEMY);

	for (int id : vEnemies)
	{
		GameObject& obj_enemy = Play::GetGameObject(id);
		Play::SetSprite(obj_enemy, "enemy_idle", 0.333f);


		Play::UpdateGameObject(obj_enemy);


		if (obj_enemy.velocity.x > 0)
		{
			DrawObjectYFlipped(obj_enemy);
		}
		else
		{
			Play::DrawObject(obj_enemy);
		}

		UpdateEnemyMovement(obj_enemy);

		std::vector<int> vAmmo = Play::CollectGameObjectIDsByType(TYPE_AMMO);
		for (int id : vAmmo)
		{
			GameObject& obj_ammo = Play::GetGameObject(id);
			if (Play::IsColliding(obj_ammo, obj_enemy))
			{
				Play::DestroyGameObject(id);
				Play::DestroyGameObject(obj_enemy.GetId());
			}
		}
	}
	
}





void UpdateEnemyMovement(GameObject& obj_enemy)
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_platform = Play::GetGameObject(obj_enemy.associatedPlatformId);

	if (obj_enemy.pos.x <= (obj_platform.pos.x) ||
		obj_enemy.pos.x >= (obj_platform.pos.x + 180))
	{
		obj_enemy.velocity.x = -(obj_enemy.velocity.x);
	}

	if (Play::IsColliding(obj_enemy, obj_player)
		&& playerState.playerHP > 0 && !playerState.hurt)
	{
		playerState.playerHP -= 1;
		gamePlayState.damage_timer = gamePlayState.stopwatch;
	}
}
