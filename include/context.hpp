#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "palette.hpp"
#include "actions.hpp"
#include "popups.hpp"

struct Context
{
    Palette palette;
    ActionRegister actionRegister;
    bool isDirty;
    std::string loadedFile;

    static auto &GetContext() { return *s_CurrentContext; }
    static constexpr void SetContext(Context *ctx) { s_CurrentContext = ctx; }
    static void SetContextToDefault();
    static auto &GetDefaultContext();
private:
    static Context *s_CurrentContext;
};

#endif // CONTEXT_HPP