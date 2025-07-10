#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <array>
#include <algorithm>
#include "lib_recomp.hpp"
#include "defines_z64o.h"
#include "defines_ooto.h"
#include "defines_mmo.h"
#include "playermodelmanager_api.h"

namespace fs = std::filesystem;

struct ModelDiskEntry {
    size_t fileSize;
    char *modelData;
    std::array<bool, PMM_FORM_MODEL_TYPE_MAX> modelTypes;
    std::string internalName;
    std::string displayName;
    std::string authorName;
};

static std::vector<ModelDiskEntry> sModelDiskEntries;

static fs::path sPMMDir;

const fs::path MODEL_DIR("models");

const fs::path ASSET_DIR("assets");

const fs::path OOT_ASSET_DIR(ASSET_DIR / "oot");

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

// Check if zobj is standalone.
// Moodifies stream input position.
bool isValidStandaloneZobj(std::ifstream &f) {
    if (!f) {
        return false;
    }

    {
        const size_t MINIMUM_STANDALONE_ZOBJ_SIZE = 0x5800;

        f.seekg(MINIMUM_STANDALONE_ZOBJ_SIZE);

        char tmp;
        f.read(&tmp, 1);

        if (f.eof()) {
            return false;
        }
    }

    f.seekg(Z64O_MODLOADER_HEADER);

    const std::array<char, sizeof("MODLOADER64")> MODLOADER64_HEADER("MODLOADER64");
    std::array<char, MODLOADER64_HEADER.size()> headerBuf;

    f.read(headerBuf.data(), headerBuf.size());

    if (f.eof()) {
        return false;
    }

    if (memcmp(MODLOADER64_HEADER.data(), headerBuf.data(), MODLOADER64_HEADER.size() - 1) == 0) {
        switch (headerBuf[headerBuf.size() - 1]) {
            case MMO_FORM_BYTE_CHILD:
            case MMO_FORM_BYTE_ADULT:
            case OOTO_FORM_BYTE_ADULT:
            case OOTO_FORM_BYTE_CHILD:
                return true;
                break;
            default:
                break;
        }
    }

    return false;
}

RECOMP_DLL_FUNC(PMMZobj_setPMMDir) {
    sPMMDir = RECOMP_ARG_STR(0);
}

void extractEmbeddedInfo(const std::vector<char> &v, ModelDiskEntry &entry) {
    const std::array<char, sizeof("PLAYERMODELINFO")> PLAYER_MODEL_INFO_HEADER_STR("PLAYERMODELINFO");

    const size_t EMBEDDED_INFO_HEADER_LOCATION = 0x5500;

    if (memcmp(v.data() + EMBEDDED_INFO_HEADER_LOCATION, PLAYER_MODEL_INFO_HEADER_STR.data(), PLAYER_MODEL_INFO_HEADER_STR.size() - 1) == 0) {
        const size_t EMBEDDED_INFO_VERSION_LOCATION = EMBEDDED_INFO_HEADER_LOCATION + PLAYER_MODEL_INFO_HEADER_STR.size() - 1;

        if (v[EMBEDDED_INFO_VERSION_LOCATION] == 1) {
            const size_t INTERNAL_NAME_FIELD_SIZE = 64;
            const size_t DISPLAY_NAME_FIELD_SIZE = 32;
            const size_t AUTHOR_NAME_FIELD_SIZE = 64;

            const size_t EMBEDDED_INTERNAL_NAME_LOCATION = EMBEDDED_INFO_HEADER_LOCATION + PLAYER_MODEL_INFO_HEADER_STR.size();
            const size_t EMBEDDED_DISPLAY_NAME_LOCATION = EMBEDDED_INTERNAL_NAME_LOCATION + INTERNAL_NAME_FIELD_SIZE;
            const size_t EMBEDDED_AUTHOR_NAME_LOCATION = EMBEDDED_DISPLAY_NAME_LOCATION + DISPLAY_NAME_FIELD_SIZE;

            auto setEntryStr = [v](size_t offset, size_t maxSize, std::string &result) {
                auto start = v.begin() + offset;
                auto end = start + maxSize;

                auto it = start;
                while (*it != '\0' && it != end) {
                    it++;
                }

                // Don't read null terminator into string
                if (it > start && *it == '\0') {
                    it--;
                }

                result = start != it ? std::string(start, it) : "";
            };

            setEntryStr(EMBEDDED_INTERNAL_NAME_LOCATION, INTERNAL_NAME_FIELD_SIZE, entry.internalName);
            setEntryStr(EMBEDDED_DISPLAY_NAME_LOCATION, DISPLAY_NAME_FIELD_SIZE, entry.displayName);
            setEntryStr(EMBEDDED_AUTHOR_NAME_LOCATION, AUTHOR_NAME_FIELD_SIZE, entry.authorName);
        }
    }
}

