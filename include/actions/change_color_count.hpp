#ifndef ACTIONS_CHANGE_COLOR_COUNT_HPP
#define ACTIONS_CHANGE_COLOR_COUNT_HPP

#include <palette.hpp>
#include "actions.hpp"
#include <vector>

namespace Actions
{
    class ChangeColorCount : public Action
    {
    public:
        ChangeColorCount(size_t _old, size_t _new);
        virtual void Apply() override;
        virtual void Revert() override;
        virtual std::string ToString() { return "ChangeColorCount"; }
        virtual void PrintDetails() override;
    private:
        size_t m_OldSize, m_NewSize;
        std::vector<Color> m_Colors;
    };
}

#endif // ACTIONS_CHANGE_COLOR_COUNT_HPP