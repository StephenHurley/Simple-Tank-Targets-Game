#include "Game.h"
#include <iostream>
#include "tank.h"
#include "Bullet.h"

// Updates per milliseconds
static double const MS_PER_UPDATE = 10.0;

////////////////////////////////////////////////////////////
Game::Game()
	: m_window(sf::VideoMode(ScreenSize::s_width, ScreenSize::s_height, 32), "YAML Tanks", sf::Style::Default)
	, m_tank(m_texture, m_wallSprites, m_targetSprites, m_level.m_targets)
	, m_aiTank(m_texture, m_wallSprites)
	, m_hud(m_font)
{
	int currentLevel = 1;

	if (!m_texture.loadFromFile("./resources/images/SpriteSheet.png"))
	{
		std::string s("Error loading spritesheet");
		throw std::exception(s.c_str());
	}

	if (!m_targetTexture.loadFromFile("./resources/images/target.png"))
	{
		std::string s("Error loading target texture");
		throw std::exception(s.c_str());
	}

	// Will generate an exception if level loading fails.
	try
	{
		LevelLoader::load(currentLevel, m_level);
	}
	catch (std::exception& e)
	{
		std::cout << "Level Loading failure." << std::endl;
		std::cout << e.what() << std::endl;
		throw e;
	}

	// Now the level data is loaded, set the tank position
	m_tank.setPosition(m_level.m_tank.m_position);

	m_window.setVerticalSyncEnabled(true);

	if (!m_bgTexture.loadFromFile("./resources/images/Background.jpg"))
	{
		std::string s("Error loading background texture");
		throw std::exception(s.c_str());
	}
	m_bgSprite.setTexture(m_bgTexture);
	
	// generate the walls and targets in the game
	generateWalls();
	generateTargets();

	// timer set to 60 seconds
	m_timer.reset(sf::Time(sf::seconds(TIMER_DURATION)));
	// Initiate the timer
	m_timer.start();

	// target timer set to 10 seconds
	m_targetTimer.reset(sf::Time(sf::seconds(m_level.m_targets[0].m_duration)));
	// Initiate the target timer
	m_targetTimer.start();

	// set up font and texts used in the game
	setUpFontAndText();

	// load and set up sounds
	initSounds();

	// read in data from text file (this will not work if the file does not yet exist. In this case, the file is created at game over, then subsequent plays the values stored in the file are read)
	readTextFile();

	// Populate the obstacle list and set the AI tank position and set up its vision cone
	m_aiTank.init(m_level.m_aiTank.m_position);
}

////////////////////////////////////////////////////////////
void Game::run()
{
	sf::Clock clock;
	sf::Int32 lag = 0;

	while (m_window.isOpen())
	{
		sf::Time dt = clock.restart();

		lag += dt.asMilliseconds();

		processEvents();

		while (lag > MS_PER_UPDATE)
		{
			update(lag);
			lag -= MS_PER_UPDATE;
		}
		update(lag);

		render();
	}
}

////////////////////////////////////////////////////////////
void Game::processEvents()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_window.close();
		}
		processGameEvents(event);
	}
}

////////////////////////////////////////////////////////////
void Game::processGameEvents(sf::Event& event)
{
	// check if the event is a key pressed event
	if (sf::Event::KeyPressed == event.type)
	{
		switch (event.key.code)
		{
		case sf::Keyboard::Escape:
			m_window.close();
			break;

		default:
			break;
		}
	}
}

void Game::generateWalls()
{
	sf::IntRect wallRect(2, 129, 33, 23);
	// Create the Walls 
	for (ObstacleData const& obstacle : m_level.m_obstacles)
	{
		sf::Sprite sprite;
		sprite.setTexture(m_texture);
		sprite.setTextureRect(wallRect);
		sprite.setOrigin(wallRect.width / 2.0, wallRect.height / 2.0);
		sprite.setPosition(obstacle.m_position);
		sprite.setRotation(obstacle.m_rotation);
		m_wallSprites.push_back(sprite);
	}
}

void Game::generateTargets()
{
	// Create the targets
	for (TargetData const& target : m_level.m_targets)
	{
		sf::Sprite sprite;
		sprite.setTexture(m_targetTexture);
		sprite.setPosition(target.m_position);
		m_targetSprites.push_back(sprite);
		
	}
}

void Game::remainingTargetTime(int i)
{
	TargetData& target = m_level.m_targets[i];

	m_remainingTime = m_targetTimer.getRemainingTime().asSeconds();

	if (m_remainingTime < 5)
	{
		m_targetTimerText.setPosition(target.m_position + sf::Vector2f{ -5,20 });
		m_targetTimerText.setString(std::to_string(m_remainingTime));
		m_targetVanishing = true;
	}
}

