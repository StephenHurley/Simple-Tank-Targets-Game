#include "TankAi.h"

////////////////////////////////////////////////////////////
TankAi::TankAi(sf::Texture const& texture, std::vector<sf::Sprite>& wallSprites)
	: m_aiBehaviour(AiBehaviour::PATROL)
	, m_texture(texture)
	, m_wallSprites(wallSprites)
	, m_steering(0, 0)
	, m_AIBullets()
{
	// Initialises the tank base and turret sprites.
	initSprites();

	// init each bullet
	for (int i = 0; i < NUM_AI_BULLETS; i++)
	{
		m_AIBullets[i].initBullet(texture);
	}

	m_AIBullets[0].initSounds();

	m_firingTimer.reset(sf::Time(sf::seconds(FIRING_COOLDOWN)));
	m_firingTimer.start();

	m_backToPatrolDelay.reset(sf::Time(sf::seconds(DELAY)));
}

////////////////////////////////////////////////////////////
void TankAi::update(Tank & playerTank, double dt)
{
	sf::Vector2f acceleration;
	sf::Vector2f vectorToPlayer = chase(playerTank.getPosition());
	sf::Vector2f vectorToPatrolDestination = patrol();

	switch (m_aiBehaviour)
	{
	case AiBehaviour::ATTACK_PLAYER:
		
		m_steering += thor::unitVector(vectorToPlayer);
		m_steering += collisionAvoidance();
		m_steering = MathUtility::truncate(m_steering, MAX_FORCE);
		acceleration = m_steering / MASS;
		m_velocity = MathUtility::truncate(m_velocity + acceleration, MAX_SPEED);

		break;

	case AiBehaviour::PATROL:

		m_steering += thor::unitVector(vectorToPatrolDestination);
		m_steering += collisionAvoidance();
		m_steering = MathUtility::truncate(m_steering, MAX_FORCE);
		acceleration = m_steering / MASS;
		m_velocity = MathUtility::truncate(m_velocity + acceleration, MAX_SPEED);
		break;

	default:
		break;
	}

	// Now we need to convert our velocity vector into a rotation angle between 0 and 359 degrees.
	// The m_velocity vector works like this: vector(1,0) is 0 degrees, while vector(0, 1) is 90 degrees.
	// So for example, 223 degrees would be a clockwise offset from 0 degrees (i.e. along x axis).
	// Note: we add 180 degrees below to convert the final angle into a range 0 to 359 instead of -PI to +PI
	auto dest = atan2(-1 * m_velocity.y, -1 * m_velocity.x) / thor::Pi * 180 + 180;

	auto currentRotation = m_rotation;

	// Find the shortest way to rotate towards the player (clockwise or anti-clockwise)
	if (std::round(currentRotation - dest) == 0.0)
	{
		m_steering.x = 0;
		m_steering.y = 0;
	}

	else if ((static_cast<int>(std::round(dest - currentRotation + 360))) % 360 < 180)
	{
		// rotate clockwise
		m_rotation = static_cast<int>((m_rotation) + 1) % 360;
	}
	else
	{
		// rotate anti-clockwise
		m_rotation = static_cast<int>((m_rotation) - 1) % 360;
	}

	updateMovement(dt);
	updateVisionCone(playerTank);

	processBullets(dt, playerTank);
}

////////////////////////////////////////////////////////////
void TankAi::render(sf::RenderWindow & window)
{
	window.draw(m_tankBase);
	window.draw(m_turret);
	window.draw(m_arrowLeft);
	window.draw(m_arrowRight);

	// renders the obstacles circles that tank avoids for debuging
	/*for (sf::CircleShape obstacleCircle : m_obstacles)
	{
		window.draw(obstacleCircle);
	}*/
	
}

////////////////////////////////////////////////////////////
void TankAi::init(sf::Vector2f position)
{
	m_tankBase.setPosition(position);
	m_turret.setPosition(position);

	setVisionCone();

	for (sf::Sprite const wallSprite : m_wallSprites)
	{
		sf::CircleShape circle(wallSprite.getTextureRect().width * 1.0f);
		circle.setOrigin(circle.getRadius(), circle.getRadius());
		circle.setPosition(wallSprite.getPosition());
		circle.setFillColor(sf::Color::Green);
		m_obstacles.push_back(circle);
	}
}