RECOMP_DLL_FUNC(PMMZobj_scanForDiskEntries) {
    if (!fs::is_directory(sPMMDir)) {
        RECOMP_RETURN(int, -1);
    }

    sModelDiskEntries.clear();

    fs::directory_entry folderEntry(sPMMDir / MODEL_DIR);

    if (!folderEntry.is_directory()) {
        RECOMP_RETURN(int, -1);
    }

    // Don't bother evaluating files >= 4 MiB
    // If a model is that big, it should be replaced through RT64 instead
    const size_t MAX_MODEL_SIZE = 4 * 1024 * 1024;

    // TODO: This entire section needs a rewrite to support more model types
    for (const auto &dirEntry : fs::directory_iterator(folderEntry.path())) {
        if (dirEntry.file_size() < MAX_MODEL_SIZE) {
            if (dirEntry.is_regular_file() || (dirEntry.is_symlink() && fs::is_regular_file(fs::weakly_canonical(dirEntry.path())))) {
                std::ifstream file(dirEntry.path(), std::ios::in | std::ios::binary);

                if (isValidStandaloneZobj(file)) {
                    file.seekg(0);

                    std::vector<char> fileBuf(std::istreambuf_iterator(file), {});

                    ModelDiskEntry mde;

                    extractEmbeddedInfo(fileBuf, mde);

                    mde.fileSize = fileBuf.size();

                    fs::path dirEntryPath = dirEntry.path();

                    if (mde.internalName == "") {
                        mde.internalName = dirEntryPath.filename().string();

                        if (mde.internalName.size() > PMM_MAX_INTERNAL_NAME_LENGTH) {
                            mde.internalName = mde.internalName.substr(0, PMM_MAX_AUTHOR_NAME_LENGTH);
                        }
                    }

                    if (mde.internalName.size() < PMM_MAX_AUTHOR_NAME_LENGTH) {
                        mde.internalName += " "; // leave 1 byte of padding
                    }

                    if (mde.displayName == "") {
                        mde.displayName = dirEntryPath.stem().string();

                        if (mde.displayName.size() > PMM_MAX_INTERNAL_NAME_LENGTH) {
                            mde.displayName = mde.displayName.substr(0, PMM_MAX_DISPLAY_NAME_LENGTH);
                        }
                    }

                    if (mde.authorName == "") {
                        mde.authorName = "N/A";
                    }

                    mde.modelData = new char[fileBuf.size()];

                    mde.modelTypes.fill(false);

                    memcpy(mde.modelData, fileBuf.data(), fileBuf.size());

                    switch (mde.modelData[Z64O_FORM_BYTE]) {
                        case OOTO_FORM_BYTE_ADULT:
                        case MMO_FORM_BYTE_ADULT:
                            mde.modelTypes[PMM_FORM_MODEL_TYPE_ADULT] = true;
                            break;

                        case OOTO_FORM_BYTE_CHILD:
                        case MMO_FORM_BYTE_CHILD:
                            mde.modelTypes[PMM_FORM_MODEL_TYPE_CHILD] = true;
                            break;

                        default:
                            break;
                    }

                    sModelDiskEntries.push_back(mde);
                }
            }
        }
    }

    RECOMP_RETURN(int, sModelDiskEntries.size());
}

RECOMP_DLL_FUNC(PMMZobj_getNumDiskEntries) {
    RECOMP_RETURN(int, sModelDiskEntries.size());
}

ModelDiskEntry *getDiskEntry(int i) {
    return i >= sModelDiskEntries.size() || i < 0 ? nullptr : &sModelDiskEntries[i];
}

bool writeEntryDataToRecompBuffer(uint8_t *rdram, recomp_context *ctx, const void *data, const size_t dataSize) {
    int bufLen = RECOMP_ARG(int, 2);

    if (bufLen < dataSize) {
        return false;
    }

    // clang-format off
    PTR(char) buf = RECOMP_ARG(PTR(char), 1);
    // clang-format on

    const char *bytes = static_cast<const char *>(data);

    for (size_t i = 0; i < dataSize; i++) {
        MEM_B(buf, i) = bytes[i];
    }
}

bool writeEntryStringToRecompBuffer(uint8_t *rdram, recomp_context *ctx, const std::string &s) {
    return writeEntryDataToRecompBuffer(rdram, ctx, s.c_str(), s.size() + 1);
}

