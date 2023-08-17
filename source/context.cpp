#include "context.hpp"

Context gContext{};

Context *Context::s_CurrentContext = &gContext;

void Context::SetContextToDefault()
{
    s_CurrentContext = &gContext;
}

auto &Context::GetDefaultContext()
{
    return gContext;
}