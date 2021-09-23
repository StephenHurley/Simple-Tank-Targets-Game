#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CollisionDetector.h"
#include "Tank.h"

class cargoMode
{

public:
	cargoMode();

	void update(double dt);

	void render(sf::RenderWindow& window);

	bool playerPicksUpCargo(Tank const& playerTank) const;

	bool playerPicksUpFuel(Tank const& playerTank) const;

	bool playerArrivesAtGoal(Tank const& playerTank) const;

	bool m_cargoCollected = false;
	bool m_fuelPickedUp = false;

private:

	sf::Sprite m_cargoSprite;
	sf::Texture m_cargoTexture;

	sf::Sprite m_goalSprite;
	sf::Texture m_goalTexture;

	sf::Sprite m_fuelPickUpSprite;
	sf::Texture m_fuelPickUpTexture;


};