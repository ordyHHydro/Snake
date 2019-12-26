#include <iostream>
#include <ctime>
#include "TileMap/TileMap.hpp"
#include "Entity/Entity.hpp"

void fatalp(const std::string& msg)
{
    std::cerr << "ERROR: " << msg << std::endl;
    exit(EXIT_FAILURE);
}

int main()
{
    srand(time(nullptr));

    /* Window icon */
    sf::Image windowIcon;
    if(!windowIcon.loadFromFile("res/window_icon.png"))
        fatalp("\"res/window_icon.png\" failed to load, aborting");

    /* Text */
    sf::Font textFont;
    if(!textFont.loadFromFile("res/bit5x3.ttf"))
        fatalp("\"res/bit5x3.ttf\" failed to load, aborting");

    /* Window settings */
    const sf::Vector2u SCREEN_SIZE(800, 600);
    const sf::Vector2u GAME_SIZE(600, 600);
    sf::RenderWindow window(sf::VideoMode(SCREEN_SIZE.x, SCREEN_SIZE.y), "Snake", sf::Style::Close);
    window.setIcon(windowIcon.getSize().x, windowIcon.getSize().y, windowIcon.getPixelsPtr());
    window.setFramerateLimit(60);

    /* Internal clock */
    sf::Clock gameClock;
    const sf::Time MIN_TICK_FREQ = sf::milliseconds(40);
    const sf::Time INITIAL_TICK_FREQ = sf::milliseconds(100);
    const sf::Time TICK_FREQ_INTERVAL = sf::milliseconds(5);
    sf::Time tickFreq = INITIAL_TICK_FREQ;

    /* Entities */
    const sf::Vector2u TILE_COUNT(20, 20);
    const TM::Map map(GAME_SIZE, TILE_COUNT);
    const TM::Tile SNAKE_START(map.getTileCount().x / 2, map.getTileCount().y / 2);
    Entity::Snake snake(map, SNAKE_START, sf::Color::Green);
    Entity::Fruit fruit(map, sf::Color::White);

    /* Text */
    const unsigned int FONT_SIZE = 20;
    sf::Text deathText("Dead, press space to restart", textFont, FONT_SIZE);
    sf::Text scoreText("", textFont, FONT_SIZE);
    sf::Text borderText("", textFont, FONT_SIZE);
    
    /* Text positions */
    const TM::Tile DEATH_TEXT_TILE(0, 0);
    const TM::Tile SCORE_TEXT_TILE(TILE_COUNT.x + 1, 0);
    const TM::Tile BORDER_TEXT_TILE(TILE_COUNT.x + 1, 2);
    deathText.setPosition(map.tileToPixel(DEATH_TEXT_TILE));
    scoreText.setPosition(map.tileToPixel(SCORE_TEXT_TILE));
    borderText.setPosition(map.tileToPixel(BORDER_TEXT_TILE));

    /* Text colors */
    deathText.setFillColor(sf::Color::Red);
    scoreText.setFillColor(sf::Color::White);
    borderText.setFillColor(sf::Color::White);

    /* Game-text border */
    sf::RectangleShape gameTextBorder(sf::Vector2f(1, GAME_SIZE.y));
    gameTextBorder.setPosition(GAME_SIZE.x, 0);
    gameTextBorder.setFillColor(sf::Color::White);

    /* Rainbow color */
    unsigned int nextColorIndex = 0;
    const unsigned int MAX_COLOR = 24;
    const sf::Color RAINBOW[MAX_COLOR] =
    {{255,   0,   0}, {255,  64,   0}, {255, 128,   0}, {255, 191,   0},
     {255, 255,   0}, {191, 255,   0}, {128, 255,   0}, { 64, 255,   0},
     {  0, 255,   0}, {  0, 255,  64}, {  0, 255, 128}, {  0, 255, 191},
     {  0, 255, 255}, {  0, 191, 255}, {  0, 128, 255}, {  0,  64, 255},
     {  0,   0, 255}, { 64,   0, 255}, {128,   0, 255}, {191,   0, 255},
     {255,   0, 255}, {255,   0, 191}, {255,   0, 128}, {255,   0,  64}};
    //Colors obtained from https://www.w3schools.com/colors/colors_picker.asp

    /* Debug flags */
    bool debugTileDraw  = false;
    bool debugTileClick = false;

    /* Game flags */
    bool inputAllowed = true;
    bool gameover     = false;
    bool borders      = true;
    bool rainbow      = false;

    std::cout << "Controls" << std::endl
              << "Movement: WASD or Arrow keys" << std::endl
              << "Rainbow : 1" << std::endl
              << "Borders : 2" << std::endl
              << "Pause   : P" << std::endl
              << std::endl
              << "Settings" << std::endl
              << "Window size: " << SCREEN_SIZE.x << 'x' << SCREEN_SIZE.y << std::endl
              << "Game size  : " << GAME_SIZE.x   << 'x' << GAME_SIZE.y   << std::endl
              << "Tile count : " << TILE_COUNT.x  << 'x' << TILE_COUNT.y  << std::endl;

    /* Window loop */
    while(window.isOpen())
    {
        /* Events */
        sf::Event event;
        while(window.pollEvent(event))
        {
            switch(event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::LostFocus:
                inputAllowed = false;
                snake.setDirection(Entity::Direction::PAUSE);
                break;
            case sf::Event::GainedFocus:
                inputAllowed = true;
                break;
            case sf::Event::KeyReleased:
                if(!inputAllowed)
                    break;

                /* Key release events */
                switch(event.key.code)
                {
                case sf::Keyboard::Num1:     //Toggles snake color
                    rainbow = !rainbow;
                    break;
                case sf::Keyboard::Num2:     //Toggles border
                    borders = !borders;
                    break;
                case sf::Keyboard::P:        //Stops the snake from moving
                    snake.setDirection(Entity::Direction::PAUSE);
                    break;
                case sf::Keyboard::F1:       //Toggles tile drawing
                    debugTileDraw = !debugTileDraw;
                    break;
                case sf::Keyboard::F2:       //Toggles tile mouse click
                    debugTileClick = !debugTileClick;
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::MouseButtonPressed:
                if(debugTileClick)
                {
                    sf::Vector2f mouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                    std::cout << "Tile: ("
                              << map.pixelToTile(mouse).x << ','
                              << map.pixelToTile(mouse).y << ")"
                              << std::endl;
                }
                break;
            default:
                break;
            }
        }

        /* Realtime input */
        if(inputAllowed)
        {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                snake.setDirection(Entity::Direction::UP);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                snake.setDirection(Entity::Direction::LEFT);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                snake.setDirection(Entity::Direction::DOWN);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                snake.setDirection(Entity::Direction::RIGHT);
        }

        /* Snake-fruit interaction */
        if(snake.getPos() == fruit.pos)
        {
            snake.grow(1);
            fruit.reset();

            /* Speed increase */
            if(tickFreq - TICK_FREQ_INTERVAL >= MIN_TICK_FREQ &&
               (snake.getSize() - 1) % 5 == 0)
                tickFreq -= sf::milliseconds(5);
        }

        /* Handle gameover state */
        if(gameover)
        {
            if(inputAllowed && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                gameover = false;
                fruit.reset();
                snake.reset(SNAKE_START);
                tickFreq = INITIAL_TICK_FREQ;
            }
        }
        else
        {
            /* Tick based actions */
            if(gameClock.getElapsedTime() >= tickFreq)
            {
                gameClock.restart();

                /* Snake color */
                if(rainbow)
                {
                    snake.setColor(RAINBOW[nextColorIndex %= MAX_COLOR]);
                    nextColorIndex++;
                }
                else
                {
                    snake.setColor(sf::Color::Green);
                }

                //This updates a collision flag
                snake.updatePos();
            }

            gameover = snake.tailCollision();

            /* Border handling */
            if(borders)
            {
                if(!map.isInBounds(snake.getPos()))
                    gameover = true;
            }
            else
            {
                TM::Tile newTile(snake.getPos());
                
                if(snake.getPos().x < 0)
                    newTile.x = TILE_COUNT.x - 1;
                else if(snake.getPos().x >= static_cast<int>(TILE_COUNT.x))
                    newTile.x = 0;
                else if(snake.getPos().getPos().y < 0)
                    newTile.y = TILE_COUNT.y - 1;
                else if(snake.getPos().y >= static_cast<int>(TILE_COUNT.y))
                    newTile.y = 0;
                
                snake.setPos(newTile);
            }
        }

        /* Text updates */
        scoreText.setString("Score: " + std::to_string(snake.getSize() - 1));
        borderText.setString("Borders: " + static_cast<std::string>(borders ? "On" : "Off"));

        /* Drawing */
        window.clear();
        if(debugTileDraw)
            map.draw(window, sf::Color::Transparent, sf::Color::White);
        snake.draw(window);
        fruit.draw(window);
        window.draw(gameTextBorder);
        if(gameover)
            window.draw(deathText);
        window.draw(scoreText);
        window.draw(borderText);
        window.display();
    }

    exit(EXIT_SUCCESS);
}
