#ifndef ACTIONS_SWAP_COLORS_HPP
#define ACTIONS_SWAP_COLORS_HPP

#include "actions.hpp"

namespace Actions
{
    class SwapColors final : public Action
    {
    public:
        SwapColors(size_t start, size_t old) : m_Start(start), m_Old(old) { }
        virtual void Apply() override;
        virtual void Revert() override;
        virtual std::string ToString() { return "SwapColors"; }
        virtual void PrintDetails() {}
    private:
        size_t m_Start, m_Old;
    };
}

#endif // ACTIONS_SWAP_COLORS_HPP