void Game::targetShot(int i)
{
	TargetData& target = m_level.m_targets[i];
	
	m_targetHitSound.play();
	m_targetTimer.reset(sf::Time(sf::seconds(target.m_duration + m_remainingTime)));
	m_targetTimer.start();
	m_targetVanishing = false;
	target.m_removedFromGame = true;
	target.m_active = false;
	m_playerScore += 10;
	m_targetsHit++;
}

void Game::genNextTarget(int i)
{
	TargetData& target = m_level.m_targets[i];

	// prevent trying to access a previous target that doesnt exist
	if (i > 0)
	{
		// if previous target does exist, deactivate it and remove it from the game to prevent it being activated again
		TargetData& previousTarget = m_level.m_targets[i - 1];
		previousTarget.m_active = false;
		previousTarget.m_removedFromGame = true;
	}

	m_targetAppearSound.play();

	// reset timer and activate new target
	m_targetTimer.reset(sf::Time(sf::seconds(target.m_duration)));
	m_targetTimer.start();

	target.m_active = true;
	// disable target vanishing warning
	m_targetVanishing = false;
	
}



void Game::setUpFontAndText()
{
	if (!m_font.loadFromFile("./resources/fonts/arial.ttf"))
	{
		std::string s("Error loading font");
		throw std::exception(s.c_str());

	}

}

void Game::updateGameTimer()
{
	// get time remaining as seconds
    m_timeRemaining = m_timer.getRemainingTime().asSeconds();

	// when game time reaches 0, game is over (stops rendering and updating, displays game over screen)
	if (m_timer.isExpired())
	{
		m_gameState = GameState::GAME_WIN;
		m_objectiveString = "Time Ran Out!";

		if (m_targetsHit && m_tank.m_bulletsFired != 0)
		{
			m_accuracy = ( m_targetsHit / m_tank.m_bulletsFired ) * 100;
		}
		else
		{
			m_accuracy = 0;
		}
	}

}

void Game::setUpEndScreen()
{
	m_gameOverText.setFont(m_font);
	m_gameOverText.setCharacterSize(30);
	m_gameOverText.setFillColor(sf::Color::Blue);
	m_gameOverText.setStyle(sf::Text::Bold | sf::Text::Underlined);

	m_gameOverText.setString("GAME OVER.Targets hit: " 

		+ std::to_string(static_cast<int>(m_targetsHit)) 
		+ " / " + std::to_string(static_cast<int>(m_level.m_targets.size())) + "\n"
		+ "Bullets Fired: " + std::to_string(static_cast<int>(m_tank.m_bulletsFired)) + "\n"
		+ "Accuracy: " + std::to_string(static_cast<int> (m_accuracy)) + "%" + "\n"
		+ "Score: " + std::to_string(static_cast<int> (m_playerScore)) + "\n" + "\n"
		+ "BEST PERFORMANCE:" + "\n"
		+ "Targets Hit: " + std::to_string(static_cast<int>(m_highestTargetsHit))
		+ " / " + std::to_string(static_cast<int>(m_level.m_targets.size())) + "\n"
		+ "Bullets Fired: " + std::to_string(static_cast<int>(m_tank.m_bulletsFiredHighScore)) + "\n"
		+ "Accuracy: " + std::to_string(static_cast<int> (m_accuracyOfHighScore)) + "%" + "\n"
		+ "Score: " + std::to_string(static_cast<int> (m_highestPlayerScore)) + "\n");

	
	m_gameOverText.setOrigin(sf::Vector2f(m_gameOverText.getGlobalBounds().width / 2,
		m_gameOverText.getGlobalBounds().height / 2));

	m_gameOverText.setPosition(ScreenSize::s_width / 2, ScreenSize::s_height / 2);
}

void Game::writeText()
{
	std::ofstream outputFile;
	outputFile.open("highScore.txt"); // creates or replaces the file

	// writes to the file
	outputFile << m_playerScore << "\n";
	outputFile << m_tank.m_bulletsFired << "\n";
	outputFile << m_targetsHit << "\n";
	outputFile << m_accuracy << "\n";

	outputFile.close(); // close the file stream
	std::cout << std::endl << "Write successful" << std::endl;

}

void Game::readTextFile()
{
	std::string currentFile = "highScore.txt";;
	std::string aMessage = "Function is being called";
	
	std::ifstream inputFile;

	inputFile.open(currentFile); // opens the file

	if (inputFile.is_open()) // check if file was opened
	{
		inputFile >> m_highestPlayerScore;
		inputFile >> m_tank.m_bulletsFiredHighScore;
		inputFile >> m_highestTargetsHit;
		inputFile >> m_accuracyOfHighScore;
		inputFile.close(); // close the file stream

		aMessage = "Reading succesful";
	}
	else
	{
		aMessage = "File reading error occurred";
	}

	std::cout << aMessage;

}

