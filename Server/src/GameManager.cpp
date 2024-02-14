#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <fstream>
#include <SocketLib.h>

#include "../include/GameManager.h"
#include "../include/GameWindow.h"
#include <json/json.h>
#include "../include/ConnectServer.h"

#define DEFAULT_BUFLEN 512
const float INPUT_BLOCK_TIME = 0.8f;

// EVENT LISTENER

ServerEventListener::ServerEventListener(GameManager* gameManager)
	: _gameManager(gameManager)
{}

ServerEventListener::~ServerEventListener()
{}

void ServerEventListener::HandleAccept(SOCKET sender) {
	SOCKET incomingSocket;
	incomingSocket = accept(sender, NULL, NULL);
	if (incomingSocket == INVALID_SOCKET) {
		closesocket(incomingSocket);
		std::cout << "Error accepting an incomming socket !" << std::endl;
		return;
	}
	_gameManager->m_Socket->Clients.push_back(incomingSocket);
	WSAAsyncSelect(incomingSocket,
		*_gameManager->m_Socket->AccessHWND(),
		WM_USER + 1, FD_READ | FD_CLOSE);
}

void ServerEventListener::HandleRead(SOCKET sender) {
	char recvbuf[DEFAULT_BUFLEN];
	int bytesRead = recv(sender, recvbuf, DEFAULT_BUFLEN, 0);
	if (bytesRead > 0) {
		// Analyser la chaîne JSON reçue
		std::string jsonReceived(recvbuf, bytesRead);
		Json::Value root;
		Json::Reader reader;
		
		bool parsingSuccessful = reader.parse(jsonReceived, root);
		if (!parsingSuccessful) {
			std::cout << "Erreur lors de l'analyse du JSON reçu : " << reader.getFormattedErrorMessages() << std::endl;
			return;
		}
		
		std::cout << "Received : " << root << std::endl;

		if (root.isMember("Key") && root["Key"] == "Picked")
			_gameManager->PickPlayer(root);
		if (root.isMember("Key") && root["Key"] == "Play")
			_gameManager->UpdateMap(root);
	}
}

void ServerEventListener::HandleClose(SOCKET sender) {
	std::cout << "Connection closed" << std::endl;
	std::vector<SOCKET>* clients = &_gameManager->m_Socket->Clients;
	for (int i = clients->size() - 1; i >= 0; i--)
	{
		if (clients->at(i) == sender) {
			clients->erase(clients->begin() + i);
			break;
		}
	}
	closesocket(sender);
}

// GAME MANAGER

GameManager::GameManager() {

	m_window = new GameWindow();
	m_icon = new sf::Image();

	m_endScreen = false;
	m_menu = true;
	m_running = true;
	m_currentTurn = 0;
	m_currentPlayer = 1;

	m_Clock = new sf::Clock();
	m_deltaTime = 0.f;
	m_fpsLimit = 1.0f / 120.0f;
	m_timeChange = 0.0f;

	m_music = new Music();

	m_previousClickState = false;

	m_eventListener = new ServerEventListener(this);
	m_Socket = new SocketLibrary::ServerSocket("21", m_eventListener);
}

/*
---------------------------------------------------------------------------------
|				Here are all the window related methods							|
---------------------------------------------------------------------------------
*/

void GameManager::SetIcon() {
	if (!m_icon->loadFromFile("rsrc/img/icon/icon.png"))
	{
		std::cout << "Error loading icon" << std::endl;
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

void GameManager::RefreshWindow() {
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
		std::cout << "Error loading object textures." << std::endl;
		exit(1);
	}

	sf::Sprite* crossSprite = new sf::Sprite();
	crossSprite->setTexture(*crossTexture);
	m_sprites.push_back(crossSprite);

	sf::Sprite* circleSprite = new sf::Sprite();
	circleSprite->setTexture(*circleTexture);
	m_sprites.push_back(circleSprite);
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
	m_player1 = 0;
	m_player2 = 0;
	if (m_sprites.empty())
		GenerateSprites();
	GenerateMap();

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
		std::cout << "Error loading " << path << std::endl;
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
		m_menu = false;
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
		std::cout << "Error loading menu background image" << std::endl;
		exit(1);
	}
	menuBackgroundSprite.setTexture(menuBackgroundTexture);
	SetIcon();
	//Playmusic("rsrc/music/menu.ogg");

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
		std::cout << "Error loading player 1 win screen background image" << std::endl;
		exit(1);
	}
	player1BackgroundSprite.setTexture(player1BackgroundTexture);
	//Playmusic("rsrc/music/endscreens/player1win.ogg");

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
		std::cout << "Error loading player 2 win screen background image" << std::endl;
		exit(1);
	}
	player2BackgroundSprite.setTexture(player2BackgroundTexture);
	//Playmusic("rsrc/music/endscreens/player2win.ogg");

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
		std::cout << "Error loading tie screen background image" << std::endl;
		exit(1);
	}
	tieBackgroundSprite.setTexture(tieBackgroundTexture);
	//Playmusic("rsrc/music/endscreens/tie.ogg");

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
|						Here are all the socket methods							|
---------------------------------------------------------------------------------
*/

void GameManager::PickPlayer(Json::Value picked)
{
	if (picked.isMember("Player1"))
		if (picked["Player1"] == 1)
			m_player1 = 1;
	if (picked.isMember("Player2"))
		if (picked["Player2"] == 1)
			m_player2 = 1;
	AlertPlayersOfPick();
}

