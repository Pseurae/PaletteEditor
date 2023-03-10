#include "palette.hpp"
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstring>

Palette::Palette()
{
    m_Colors.resize(1);
}

Palette::~Palette()
{
}

static const char sText_JASC_PAL[] = "JASC-PAL";
static const char sText_PAL_0100[] = "0100";

void Palette::LoadFromFile(std::ifstream &stream)
{
    std::vector<Color> new_colors;
    std::string line;
    std::stringstream sstream;

    uint16_t num_colors;
    int r, g, b;

    std::getline(stream, line);

    if (strncmp(line.c_str(), sText_JASC_PAL, 8) != 0) 
        throw std::string("Invalid JASC-PAL signature.");

    std::getline(stream, line);
    if (strncmp(line.c_str(), sText_PAL_0100, 4) != 0) 
        throw std::string("Unsupported JASC-PAL version.");

    std::getline(stream, line);
    num_colors = std::stoi(line);

    if (num_colors < 1 || num_colors > 256)
        throw std::string("Unsupported number of colors. (Color count must be between 1 and 256)");

    for (int i = 0; i < num_colors; i++)
    {
        std::getline(stream, line);
        sstream = std::stringstream(line);

        if (!(sstream >> r >> g >> b))
            throw std::string("Error parsing color components.");

        if (r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
            throw std::string("Color component value must be between 0 and 255.");

        new_colors.push_back({ 
            static_cast<float>(r / 255.0f), 
            static_cast<float>(g / 255.0f), 
            static_cast<float>(b / 255.0f) 
        });
    }

    m_Colors = new_colors;
    return;
}

void Palette::SaveToFile(std::ofstream &stream)
{
    stream << sText_JASC_PAL << "\r\n";
    stream << sText_PAL_0100 << "\r\n";
    stream << m_Colors.size() << "\r\n";

    for (auto &color: m_Colors)
    {
        int r, g, b;
        r = static_cast<uint8_t>(color[0] * 255.0f);
        g = static_cast<uint8_t>(color[1] * 255.0f);
        b = static_cast<uint8_t>(color[2] * 255.0f);

        stream << r << ' ' << g << ' ' << b << "\r\n";
    }
}

ColorList *Palette::GetColorList(void)
{
    return &m_Colors;
}