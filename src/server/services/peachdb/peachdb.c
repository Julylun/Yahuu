#include "peachdb.h"
#include <stdio.h>
#include <stdio.h>      // For printf, fopen, fprintf, fclose
#include <sys/stat.h>   // For stat, mkdir
#include <unistd.h>     // For access (to check file existence)
#include <errno.h>      // For error handling with perror

#include "functions/files/files.h"



int Peach_initPeachDb()
{
    const char *dataDir = "peachdata";
    const char *collectionsDir = "peachdata/collections";
    const char *indexFile = "peachdata/index.mpdb";
    const char *indexExtension = ".mpdb";
    const char *collectionExtension = ".lpdb";

    // Check if 'peachdata' directory exists
    if (!directoryExists(dataDir)) {
        if (mkdir(dataDir, 0755) != 0) {
            perror("Failed to create 'peachdata' directory");
            return -1;
        }
    }

    // Check if 'peachdata/collections' directory exists
    if (!directoryExists(collectionsDir)) {
        if (mkdir(collectionsDir, 0755) != 0) {
            perror("Failed to create 'peachdata/collections' directory");
            return -1;
        }
    }

    // Check if 'peachdata/index.pdb' file exists
    if (access(indexFile, F_OK) != 0) {
        FILE *file = fopen(indexFile, "w");
        if (!file) {
            perror("Failed to create 'peachdata/index.mpdb' file");
            return -1;
        }
        fprintf(file, "0\n");
        fclose(file);
    }

    return 0;
}
