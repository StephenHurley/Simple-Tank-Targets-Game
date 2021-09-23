#pragma once

#include <SFML/Graphics.hpp>
#include "GameState.h"

/// <summary>
/// @brief A basic HUD implementation.
/// 
/// Shows current game state only.
/// </summary>

class HUD
{
public:
	/// <summary>
	/// @brief Default constructor that stores a font for the HUD and initialises the general HUD appearance.
	/// </summary>
	HUD(sf::Font& hudFont);

	/// <summary>
	/// @brief Checks the current game state and sets the appropriate status text on the HUD.
	/// </summary>
	/// <param name="gameState">The current game state</param>
	void update(GameState const& gameState, int t_gameTimeRemaining, int t_fuelRemaining, int t_score, std::string objectiveText);

	/// <summary>
	/// @brief Draws the HUD outline and text.
	///
	/// </summary>
	/// <param name="window">The SFML Render window</param>
	void render(sf::RenderWindow& window);

private:
	// The font for this HUD.
	sf::Font m_textFont;

	// A container for the current gamestate text.
	sf::Text m_gameStateText;

	sf::Text m_fuelText;

	sf::Text m_timerText;

	sf::Text m_objectiveText;

	sf::Text m_scoreText;

	// A simple background shape for the HUD.
	sf::RectangleShape m_hudOutline;
};