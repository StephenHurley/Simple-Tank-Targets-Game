#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CollisionDetector.h"
#include "Bullet.h"
#include "Thor/Time.hpp"

/// <summary>
/// @brief A simple tank controller.
/// 
/// This class will manage all tank movement and rotations.
/// </summary>
class Tank
{
public:
	/// <summary>
/// @brief Constructor that stores drawable state (texture, sprite) for the tank.
/// Stores references to the texture and container of wall sprites. 
/// Creates sprites for the tank base and turret from the supplied texture.
/// </summary>
/// <param name="t_texture">A reference to the sprite sheet texture</param>
///< param name="t_wallSprites">A reference to the container of wall sprites</param>  
	Tank(sf::Texture const& texture, std::vector<sf::Sprite>& t_wallSprites,
		std::vector<sf::Sprite>& t_targetSprites, std::vector<TargetData>& t_targets);

	void update(double dt);
	void render(sf::RenderWindow& window);
	void setPosition(sf::Vector2f& position);
	/// <summary>
/// @brief Increases the speed by 1, max speed is capped at 100.
/// 
/// </summary>
	void increaseSpeed();

	/// <summary>
	/// @brief Decreases the speed by 1, min speed is capped at -100.
	/// 
	/// </summary>
	void decreaseSpeed();

	/// <summary>
	/// @brief Increases the rotation by 1 degree, wraps to 0 degrees after 359.
	/// 
	/// </summary>
	void increaseRotation();

	/// <summary>
	/// @brief Decreases the rotation by 1 degree, wraps to 359 degrees after 0.
	/// 
	/// </summary>
	void decreaseRotation();

	// checks for player input and calls appropriate function, such as pressing up key to increase speed
	void handleKeyInput();

	//indepently rotate the turret without affecting the tank base
	void increaseTurretRotation();
	void decreaseTurretRotation();

	//recenter the turret to allign with tank base
	void centreTurret();

/// <summary>
/// @brief Checks for collisions between the tank and the walls.
/// 
/// </summary>
/// <returns>True if collision detected between tank and wall.</returns>
	bool checkWallCollision();

	/// <summary>
/// @brief Stops the tank if moving and applies a small increase in speed in the opposite direction of travel.
/// If the tank speed is currently 0, the rotation is set to a value that is less than the previous rotation value
///  (scenario: tank is stopped and rotates into a wall, so it gets rotated towards the opposite direction).
/// If the tank is moving, further rotations are disabled and the previous tank position is restored.
/// The tank speed is adjusted so that it will travel slowly in the opposite direction. The tank rotation 
///  is also adjusted as above if necessary (scenario: tank is both moving and rotating, upon wall collision it's 
///  speed is reversed but with a smaller magnitude, while it is rotated in the opposite direction of it's 
///  pre-collision rotation).
/// </summary>
	void deflect();


	/// <summary>
/// @brief Reads the player tank position.
/// <returns>The tank base position.</returns>
/// </summary>
	sf::Vector2f getPosition() const;

	sf::Sprite getBase() const;
	sf::Sprite getTurret() const;

	// fuel in the player tank
	double m_fuel = 3000.0f;

	bool m_damaged = false;

	Bullet m_bullets[NUM_PLAYER_BULLETS];
	float m_bulletsFired = 0;
	float m_bulletsFiredHighScore = 0;

private:
	void initSprites();
	void initSounds();
	void adjustRotation();
	void gradualStop();
	void processBullets(double dt);

	sf::Sprite m_tankBase;
	sf::Sprite m_turret;
	sf::Texture const& m_texture;

	sf::SoundBuffer m_tankFiringBuffer;
	sf::Sound m_tankFiringSound;

	sf::SoundBuffer m_tankMovingBuffer;
	sf::Sound m_tankMovingSound;
	bool m_toggleMovingSound = false;
	bool m_stopped = true;

	void soundControl();

	// A reference to the container of wall sprites.
	std::vector<sf::Sprite>& m_wallSprites;

	// reference to container of target sprites
	std::vector<sf::Sprite>& m_targetSprites;

	std::vector<TargetData>& m_targets; 

	// The tank speed.
	double m_speed{ 0.0 };
	const float MAX_SPEED{ 100.0 };

	// previous position and speed of tank used in collison processing
	sf::Vector2f m_previousPosition;
	double m_previousSpeed;

	// The current rotation as applied to tank base and turret.
	double m_baseRotation{ 0.0 };
	double m_turretRotation{ 0.0 };

	// previous rotations of base and tank used in case of collison or attempting to rotate into a wall
	double m_previousBaseRotation{ 0.0 };
	double m_previousTurretRotation{ 0.0 };

	// store previous turret pos to control toggling turret rotating sound 
	sf::Vector2f m_previousTurretPosition;

	// control when rotation can be done
	bool m_enableRotation = true;

	// timer that controls the rate of fire of the tank
	thor::Timer m_firingTimer;
	static constexpr float FIRING_COOLDOWN = 1.0f;
	
};
