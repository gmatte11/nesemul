#include "openfile_dialog.h"
#include "platform_defines.h"

bool openfile_dialog(std::wstring& filepath)
{
#if IS_WINDOWS
    WCHAR path[MAX_PATH] = {};

    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof(OPENFILENAMEW);
    dialog.lpstrFile = path;
    dialog.nMaxFile = sizeof(path);
    dialog.lpstrInitialDir = L".\\data";
    dialog.Flags = OFN_DONTADDTORECENT | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&dialog) == TRUE)
    {
        filepath = path;
        return true;
    }
#endif
    return false;
}