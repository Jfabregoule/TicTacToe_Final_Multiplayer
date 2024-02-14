#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>

#include "../include/GameManager.h"
#include "../include/GameWindow.h"
#include "../thirdparties/jsoncpp/include/json/json.h"
#include "../include/Connect.h"

const float INPUT_BLOCK_TIME = 0.8f;

#define DEFAULT_BUFLEN 512


GameManager::GameManager() {
	//std::cout << "CLIENT" << std::endl;
	m_window = new GameWindow();
	m_icon = new sf::Image();

	m_endScreen = false;
	m_menu = true;
	m_running = true;
	m_username = true;
	m_choiceScreen = false;

	m_currentTurn = 0;
	m_currentPlayer = 1;

	m_playerNumberSelf = -1;
	m_playerNumberEnemy = -1;

	m_player1 = 0;
	m_player2 = 0;

	m_playerSpectator = false;

	username = "";

	m_Clock = new sf::Clock();
	m_deltaTime = 0.f;
	m_fpsLimit = 1.0f / 120.0f;
	m_timeChange = 0.0f;

	m_music = new Music();

	m_previousClickState = false;

	m_connect = new Connect(*this);

	if (!font.loadFromFile("rsrc/font/Caveat-Regular.ttf")) {
		std::cerr << "Erreur lors du chargement de la police" << std::endl;
	}
}

/*
---------------------------------------------------------------------------------
|				Here are all the window related methods							|
---------------------------------------------------------------------------------
*/

void GameManager::SetIcon() {
	if (!m_icon->loadFromFile("rsrc/img/icon/icon.png"))
	{
		//std::cout << "Error loading icon" << std::endl;
		exit(1);
	}
	m_window->w_window->setIcon(m_icon->getSize().x, m_icon->getSize().y, m_icon->getPixelsPtr());
}

void	GameManager::Sleep() {
	m_window->w_window->clear();
	sf::sleep(sf::milliseconds(800));
}

void	GameManager::CloseWindow() {
	m_music->stop();
	delete m_music;
	m_window->w_window->close();
	exit(0);
}

void GameManager::LimitFps(float fps) {
	m_deltaTime = m_Clock->restart().asSeconds();
	m_timeChange += m_deltaTime;
	if (m_deltaTime < m_fpsLimit) {
		sleep(seconds(m_fpsLimit - m_deltaTime));
		m_deltaTime += m_fpsLimit - m_deltaTime;
	}
	fps = 1.0f / m_deltaTime;
}

void GameManager::DrawTerrain() {
	sf::RectangleShape collumn1(sf::Vector2f(600.0f, 5.f));
	collumn1.setPosition(202.5f, 0.0f);
	collumn1.rotate(90.0f);
	m_window->w_window->draw(collumn1);

	sf::RectangleShape collumn2(sf::Vector2f(600.0f, 5.f));
	collumn2.setPosition(402.5f, 0.0f);
	collumn2.rotate(90.0f);
	m_window->w_window->draw(collumn2);

	sf::RectangleShape row1(sf::Vector2f(600.0f, 5.f));
	row1.setPosition(0.0f, 202.5f);
	m_window->w_window->draw(row1);

	sf::RectangleShape row2(sf::Vector2f(600.0f, 5.f));
	row2.setPosition(0.0f, 402.5f);
	m_window->w_window->draw(row2);
}

void GameManager::DrawBoard() {
	int		j = 0;
	bool	isEmpty = true;

	float	posX = 98.75f;
	float	posY = 98.75f;

	sf::Sprite currentSprite;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			switch (m_map[i][j])
			{
			case '0':
				isEmpty = true;
				break;
			case 'x':
				isEmpty = false;
				currentSprite = *m_sprites.at(0);
				break;
			case '.':
				isEmpty = false;
				currentSprite = *m_sprites.at(1);
				break;
			}
			if (!isEmpty)
			{
				currentSprite.setOrigin(75.0f, 75.0f);
				currentSprite.setPosition(posX, posY);
				m_window->w_window->draw(currentSprite);
			}
			posX += 200;
		}
		posX = 98.75f;
		posY += 200;
	}
}

void GameManager::DrawWord() {
	if (m_username) {
		m_window->w_window->draw(m_textList[0]);
		m_window->w_window->draw(m_textList[1]);
	}
	else if (m_menu) {
		m_window->w_window->draw(m_textList[2]);
		m_window->w_window->draw(m_textList[3]);
	}
}

