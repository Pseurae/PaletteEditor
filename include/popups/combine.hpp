#ifndef POPUPS_COMBINE_HPP
#define POPUPS_COMBINE_HPP

#include "popups.hpp"
#include "palette.hpp"
#include "context.hpp"

namespace Popups
{
    class Combine final : public Popup
    {
    public:
        Combine();
        virtual void PreDraw() override;
        virtual void Draw() override;
    private:
        void CombineFiles();
        void FileDetails();
        void DetailsBar();
        void Load();
        void PaletteEditor();
        std::vector <std::string> m_Files;
        Palette m_Palette;
    };
}

#endif // POPUPS_COMBINE_HPP