void Game::initSounds()
{
	if (!m_targetHitBuffer.loadFromFile("./resources/sounds/TargetBreak.wav"))
	{
		std::string s("Error loading target break sound");
		throw std::exception(s.c_str());
	}

	m_targetHitSound.setBuffer(m_targetHitBuffer);
	m_targetHitSound.setVolume(30.0f);

	if (!m_targetAppearBuffer.loadFromFile("./resources/sounds/TargetAppear.wav"))
	{
		std::string s("Error loading target break sound");
		throw std::exception(s.c_str());
	}

	m_targetAppearSound.setBuffer(m_targetAppearBuffer);
	m_targetAppearSound.setVolume(30.0f);
}



////////////////////////////////////////////////////////////
void Game::update(double dt)
{

	m_hud.update(m_gameState,m_timeRemaining, m_tank.m_fuel, m_playerScore, m_objectiveString);

	if (m_aiTank.collidesWithPlayer(m_tank))
	{
		m_gameState = GameState::GAME_LOSE;
	}

	switch (m_gameState)
	{
	case GameState::GAME_RUNNING:

		m_tank.update(dt);

		if (m_tank.m_fuel <= 0)
		{
			m_gameState = GameState::GAME_LOSE;
		}

		m_aiTank.update(m_tank, dt);

		// check cargo mode logic
		// cargo not picked up yet
		if (m_cargoMode.m_cargoCollected == false)
		{
			if (m_cargoMode.playerPicksUpCargo(m_tank))
			{
				m_cargoMode.m_cargoCollected = true;
				m_objectiveString = "Deliver Ammo To Base!";
			}
		}
		else // cargo picked up, check if player is at goal
		{
			if (m_cargoMode.playerArrivesAtGoal(m_tank))
			{
				m_objectiveString = "Delivered Cargo!";
				m_gameState = GameState::GAME_WIN;
			}
		}
		
		// fuel not picked up yet
		if (m_cargoMode.m_fuelPickedUp == false)
		{
			// player is on fuel pickup
			if (m_cargoMode.playerPicksUpFuel(m_tank))
			{
				m_tank.m_fuel += 1000;
				
				// prevent over refilling
				if (m_tank.m_fuel > 3000)
				{
					m_tank.m_fuel = 3000;
				}
				m_cargoMode.m_fuelPickedUp = true;
			}
		}
		
		for (int i = 0; i < m_level.m_targets.size(); i++)
		{
			TargetData& target = m_level.m_targets[i];

			if (target.m_active)
			{
				remainingTargetTime(i);

				if (target.m_shot)
				{
					targetShot(i);
				}
			}

			//target can only pop up when previous target time has expired
			if (m_targetTimer.isExpired())
			{
				if (target.m_active == false && target.m_removedFromGame == false)
				{
					// current target is not active, deactivate last one and activate current one
					genNextTarget(i);

				}
				//removes last final target if player manages to break them all before time runs out
				else if (target.m_active == true && i == m_level.m_targets.size() - 1)
				{
					target.m_active = false;
					target.m_removedFromGame = true;
				}
			}
		}

		updateGameTimer();

		break;

	case GameState::GAME_WIN:

		if (m_readAndWrite == false)
		{
			if (m_playerScore >= m_highestPlayerScore)
			{
				writeText();
				readTextFile();
			}

			setUpEndScreen();

			m_readAndWrite = true;
		}
		
		break;

	case GameState::GAME_LOSE:

		setUpEndScreen();

		break;

	default:
		break;
	}

}

////////////////////////////////////////////////////////////
void Game::render()
{
	m_window.clear(sf::Color(0, 0, 0, 0));

	m_window.draw(m_bgSprite);

	switch (m_gameState)
	{
	case GameState::GAME_RUNNING:

		renderTargets();

		m_tank.render(m_window);

		m_aiTank.render(m_window);

		m_cargoMode.render(m_window);

		// draws the wall obstacles
		for (sf::Sprite sprite : m_wallSprites)
		{
			m_window.draw(sprite);
		}

		// draws active player bullets
		for (int i = 0; i < NUM_PLAYER_BULLETS; i++)
		{
			m_tank.m_bullets[i].render(m_window);
		}

		// draws active ai bullets
		for (int i = 0; i < NUM_AI_BULLETS; i++)
		{
			m_aiTank.m_AIBullets[i].render(m_window);
		}

		if (m_targetVanishing)
		{
			m_window.draw(m_targetTimerText);
		}

		break;

	case GameState::GAME_WIN:
		
		m_window.draw(m_gameOverText);

		break;

	case GameState::GAME_LOSE:
		
		m_window.draw(m_gameOverText);

		break;

	default:
		break;
	}

	m_hud.render(m_window);

	m_window.display();

}

void Game::renderTargets()
{
	// loop through all targets in the game
	for (int i = 0; i < m_level.m_targets.size(); i++)
	{
		// get target data
		TargetData const& target = m_level.m_targets[i];
		// get sprite that coresponds to current target data
		sf::Sprite sprite = m_targetSprites[i];

		// draw target sprite if coresponding target is active
		if (target.m_active)
		{
			m_window.draw(sprite);
		}
	}
}