void GameManager::RefreshWindow() {
	m_username = false;
	m_window->RefreshScreen();
	DrawTerrain();
	DrawBoard();
	m_window->w_window->display();
}

/*
---------------------------------------------------------------------------------
|				Here are all the generation methods								|
---------------------------------------------------------------------------------
*/

void GameManager::GenerateSprites() {
	sf::Texture* crossTexture = new sf::Texture();
	sf::Texture* circleTexture = new sf::Texture();
	if (!crossTexture->loadFromFile("rsrc/img/objects/cross.png") or !circleTexture->loadFromFile("rsrc/img/objects/circle.png")) {
		//std::cout << "Error loading object textures." << std::endl;
		exit(1);
	}

	sf::Sprite* crossSprite = new sf::Sprite();
	crossSprite->setTexture(*crossTexture);
	m_sprites.push_back(crossSprite);

	sf::Sprite* circleSprite = new sf::Sprite();
	circleSprite->setTexture(*circleTexture);
	m_sprites.push_back(circleSprite);
}

void GameManager::GenerateText() {
	sf::Text textInstruction("Entrez votre nom d'utilisateur :", font, 30);
	textInstruction.setPosition(100, 200);

	sf::Text textUsername("", font, 30);
	textUsername.setPosition(100, 250);


	m_textList.push_back(textInstruction);
	m_textList.push_back(textUsername);
}

void GameManager::GenerateScoreText() {
	sf::Text scoreText(username + "'s Scores :" + std::to_string(m_score), font, 30);
	scoreText.setPosition(75, 50);


	m_scoreText = scoreText;
}

void GameManager::GenerateMap() {
	int	j = 0;

	for (int i = 0; i < 3; i++)
	{
		for (j; j < 3; j++)
		{
			m_map[i][j] = '0';
		}
		m_map[i][j] = '\0';
		j = 0;
	}
}

void GameManager::Generate() {
	if (m_sprites.empty())
		GenerateSprites();
	m_currentPlayer = 1;
	GenerateMap();
	GenerateText();

	//GenerateHud();
}

/*
---------------------------------------------------------------------------------
|				Here are all the music related methods							|
---------------------------------------------------------------------------------
*/

void	GameManager::PlayMusic(const char* path) {
	m_music->stop();
	if (!m_music->openFromFile(path)) {
		//std::cout << "Error loading " << path << std::endl;
		exit(1);
	}
	m_music->setVolume(10.0f);
	m_music->play();
	m_music->setLoop(true);
}

/*
---------------------------------------------------------------------------------
|				Here are all the menu related methods							|
---------------------------------------------------------------------------------
*/

void GameManager::ChooseMenu() {
	sf::Vector2i	position = sf::Mouse::getPosition(*m_window->w_window);
	sf::Vector2u	windowSize = m_window->w_window->getSize();

	if (position.y <= windowSize.y / 2)
	{
		if (m_player1 == 1 and m_player2 == 1)
		{
			m_menu = false;
			m_choiceScreen = false;
			m_playerSpectator = true;
		}
		else {
			m_playerSpectator = false;
			ChoicePlayerScreen();
		}
	}
	else if (position.y > windowSize.y / 2) {
		m_menu = false;
		m_running = false;
	}
}

void GameManager::Menu() {
	Event		event;
	sf::Texture	menuBackgroundTexture;
	sf::Sprite	menuBackgroundSprite;

	if (!menuBackgroundTexture.loadFromFile("rsrc/img/menu/background.png")) {
		//std::cout << "Error loading menu background image" << std::endl;
		exit(1);
	}
	menuBackgroundSprite.setTexture(menuBackgroundTexture);
	SetIcon();
	//PlayMusic("rsrc/music/menu.ogg");

	m_timeChange = 0.0f;
	while (m_menu) {

		while (m_window->w_window->pollEvent(event))
		{
			if (m_timeChange > INPUT_BLOCK_TIME) {
				if (event.type == Event::Closed)
					CloseWindow();

				if (Mouse::isButtonPressed(Mouse::Button::Left) && m_window->w_window->hasFocus())
					ChooseMenu();
			}
		}
		m_window->w_window->draw(menuBackgroundSprite);
		GenerateScoreText();
		m_window->w_window->draw(m_scoreText);
		m_window->w_window->display();
		LimitFps(60.0f);
	}
	RefreshWindow();
	m_timeChange = 0.0f;
}

