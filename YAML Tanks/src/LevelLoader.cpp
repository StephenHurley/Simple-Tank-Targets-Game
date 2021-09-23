#include "LevelLoader.h"

// enum for corners of screen, in clockwise order, starting at top left
// this is used to determine the inital tank spawn
enum CORNERS {TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT};

/// The various operator >> overloads below are non-member functions used to extract
///  the game data from the YAML data structure.

/// <summary>
/// @brief Extracts the obstacle type, position and rotation values.
/// 
/// </summary>
/// <param name="t_obstacleNode">A YAML node</param>
/// <param name="t_obstacle">A simple struct to store the obstacle data</param>
////////////////////////////////////////////////////////////
void operator >> (const YAML::Node& t_obstacleNode, ObstacleData& t_obstacle)
{
	t_obstacle.m_type = t_obstacleNode["type"].as<std::string>();
	t_obstacle.m_position.x = t_obstacleNode["position"]["x"].as<float>();
	t_obstacle.m_position.y = t_obstacleNode["position"]["y"].as<float>();
	t_obstacle.m_rotation = t_obstacleNode["rotation"].as<double>();
}

/// <summary>
/// @brief Extracts the target type and position values.
/// 
/// </summary>
/// <param name="t_targetNode">A YAML node</param>
/// <param name="t_target">A simple struct to store the target data</param>
////////////////////////////////////////////////////////////
void operator >> (const YAML::Node& t_targetNode, TargetData& t_target)
{
	// extract the max offSet value of the target
	int maxOffSet = t_targetNode["position"]["maxOffSet"].as<float>();

	// double the offset value
	int doubleMaxOffSet = maxOffSet *2;

	// generate random number between this new doubled offset, then subtract the original max offset
	// This allows negative offset values to be generated
	int randomOffSet = (rand() % doubleMaxOffSet) - maxOffSet;

	t_target.m_type = t_targetNode["type"].as<std::string>();
	t_target.m_position.x = t_targetNode["position"]["x"].as<float>() + randomOffSet;

	randomOffSet = (rand() % doubleMaxOffSet) - maxOffSet;
	t_target.m_position.y = t_targetNode["position"]["y"].as<float>() + randomOffSet;

	t_target.m_duration = t_targetNode["duration"].as<float>();
	t_target.m_active = t_targetNode["active"].as<bool>();
	t_target.m_removedFromGame = t_targetNode["removedFromGame"].as<bool>();
	t_target.m_shot = t_targetNode["shot"].as<bool>();
}

/// <summary>
/// @brief Extracts the filename for the game background texture.
/// 
/// </summary>
/// <param name="t_backgroundNode">A YAML node</param>
/// <param name="t_background">A simple struct to store background related data</param>
////////////////////////////////////////////////////////////
void operator >> (const YAML::Node& t_backgroundNode, BackgroundData& t_background)
{
	t_background.m_fileName = t_backgroundNode["file"].as<std::string>();
}

/// <summary>
/// @brief Extracts the initial screen position for the player tank.
/// 
/// </summary>
/// <param name="t_tankNode">A YAML node</param>
/// <param name="t_tank">A simple struct to store data related to the player tank</param>
////////////////////////////////////////////////////////////
void operator >> (const YAML::Node& t_tankNode, TankData& t_tank)
{
	// get num in range 0 - 3, use this to determine which corner tank spawns in
	/*int randomCorner = rand() % 4;

	if (randomCorner == TOP_LEFT)
	{
		t_tank.m_position.x = t_tankNode["topLeft"]["x"].as<float>();
		t_tank.m_position.y = t_tankNode["topLeft"]["y"].as<float>();
	}
	else if (randomCorner == TOP_RIGHT)
	{
		t_tank.m_position.x = t_tankNode["topRight"]["x"].as<float>();
		t_tank.m_position.y = t_tankNode["topRight"]["y"].as<float>();
	}
	else if (randomCorner == BOTTOM_RIGHT)
	{
		t_tank.m_position.x = t_tankNode["bottomRight"]["x"].as<float>();
		t_tank.m_position.y = t_tankNode["bottomRight"]["y"].as<float>();
	}
	else
	{
		t_tank.m_position.x = t_tankNode["bottomLeft"]["x"].as<float>();
		t_tank.m_position.y = t_tankNode["bottomLeft"]["y"].as<float>();
	}*/
	
	t_tank.m_position.x = t_tankNode["tempPosition"]["x"].as<float>();
	t_tank.m_position.y = t_tankNode["tempPosition"]["y"].as<float>();
}

/// <summary>
/// @brief Top level function that extracts various game data from the YAML data stucture.
/// 
/// Invokes other functions to extract the background, tank, target and obstacle data.
//   Because there are multiple obstacles and targets, obstacle and target data are stored in vectors.
/// </summary>
/// <param name="t_levelNode">A YAML node</param>
/// <param name="t_level">A simple struct to store level data for the game</param>
////////////////////////////////////////////////////////////
void operator >> (const YAML::Node& t_levelNode, LevelData& t_level)
{
	t_levelNode["background"] >> t_level.m_background;

	t_levelNode["tank"] >> t_level.m_tank;

	t_levelNode["ai_tank"] >> t_level.m_aiTank;

	const YAML::Node& obstaclesNode = t_levelNode["obstacles"].as<YAML::Node>();
	for (unsigned i = 0; i < obstaclesNode.size(); ++i)
	{
		ObstacleData obstacle;
		obstaclesNode[i] >> obstacle;
		t_level.m_obstacles.push_back(obstacle);
	}

	const YAML::Node& targetsNode = t_levelNode["targets"].as<YAML::Node>();
	for (unsigned i = 0; i < targetsNode.size(); ++i)
	{
		TargetData target;
		targetsNode[i] >> target;
		t_level.m_targets.push_back(target);
	}
}

////////////////////////////////////////////////////////////
void LevelLoader::load(int t_levelNr, LevelData& t_level)
{
	std::string filename = "./resources/levels/level" + std::to_string(t_levelNr) + ".yaml";

	try
	{
		YAML::Node baseNode = YAML::LoadFile(filename);
		if (baseNode.IsNull())
		{
			std::string message("File: " + filename + " not found");
			throw std::exception(message.c_str());
		}
		baseNode >> t_level;
	}
	catch (YAML::ParserException& e)
	{
		std::string message(e.what());
		message = "YAML Parser Error: " + message;
		throw std::exception(message.c_str());
	}
	catch (std::exception& e)
	{
		std::string message(e.what());
		message = "Unexpected Error: " + message;
		throw std::exception(message.c_str());
	}
}


