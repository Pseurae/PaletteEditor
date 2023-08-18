#include "palette.hpp"

static constexpr char sText_JASC_PAL[] = "JASC-PAL";
static constexpr char sText_PAL_0100[] = "0100";

Palette::Palette()
{
    m_Colors.resize(1);
}

void Palette::LoadFromFile(const std::string &fname)
{
    std::ifstream stream(fname);
    if (stream.is_open())
    {
        LoadFromFile(stream);
    }
    stream.close();
}

void Palette::LoadFromFile(std::ifstream &stream)
{
    std::vector<Color> new_colors;
    std::string line;

    uint16_t num_colors;
    int r, g, b;

    stream >> line;
    if (line != sText_JASC_PAL) 
        throw ("Invalid JASC-PAL signature.");

    stream >> line;
    if (line != sText_PAL_0100) 
        throw ("Unsupported JASC-PAL version.");

    if (!(stream >> num_colors))
        throw ("Could not parse number of colors.");

    if (num_colors < 1 || num_colors > 256)
        throw ("Unsupported number of colors. (Color count must be between 1 and 256)");

    for (int i = 0; i < num_colors; i++)
    {
        if (!(stream >> r >> g >> b))
            throw ("Error parsing color components.");

        if (r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
            throw ("Color component value must be between 0 and 255.");

        new_colors.push_back({ 
            static_cast<float>(r / 255.0f), 
            static_cast<float>(g / 255.0f), 
            static_cast<float>(b / 255.0f) 
        });
    }

    m_Colors = new_colors;
    return;
}

void Palette::SaveToFile(const std::string &fname)
{
    std::ofstream stream(fname);
    if (stream.is_open())
        SaveToFile(stream);
    stream.close();
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
