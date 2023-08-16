#ifndef POPUPS_PROMPT_HPP
#define POPUPS_PROMPT_HPP

#include "popups.hpp"

namespace Popups
{
    class Prompt : public Popup
    {
    public:
        Prompt(const std::string &name, const std::string &message, PopupCallback yes = nullptr, PopupCallback no = nullptr);
        virtual void PreDraw() override;
        virtual void Draw() override;
    private:
        std::string m_Message;
        PopupCallback m_YesCallback, m_NoCallback;
    };
}

#endif // POPUPS_PROMPT_HPP