RECOMP_DLL_FUNC(PMMZobj_entryInternalNameLength) {
    int index = RECOMP_ARG(int, 0);

    ModelDiskEntry *entry = getDiskEntry(index);

    if (!entry) {
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, entry->internalName.size());
}

RECOMP_DLL_FUNC(PMMZobj_entryDisplayNameLength) {
    int index = RECOMP_ARG(int, 0);

    ModelDiskEntry *entry = getDiskEntry(index);

    if (!entry) {
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, entry->displayName.size());
}

RECOMP_DLL_FUNC(PMMZobj_entryAuthorNameLength) {
    int index = RECOMP_ARG(int, 0);

    ModelDiskEntry *entry = getDiskEntry(index);

    if (!entry) {
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, entry->authorName.size());
}

RECOMP_DLL_FUNC(PMMZobj_writeInternalNameToBuffer) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    RECOMP_RETURN(bool, writeEntryStringToRecompBuffer(rdram, ctx, entry->internalName));
}

RECOMP_DLL_FUNC(PMMZobj_writeDisplayNameToBuffer) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    RECOMP_RETURN(bool, writeEntryStringToRecompBuffer(rdram, ctx, entry->displayName));
}

RECOMP_DLL_FUNC(PMMZobj_writeAuthorNameToBuffer) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    RECOMP_RETURN(bool, writeEntryStringToRecompBuffer(rdram, ctx, entry->authorName));
}

RECOMP_DLL_FUNC(PMMZobj_getEntryFileSize) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, entry->fileSize);
}

RECOMP_DLL_FUNC(PMMZobj_getEntryFileData) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(bool, writeEntryDataToRecompBuffer(rdram, ctx, entry->modelData, entry->fileSize));
}

RECOMP_DLL_FUNC(PMMZobj_createDirectory) {
    RECOMP_RETURN(bool, fs::create_directories(RECOMP_ARG_STR(0)));
}

RECOMP_DLL_FUNC(PMMZobj_isDirectoryExist) {
    RECOMP_RETURN(bool, fs::is_directory(RECOMP_ARG_STR(0)));
}

RECOMP_DLL_FUNC(PMMZobj_readEntryU8) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    int offset = RECOMP_ARG(int, 1);

    if (offset >= entry->fileSize) {
        RECOMP_RETURN(bool, false);
    }

    // clang-format off
    PTR(unsigned char) out = RECOMP_ARG(PTR(unsigned char), 2);
    // clang-format on

    if (!out) {
        RECOMP_RETURN(bool, false);
    }

    MEM_BU(static_cast<size_t>(out), 0) = entry->modelData[offset];

    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PMMZobj_readEntryU16) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    int offset = RECOMP_ARG(int, 1);

    if (offset >= entry->fileSize + sizeof(uint16_t)) {
        RECOMP_RETURN(bool, false);
    }

    // clang-format off
    PTR(unsigned char) out = RECOMP_ARG(PTR(unsigned char), 2);
    // clang-format on

    if (!out) {
        RECOMP_RETURN(bool, false);
    }

    MEM_HU(static_cast<size_t>(out), 0) = (static_cast<uint16_t>(entry->modelData[offset]) << 8) | entry->modelData[offset + 1];

    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PMMZobj_readEntryU32) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    int offset = RECOMP_ARG(int, 1);

    if (offset >= entry->fileSize + sizeof(uint32_t)) {
        RECOMP_RETURN(bool, false);
    }

    // clang-format off
    PTR(unsigned char) out = RECOMP_ARG(PTR(unsigned char), 2);
    // clang-format on

    if (!out) {
        RECOMP_RETURN(bool, false);
    }

    MEM_W(static_cast<size_t>(out), 0) =
        (static_cast<uint32_t>(entry->modelData[offset]) << 24) |
        (static_cast<uint32_t>(entry->modelData[offset + 1]) << 16) |
        (static_cast<uint32_t>(entry->modelData[offset + 2]) << 8) |
        (static_cast<uint32_t>(entry->modelData[offset + 3]));

    RECOMP_RETURN(bool, true);
}

RECOMP_DLL_FUNC(PMMZobj_getEntryFormType) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    PlayerModelManager_FormModelType type = RECOMP_ARG(PlayerModelManager_FormModelType, 1);

    if (type < 0 || type >= PMM_FORM_MODEL_TYPE_MAX) {
        RECOMP_RETURN(bool, false);
    }

    RECOMP_RETURN(bool, entry->modelTypes[type]);
}
