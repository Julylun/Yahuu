#include "files.h"

/**
 * Check if a directory exists.
 *
 * @param path The path to the directory.
 * @return 1 if the directory exists, 0 otherwise.
 */
static int directoryExists(const char *path)
{
    struct stat st;

    //Return 0 if successfully
    if (stat(path, &st) == 0) {
        // Is this a folder?
        return S_ISDIR(st.st_mode);
    }
    return 0;
}