/*
---------------------------------------------------------------------------------
|					Here are all the end screens methods						|
---------------------------------------------------------------------------------
*/

void GameManager::ChooseEnd() {
	sf::Vector2i	position = sf::Mouse::getPosition(*m_window->w_window);
	sf::Vector2u	windowSize = m_window->w_window->getSize();

	if (position.y <= windowSize.y / 2) {
		m_menu = true;
		m_endScreen = false;

		m_playerNumberSelf = -1;
		m_playerNumberEnemy = -1;
		Menu();
	}
	else if (position.y > windowSize.y / 2) {
		m_endScreen = false;
		m_menu = false;
		m_running = false;
	}
}

void GameManager::Player1WinScreen() {
	Event		event;
	sf::Texture	player1BackgroundTexture;
	sf::Sprite	player1BackgroundSprite;

	if (!player1BackgroundTexture.loadFromFile("rsrc/img/end/player1background.png")) {
		//std::cout << "Error loading player 1 win screen background image" << std::endl;
		exit(1);
	}
	player1BackgroundSprite.setTexture(player1BackgroundTexture);
	//PlayMusic("rsrc/music/endscreens/player1win.ogg");

	m_endScreen = true;
	m_timeChange = 0.0f;
	while (m_endScreen) {

		while (m_window->w_window->pollEvent(event))
		{
			if (m_timeChange > INPUT_BLOCK_TIME)
			{
				if (event.type == Event::Closed)
					CloseWindow();

				if (Mouse::isButtonPressed(Mouse::Button::Left) && m_window->w_window->hasFocus())
					ChooseEnd();
			}
		}
		m_window->w_window->draw(player1BackgroundSprite);
		m_window->w_window->display();
		LimitFps(60.0f);
	}
}

void GameManager::Player2WinScreen() {
	Event		event;
	sf::Texture	player2BackgroundTexture;
	sf::Sprite	player2BackgroundSprite;

	if (!player2BackgroundTexture.loadFromFile("rsrc/img/end/player2background.png")) {
		//std::cout << "Error loading player 2 win screen background image" << std::endl;
		exit(1);
	}
	player2BackgroundSprite.setTexture(player2BackgroundTexture);
	//PlayMusic("rsrc/music/endscreens/player2win.ogg");

	m_endScreen = true;
	m_timeChange = 0.0f;
	while (m_endScreen) {

		while (m_window->w_window->pollEvent(event))
		{
			if (event.type == Event::Closed)
				CloseWindow();

			if (Mouse::isButtonPressed(Mouse::Button::Left) && m_window->w_window->hasFocus())
				ChooseEnd();
		}
		m_window->w_window->draw(player2BackgroundSprite);
		m_window->w_window->display();
		LimitFps(60.0f);
	}
}

void GameManager::TieScreen() {
	Event		event;
	sf::Texture	tieBackgroundTexture;
	sf::Sprite	tieBackgroundSprite;

	if (!tieBackgroundTexture.loadFromFile("rsrc/img/end/tiebackground.png")) {
		//std::cout << "Error loading tie screen background image" << std::endl;
		exit(1);
	}
	tieBackgroundSprite.setTexture(tieBackgroundTexture);
	//PlayMusic("rsrc/music/endscreens/tie.ogg");

	m_endScreen = true;
	m_timeChange = 0.0f;
	while (m_endScreen) {

		while (m_window->w_window->pollEvent(event))
		{
			if (event.type == Event::Closed)
				CloseWindow();

			if (Mouse::isButtonPressed(Mouse::Button::Left) && m_window->w_window->hasFocus())
				ChooseEnd();
		}
		m_window->w_window->draw(tieBackgroundSprite);
		m_window->w_window->display();
		LimitFps(60.0f);
	}
}

/*
---------------------------------------------------------------------------------
|						Here are all the main methods							|
---------------------------------------------------------------------------------
*/

