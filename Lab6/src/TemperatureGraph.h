#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct TempPoint {
    std::string label;  
    double value;       
};

class TemperatureGraph {
public:
    void setData(const std::vector<TempPoint>& d);

    void draw(sf::RenderWindow& window,
              sf::Vector2f pos,
              sf::Vector2f size,
              const sf::Font& font);

private:
    std::vector<TempPoint> data;
};