void GameManager::UpdateMap(Json::Value play) {
	if (play.isMember("FirstLine") || play.isMember("SecondLine") || play.isMember("ThirdLine")) {
		std::string mapString;
		if (play.isMember("FirstLine"))
		{
			mapString = play["FirstLine"].asString();
			std::cout << mapString << std::endl;
			for (int i = 0; i < 3; ++i) {
				m_map[0][i] = mapString[i];
			}
			m_map[0][3] = '\0';
		}
		if (play.isMember("SecondLine"))
		{
			mapString = play["SecondLine"].asString();
			std::cout << mapString << std::endl;
			for (int i = 0; i < 3; ++i) {
				m_map[1][i] = mapString[i];
			}
			m_map[1][3] = '\0';
		}
		if (play.isMember("ThirdLine"))
		{
			mapString = play["ThirdLine"].asString();
			std::cout << mapString << std::endl;
			for (int i = 0; i < 3; ++i) {
				m_map[2][i] = mapString[i];
			}
			m_map[2][3] = '\0';
		}
		if (play.isMember("CurrentPlayer"))
		{
			if (play["CurrentPlayer"].asInt() == 1)
				m_currentPlayer = 2;
			else if (play["CurrentPlayer"].asInt() == 2)
				m_currentPlayer = 1;
		}
	}
	UpdateClients();
}

void GameManager::UpdateClients() {
	Json::Value root;
	root["Key"] = "Play";
	root["FirstLine"] = m_map[0];
	root["SecondLine"] = m_map[1];
	root["ThirdLine"] = m_map[2];
	root["CurrentPlayer"] = m_currentPlayer;
	std::cout << "Sending" << std::endl;
	std::cout << root << std::endl;
	std::string jsonToSend = root.toStyledString();

	for (SOCKET clientSocket : m_Socket->Clients) {
		int bytesSent = send(clientSocket, jsonToSend.c_str(), jsonToSend.length(), 0);
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Error sending data to client" << std::endl;
			// Gérer l'erreur, par exemple, fermer la connexion avec le client défaillant
		}
	}
}

void GameManager::AlertPlayersOfPick()
{
	Json::Value root;
	root["Key"] = "Picked";
	root["Player1"] = m_player1;
	root["Player2"] = m_player2;
	std::string jsonToSend = root.toStyledString();
	for (SOCKET clientSocket : m_Socket->Clients) {
		int bytesSent = send(clientSocket, jsonToSend.c_str(), jsonToSend.length(), 0);
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Error sending data to client" << std::endl;
			// Gérer l'erreur, par exemple, fermer la connexion avec le client défaillant
		}
	}
}

/*
---------------------------------------------------------------------------------
|						Here are all the main methods							|
---------------------------------------------------------------------------------
*/

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
	}
}

void GameManager::SendScore(int winner) {
	Json::Value root;



	if (winner == 1)
	{
		m_players[m_player1Username] += 1;
	}
	else if (winner == 2)
	{
		m_players[m_player2Username] += 1;
	}

	root["Player1Score"] = m_players[m_player1Username];
	root["Player2Score"] = m_players[m_player2Username];
	root["Key"] = "Score";

	std::string jsonToSend = root.toStyledString();

	for (SOCKET clientSocket : m_Socket->Clients) {
		int bytesSent = send(clientSocket, jsonToSend.c_str(), jsonToSend.length(), 0);
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Error sending data to client" << std::endl;
			// Gérer l'erreur, par exemple, fermer la connexion avec le client défaillant
		}
	}
}

void GameManager::EndCheck() {
	// Check rows
	for (int i = 0; i < 3; i++) {
		if (m_map[i][0] == 'x' && m_map[i][1] == 'x' && m_map[i][2] == 'x') {
			Generate();
			SendScore(1);
			UpdateClients();
			return;
		}
		if (m_map[i][0] == '.' && m_map[i][1] == '.' && m_map[i][2] == '.') {
			Generate();
			SendScore(2);
			UpdateClients();
			return;
		}
	}

	// Check columns
	for (int j = 0; j < 3; j++) {
		if (m_map[0][j] == 'x' && m_map[1][j] == 'x' && m_map[2][j] == 'x') {
			Generate();
			SendScore(1);
			UpdateClients();
			return;
		}
		if (m_map[0][j] == '.' && m_map[1][j] == '.' && m_map[2][j] == '.') {
			Generate();
			SendScore(2);
			UpdateClients();
			return;
		}
	}

	// Check diagonals
	if ((m_map[0][0] == 'x' && m_map[1][1] == 'x' && m_map[2][2] == 'x') ||
		(m_map[0][2] == 'x' && m_map[1][1] == 'x' && m_map[2][0] == 'x')) {
		Generate();
		SendScore(1);
		UpdateClients();
		return;
	}
	if ((m_map[0][0] == '.' && m_map[1][1] == '.' && m_map[2][2] == '.') ||
		(m_map[0][2] == '.' && m_map[1][1] == '.' && m_map[2][0] == '.')) {
		Generate();
		SendScore(2);
		UpdateClients();
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
		Generate();
		SendScore(0);
		UpdateClients();
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

			if (currentClickState && !m_previousClickState && m_window->w_window->hasFocus())
				Place();

			m_previousClickState = currentClickState;
		}
	}
}

void GameManager::Start() {
	float	fps = 0;
	m_Socket->Initialize();
	Generate();
	//Menu();
	m_menu = false;
	//Playmusic("rsrc/music/theme.ogg");
	while (m_running)
	{
		RefreshWindow();
		HandleEvents();
		EndCheck();
		LimitFps(fps);
		if (m_menu) {
			Generate();
			Menu();
			//Playmusic("rsrc/music/theme.ogg");
		}
	}
}

/*
---------------------------------------------------------------------------------
|						Here is the constructor methods							|
---------------------------------------------------------------------------------
*/

GameManager::~GameManager() {
	delete m_Socket;
	delete m_eventListener;
	delete m_window;
	delete m_icon;
	m_music->stop();
	delete m_music;
	m_sprites.clear();
}