bool TankAi::collidesWithPlayer(Tank const& playerTank) const
{
	// Checks if the AI tank has collided with the player tank.
	if (CollisionDetector::collision(m_turret, playerTank.getTurret()) ||
		CollisionDetector::collision(m_tankBase, playerTank.getBase()))
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////
sf::Vector2f TankAi::chase(sf::Vector2f playerPosition) const
{
	sf::Vector2f answer;
	answer = (playerPosition - m_tankBase.getPosition());
	return answer;
}

sf::Vector2f TankAi::patrol()
{
	if (m_patrolDestinationGenerated == false)
	{
		float x = rand() % ScreenSize::s_width;
		float y = rand() % ScreenSize::s_height;
		m_destination = { x, y };
		m_patrolDestinationGenerated = true;
	}

	sf::Vector2f answer;
	answer = (m_destination - m_tankBase.getPosition());
	return answer;
}

float TankAi::distanceBetween(sf::Vector2f playerPos, sf::Vector2f aiPos)
{
	float distanceBetween = ((aiPos.x - playerPos.x) * (aiPos.x - playerPos.x)) + ((aiPos.y - playerPos.y) * (aiPos.y - playerPos.y));
	distanceBetween = std::sqrt(distanceBetween);
	return distanceBetween;
}

////////////////////////////////////////////////////////////
sf::Vector2f TankAi::collisionAvoidance()
{
	auto headingRadians = thor::toRadian(m_rotation);
	sf::Vector2f headingVector(std::cos(headingRadians) * MAX_SEE_AHEAD, std::sin(headingRadians) * MAX_SEE_AHEAD);
	m_ahead = m_tankBase.getPosition() + headingVector;
	
	m_halfAhead = m_tankBase.getPosition() + (headingVector * 0.5f);
	const sf::CircleShape mostThreatening = findMostThreateningObstacle();
	sf::Vector2f avoidance(0, 0);

	// tank is on possibily on course to collide with most threatening obstacle
	if (mostThreatening.getRadius() != 0.0)
	{	
		// check if ahead point is just barely inside or outside the most threathning radius
		// if true, ignore avoidance
		if (MathUtility::distance(m_ahead, mostThreatening.getPosition()) >= mostThreatening.getRadius() * 0.9 &&
			MathUtility::distance(m_ahead, mostThreatening.getPosition()) <= mostThreatening.getRadius() * 1.1)
		{
			avoidance *= 0.0f;
		}
		else // tank will collide, calculate its path to avoid colliding 
		{
			avoidance.x = m_ahead.x - mostThreatening.getPosition().x;
			avoidance.y = m_ahead.y - mostThreatening.getPosition().y;
			avoidance = thor::unitVector(avoidance);
			avoidance *= MAX_AVOID_FORCE;
		}
		
	}
	else // not close enough to most threathening to try avoid it
	{
		avoidance *= 0.0f;
	}
	return avoidance;
}

////////////////////////////////////////////////////////////
const sf::CircleShape TankAi::findMostThreateningObstacle()
{
	sf::CircleShape mostThreatening;
	mostThreatening.setRadius(0);

	for (int i = 0; i < m_obstacles.size(); i++)
	{
		sf::CircleShape obstacle = m_obstacles[i];
		bool collide = (MathUtility::lineIntersectsCircle(m_ahead, m_halfAhead, obstacle));

		if (collide && (mostThreatening.getRadius() != 0 || MathUtility::distance(m_tankBase.getPosition(), obstacle.getPosition()) <
			MathUtility::distance(m_tankBase.getPosition(), mostThreatening.getPosition())))
		{
			mostThreatening = obstacle;
		}
	}

	return mostThreatening;
}

////////////////////////////////////////////////////////////
void TankAi::initSprites()
{
	// Initialise the tank base
	m_tankBase.setTexture(m_texture);
	sf::IntRect baseRect(103, 43, 79, 43);
	m_tankBase.setTextureRect(baseRect);
	m_tankBase.setOrigin(baseRect.width / 2.0, baseRect.height / 2.0);
	m_tankBase.setScale(0.5, 0.5);

	// Initialise the turret
	m_turret.setTexture(m_texture);
	sf::IntRect turretRect(122, 1, 83, 31);
	m_turret.setTextureRect(turretRect);
	m_turret.setOrigin(turretRect.width / 3.0, turretRect.height / 2.0);
	m_turret.setScale(0.5, 0.5);
}

void TankAi::setVisionCone()
{
	m_visionConeLeft = m_turret.getPosition();
	m_visionConeRight = m_turret.getPosition();

	// Setup the arrow visualisation
	m_arrowLeft.setStyle(thor::Arrow::Style::Forward);
	m_arrowLeft.setColor(sf::Color::Red);
	m_arrowLeft.setPosition(m_visionConeLeft);
	m_arrowRight.setStyle(thor::Arrow::Style::Forward);
	m_arrowRight.setColor(sf::Color::Red);
	m_arrowRight.setPosition(m_visionConeRight);

	m_arrowLeft.setDirection(visionConeLengthPatrol * thor::rotatedVector(m_visionConeDir, -visionConeAnglePatrol));
	m_arrowRight.setDirection(visionConeLengthPatrol * thor::rotatedVector(m_visionConeDir, visionConeAnglePatrol));
}


////////////////////////////////////////////////////////////
void TankAi::updateMovement(double dt)
{
	double speed = thor::length(m_velocity);
	sf::Vector2f newPos(m_tankBase.getPosition().x + std::cos(MathUtility::DEG_TO_RAD  * m_rotation) * speed * (dt / 1000),
		m_tankBase.getPosition().y + std::sin(MathUtility::DEG_TO_RAD  * m_rotation) * speed * (dt / 1000));
	m_tankBase.setPosition(newPos.x, newPos.y);
	m_tankBase.setRotation(m_rotation);
	m_turret.setPosition(m_tankBase.getPosition());

	switch (m_aiBehaviour)
	{
	case AiBehaviour::ATTACK_PLAYER:

		m_turret.setRotation(m_rotation);
		break;

	case AiBehaviour::PATROL:
		m_turret.setRotation(m_visionConeSweepAngle);
		break;
	} 
}

void TankAi::updateVisionCone(Tank const& playerTank)
{
	// store ai turret and player positions
	sf::Vector2f AIturretPos = m_turret.getPosition();
	sf::Vector2f playerTankPos = playerTank.getPosition();

	// update locations of vision cone origin
	m_visionConeLeft = AIturretPos;
	m_visionConeRight = AIturretPos;
	m_arrowLeft.setPosition(m_visionConeLeft);
	m_arrowRight.setPosition(m_visionConeRight);

	// directions each arrow points in
	sf::Vector2f arrowLeftDirection;
	sf::Vector2f arrowRightDirection;

	// end points of each arrow
	sf::Vector2f arrowLeftEndPoint;
	sf::Vector2f arrowRightEndPoint;
	
	switch (m_aiBehaviour)
	{
	case AiBehaviour::ATTACK_PLAYER: // chasing player, longer and narrower vision cone

		// player halfway inside cone, make cone wider but shorter
		if (distanceBetween(playerTankPos, AIturretPos) <= visionConeLengthAttack / 2)
		{
			arrowLeftDirection = visionConeLengthPlayerHalfway * thor::rotatedVector(m_visionConeDir, m_rotation - visionConeAnglePlayerHalfway);
			arrowRightDirection = visionConeLengthPlayerHalfway * thor::rotatedVector(m_visionConeDir, m_rotation + visionConeAnglePlayerHalfway);
		}
		else // keep narrowed and longer vision cone
		{
			arrowLeftDirection = visionConeLengthAttack * thor::rotatedVector(m_visionConeDir, m_rotation - visionConeAngleAttack);
			arrowRightDirection = visionConeLengthAttack * thor::rotatedVector(m_visionConeDir, m_rotation + visionConeAngleAttack);
		}

		m_arrowLeft.setDirection(arrowLeftDirection);
		m_arrowRight.setDirection(arrowRightDirection);

		arrowLeftEndPoint = AIturretPos + arrowLeftDirection;
		arrowRightEndPoint = AIturretPos + arrowRightDirection;

		if (isLeft(arrowLeftEndPoint, AIturretPos, playerTankPos) == false &&
			isLeft(arrowRightEndPoint, AIturretPos, playerTankPos))
		{
			// player still in cone, no change needed

			// set timer back to 3 seconds
			// probably inefficent to keep reseting timer but this prevents bug
			// bug in question: leaving the vision cone starts the timer to switch back to patrol. Lets say a player leaves the cone for 2 seconds, then enters the cone again.
			// after 1 second (the time remaining on the switch delay), the AI will switch back to patrol, even if the player is still in the cone. Reseting the timer here ensures the player 
			// has to be outside the cone for a full 3 seconds for the ai to switch
			m_backToPatrolDelay.reset(sf::Time(sf::seconds(DELAY)));
		}
		else
		{
			// start 3 second delay to change back to Patrol
			m_backToPatrolDelay.start();
		}

		if (m_backToPatrolDelay.isExpired())
		{
			m_aiBehaviour = AiBehaviour::PATROL;
			m_backToPatrolDelay.reset(sf::Time(sf::seconds(DELAY)));
		}

		break;

	case AiBehaviour::PATROL: // moving to random point on map, shorter and wider vision cone that sweeps 360 degrees
		arrowLeftDirection = visionConeLengthPatrol * thor::rotatedVector(m_visionConeDir, m_visionConeSweepAngle - visionConeAnglePatrol);
		m_arrowLeft.setDirection(arrowLeftDirection);
		arrowRightDirection = visionConeLengthPatrol * thor::rotatedVector(m_visionConeDir, m_visionConeSweepAngle + visionConeAnglePatrol);
		m_arrowRight.setDirection(arrowRightDirection);

		arrowLeftEndPoint = AIturretPos + arrowLeftDirection;
		arrowRightEndPoint = AIturretPos + arrowRightDirection;

		// check if player could be seen by ai
		if (distanceBetween(playerTankPos, AIturretPos) <= visionConeLengthPatrol)
		{
			// check if player is in the vision cone, player is to the right of left arrow and to the left of the right arrow
			if (isLeft(arrowLeftEndPoint, AIturretPos, playerTankPos) == false &&
				isLeft(arrowRightEndPoint, AIturretPos, playerTankPos))
			{
				// tank has spotted player, switch behaviour
				m_aiBehaviour = AiBehaviour::ATTACK_PLAYER;
				m_rotation = m_visionConeSweepAngle;
				
			}
		}

		m_visionConeSweepAngle += 0.5f;

		if (m_visionConeSweepAngle == 360.0f)
		{
			m_visionConeSweepAngle = 0;
		}

		break;

	default:
		break;
	}
}

void TankAi::processBullets(double dt, Tank & playerTank)
{
	for (int i = 0; i < NUM_AI_BULLETS; i++)
	{
		// only allow another bullet to be fired after 1 second
		if (m_firingTimer.isExpired())
		{
			// only allow bullets to be fired in attack player behaviour
			if (m_aiBehaviour == AiBehaviour::ATTACK_PLAYER)
			{
				// check if bullet is ready to be fired
				if (m_AIBullets[i].m_canFire)
				{
					// check if in reasonable distance to player
					if (distanceBetween(playerTank.getPosition(), m_turret.getPosition()) <= visionConeLengthAttack * 0.75)
					{
						m_AIBullets[i].m_firing = true;
						m_AIBullets[i].m_canFire = false;
					}
				}
			}
		
			// bullet being fired, reset timer and find start point
			if (m_AIBullets[i].m_firing)
			{
				m_firingTimer.reset(sf::Time(sf::seconds(FIRING_COOLDOWN)));
				m_firingTimer.start();

				m_AIBullets[i].findStartPoint(m_turret.getRotation(), m_turret.getPosition());
			}
		}

		// if start point is found, move bullet along created vector
		if (m_AIBullets[i].m_traveling)
		{
			m_AIBullets[i].update(dt);
			m_AIBullets[i].wallCollision(m_wallSprites);

			// BROKEN
			//m_AIBullets[i].playerCollision(playerTank);
			
		}
	}
}

bool TankAi::isLeft(sf::Vector2f t_linePoint1, sf::Vector2f t_linePoint2, sf::Vector2f t_point) const
{
	/*bool isLeft(sf::Vector2f t_linePoint1,
		sf::Vector2f t_linePoint2,
		sf::Vector2f t_point) const*/
	
	// return ( (x2-x1) * (y3-y1) ) - ( (y2 - y1) * (x3-x1) )

	return ((t_linePoint2.x - t_linePoint1.x) *
		(t_point.y - t_linePoint1.y) -
		(t_linePoint2.y - t_linePoint1.y) *
		(t_point.x - t_linePoint1.x)) > 0;

}
