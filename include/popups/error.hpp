#ifndef POPUPS_ERROR_HPP
#define POPUPS_ERROR_HPP

#include "popups.hpp"

namespace Popups
{
    class Error final : public Popup
    {
    public:
        Error(const std::string &name, const std::string &message, PopupCallback ok = nullptr);
        virtual void PreDraw() override;
        virtual void Draw() override;
    private:
        std::string m_Message;
        PopupCallback m_OkCallback;
    };
}

#endif // POPUPS_ERROR_HPP