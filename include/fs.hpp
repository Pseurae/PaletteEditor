#ifndef FS_HPP
#define FS_HPP

#include <string>
#include <functional>

namespace fs
{
    using PromptCallback = std::function<void(const char *)>;

    std::string GetFilename(const std::string &path);
    bool OpenFilePrompt(PromptCallback cb, const char *defaultPath = nullptr);
    bool SaveFilePrompt(PromptCallback cb);
}

#endif // FS_HPP