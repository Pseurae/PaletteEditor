#ifndef ACTIONS_SWAP_COLORS_HPP
#define ACTIONS_SWAP_COLORS_HPP

#include "actions.hpp"

namespace Actions
{
    class SwapColors final : public Action
    {
    public:
        SwapColors(size_t start, size_t end) : m_Start(start), m_End(end) { }
        virtual void Apply() override;
        virtual void Revert() override;
        virtual std::string ToString() { return "SwapColors"; }
        virtual void PrintDetails() override;
    private:
        size_t m_Start, m_End;
    };
}

#endif // ACTIONS_SWAP_COLORS_HPP