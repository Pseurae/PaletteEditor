#include "context.hpp"
#include <fstream>

std::vector<std::unique_ptr<Context>> Context::s_OpenContexts;
Context *Context::s_CurrentContext = 0;

Context::Context(const std::string &fname) : loadedFile(fname), isDirty(false)
{
    palette.LoadFromFile(fname);
}

Context &Context::CreateNewContext()
{
    s_OpenContexts.push_back(std::make_unique<Context>());
    s_CurrentContext = s_OpenContexts.back().get();
    return *s_CurrentContext;
}

Context &Context::CreateNewContext(const std::string &fname)
{
    s_OpenContexts.push_back(std::make_unique<Context>(fname));
    s_CurrentContext = s_OpenContexts.back().get();
    return *s_CurrentContext;
}

void Context::RemoveContext(size_t i)
{
    s_OpenContexts.erase(s_OpenContexts.begin() + i);
}
