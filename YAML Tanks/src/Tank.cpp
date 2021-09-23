#include "Tank.h"
#include "MathUtility.h"

Tank::Tank(sf::Texture const& t_texture, std::vector<sf::Sprite>& t_wallSprites,
	std::vector<sf::Sprite>& t_targetSprites, std::vector<TargetData>& t_targets)
	: m_texture(t_texture)
	, m_wallSprites(t_wallSprites)
	, m_targetSprites(t_targetSprites)
	, m_targets(t_targets)
	, m_bullets()
{
	// Initialises the tank base and turret sprites.
	initSprites();
	initSounds();

	// init each bullet
	for (int i = 0; i < NUM_PLAYER_BULLETS; i++)
	{
		m_bullets[i].initBullet(t_texture);
	}

	m_bullets[0].initSounds();

	m_firingTimer.reset(sf::Time(sf::seconds(FIRING_COOLDOWN)));
	m_firingTimer.start();

}

void Tank::update(double dt)
{
	// first check for key input
	handleKeyInput();

	// store previous position in case of collision
	m_previousPosition = m_tankBase.getPosition();
	
	// store previous pos of turret
	m_previousTurretPosition = m_turret.getPosition();

	float m_rotationRadians = { 0.0f };

	// base updating
	m_rotationRadians = m_baseRotation * MathUtility::DEG_TO_RAD;
	m_tankBase.setRotation(m_baseRotation);
	float newXBase = m_tankBase.getPosition().x + cos(m_rotationRadians) * m_speed * (dt / 1000);
	float newYBase = m_tankBase.getPosition().y + sin(m_rotationRadians) * m_speed * (dt / 1000);
	m_tankBase.setPosition(newXBase, newYBase);

	// turret updating
	m_rotationRadians = m_turretRotation * MathUtility::DEG_TO_RAD;
	m_turret.setRotation(m_turretRotation);
	float newXTurret = m_tankBase.getPosition().x + cos(m_rotationRadians) * m_speed * (dt / 1000);
	float newYTurret = m_tankBase.getPosition().y + sin(m_rotationRadians) * m_speed * (dt / 1000);
	m_turret.setPosition(newXTurret, newYTurret);

	processBullets(dt);

	//make tank gradually come to a stop without further player input
	gradualStop();

	// if previous position is same as new, tank is stopped
	if (m_previousPosition == sf::Vector2f(newXBase, newYBase))
	{
		m_stopped = true;
	}

	// control if moving sound should play or not
	soundControl();

	// decrease fuel supply by 1/200 of speed
	if (m_speed > 0)
	{
		m_fuel = m_fuel - (m_speed / 200);
	}
	else if (m_speed < 0) // prevents refueling when reversing
	{
		m_fuel = m_fuel + (m_speed / 200);
	}

	if (checkWallCollision())
	{
		deflect();
		m_stopped = true;
	}
}

void Tank::render(sf::RenderWindow & window) 
{
	window.draw(m_tankBase);
	window.draw(m_turret);
}

void Tank::setPosition(sf::Vector2f &position)
{
	m_tankBase.setPosition(position);
	m_turret.setPosition(position);
}

////////////////////////////////////////////////////////////
void Tank::increaseSpeed()
{
	m_previousSpeed = m_speed;
	if (m_speed < MAX_SPEED)
	{
		m_speed += 5;
	}
}

////////////////////////////////////////////////////////////
void Tank::decreaseSpeed()
{
	m_previousSpeed = m_speed;
	if (m_speed > -MAX_SPEED)
	{
		m_speed -= 5;
	}
}

////////////////////////////////////////////////////////////
void Tank::increaseRotation()
{
	// store previous rotation before updating
	m_previousBaseRotation = m_baseRotation;
	m_baseRotation += 1;
	// also increase turret rotation so when base rotates the turret rotates the same ammount
	increaseTurretRotation();
	if (m_baseRotation == 360.0)
	{
		m_baseRotation = 0;
	}
}

////////////////////////////////////////////////////////////
void Tank::decreaseRotation()
{
	// store previous rotation before updating
	m_previousBaseRotation = m_baseRotation;
	m_baseRotation -= 1;
	// also decrease turret rotation so when base rotates the turret rotates the same ammount
	decreaseTurretRotation();
	if (m_baseRotation == 0.0)
	{
		m_baseRotation = 359.0;
	}
}

void Tank::handleKeyInput()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		increaseSpeed();
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		decreaseSpeed();
	}

	// allow player access to these commands as long as m_enableRotation is true
	// (it is false while processing a collison and re_enabled after it is solved)
	if (m_enableRotation == true)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			decreaseRotation();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			increaseRotation();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
		{
			decreaseTurretRotation();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
		{
			increaseTurretRotation();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
		{
			centreTurret();
		}

	}
	
}

void Tank::increaseTurretRotation()
{
	// store previous rotation before updating
	m_previousTurretRotation = m_turretRotation;
	m_turretRotation += 1;

	if (m_turretRotation == 360.0)
	{
		m_turretRotation = 0;
	}
}

void Tank::decreaseTurretRotation()
{
	// store previous rotation before updating
	m_previousTurretRotation = m_turretRotation;
	m_turretRotation -= 1;
	if (m_turretRotation == 0.0)
	{
		m_turretRotation = 359.0;
	}
}

