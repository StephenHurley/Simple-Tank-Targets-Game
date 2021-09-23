#include "HUD.h"

////////////////////////////////////////////////////////////
HUD::HUD(sf::Font& hudFont)
	: m_textFont(hudFont)
{
	m_gameStateText.setFont(hudFont);
	m_gameStateText.setCharacterSize(30);
	m_gameStateText.setFillColor(sf::Color::Blue);
	m_gameStateText.setPosition(sf::Vector2f(1200, 5));
	m_gameStateText.setString("Game Running");
	m_gameStateText.setStyle(sf::Text::Bold | sf::Text::Underlined);

	//Setting up our hud properties 
	m_hudOutline.setSize(sf::Vector2f(1440.0f, 40.0f));
	m_hudOutline.setFillColor(sf::Color(0, 0, 0, 38));
	m_hudOutline.setOutlineThickness(-.5f);
	m_hudOutline.setOutlineColor(sf::Color(0, 0, 0, 100));
	m_hudOutline.setPosition(0, 0);

	m_fuelText.setFont(hudFont);
	m_fuelText.setCharacterSize(30);
	m_fuelText.setFillColor(sf::Color::Blue);
	m_fuelText.setPosition(sf::Vector2f(5, 5));
	m_fuelText.setString("");
	m_fuelText.setStyle(sf::Text::Bold | sf::Text::Underlined);

	m_timerText.setFont(hudFont);
	m_timerText.setCharacterSize(30);
	m_timerText.setFillColor(sf::Color::Blue);
	m_timerText.setPosition(sf::Vector2f(335, 5));
	m_timerText.setString("");
	m_timerText.setStyle(sf::Text::Bold | sf::Text::Underlined);

	m_objectiveText.setFont(hudFont);
	m_objectiveText.setCharacterSize(30);
	m_objectiveText.setFillColor(sf::Color::Blue);
	m_objectiveText.setPosition(sf::Vector2f(650, 5));
	m_objectiveText.setString("");
	m_objectiveText.setStyle(sf::Text::Bold | sf::Text::Underlined);

	m_scoreText.setFont(hudFont);
	m_scoreText.setCharacterSize(30);
	m_scoreText.setFillColor(sf::Color::Blue);
	m_scoreText.setPosition(sf::Vector2f(1270, 5));
	m_scoreText.setString("");
	m_scoreText.setStyle(sf::Text::Bold | sf::Text::Underlined);
}

////////////////////////////////////////////////////////////
void HUD::update(GameState const& gameState, int t_gameTimeRemaining, int t_fuelRemaining, int t_score, std::string t_objectiveText)
{
	switch (gameState)
	{
	case GameState::GAME_RUNNING:
		break;
	case GameState::GAME_WIN:
		m_gameStateText.setString("You Won");
		break;
	case GameState::GAME_LOSE:
		m_gameStateText.setString("You Lost");
		break;
	default:
		break;
	}

	m_timerText.setString("Time Remaining: " + std::to_string(t_gameTimeRemaining));
	m_fuelText.setString("Fuel Remaining: " + std::to_string(t_fuelRemaining));
	m_scoreText.setString("Score: " + std::to_string(t_score));
	m_objectiveText.setString("Current Objective: " + t_objectiveText);
}

void HUD::render(sf::RenderWindow& window)
{
	window.draw(m_hudOutline);
	window.draw(m_timerText);
	window.draw(m_fuelText);
	window.draw(m_scoreText);
	window.draw(m_objectiveText);
	//window.draw(m_gameStateText);
}