void GameManager::FormatAndSendMap() {
	// Cr�ation d'un objet Json::Value
	const char* formatedJson;
	Json::Value root;
	int			sendResult;

	// Ajout des lignes de la carte au JSON
	root["Key"] = "Play";
	root["FirstLine"] = m_map[0];
	root["SecondLine"] = m_map[1];
	root["ThirdLine"] = m_map[2];

	// Ajout du joueur courant au JSON
	root["CurrentPlayer"] = m_playerNumberSelf;

	// Cr�ation d'un objet Json::StyledWriter pour une sortie format�e
	Json::StyledWriter writer;

	// Convertir le Json::Value en cha�ne JSON format�e
	std::string jsonOutput = writer.write(root);

	// Allouer de la m�moire pour la cha�ne JSON en tant que const char*
	char* jsonString = new char[jsonOutput.size() + 1];
	strcpy_s(jsonString, jsonOutput.size() + 1, jsonOutput.c_str());

	formatedJson = jsonString;

	sendResult = m_connect->Send(formatedJson);

	delete[] jsonString;

	if (sendResult != 0) {
		std::cerr << "Error sending JSON to client." << std::endl;
		// G�rer l'erreur de mani�re appropri�e dans votre application
	}
}

void GameManager::FormatAndSendInit() {
	// Cr�ation d'un objet Json::Value
	const char* formatedJson;
	Json::Value root;
	int			sendResult;


	// Ajout du joueur courant au JSON
	root["Key"] = "Init";
	root["Username"] = username;

	// Cr�ation d'un objet Json::StyledWriter pour une sortie format�e
	Json::StyledWriter writer;

	// Convertir le Json::Value en cha�ne JSON format�e
	std::string jsonOutput = writer.write(root);

	// Allouer de la m�moire pour la cha�ne JSON en tant que const char*
	char* jsonString = new char[jsonOutput.size() + 1];
	strcpy_s(jsonString, jsonOutput.size() + 1, jsonOutput.c_str());

	formatedJson = jsonString;

	//std::cout << "Sending :" << formatedJson << std::endl;

	sendResult = m_connect->Send(formatedJson);

	delete[] jsonString;

	if (sendResult != 0) {
		std::cerr << "Error sending JSON to client." << std::endl;
		// G�rer l'erreur de mani�re appropri�e dans votre application
	}
}

void GameManager::FormatAndSendPlayer() {
	// Cr�ation d'un objet Json::Value
	const char* formatedJson;
	Json::Value root;
	int			sendResult;


	// Ajout du joueur courant au JSON
	root["Key"] = "Picked";
	root["Username"] = username;
	root["PlayerNumber"] = m_playerNumberSelf;
	if (m_player1 == 1)
	{
		root["Player1"] = 1;
	}
	else {
		root["Player1"] = 0;
	}
	if (m_player2 == 1) {
		root["Player2"] = 1;
	}
	else {
		root["Player2"] = 0;
	}

	// Cr�ation d'un objet Json::StyledWriter pour une sortie format�e
	Json::StyledWriter writer;

	// Convertir le Json::Value en cha�ne JSON format�e
	std::string jsonOutput = writer.write(root);

	// Allouer de la m�moire pour la cha�ne JSON en tant que const char*
	char* jsonString = new char[jsonOutput.size() + 1];
	strcpy_s(jsonString, jsonOutput.size() + 1, jsonOutput.c_str());

	formatedJson = jsonString;

	//std::cout << "Sending :" << formatedJson << std::endl;

	sendResult = m_connect->Send(formatedJson);

	delete[] jsonString;

	if (sendResult != 0) {
		std::cerr << "Error sending JSON to client." << std::endl;
		// G�rer l'erreur de mani�re appropri�e dans votre application
	}
}

void GameManager::Place() {
	char			c;
	char* toReplace = nullptr;
	sf::Vector2i	position = sf::Mouse::getPosition(*m_window->w_window);
	sf::Vector2u	windowSize = m_window->w_window->getSize();

	int i = -1, j = -1;
	if (m_currentPlayer == 1)
		c = 'x';
	else
		c = '.';
	if (position.x <= windowSize.x / 3)
		j = 0;
	else if (position.x >= windowSize.x / 3 and position.x <= windowSize.x / 3 * 2)
		j = 1;
	else if (position.x >= windowSize.x / 3 * 2)
		j = 2;

	if (position.y <= windowSize.y / 3)
		i = 0;
	else if (position.y >= windowSize.y / 3 and position.y <= windowSize.y / 3 * 2)
		i = 1;
	else if (position.y >= windowSize.y / 3 * 2)
		i = 2;

	if (i != -1 && j != -1 && m_map[i][j] == '0') {
		toReplace = &m_map[i][j];
		*toReplace = c;

		if (m_currentPlayer == 1)
			m_currentPlayer = 2;
		else
			m_currentPlayer = 1;
		FormatAndSendMap();
	}
}

