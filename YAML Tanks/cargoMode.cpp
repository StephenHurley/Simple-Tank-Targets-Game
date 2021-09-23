#include "cargoMode.h"

cargoMode::cargoMode()
{
	if (!m_cargoTexture.loadFromFile("./resources/images/ammoBox.png"))
	{
		std::string s("Error loading ammoBox");
		throw std::exception(s.c_str());
	}

	m_cargoSprite.setTexture(m_cargoTexture);
	m_cargoSprite.setScale(0.05, 0.05);

	m_cargoSprite.setPosition(200, 100);

	if (!m_goalTexture.loadFromFile("./resources/images/base.png"))
	{
		std::string s("Error loading base");
		throw std::exception(s.c_str());
	}

	m_goalSprite.setTexture(m_goalTexture);
	m_goalSprite.setScale(0.2, 0.2);

	m_goalSprite.setPosition(1200, 700);

	if (!m_fuelPickUpTexture.loadFromFile("./resources/images/fuelPickUp.png"))
	{
		std::string s("Error loading fuel pickup");
		throw std::exception(s.c_str());
	}

	m_fuelPickUpSprite.setTexture(m_fuelPickUpTexture);
	m_fuelPickUpSprite.setScale(0.08, 0.08);

	m_fuelPickUpSprite.setPosition(650, 375);
}

void cargoMode::update(double dt)
{
}

void cargoMode::render(sf::RenderWindow& window)
{
	window.draw(m_goalSprite);

	if (m_fuelPickedUp == false)
	{
		window.draw(m_fuelPickUpSprite);
	}
	if (m_cargoCollected == false)
	{
		window.draw(m_cargoSprite);
	}
	
}

bool cargoMode::playerPicksUpCargo(Tank const& playerTank) const
{
	// check if player is on cargo
	if (CollisionDetector::collision(m_cargoSprite, playerTank.getTurret()) ||
		CollisionDetector::collision(m_cargoSprite, playerTank.getBase()))
	{
		return true;
	}

	return false;
}

bool cargoMode::playerPicksUpFuel(Tank const& playerTank) const
{
	// check if player is on fuel
	if (CollisionDetector::collision(m_fuelPickUpSprite, playerTank.getTurret()) ||
		CollisionDetector::collision(m_fuelPickUpSprite, playerTank.getBase()))
	{
		return true;
	}
	return false;
}

bool cargoMode::playerArrivesAtGoal(Tank const& playerTank) const
{
	// check if player is on goal
	if (CollisionDetector::collision(m_goalSprite, playerTank.getTurret()) ||
		CollisionDetector::collision(m_goalSprite, playerTank.getBase()))
	{
		return true;
	}
	return false;
}
