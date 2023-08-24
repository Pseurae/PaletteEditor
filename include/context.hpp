#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "palette.hpp"
#include "actions.hpp"
#include "popups.hpp"

struct Context
{
    Palette palette;
    ActionRegister actionRegister;
    bool isDirty = false;
    std::string loadedFile;

    Context() : palette(1) { }
    Context(const std::string &fname);

    static auto &GetContext() { return *s_CurrentContext; }
    static void SetContext(size_t idx) { s_CurrentContext = s_OpenContexts[idx].get(); }

    static const bool HasNoContext() { return s_OpenContexts.empty() || s_CurrentContext == nullptr; }

    static Context &CreateNewContext();
    static Context &CreateNewContext(const std::string &fname);
    static auto &GetOpenContexts() { return s_OpenContexts; } 
    static void RemoveContext(size_t i);
private:
    static std::vector<std::unique_ptr<Context>> s_OpenContexts; 
    static Context *s_CurrentContext;
};

#endif // CONTEXT_HPP