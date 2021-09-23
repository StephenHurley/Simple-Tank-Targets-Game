#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "ScreenSize.h"
#include "LevelLoader.h"
#include "Tank.h"
#include "TankAI.h"
#include "Bullet.h"
#include "Thor/Time.hpp"
#include "Globals.h"
#include "GameState.h"
#include "HUD.h"
#include "cargoMode.h"

/// <summary>
/// @author Stephen Hurley
/// @date September 2020 - March 2021
/// @version 1.0
/// 
/// </summary>

/// <summary>
/// @brief Main class for the SFML Playground project.
/// 
/// This will be a single class framework for learning about SFML. 
/// Example usage:
///		Game game;
///		game.run();
/// </summary>

class Game
{
public:

	/// <summary>
	/// @brief Default constructor that initialises the SFML window, 
	///   and sets vertical sync enabled. 
	/// </summary>
	/// 
	Game();

	/// <summary>
	/// @brief the main game loop.
	/// 
	/// A complete loop involves processing SFML events, updating and drawing all game objects.
	/// The actual elapsed time for a single game loop results (lag) is stored. If this value is 
	///  greater than the notional time for one loop (MS_PER_UPDATE), then additional updates will be 
	///  performed until the lag is less than the notional time for one loop.
	/// The target is one update and one render cycle per game loop, but slower PCs may 
	///  perform more update than render operations in one loop.
	/// </summary>
	void run();

protected:
	/// <summary>
	/// @brief Placeholder to perform updates to all game objects.
	/// </summary>
	/// <param name="time">update delta time</param>
	void update(double dt);

	/// <summary>
	/// @brief Draws the background and foreground game objects in the SFML window.
	/// The render window is always cleared to black before anything is drawn.
	/// </summary>
	void render();

	void renderTargets();

	/// <summary>
	/// @brief Checks for events.
	/// Allows window to function and exit. 
	/// Events are passed on to the Game::processGameEvents() method.
	/// </summary>	
	void processEvents();

	/// <summary>
	/// @brief Handles all user input.
	/// </summary>
	/// <param name="event">system event</param>
	void processGameEvents(sf::Event&);

	/// <summary>
/// @brief Creates the wall sprites and loads them into a vector.
/// Note that sf::Sprite is considered a light weight class, so 
///  storing copies (instead of pointers to sf::Sprite) in std::vector is acceptable.
/// </summary>
	void generateWalls();

	// generates targets and loads them into a vector like generateWalls() does with wall sprites
	void generateTargets();

	//check remaining time of current target. if it has less than 5 seconds of being active,
	// display its countdown 
	void remainingTargetTime(int i);
	// check if target was shot. If it was, reset timer with the remaining time of the previous target added on as reward
	void targetShot(int i);
	// check if target has expired (time to shoot it has run out) if it was, spawn next target
	void genNextTarget(int i);

	void setUpFontAndText();

	// updates the gameTimer text
	void updateGameTimer();

	int m_timeRemaining;

	void setUpEndScreen();

	// writes to a text file to store highest score
	void writeText();

	// reads text file to get highest score;
	void readTextFile();

	// load and set up sounds
	void initSounds();
	
	LevelData m_level;

	// An instance representing the player controlled tank.
	Tank m_tank;

	// instance representing ai controlled tank
	TankAi m_aiTank;

	// instance representing fuel pickup, base goal and cargo pickup
	cargoMode m_cargoMode;

	// sprites for obstacles
	std::vector<sf::Sprite> m_wallSprites;
	// A texture for the sprite sheet
	sf::Texture m_texture;

	// texture and sprite for background
	sf::Texture m_bgTexture; 
	sf::Sprite m_bgSprite;

	// sprites for targets
	std::vector<sf::Sprite> m_targetSprites;
	// A texture for the sprite sheet
	sf::Texture m_targetTexture;

	// main window
	sf::RenderWindow m_window;

	// text and font for game over text and HUD
	sf::Text m_gameOverText;
	sf::Font m_font;

	// string containing current objective
	std::string m_objectiveString = "Collect the Ammo!";

	// sound for target being hit by bullet
	sf::SoundBuffer m_targetHitBuffer;
	sf::Sound m_targetHitSound;

	// sound for target being appearing
	sf::SoundBuffer m_targetAppearBuffer;
	sf::Sound m_targetAppearSound;

	// text used to display how much time a target has remaining
	sf::Text m_targetTimerText;
	int m_remainingTime;

	// timer used for in game time, where upon reaching 0 the game is over
	// timer is in seconds 
	thor::Timer m_timer;
	static constexpr float TIMER_DURATION = 60.0f;

	// timer used for targets, in seconds
	thor::Timer m_targetTimer;

	// control when to display indictator of target about to vanish
	bool m_targetVanishing = false;

	// ensures read and writing to file only happens once
	bool m_readAndWrite = false;

	int m_playerScore = 0;
	int m_highestPlayerScore = 0;

	float m_highestTargetsHit = 0;
	float m_targetsHit = 0;

	float m_accuracyOfHighScore = 0;
	float m_accuracy = 0;

	// The initial game state set to GAME_RUNNING
	GameState m_gameState{ GameState::GAME_RUNNING };

	HUD m_hud;
};
