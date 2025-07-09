#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "yazmtcorelib_api.h"
#include "playermodelmanager_api.h"
#include "libc/string.h"

RECOMP_IMPORT(".", int PMMZobj_scanForDiskEntries(const char *str));
RECOMP_IMPORT(".", int PMMZobj_getNumDiskEntries());
RECOMP_IMPORT(".", int PMMZobj_entryInternalNameLength(int i));
RECOMP_IMPORT(".", int PMMZobj_entryDisplayNameLength(int i));
RECOMP_IMPORT(".", int PMMZobj_entryAuthorNameLength(int i));
RECOMP_IMPORT(".", bool PMMZobj_writeInternalNameToBuffer(int i, char *buffer, int bufferSize));
RECOMP_IMPORT(".", bool PMMZobj_writeDisplayNameToBuffer(int i, char *buffer, int bufferSize));
RECOMP_IMPORT(".", bool PMMZobj_writeAuthorNameToBuffer(int i, char *buffer, int bufferSize));
RECOMP_IMPORT(".", int PMMZobj_getEntryFileSize(int i));
RECOMP_IMPORT(".", bool PMMZobj_getEntryFileData(int i, char *buffer, int bufferSize));
RECOMP_IMPORT(".", bool PMMZobj_createDirectory(char *path));
RECOMP_IMPORT(".", bool PMMZobj_isDirectoryExist(char *path));
RECOMP_IMPORT(".", bool PMMZobj_readEntryU8(int i, int offset, u8 *));
RECOMP_IMPORT(".", bool PMMZobj_readEntryU16(int i, int offset, u16 *));
RECOMP_IMPORT(".", bool PMMZobj_readEntryU32(int i, int offset, u32 *));

typedef struct {
    Gfx displayLists[PMM_DL_MAX];
    char *internalName;
    char *displayName;
    char *authorName;
    void *buffer;
} FormModel;

static FormModel *sFormModels;

#define MAIN_DIR "playermodelmanager"
#define MODEL_DIR MAIN_DIR "/models"

typedef struct {
    char *str;
    size_t length;
} StringInfo;

// Combine, at most, 32 path strings
char *getCombinedPath(int count, ...) {
    const MAX_COUNT = 32;

    if (count > MAX_COUNT) {
        return NULL;
    }

    char *combinedPath = NULL;

    size_t finalPathLen = 0;

    StringInfo *pathStrings[count];

    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; ++i) {
        StringInfo *strInfo = &pathStrings[i];
        strInfo->str = NULL;
        strInfo->length = 0;

        strInfo->str = va_arg(args, char *);

        strInfo->length = strlen(strInfo->str);

        finalPathLen += strInfo->length;
    }

    va_end(args);

    // make room for dir seperators
    finalPathLen += count - 1;

    // room for null byte
    ++finalPathLen;

    combinedPath = recomp_alloc(finalPathLen * sizeof(char));
    char *pos = combinedPath;

    for (int i = 0; i < count; ++i) {
        StringInfo *strInfo = &pathStrings[i];

        for (int j = 0; j < strInfo->length; ++j) {
            *pos = strInfo->str[j];
            ++pos;
        }

        // add directory seperators between path names
        if (i < count - 1) {
            // Windows/Mac/Linux all support forward slash dir seperator
            // so there's no need to handle back slash
            pos[0] = '/';
            ++pos;
        }
    }

    combinedPath[finalPathLen - 1] = '\0';

    return combinedPath;
}

PLAYERMODELMANAGER_CALLBACK_REGISTER_MODELS void registerDiskModels() {
    char *modDir = (char *)recomp_get_mod_folder_path();

    char *fullModelDir = getCombinedPath(2, modDir, MODEL_DIR);

    recomp_free(modDir);

    PMMZobj_createDirectory(fullModelDir);

    if (!PMMZobj_isDirectoryExist(fullModelDir)) {
        recomp_printf("PlayerModelManager_ZobjSupport: Could not find %s directory", fullModelDir);
        recomp_free(fullModelDir);
        return;
    }

    int numDiskEntries = PMMZobj_scanForDiskEntries(fullModelDir);

    for (int i = 0; i < numDiskEntries; ++i) {
        int internalLen = PMMZobj_entryInternalNameLength(i);

        if (internalLen < 1) {
            continue;
        }




    }
}
