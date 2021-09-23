#include "Bullet.h"
#include "MathUtility.h"
#include "ScreenSize.h"
#include <iostream>

// blank default constructor as properites are defined in header
Bullet::Bullet()
{
}

void Bullet::findStartPoint(double t_turretRotation, sf::Vector2f t_turretSpritePos)
{
	double rotation = t_turretRotation;
	m_bulletSprite.rotate(rotation);

	sf::Vector2f turretPos = t_turretSpritePos;

	m_startPoint = sf::Vector2f(
		turretPos.x + 1 * std::cos(rotation * MathUtility::DEG_TO_RAD),
		turretPos.y + 1 * std::sin(rotation * MathUtility::DEG_TO_RAD)
	);

	m_bulletSprite.setPosition(m_startPoint);

	//get unit vector used to move the bullet
	m_startPoint = thor::unitVector(m_startPoint - turretPos);
	m_firing = false;
	m_traveling = true;

}

void Bullet::update(double dt)
{
		m_bulletSprite.move(m_startPoint.x * PROJECTILE_SPEED * (dt / 1000),
			m_startPoint.y * PROJECTILE_SPEED * (dt / 1000));

		// check if bullet has gone off screen
		if (checkGameWorldBoundaries())
		{
			m_bulletSprite.setPosition(m_offScreenPosition);
			m_bulletSprite.setRotation(270);
			m_canFire = true;
			m_traveling = false;
		}
	
}

void Bullet::render(sf::RenderWindow& window)
{
	window.draw(m_bulletSprite);
}

void Bullet::setPosition(sf::Vector2f& position)
{
	m_bulletSprite.setPosition(position);
}

void Bullet::handleKeyInputs()
{
	if (m_canFire)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_firing = true;
			m_canFire = false;
		}
		
	}
}

void Bullet::wallCollision(std::vector<sf::Sprite>& t_wallSprites)
{
	for (sf::Sprite const& sprite : t_wallSprites)
	{
		//	Checks if bullet sprite has collided with any walls, if it has reset that bullet
		if (CollisionDetector::collision(m_bulletSprite, sprite))
		{
			m_wallHitSound.play();
			m_bulletSprite.setPosition(m_offScreenPosition);
			m_bulletSprite.setRotation(270);
			m_canFire = true;
			m_traveling = false;
			
		}
	}
}

void Bullet::targetCollision(std::vector<sf::Sprite>& t_targetSprites, std::vector<TargetData>& t_targets)
{
	for (int i = 0; i < t_targets.size(); i++)
	{
		if (t_targets[i].m_active == true)
		{
			// get target data and coresponding sprite
			TargetData const& target = t_targets[i];
			sf::Sprite const& sprite = t_targetSprites[i];

			//Checks if bullet sprite has collided with active target.
			//If it has, reset that bullet and destroy that target
			if (CollisionDetector::collision(m_bulletSprite, sprite))
			{
				// destroys target in game.cpp
				t_targets[i].m_shot = true;

				// if there is another target avaliable to spawn, spawn it
				if (i + 1 < t_targets.size())
				{
					t_targets[i + 1].m_active = true;
				}

				m_bulletSprite.setPosition(m_offScreenPosition);
				m_bulletSprite.setRotation(270);
				m_canFire = true;
				m_traveling = false;

			}

			// break out of loop after finding and checking active target
			break;
		}
	}
}

//void Bullet::playerCollision(Tank& playerTank)
//{
//	//	Checks if bullet sprite has collided with player, reset that bullet if it has and make player enter damged state
//	if (CollisionDetector::collision(m_bulletSprite, playerTank.getBase()))
//	{
//		m_wallHitSound.play();
//		m_bulletSprite.setPosition(m_offScreenPosition);
//		m_bulletSprite.setRotation(270);
//		m_canFire = true;
//		m_traveling = false;
//		playerTank.m_damaged = true;
//
//	}
//}

bool Bullet::checkGameWorldBoundaries()
{
	bool result = false;
	sf::Vector2f bulletPos = m_bulletSprite.getPosition();

		// bullet has gone off the left or right side of screen
		if (bulletPos.x > ScreenSize::s_width || bulletPos.x < 0)
		{
			result = true;
		}
		// bullet has gone off the top or bottom of screen
		else if (bulletPos.y > ScreenSize::s_height || bulletPos.y < 0)
		{
			result = true;
		}
	
	return result;
}

void Bullet::initBullet(sf::Texture const& t_texture)
{
	// Initialise the bullets
	m_bulletSprite.setTexture(t_texture);
	sf::IntRect bulletRect(7, 176, 7, 10);
	m_bulletSprite.setTextureRect(bulletRect);
	m_bulletSprite.setOrigin(bulletRect.width / 2.0, bulletRect.height / 2.0);
	m_bulletSprite.setRotation(270);
	m_bulletSprite.setPosition(m_offScreenPosition);
}

void Bullet::initSounds()
{
	if (!m_wallHitBuffer.loadFromFile("./resources/sounds/Explosion.wav"))
	{
		std::string s("Error loading explosion (wall hit) sound");
		throw std::exception(s.c_str());
	}

	m_wallHitSound.setBuffer(m_wallHitBuffer);
	m_wallHitSound.setVolume(30.0f);
}

