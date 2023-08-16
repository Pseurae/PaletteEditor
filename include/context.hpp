#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "palette.hpp"
#include "actions.hpp"

struct Context
{
    static Palette palette;
    static ActionRegister actionRegister;
    static bool isDirty;
    static std::string loadedFile;
};

#endif // CONTEXT_HPP