void Tank::centreTurret()
{
	m_turretRotation = m_baseRotation;
}

bool Tank::checkWallCollision()
{
	for (sf::Sprite const& sprite : m_wallSprites)
	{
		// Checks if either the tank base or turret has collided with the current wall sprite.
		if (CollisionDetector::collision(m_turret, sprite) ||
			CollisionDetector::collision(m_tankBase, sprite))
		{
			return true;
		}
	}
	return false;
}

void Tank::deflect()
{
	// In case tank was rotating.
	adjustRotation();

	// If tank was moving.
	if (m_speed != 0)
	{
		// Temporarily disable turret rotations on collision.
		m_enableRotation = false;
		// Back up to position in previous frame.
		m_tankBase.setPosition(m_previousPosition);
		// Apply small force in opposite direction of travel.
		if (m_previousSpeed < 0)
		{
			m_speed = 5;
		}
		else
		{
			m_speed = -5;
		}
		// re-enable turret rotation
		m_enableRotation = true;
	}
}


sf::Vector2f Tank::getPosition() const
{
	return m_tankBase.getPosition();
}

sf::Sprite Tank::getBase() const
{
	return m_tankBase;
}

sf::Sprite Tank::getTurret() const
{
	return m_turret;
}

void Tank::initSprites()
{
	// Initialise the tank base
	m_tankBase.setTexture(m_texture);
	sf::IntRect baseRect(2, 43, 79, 43);
	m_tankBase.setTextureRect(baseRect);
	m_tankBase.setOrigin(baseRect.width / 2.0, baseRect.height / 2.0);
	m_tankBase.setScale(0.5, 0.5);
	
	// Initialise the turret
	m_turret.setTexture(m_texture);
	sf::IntRect turretRect(19, 1, 83, 31);
	m_turret.setTextureRect(turretRect);
	m_turret.setOrigin(turretRect.width / 3.0, turretRect.height / 2.0);

	m_turret.setScale(0.5, 0.5);

}

void Tank::initSounds()
{
	if (!m_tankFiringBuffer.loadFromFile("./resources/sounds/TankFiring.wav"))
	{
		std::string s("Error loading tank firing sound");
		throw std::exception(s.c_str());
	}

	m_tankFiringSound.setBuffer(m_tankFiringBuffer);
	m_tankFiringSound.setVolume(10.0f);

	if (!m_tankMovingBuffer.loadFromFile("./resources/sounds/TankMoving.wav"))
	{
		std::string s("Error loading tank moving sound");
		throw std::exception(s.c_str());
	}

	m_tankMovingSound.setBuffer(m_tankMovingBuffer);
	m_tankMovingSound.setVolume(5.0f);
	m_tankMovingSound.setLoop(true);
}

void Tank::adjustRotation()
{
	// If tank was rotating...
	if (m_baseRotation != m_previousBaseRotation)
	{
		// Work out which direction to rotate the tank base post-collision.
		if (m_baseRotation > m_previousBaseRotation)
		{
			m_baseRotation = m_previousBaseRotation - 1;
		}
		else
		{
			m_baseRotation = m_previousBaseRotation + 1;
		}
	}
	// If turret was rotating while tank was moving
	if (m_turretRotation != m_previousTurretRotation)
	{
		// Set the turret rotation back to it's pre-collision value.
		m_turretRotation = m_previousTurretRotation;
	}
}

void Tank::gradualStop()
{
	if (m_speed > 0)
	{
		m_stopped = false;
		m_speed = m_speed - 0.1;
	}
	if (m_speed < 0)
	{
		m_stopped = false;
		m_speed = m_speed + 0.1;
	}

	//range [-100] - [100] for speed
	double high = MAX_SPEED, low = -MAX_SPEED;
	m_speed = std::clamp(m_speed, low, high);

}

void Tank::processBullets(double dt)
{
	for (int i = 0; i < NUM_PLAYER_BULLETS; i++)
	{
		// only allow another bullet to be fired after 1 second
		if (m_firingTimer.isExpired())
		{
			// check if player pressed space
			m_bullets[i].handleKeyInputs();

			// player pressed space to fire, reset timer and find start point
			if (m_bullets[i].m_firing)
			{
				m_tankFiringSound.play();

				m_firingTimer.reset(sf::Time(sf::seconds(FIRING_COOLDOWN)));
				m_firingTimer.start();

				m_bulletsFired++;

				m_bullets[i].findStartPoint(m_turret.getRotation(), m_turret.getPosition());
			}
		}

		// if start point is found, move bullet along created vector
		if (m_bullets[i].m_traveling)
		{
			m_bullets[i].update(dt);
			m_bullets[i].wallCollision(m_wallSprites);
			m_bullets[i].targetCollision(m_targetSprites, m_targets);
		}
	}
}

void Tank::soundControl()
{
	// tank moving sound effect
	// tank is moving, toggle sound
	if (m_toggleMovingSound && m_stopped == false)
	{
		m_tankMovingSound.play();
		m_toggleMovingSound = false;
	}
	// if tank is stopped, pause its sound
	if (m_stopped)
	{
		m_tankMovingSound.pause();
		m_toggleMovingSound = true;
	}
}

