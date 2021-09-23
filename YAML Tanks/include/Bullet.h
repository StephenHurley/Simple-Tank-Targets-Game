#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CollisionDetector.h"
#include "Globals.h"
#include "LevelLoader.h"

// needed by "playerCollision" function, but cant be used. Causes multiple compiler errors
//#include "Tank.h"

class Bullet
{
public:

	Bullet();

	// finds start point of bullets flight path
	void findStartPoint(double t_turretRotation, sf::Vector2f t_turretSpritePos);

	/// <summary>
	/// @brief updates bullet
	/// dt is used for travel speed, reference of wall sprites is used for bullet wall collision,
	/// reference of target sprites used for bullet target collision, reference of target data is used to toggle
	/// active status for targets when they are hit by a bullet
	/// </summary>
	/// <param name="dt"></param>
	/// <param name="t_wallSprites"></param>
	/// <param name="t_targetSprites"></param>
	/// <param name="t_targets"></param>
	void update(double dt);

	void render(sf::RenderWindow& window);

	void setPosition(sf::Vector2f& position);

	// keys if player has pressed space to fire bullet
	void handleKeyInputs();

	// checks for wall collisions with bullet
	void wallCollision(std::vector<sf::Sprite>& t_wallSprites);
	// checks for target collisons with bullet and toggles target active status
	void targetCollision(std::vector<sf::Sprite>& t_targetSprites, std::vector<TargetData>& t_targets);


	// BROKEN
	// checks for player collisions with bullet (ai tank only)
	/*void playerCollision(Tank& playerTank);*/

	// checks if the bullet has gone off screen aka game world
	bool checkGameWorldBoundaries();

	// init bullets, called in tank constructor 
	void initBullet(sf::Texture const& texture);

	// load and init sound files
	void initSounds();


	// control bools
	bool m_canFire = true; // allows a bullet to be fired
	bool m_firing = false; // bullet is being fired, find starting position while this is true, make false again after starting position is found
	bool m_traveling = false;


private:

	sf::Sprite m_bulletSprite;
	sf::Texture m_texture;

	// sound for bullet hiting a wall
	sf::SoundBuffer m_wallHitBuffer;
	sf::Sound m_wallHitSound;

	// start point of the bullet's path
	sf::Vector2f m_startPoint;

	// off screen position of projectile so an unactive projectile does not mess with the game world
	sf::Vector2f m_offScreenPosition = { 3000.0f, 3000.0f };

	double const PROJECTILE_SPEED = 200;

};