void GameManager::EndCheck() {
	// Check rows
	for (int i = 0; i < 3; i++) {
		if (m_map[i][0] == 'x' && m_map[i][1] == 'x' && m_map[i][2] == 'x') {
			m_player1 = 0;
			m_player2 = 0;
			Generate();
			Player1WinScreen();
			return;
		}
		if (m_map[i][0] == '.' && m_map[i][1] == '.' && m_map[i][2] == '.') {
			m_player1 = 0;
			m_player2 = 0;
			Generate();
			Player2WinScreen();
			return;
		}
	}

	// Check columns
	for (int j = 0; j < 3; j++) {
		if (m_map[0][j] == 'x' && m_map[1][j] == 'x' && m_map[2][j] == 'x') {
			m_player1 = 0;
			m_player2 = 0;
			Generate();
			Player1WinScreen();
			return;
		}
		if (m_map[0][j] == '.' && m_map[1][j] == '.' && m_map[2][j] == '.') {
			m_player1 = 0;
			m_player2 = 0;
			Generate();
			Player2WinScreen();
			return;
		}
	}

	// Check diagonals
	if ((m_map[0][0] == 'x' && m_map[1][1] == 'x' && m_map[2][2] == 'x') ||
		(m_map[0][2] == 'x' && m_map[1][1] == 'x' && m_map[2][0] == 'x')) {
		m_player1 = 0;
		m_player2 = 0;
		Generate();
		Player1WinScreen();
		return;
	}
	if ((m_map[0][0] == '.' && m_map[1][1] == '.' && m_map[2][2] == '.') ||
		(m_map[0][2] == '.' && m_map[1][1] == '.' && m_map[2][0] == '.')) {
		m_player1 = 0;
		m_player2 = 0;
		Generate();
		Player2WinScreen();
		return;
	}

	// Check tie
	bool isTie = true;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (m_map[i][j] == '0') {
				isTie = false;
				break;
			}
		}
	}

	if (isTie) {
		m_player1 = 0;
		m_player2 = 0;
		Generate();
		TieScreen();
	}
}

void GameManager::HandleEvents() {
	Event	event;
	bool	currentClickState;

	while (m_window->w_window->pollEvent(event))
	{
		if (m_timeChange > INPUT_BLOCK_TIME)
		{
			currentClickState = Mouse::isButtonPressed(Mouse::Button::Left);
			if (event.type == Event::Closed)
				CloseWindow();

			if (currentClickState && !m_previousClickState && m_window->w_window->hasFocus()) {
				if (m_currentPlayer == m_playerNumberSelf and m_player1 == 1 and m_player2 == 1 and !m_playerSpectator)
				{
					Place();
					currentClickState = false;
				}
			}

			m_previousClickState = currentClickState;
		}
	}
}

void GameManager::Start() {
	float	fps = 0;

	Generate();
	//Menu();
	//PlayMusic("rsrc/music/theme.ogg");
	enterNameScreen();
	while (m_running)
	{
		RefreshWindow();
		HandleEvents();
		EndCheck();
		LimitFps(fps);
		if (m_menu) {
			Generate();
			enterNameScreen();
			//Menu();
			//PlayMusic("rsrc/music/theme.ogg");
		}
	}
}


/*
---------------------------------------------------------------------------------
|						Here is the Multiplayers methods						|
---------------------------------------------------------------------------------
*/

void GameManager::ChoosePlayer() {
	sf::Vector2i	position = sf::Mouse::getPosition(*m_window->w_window);
	sf::Vector2u	windowSize = m_window->w_window->getSize();

	if (position.y <= windowSize.y / 2) {
		if (PlayerVerification(1))
		{
			m_player1 = 1;
			FormatAndSendPlayer();
			m_menu = false;
			m_choiceScreen = false;
			Generate();
		}else if (m_player1 == 1 and m_player2 == 1) {
			m_playerSpectator = true;
			m_menu = false;
			m_choiceScreen = false;
			Generate();
		}
	}
	else if (position.y > windowSize.y / 2) {
		if (PlayerVerification(2))
		{
			//std::cout << "Player 2 picked" << std::endl;
			m_player2 = 1;
			FormatAndSendPlayer();
			m_menu = false;
			m_choiceScreen = false;
			Generate();
		}else if (m_player1 == 1 and m_player2 == 1) {
			m_playerSpectator = true;
			m_menu = false;
			m_choiceScreen = false;
			Generate();
		}
	}
}

