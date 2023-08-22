#ifndef POPUPS_SPLIT_HPP
#define POPUPS_SPLIT_HPP

#include "popups.hpp"

namespace Popups
{
    class Split final : public Popup
    {
    public:
        Split();
        virtual void PreDraw() override;
        virtual void Draw() override;
        virtual void ProcessShortcuts(int key, int mods) override;
    private:
        void Load();
        size_t m_NumColors = 1;
    };
}

#endif // POPUPS_SPLIT_HPP
