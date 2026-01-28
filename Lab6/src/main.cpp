#include <SFML/Graphics.hpp>
#include "TemperatureGraph.h"
#include "HTTPClient.h"
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode({1200u,700u}), "Temperature Monitor");
    window.setFramerateLimit(30);

    HTTPClient http("127.0.0.1", 8080);
    TemperatureGraph graph;

    bool hourly = true;

    sf::Font font;
    if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
		std::cout << "Failed to load font\n";
		return -1;
	}

    while (window.isOpen())
    {
        while (auto ev = window.pollEvent())
        {
            if (ev->is<sf::Event::Closed>())
                window.close();

            if (auto key = ev->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::H) hourly = true;
                if (key->code == sf::Keyboard::Key::D) hourly = false;
            }
        }

        auto data = hourly ? http.getHourly() : http.getDaily();
        graph.setData(data);

        window.clear(sf::Color(18,18,18));
        graph.draw(window, {100.f,100.f}, {900.f,450.f}, font);

        sf::Text title(font, "Temperature Monitoring System", 26);
        title.setPosition({350.f, 20.f});
        window.draw(title);
		
		sf::Text mode(font,hourly ? "Hourly Temperature (H)" : "Daily Temperature (D)",18);
        mode.setPosition({20.f, 20.f});
        window.draw(mode);

        window.display();
    }
}

