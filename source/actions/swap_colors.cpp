#include "actions/swap_colors.hpp"
#include "context.hpp"
#include "palette.hpp"

static void SwapColors(Color &color1, Color &color2)
{
    Color tmp = color1;
    color1 = color2;
    color2 = tmp;
}

namespace Actions
{
    void SwapColors::Apply()
    {
        ::SwapColors(Context::palette[m_Start], Context::palette[m_Old]);
    }

    void SwapColors::Revert()
    {
        ::SwapColors(Context::palette[m_Start], Context::palette[m_Old]);
    }
}