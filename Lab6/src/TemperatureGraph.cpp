#include "TemperatureGraph.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

void TemperatureGraph::setData(const std::vector<TempPoint>& d) {
    data = d;
}

void TemperatureGraph::draw(sf::RenderWindow& window,
                            sf::Vector2f pos,
                            sf::Vector2f size,
                            const sf::Font& font)
{
    if (data.empty()) return;

    float left = pos.x;
    float top = pos.y;
    float width = size.x;
    float height = size.y;

    double maxVal = data[0].value;
    double minVal = data[0].value;
    for (auto& p : data) {
        maxVal = std::max(maxVal, p.value);
        minVal = std::min(minVal, p.value);
    }
    double range = maxVal - minVal;
    if (range == 0) range = 1;

    sf::RectangleShape border(size);
    border.setPosition(pos);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(1.f);
    window.draw(border);

    sf::VertexArray line(sf::PrimitiveType::LineStrip, data.size());
    float stepX = (data.size() > 1) ? width / (data.size() - 1) : 0;

    for (size_t i = 0; i < data.size(); ++i) {
        float norm = static_cast<float>((data[i].value - minVal) / range);
        float x = left + i * stepX;
        float y = top + height - norm * height;

        line[i].position = { x, y };
        line[i].color = sf::Color(0, 200, 0);
    }

    window.draw(line);

    for (int i = 0; i <= 5; ++i) {
        float y = top + height - i * height / 5.f;
        double val = minVal + i * range / 5.0;

        sf::Text label(font, std::to_string((int)val), 14);
        label.setPosition({ left - 45.f, y - 10.f });
        window.draw(label);
    }

    size_t step = std::max<size_t>(1, data.size() / 8);
    for (size_t i = 0; i < data.size(); i += step) {
        float x = left + i * stepX;
        sf::Text label(font, data[i].label, 12);
        label.setPosition({ x - 10.f, top + height + 5.f });
        window.draw(label);
    }
}