void GameManager::enterNameScreen() {
	Event		event;
	sf::Texture	menuBackgroundTexture;
	sf::Sprite	menuBackgroundSprite;

	if (!menuBackgroundTexture.loadFromFile("rsrc/img/menu/PlayerName.png")) {
		//std::cout << "Error loading menu background image" << std::endl;
		exit(1);
	}

	menuBackgroundSprite.setTexture(menuBackgroundTexture);
	while (m_username) {
		while (m_window->w_window->pollEvent(event))
		{
			if (m_timeChange > INPUT_BLOCK_TIME)
			{
				if (event.type == Event::Closed)
					CloseWindow();

				if (event.type == sf::Event::TextEntered) {
					if (event.text.unicode < 128) {
						char enteredChar = static_cast<char>(event.text.unicode);
						if (enteredChar == '\b' && !username.empty()) {
							// Backspace : supprimer le dernier caract�re
							username.pop_back();
						}
						else if (enteredChar != '\b') {
							// Ajouter le caract�re � la cha�ne du nom d'utilisateur
							username += enteredChar;
						}
						// Mettre � jour le texte affich�
						m_textList[1].setString(username);
					}
				}
				if (event.key.code == sf::Keyboard::Enter) {
					FormatAndSendInit();
					m_username = false;
					m_menu = true;
					Menu();
				}
			}
		}
		m_window->w_window->draw(menuBackgroundSprite);
		DrawWord();
		m_window->w_window->display();
		LimitFps(60.0f);
	}
}

void GameManager::ChoicePlayerScreen() {
	Event		event;
	sf::Texture	choiceBackgroundTexture;
	sf::Sprite	choiceBackgroundSprite;

	if (!choiceBackgroundTexture.loadFromFile("rsrc/img/playerChoice/choiceBackground.png")) {
		//std::cout << "Error loading choice player screen background image" << std::endl;
		exit(1);
	}
	choiceBackgroundSprite.setTexture(choiceBackgroundTexture);
	//PlayMusic("rsrc/music/endscreens/player1win.ogg");

	m_choiceScreen = true;
	m_timeChange = 0.0f;
	//m_timeChange = 0.0f;
	while (m_choiceScreen) {

		while (m_window->w_window->pollEvent(event))
		{
			if (m_timeChange > INPUT_BLOCK_TIME)
			{
				if (event.type == Event::Closed)
					CloseWindow();

				if (Mouse::isButtonPressed(Mouse::Button::Left) && m_window->w_window->hasFocus())
					ChoosePlayer();
			}
		}
		m_window->w_window->draw(choiceBackgroundSprite);
		m_window->w_window->display();
		LimitFps(60.0f);
	}
}

bool GameManager::PlayerVerification(int playerNumber) {
	if ((m_player1 == 0 and playerNumber == 1) or (m_player2 == 0 and playerNumber == 2))
	{
		m_playerNumberSelf = playerNumber;

		if (playerNumber == 1) {
			m_playerNumberEnemy = 2;
		}
		else {
			m_playerNumberEnemy = 1;
		}

		/*if (m_playerNumberEnemy == -1) {

			if (playerNumber == 1) {
				m_playerNumberEnemy = 2;
			}
			else {
				m_playerNumberEnemy = 1;
			}

			return true;
		}
		else if (m_playerNumberEnemy == 1 && playerNumber == 2) {
			m_playerNumberSelf = 2;
			return true;
		}
		else if (m_playerNumberEnemy == 2 && playerNumber == 1) {
			m_playerNumberSelf = 1;
			return true;
		}*/
		return true;
	}
	
	return false;
}

/*
---------------------------------------------------------------------------------
|						Here is the constructor methods							|
---------------------------------------------------------------------------------
*/

GameManager::~GameManager() {
	delete m_window;
	delete m_icon;
	m_music->stop();
	delete m_music;
	m_sprites.clear();
}