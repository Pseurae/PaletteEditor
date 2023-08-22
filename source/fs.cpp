#include "fs.hpp"
#include "nfd.h"
#include <filesystem>

namespace 
{
    const nfdfilteritem_t sFilterPatterns[] = { {"Palette Files", "pal"} };
}

namespace fs
{
    std::string GetFilename(const std::string &path)
    {
        return std::filesystem::path(path).filename().string();
    }

    bool OpenFilePrompt(PromptCallback cb, const char *defaultPath)
    {
        const nfdpathset_t *paths;
        nfdresult_t result = NFD_OpenDialogMultipleU8(&paths, sFilterPatterns, 1, nullptr);

        if (result == NFD_OKAY)
        {
            nfdpathsetsize_t numPaths;
            NFD_PathSet_GetCount(paths, &numPaths);

            for (nfdpathsetsize_t i = 0; i < numPaths; ++i)
            {
                nfdchar_t* path;
                NFD_PathSet_GetPath(paths, i, &path);
                cb(path);
                NFD_PathSet_FreePath(path);
            }

            NFD_PathSet_Free(paths);
            return true;
        }
        return false;
    }

    bool SaveFilePrompt(PromptCallback cb)
    {
        char *path;
        nfdresult_t result = NFD_SaveDialog(&path, sFilterPatterns, 1, 0, 0);

        if (result == NFD_OKAY)
        {
            cb(path);
            NFD_FreePathU8(path);
            return true;
        }

        return false;
    }
}
