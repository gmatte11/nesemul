#include "openfile_dialog.h"
#include "platform_defines.h"

#if IS_WINDOWS
#include <windows.h>
#endif

bool openfile_dialog(std::string& filepath)
{
#if IS_WINDOWS
    TCHAR path[MAX_PATH] = {};

    OPENFILENAME dialog = {};
    dialog.lStructSize = sizeof(OPENFILENAME);
    dialog.lpstrFile = path;
    dialog.nMaxFile = sizeof(path);
    dialog.lpstrInitialDir = ".\\data";
    dialog.Flags = OFN_DONTADDTORECENT;

    if (GetOpenFileName(&dialog) == TRUE)
    {
        filepath = path;
        return true;
    }
#endif
    return false;
}