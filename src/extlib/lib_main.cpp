#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <array>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include "lib_recomp.hpp"
#include "defines_z64o.h"
#include "defines_ooto.h"
#include "defines_mmo.h"
#include "playermodelmanager_api.h"
#include "sha1.hpp"
#include "object_table_oot.hpp"
#include "yaz0.hpp"

namespace fs = std::filesystem;

struct ModelDiskEntry {
    std::vector<char> modelData;
    std::array<bool, PMM_MODEL_TYPE_MAX> modelTypes;
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

    // Only 128 MiB of models should be loaded at the absolute maximum
    // Recomp only has 512 MiB of RAM
    // This upper limmit should never be anywhere close to reached
    const size_t MAX_TOTAL_SIZE = 128 * 1024 * 1024;

    size_t runningTotalSize = 0;

    // TODO: This entire section needs a rewrite to support more model types
    for (const auto &dirEntry : fs::directory_iterator(folderEntry.path())) {
        if (runningTotalSize < MAX_TOTAL_SIZE &&dirEntry.file_size() < MAX_MODEL_SIZE) {
            if (dirEntry.is_regular_file() || (dirEntry.is_symlink() && fs::is_regular_file(fs::weakly_canonical(dirEntry.path())))) {
                std::ifstream file(dirEntry.path(), std::ios::binary);

                if (isValidStandaloneZobj(file)) {
                    file.seekg(0);

                    std::vector<char> fileBuf(std::istreambuf_iterator<char> {}, {});

                    ModelDiskEntry mde;

                    extractEmbeddedInfo(fileBuf, mde);

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

                    mde.modelData = fileBuf;

                    mde.modelTypes.fill(false);

                    switch (mde.modelData[Z64O_FORM_BYTE]) {
                        case OOTO_FORM_BYTE_ADULT:
                        case MMO_FORM_BYTE_ADULT:
                            mde.modelTypes[PMM_MODEL_TYPE_ADULT] = true;
                            break;

                        case OOTO_FORM_BYTE_CHILD:
                        case MMO_FORM_BYTE_CHILD:
                            mde.modelTypes[PMM_MODEL_TYPE_CHILD] = true;
                            break;

                        default:
                            break;
                    }

                    sModelDiskEntries.push_back(mde);
                    runningTotalSize += fileBuf.size();
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

bool writeDataToRecompBuffer(uint8_t *rdram, recomp_context *ctx, PTR(char) recompBuf, int recompBufSize, const void *data, size_t dataSize) {
    if (!recompBuf) {
        return false;
    }

    if (recompBufSize < dataSize) {
        return false;
    }

    const char *bytes = static_cast<const char *>(data);

    for (size_t i = 0; i < dataSize; i++) {
        MEM_B(recompBuf, i) = bytes[i];
    }

    return true;
}

bool writeEntryDataToRecompBuffer(uint8_t *rdram, recomp_context *ctx, const void *data, size_t dataSize) {
    int bufLen = RECOMP_ARG(int, 2);

    // clang-format off
    PTR(char) buf = RECOMP_ARG(PTR(char), 1);
    // clang-format on

    return writeDataToRecompBuffer(rdram, ctx, buf, bufLen, data, dataSize);
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

    RECOMP_RETURN(int, entry->modelData.size());
}

RECOMP_DLL_FUNC(PMMZobj_getEntryFileData) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    RECOMP_RETURN(bool, writeEntryDataToRecompBuffer(rdram, ctx, entry->modelData.data(), entry->modelData.size()));
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

    if (offset >= entry->modelData.size()) {
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

    if (offset >= entry->modelData.size() + sizeof(uint16_t)) {
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

    if (offset >= entry->modelData.size() + sizeof(uint32_t)) {
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

RECOMP_DLL_FUNC(PMMZobj_isModelType) {
    ModelDiskEntry *entry = getDiskEntry(RECOMP_ARG(int, 0));

    if (!entry) {
        RECOMP_RETURN(bool, false);
    }

    int type = RECOMP_ARG(int, 1);

    if (type < 0 || type >= PMM_MODEL_TYPE_MAX) {
        RECOMP_RETURN(bool, false);
    }

    RECOMP_RETURN(bool, entry->modelTypes[type]);
}

RECOMP_DLL_FUNC(PMMZobj_clearDiskEntries) {
    sModelDiskEntries.clear();
    sModelDiskEntries.shrink_to_fit();
}

static const std::unordered_map<std::string, unsigned> DMA_DATA_OFFSETS = {
    {"328a1f1beba30ce5e178f031662019eb32c5f3b5", 0x7950},  // PAL 1.0
    {"cfbb98d392e4a9d39da8285d10cbef3974c2f012", 0x7950},  // PAL 1.1
    {"0227d7c0074f2d0ac935631990da8ec5914597b4", 0x7170},  // PAL GC
    {"f46239439f59a2a594ef83cf68ef65043b1bffe2", 0x7170},  // PAL MQ
    {"cee6bc3c2a634b41728f2af8da54d9bf8cc14099", 0x12F70}, // PAL GC (Debug)
    {"079b855b943d6ad8bd1eb026c0ed169ecbdac7da", 0x12F70}, // PAL MQ (Debug)
    {"50bebedad9e0f10746a52b07239e47fa6c284d03", 0x12F70}, // PAL MQ (Debug)
    {"cfecfdc58d650e71a200c81f033de4e6d617a9f6", 0x12F70}, // PAL MQ (Debug)
    {"ad69c91157f6705e8ab06c79fe08aad47bb57ba7", 0x7430},  // NTSC 1.0 (US)
    {"d3ecb253776cd847a5aa63d859d8c89a2f37b364", 0x7430},  // NTSC 1.1 (US)
    {"41b3bdc48d98c48529219919015a1af22f5057c2", 0x7430},  // NTSC 1.2 (US)
    {"c892bbda3993e66bd0d56a10ecd30b1ee612210f", 0x7430},  // NTSC 1.0 (JP)
    {"dbfc81f655187dc6fefd93fa6798face770d579d", 0x7430},  // NTSC 1.1 (JP)
    {"fa5f5942b27480d60243c2d52c0e93e26b9e6b86", 0x7430},  // NTSC 1.2 (JP)
    {"b82710ba2bd3b4c6ee8aa1a7e9acf787dfc72e9b", 0x7170},  // NTSC GC (US)
    {"8b5d13aac69bfbf989861cfdc50b1d840945fc1d", 0x7170},  // NTSC MQ (US)
    {"0769c84615422d60f16925cd859593cdfa597f84", 0x7170},  // NTSC GC (JP)
    {"2ce2d1a9f0534c9cd9fa04ea5317b80da21e5e73", 0x7170},  // NTSC GC CE (JP)
    {"dd14e143c4275861fe93ea79d0c02e36ae8c6c2f", 0x7170},  // NTSC MQ (JP)
};

const int ROM_LITTLE_ENDIAN = 0x40;
const int ROM_BYTESWAPPED = 0x37;

void doLE2BE(std::string &rom) {
    for (size_t i = 0; i < rom.size(); i += 4) {
        char b0 = rom[i];
        char b1 = rom[i + 1];
        char b2 = rom[i + 2];
        char b3 = rom[i + 3];

        rom[i] = b3;
        rom[i + 1] = b2;
        rom[i + 2] = b1;
        rom[i + 3] = b0;
    }
}

void doBS2BE(std::string &rom) {
    for (size_t i = 0; i < rom.size(); i += 2) {
        char tmp = rom[i];
        rom[i] = rom[i + 1];
        rom[i + 1] = tmp;
    }
}

uint32_t toU32(const char *buf) {
    uint32_t b0 = buf[0];
    uint32_t b1 = buf[1];
    uint32_t b2 = buf[2];
    uint32_t b3 = buf[3];

    return b0 << 24 | b1 << 16 | b2 << 8 | b3;
}

struct DMADataEntry {
    std::pair<uint32_t, uint32_t> virtualAddr;
    std::pair<uint32_t, uint32_t> physicalAddr;

    const bool isCompressed() {
        return physicalAddr.second == 0;
    }

    const bool isExist() {
        return physicalAddr.first == 0xFFFFFFFF;
    }

    const size_t getUncompressedSize() {
        if (virtualAddr.first > virtualAddr.second) {
            return 0;
        }

        return virtualAddr.second - virtualAddr.first;
    }

    const size_t getCompressedSize() {
        if (!isCompressed()) {
            return 0;
        }

        if (physicalAddr.first > physicalAddr.second) {
            return 0;
        }

        return physicalAddr.second - physicalAddr.first;
    }

    DMADataEntry(): virtualAddr({0, 0}), physicalAddr({0xFFFFFFFF, 0xFFFFFFFF}) {}

    static void fillEntry(DMADataEntry &entry, const char *rom, unsigned dmaStart, unsigned index) {
        const char *entryRaw = rom + dmaStart + index * (sizeof(virtualAddr) + sizeof(physicalAddr));

        entry.virtualAddr.first = toU32(entryRaw);
        entry.virtualAddr.second = toU32(entryRaw + sizeof(uint32_t));
        entry.physicalAddr.first = toU32(entryRaw + sizeof(uint32_t) * 2);
        entry.physicalAddr.second = toU32(entryRaw + sizeof(uint32_t) * 3);
    }

    static const unsigned OOT_OBJECT_TABLE_START_INDEX = 498;

    static void fillEntry(DMADataEntry &entry, const char *rom, unsigned dmaStart, ObjectIdOOT id) {
        fillEntry(entry, rom, dmaStart, static_cast<unsigned>(id) - 1 + OOT_OBJECT_TABLE_START_INDEX);
    }

    DMADataEntry(const char *rom, unsigned dmaStart, unsigned index) {
        DMADataEntry::fillEntry(*this, rom, dmaStart, index);
    }

    DMADataEntry(const char *rom, unsigned dmaStart, ObjectIdOOT id) {
        DMADataEntry::fillEntry(*this, rom, dmaStart, id);
    }
};

RECOMP_DLL_FUNC(PMMZobj_extractGameplayKeep) {
    // clang-format off
    PTR(char) rdramBuf = RECOMP_ARG(PTR(char), 0);
    // clang-format on

    if (!rdramBuf) {
        RECOMP_RETURN(bool, false);
    }

    unsigned rdramBufSize = RECOMP_ARG(unsigned, 1);

    fs::path romPathZ64 = sPMMDir / "oot.z64";
    fs::path romPathN64 = sPMMDir / "oot.n64";
    fs::path romPathV64 = sPMMDir / "oot.v64";
    fs::path *romPath = nullptr;

    if (fs::exists(romPathZ64)) {
        romPath = &romPathZ64;
    } else if (fs::exists(romPathN64)) {
        romPath = &romPathN64;
    } else if (fs::exists(romPathV64)) {
        romPath = &romPathV64;
    }

    if (romPath) {
        std::string rom;

        {
            std::ifstream file(*romPath, std::ios::binary);
            rom.assign(std::istreambuf_iterator<char>{}, {});
        }

        if (rom.size() > 0) {
            if (rom[0] == ROM_LITTLE_ENDIAN) {
                doLE2BE(rom);
            } else if (rom[0] == ROM_BYTESWAPPED) {
                doBS2BE(rom);
            }

            SHA1 checksum;
            checksum.update(rom);

            std::string checksumStr = checksum.final();

            auto dmaIt = DMA_DATA_OFFSETS.find(checksumStr);

            if (dmaIt != DMA_DATA_OFFSETS.end()) {
                unsigned dma = dmaIt->second;

                DMADataEntry gkEntry = DMADataEntry(rom.data(), dma, ObjectIdOOT::OBJECT_GAMEPLAY_KEEP);

                size_t gkSize = gkEntry.getUncompressedSize();

                if (gkSize) {
                    std::unique_ptr<uint8_t> gkBuf(new uint8_t[gkSize]);

                    bool isExtracted = false;
                    if (gkEntry.isCompressed()) {
                        size_t gkSizeCompressed = gkEntry.getCompressedSize();

                        if (gkSizeCompressed) {
                            yaz0_decompress(gkSize, gkSizeCompressed, reinterpret_cast<const uint8_t*>(rom.data() + gkEntry.physicalAddr.first), gkBuf.get());
                            isExtracted = true;
                        }
                    }
                    else {
                        memcpy(gkBuf.get(), rom.data() + gkEntry.physicalAddr.first, gkSize);
                        isExtracted = true;
                    }

                    if (isExtracted) {
                        fs::create_directories(OOT_ASSET_DIR);

                        std::ofstream gkOut(OOT_ASSET_DIR / "gameplay_keep.zobj", std::ios::binary);
                        
                        gkOut.write(reinterpret_cast<char *>(gkBuf.get()), gkSize);

                        RECOMP_RETURN(bool, writeDataToRecompBuffer(rdram, ctx, rdramBuf, rdramBufSize, gkBuf.get(), gkSize));
                    }
                }
            }
        }
    }

    RECOMP_RETURN(bool, false);
}
