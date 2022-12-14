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

		// Place enemy at random distance along platform
		int init_distance = Play::RandomRoll(Play::GetSpriteWidth("platform")
			- Play::GetSpriteWidth("enemy"));
		obj_enemy.pos.x = obj_platform.pos.x + (Play::GetSpriteWidth("enemy") / 2) + init_distance;

		n++;
	}
}


//------------------------------------------------------------------------------
void UpdateEnemies()
{
	std::vector<int> vEnemies = Play::CollectGameObjectIDsByType(TYPE_ENEMY);
	bool hasEnemyBeenHit = false;

	UpdateEnemyChunks();

	for (int id_enemy : vEnemies)
	{
		GameObject& obj_enemy = Play::GetGameObject(id_enemy);
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
		for (int id_ammo : vAmmo)
		{
			GameObject& obj_ammo = Play::GetGameObject(id_ammo);
			// Breaks if enemy is hit when more than one ammo objects exist
			if (Play::IsColliding(obj_ammo, obj_enemy))
			{
				Play::DestroyGameObject(id_ammo);
				hasEnemyBeenHit = true;
			}
		}

		if (hasEnemyBeenHit)
		{
			CreateEnemyChunks(obj_enemy);
			Play::DestroyGameObject(id_enemy);
			Play::PlayAudio("explosion.mp3");
			hasEnemyBeenHit = false;
		}
	}
	
}



//------------------------------------------------------------------------------


void UpdateEnemyMovement(GameObject& obj_enemy)
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	GameObject& obj_platform = Play::GetGameObject(obj_enemy.associatedPlatformId);

	if (obj_enemy.pos.x <= (obj_platform.pos.x) ||
		obj_enemy.pos.x >= (obj_platform.pos.x + 180))
	{
		obj_enemy.velocity.x = -(obj_enemy.velocity.x);
	}

	if (Play::IsColliding(obj_enemy, obj_player) && playerState.playerHP > 0 && 
		!playerState.hurt && playerState.state != STATE_SWING)
	{
		playerState.playerHP -= 1;
		Play::PlayAudio("water-bleep.mp3");
		gamePlayState.damage_timer = gamePlayState.stopwatch;
	}
}


//------------------------------------------------------------------------------


void CreateEnemyChunks(GameObject& obj_enemy)
{
	//looks better to create 'chunks' using a random angle rather than uniformly
	//  every pi / 6
	float i = 0.0f;
	while(i <= 2*PLAY_PI)
	{
		int id = Play::CreateGameObject(TYPE_CHUNKS, { obj_enemy.pos }, 0, "purple_body");
		GameObject& obj_chunk = Play::GetGameObject(id);

		if (Play::RandomRoll(3) == 3)
		{
			Play::SetSprite(obj_chunk, "pink_body", 0.0f);
		}

		Play::SetGameObjectDirection(obj_chunk, 6, i );
		obj_chunk.acceleration.y = 0.5f;

		i += static_cast<float>(Play::RandomRoll(100)) / 100.0f;
	}
}

void UpdateEnemyChunks()
{
	std::vector<int> vChunks = Play::CollectGameObjectIDsByType(TYPE_CHUNKS);
	for (int id : vChunks)
	{
		GameObject& obj_chunk = Play::GetGameObject(id);
		Play::UpdateGameObject(obj_chunk);
		Play::DrawObject(obj_chunk);

		if (!Play::IsVisible(obj_chunk))
		{
			Play::DestroyGameObject(id);
		}
	}
}