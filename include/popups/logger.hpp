#ifndef POPUPS_LOGGER_HPP
#define POPUPS_LOGGER_HPP

#include "popups.hpp"

namespace Popups
{
    class Logger final : public Popup
    {
    public:
        Logger();
        virtual void PreDraw() override;
        virtual void Draw() override;
    };
}

#endif // POPUPS_LOGGER_HPP