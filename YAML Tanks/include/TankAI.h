#pragma once
#include "MathUtility.h"
#include "Tank.h"
#include "CollisionDetector.h"
#include "ScreenSize.h"
#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>
#include <Thor/Shapes.hpp>
#include <iostream>
#include <queue>

class TankAi
{
public:
	/// <summary>
	/// @brief Constructor that stores a reference to the obstacle container.
	/// Initialises steering behaviour to seek (player) mode, sets the AI tank position and
	///  initialises the steering vector to (0,0) meaning zero force magnitude.
	/// </summary>
	/// <param name="texture">A reference to the sprite sheet texture</param>
	///< param name="wallSprites">A reference to the container of wall sprites</param>
	TankAi(sf::Texture const& texture, std::vector<sf::Sprite>& wallSprites);

	void update(Tank & playerTank, double dt);

	/// <summary>
	/// @brief Draws the tank base and turret.
	///
	/// </summary>
	/// <param name="window">The SFML Render window</param>
	void render(sf::RenderWindow& window);

	/// <summary>
	/// @brief Initialises the obstacle container and sets the tank base/turret sprites to the specified position.
	/// <param name="position">An x,y position</param>
	/// </summary>
	void init(sf::Vector2f position);

	/// <summary>
/// @brief Checks for collision between the AI and player tanks.
///
/// </summary>
/// <param name="player">The player tank instance</param>
/// <returns>True if collision detected between AI and player tanks.</returns>
	bool collidesWithPlayer(Tank const& playerTank) const;

	Bullet m_AIBullets[NUM_AI_BULLETS];

private:
	void initSprites();

	void setVisionCone();

	void updateMovement(double dt);

	void updateVisionCone(Tank const& playerTank);

	void processBullets(double dt, Tank& playerTank);

	bool isLeft(sf::Vector2f t_linePoint1,
		sf::Vector2f t_linePoint2,
		sf::Vector2f t_point) const;

	sf::Vector2f chase(sf::Vector2f playerPosition) const;
	sf::Vector2f patrol();

	float distanceBetween(sf::Vector2f playerPos, sf::Vector2f aiPos);

	sf::Vector2f collisionAvoidance();

	const sf::CircleShape findMostThreateningObstacle();

	// A reference to the sprite sheet texture.
	sf::Texture const & m_texture;

	// A sprite for the tank base.
	sf::Sprite m_tankBase;

	// A sprite for the turret
	sf::Sprite m_turret;

	// A reference to the container of wall sprites.
	std::vector<sf::Sprite> & m_wallSprites;

	// The current rotation as applied to tank base and turret.
	float m_rotation{ 0.0 };

	// Current velocity.
	sf::Vector2f m_velocity;

	// Steering vector.
	sf::Vector2f m_steering;

	// The ahead vector.
	sf::Vector2f m_ahead;

	// The half-ahead vector.
	sf::Vector2f m_halfAhead;

	// point tank will make its way to when not chasing player
	sf::Vector2f m_destination;

	// The maximum see ahead range.
	static float constexpr MAX_SEE_AHEAD{ 50.0f };

	// The maximum avoidance turn rate.
	static float constexpr MAX_AVOID_FORCE{ 50.0f };

	static float constexpr MAX_FORCE{ 10.0f };

	static float constexpr MASS{ 10.0f };

	// The maximum speed for this tank.
	float MAX_SPEED = 50.0f;

	// A container of circles that represent the obstacles to avoid.
	std::vector<sf::CircleShape> m_obstacles;

	enum class AiBehaviour
	{
		ATTACK_PLAYER,
		PATROL 
	} m_aiBehaviour;

	bool m_patrolDestinationGenerated = false;

	// Vision cone vector...initially points along the x axis.
	sf::Vector2f m_visionConeDir{ 1, 0 };

	// Vision cone visualisation...vision cone origin
	sf::Vector2f m_visionConeLeft;

	sf::Vector2f m_visionConeRight;

	thor::Arrow m_arrowLeft;
	thor::Arrow m_arrowRight;

	float m_visionConeSweepAngle = 0;

	// various vision cone lengths for each mode
	float visionConeLengthPatrol{ 200.0f };
	float visionConeLengthAttack{ 500.0f };
	float visionConeLengthPlayerHalfway{ 330.0f };

	// various vision cone angles for each mode
	float visionConeAnglePatrol{ 30.0f };
	float visionConeAngleAttack{ 20.0f };
	float visionConeAnglePlayerHalfway{ 40.0f };

	// timer that controls the rate of fire of the tank
	thor::Timer m_firingTimer;
	static constexpr float FIRING_COOLDOWN = 2.0f;

	thor::Timer m_backToPatrolDelay;
	static constexpr float DELAY = 3.0f;

};

