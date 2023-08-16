#ifndef ACTIONS_MODIFY_COLOR_HPP
#define ACTIONS_MODIFY_COLOR_HPP

#include "actions.hpp"
#include "palette.hpp"

namespace Actions
{
    class ModifyColor final : public Action
    {
    public:
        ModifyColor(size_t idx, const Color &old_color, const Color &new_color) :
            m_Index(idx), m_OldColor(old_color), m_NewColor(new_color) {}
        virtual void Apply() override;
        virtual void Revert() override;
        virtual std::string ToString() { return "ModifyColor"; }
        virtual void PrintDetails() {}
    private:
        size_t m_Index;
        Color m_OldColor, m_NewColor;
    };
}

#endif // ACTIONS_MODIFY_COLOR_HPP