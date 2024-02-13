#pragma once

class GameWindow;
class Connect;

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics.hpp>
#include "../Thread.h"
#include "../thirdparties/jsoncpp/include/json/json.h"

class GameManager : public Thread
{

private:

	// Window attributes

	GameWindow*					m_window;
	sf::Image*					m_icon;

	// Game attributes

	bool						m_running;

	bool						m_previousClickState;

	bool						m_menu;
	bool						m_endScreen;
	bool						m_choiceScreen;
	bool						m_username;

	int							m_playerNumberEnemy;

	bool						m_playerSpectator;

	std::vector<sf::Text>		m_textList;

	sf::Text					m_scoreText;

	std::string					username;

	sf::Font					font;

	sf::Music* m_music;

	// Time attributes

	sf::Clock* m_Clock;
	float						m_deltaTime;
	float						m_fpsLimit;
	float						m_timeChange;

	// Textures attributes

	std::vector<sf::Sprite*>	m_sprites;

	// Connexion attributes

	Connect*					m_connect;

public:

	int							m_playerNumberSelf;
	char						m_map[3][4];
	int							m_currentTurn;
	int							m_currentPlayer;
	int							m_score;

	int							m_player1;
	int							m_player2;

	// Constructor/Destructor

	GameManager();
	~GameManager();

	// Called in main

	void		Start();
	virtual void EnterThreadFunction();
	virtual void ExecuteThreadFunction();
	virtual void ExitThreadFunction();

private:

	// Window related

	void		SetIcon();
	void		CloseWindow();
	void		LimitFps(float fps);
	void		DrawTerrain();
	void		DrawBoard();
	void		DrawWord();
	void		Sleep();
	void		RefreshWindow();

	// Generation related

	void		GenerateSprites();
	void		GenerateText();
	void		GenerateScoreText();
	void		GenerateMap();
	void		Generate();

	// Music related

	void		PlayMusic(const char* path);

	// Menu related

	void		ChooseMenu();
	void		Menu();

	// End screen related

	void		ChooseEnd();
	void		Player1WinScreen();
	void		Player2WinScreen();
	void		TieScreen();

	// Connexion related

	void		FormatAndSendMap();
	void		FormatAndSendPlayer();
	void		FormatAndSendInit();

	// Main methods

	void		Place();
	void		EndCheck();
	void		HandleEvents();

	// Multiplayer Methods
	char*		convertJsonToString(const Json::Value& json, std::string key);
	Json::Value convertStringToJson(const std::string& jsonString);
	void		convertJsonToMap(Json::Value& json);
	void		ChoicePlayerScreen();
	void		enterNameScreen();
	void		ChoosePlayer();
	bool		PlayerVerification(int playerNumber);
};
