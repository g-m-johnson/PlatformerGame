#pragma once

void UpdatePlayer();
void HandlePlayerControls();
bool PlayerAndPlatformCollision();
void SwingMechanic();



// Checks if player is pressing left button so can aim
void CheckForAiming();

// Draws a target a set distance away from the origin of the sprite
void AimProjectile();

Point2D DrawTarget(float);

// Updates and draws all active ammo
void UpdateAmmo();