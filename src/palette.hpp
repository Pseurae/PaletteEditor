#pragma once

#include <vector>
#include <array>
#include <fstream>

// ImGui's color editor needs an array of floats
using Color = std::array<float, 3>;
using ColorList = std::vector<Color>;

class Palette
{
private:
    ColorList m_Colors;
public:
    Palette();
    ~Palette();

    void LoadFromFile(std::ifstream &stream);
    void SaveToFile(std::ofstream &stream);
    ColorList &GetColorList(void);
    size_t GetPaletteSize(void) { return m_Colors.size(); }
    void ResizePalette(int size) { return m_Colors.resize(size); }
};
