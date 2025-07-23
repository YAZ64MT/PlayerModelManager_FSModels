#include "global.h"
#include "modding.h"
#include "recomputils.h"
#include "globalobjects_api.h"
#include "playermodelmanager_api.h"

RECOMP_IMPORT(".", bool PMMZobj_extractGameplayKeep(void *buffer, int bufferSize));
RECOMP_IMPORT(".", bool PMMZobj_extractChildLink(void *buffer, int bufferSize));
RECOMP_IMPORT(".", bool PMMZobj_extractAdultLink(void *buffer, int bufferSize));

u8 *gGameplayKeepOOT = NULL;

u8 *gLinkChildOOT = NULL;

u8 *gLinkAdultOOT = NULL;

static bool sIsGameplayKeepHandled = false;
static bool sIsRealGameplayKeepLoaded = false;

void *getGameplayKeepOOT() {
    return gGameplayKeepOOT;
}

#define OOT_NTSC_GAMEPLAY_KEEP_SIZE 0x05BCE0

void loadGameplayKeepOOT() {
    if (!sIsGameplayKeepHandled) {
        gGameplayKeepOOT = recomp_alloc(OOT_NTSC_GAMEPLAY_KEEP_SIZE);

        if (!PMMZobj_extractGameplayKeep(gGameplayKeepOOT, OOT_NTSC_GAMEPLAY_KEEP_SIZE)) {
            for (int i = 0; i < OOT_NTSC_GAMEPLAY_KEEP_SIZE; i += sizeof(Gfx)) {
                gSPEndDisplayList(gGameplayKeepOOT + i);
            }
        } else {
            sIsRealGameplayKeepLoaded = true;
        }

        sIsGameplayKeepHandled = true;
    }
}

void setupFaceTextures(PlayerModelManagerHandle h, u8 *zobj);

#define OOT_LINK_CHILD_SKELETON 0x0602CF6C
#define OOT_LINK_CHILD_SIZE 0x2CF80
#define OOT_LINK_CHILD_LFIST 0x13E18
#define OOT_LINK_CHILD_RFIST 0x14320
#define OOT_LINK_CHILD_BOTTLE_HAND 0x15FD0
#define OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_SWORD 0x13F38
#define OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_TLUT 0x06A118
#define OOT_LINK_CHILD_SHEATH 0x15408

Gfx sKokiriSwordHilt[] = {
    gsDPPipeSync(),
    gsDPSetTextureLUT(G_TT_RGBA16),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsSPClearGeometryMode(G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsSPSetGeometryMode(G_CULL_BACK | G_FOG | G_LIGHTING),
    gsDPSetPrimColor(0, 0, 255, 255, 255, 255),
    gsDPNoOp(), // branch goes here
};

Gfx sKokiriSwordBlade[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsSPSetGeometryMode(G_CULL_BACK | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsDPPipeSync(),
    gsDPNoOp(), // branch goes here
};

void repointSkeletonAndDisableLOD(FlexSkeletonHeader *skel, GlobalObjectsSegmentMap segments) {

    LodLimb **limbs = (LodLimb **)skel->sh.segment;

    for (int i = 0; i < skel->sh.limbCount; ++i) {
        LodLimb *limb = limbs[i];

        if (limb->dLists[0]) {
            GlobalObjects_rebaseDL(limb->dLists[0], segments);
            limb->dLists[1] = limb->dLists[0];
        }
    }
}

#define REPOINT_AND_SET(handle, dlid, base, dlOffset, segments)     \
    {                                                               \
        Gfx *_g = (Gfx *)(SEGMENTED_TO_GLOBAL_PTR(base, dlOffset)); \
        GlobalObjects_rebaseDL(_g, segments);                       \
        PlayerModelManager_setDisplayList(handle, dlid, _g);        \
    }                                                               \
    (void)0

static PlayerModelManagerHandle sOoTChildHumanHandle = 0;
static bool sWasChildRegisterAttempted = false;

void registerChildLink() {
    if (sWasChildRegisterAttempted) {
        return;
    }

    gLinkChildOOT = recomp_alloc(OOT_LINK_CHILD_SIZE);

    loadGameplayKeepOOT();

    if (sIsRealGameplayKeepLoaded && PMMZobj_extractChildLink(gLinkChildOOT, OOT_LINK_CHILD_SIZE)) {
        // PMMZobj_extractChildLink(gLinkChildOOT, OOT_LINK_CHILD_SIZE);

        GlobalObjectsSegmentMap cLinkSegs = {0};
        cLinkSegs[0x06] = gLinkChildOOT;
        cLinkSegs[0x04] = gGameplayKeepOOT;

        GlobalObjects_globalizeLodLimbSkeleton(gLinkChildOOT, (FlexSkeletonHeader *)OOT_LINK_CHILD_SKELETON);

        FlexSkeletonHeader *skel = SEGMENTED_TO_GLOBAL_PTR(gLinkChildOOT, OOT_LINK_CHILD_SKELETON);

        repointSkeletonAndDisableLOD(skel, cLinkSegs);

        PlayerModelManagerHandle h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_object_link_child", PMM_MODEL_TYPE_CHILD);
        sOoTChildHumanHandle = h;
        PlayerModelManager_setDisplayName(h, "Young Link (OOT)");
        PlayerModelManager_setAuthor(h, "Nintendo");
        PlayerModelManager_setSkeleton(h, skel);
        setupFaceTextures(h, gLinkChildOOT);

        // Kokiri Sword (Hilt)
        GlobalObjects_rebaseDL((Gfx *)(gLinkChildOOT + OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_SWORD), cLinkSegs);
        const uintptr_t KOKIRI_SWORD_HILT_START = 0x14048;
        const uintptr_t KOKIRI_SWORD_HILT_END = 0x14110;
        gSPEndDisplayList(gLinkChildOOT + KOKIRI_SWORD_HILT_END);
        gSPBranchList(&sKokiriSwordHilt[ARRAY_COUNT(sKokiriSwordHilt) - 1], gLinkChildOOT + KOKIRI_SWORD_HILT_START);

        // Kokiri Sword (Blade)
        const uintptr_t KOKIRI_SWORD_BLADE_START = KOKIRI_SWORD_HILT_END + sizeof(Gfx);
        gSPBranchList(&sKokiriSwordBlade[ARRAY_COUNT(sKokiriSwordBlade) - 1], gLinkChildOOT + KOKIRI_SWORD_BLADE_START);

        Mtx swordBack;
        guPosition(&swordBack, 0.f, 0.f, 0.f, 1.f, -440.f, -211.f, 0.f);

#define REPOINT_SET_CHILD(dloffset, pmmdl) REPOINT_AND_SET(h, pmmdl, gLinkChildOOT, dloffset, cLinkSegs)
        REPOINT_SET_CHILD(OOT_LINK_CHILD_LFIST, PMM_DL_LFIST);
        REPOINT_SET_CHILD(OOT_LINK_CHILD_RFIST, PMM_DL_RFIST);
        REPOINT_SET_CHILD(OOT_LINK_CHILD_BOTTLE_HAND, PMM_DL_LHAND_BOTTLE);
        REPOINT_SET_CHILD(OOT_LINK_CHILD_SHEATH, PMM_DL_SWORD_KOKIRI_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_HILT, sKokiriSwordHilt);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_BLADE, sKokiriSwordBlade);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_KOKIRI_BACK, &swordBack);

#undef REPOINT_SET_CHILD
    } else {
        recomp_printf("PMM_FS: Could not load OOT child Link from file system!\n");
        recomp_free(gLinkChildOOT);
        gLinkChildOOT = NULL;
    }

    sWasChildRegisterAttempted = true;
}

#define OOT_LINK_ADULT_SKELETON 0x060377F4
#define OOT_LINK_ADULT_SIZE 0x37800
#define OOT_LINK_ADULT_LFIST 0x21CE8
#define OOT_LINK_ADULT_RFIST 0x226E0
#define OOT_LINK_ADULT_BOTTLE_HAND 0x24B58
#define OOT_LINK_ADULT_FPS_RIGHT_FOREARM 0x29FA0
#define OOT_LINK_ADULT_FPS_LEFT_FOREARM 0x29918
#define OOT_LINK_ADULT_FPS_LEFT_HAND 0x29C20
#define OOT_LINK_ADULT_FPS_RIGHT_HAND_AND_BOW 0x2A248
#define OOT_LINK_ADULT_RIGHT_HAND_AND_OCARINA 0x24698
#define OOT_LINK_ADULT_RIGHT_HAND_AND_MIRROR_SHIELD 0x241C0
#define OOT_LINK_ADULT_RIGHT_HAND_AND_HYLIAN_SHIELD 0x22970
#define OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD 0x21F78
#define OOT_LINK_ADULT_SHEATH 0x249D8
#define OOT_LINK_ADULT_RIGHT_HAND_HOLDING_HOOKSHOT 0x24D70
#define OOT_LINK_ADULT_HOOKSHOT_HOOK 0x2B288
#define OOT_LINK_ADULT_HOOKSHOT_CHAIN 0x2AFF0
#define OOT_LINK_ADULT_FPS_RIGHT_HAND_HOLDING_HOOKSHOT 0x2A738
#define OOT_LINK_ADULT_RIGHT_HAND_HOLDING_BOW 0x22DA8
#define OOT_LINK_ADULT_BOW_STRING 0x2B108
#define OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS 0x238C8
#define OOT_LINK_ADULT_BOTTLE 0x2AD58

static Gfx sMasterSwordHilt[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsSPSetGeometryMode(G_FOG | G_LIGHTING),
    gsSPDisplayList(0x0C000000),
    gsDPPipeSync(),
    gsSPNoOp(), // branch command goes here
};

static Gfx sBiggoronSwordBlade[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsSPSetGeometryMode(G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsDPPipeSync(),
    gsSPNoOp(), // branch command goes here
};

static bool sWasAdultRegisterAttempted = false;
static PlayerModelManagerHandle sOoTAdultHumanHandle = 0;
static PlayerModelManagerHandle sOoTAdultFierceDeityHandle = 0;

void registerAdultLink() {
    if (sWasAdultRegisterAttempted) {
        return;
    }

    gLinkAdultOOT = recomp_alloc(OOT_LINK_ADULT_SIZE);

    loadGameplayKeepOOT();

    if (sIsRealGameplayKeepLoaded && PMMZobj_extractAdultLink(gLinkAdultOOT, OOT_LINK_ADULT_SIZE)) {
        // PMMZobj_extractAdultLink(gLinkAdultOOT, OOT_LINK_ADULT_SIZE);

        GlobalObjectsSegmentMap aLinkSegs = {0};
        aLinkSegs[0x06] = gLinkAdultOOT;
        aLinkSegs[0x04] = gGameplayKeepOOT;

        GlobalObjects_globalizeLodLimbSkeleton(gLinkAdultOOT, (FlexSkeletonHeader *)OOT_LINK_ADULT_SKELETON);

        FlexSkeletonHeader *skel = SEGMENTED_TO_GLOBAL_PTR(gLinkAdultOOT, OOT_LINK_ADULT_SKELETON);

        repointSkeletonAndDisableLOD(skel, aLinkSegs);

        // Human
        PlayerModelManagerHandle h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_object_link_boy_adult", PMM_MODEL_TYPE_ADULT);
        sOoTAdultHumanHandle = h;
        PlayerModelManager_setDisplayName(h, "Adult Link (OOT)");
        PlayerModelManager_setAuthor(h, "Nintendo");
        PlayerModelManager_setSkeleton(h, skel);
        setupFaceTextures(h, gLinkAdultOOT);

        // Unglue FPS right hand from bow
        const uintptr_t BOW_START_DRAW_OFFSET = 0x2A2C0;
        const uintptr_t HAND_START_DRAW_OFFSET = 0x2A3F8;
        gSPBranchList(gLinkAdultOOT + BOW_START_DRAW_OFFSET, SEGMENT_ADDR(6, HAND_START_DRAW_OFFSET));

        // Unglue Ocarina of Time from hand
        const uintptr_t OCARINA_HAND_START_DRAW_OFFSET = 0x24740;
        const uintptr_t OCARINA_START_DRAW_OFFSET = 0x248D8;
        gSPBranchList(gLinkAdultOOT + OCARINA_HAND_START_DRAW_OFFSET, SEGMENT_ADDR(6, OCARINA_START_DRAW_OFFSET));

        // Unglue fist from Mirror Shield
        const uintptr_t MIRROR_SHIELD_HAND_START_DRAW_OFFSET = 0x24378;
        const uintptr_t MIRROR_SHIELD_BODY_START_DRAW_OFFSET = 0x245A8;
        gSPBranchList(gLinkAdultOOT + MIRROR_SHIELD_HAND_START_DRAW_OFFSET, SEGMENT_ADDR(6, MIRROR_SHIELD_BODY_START_DRAW_OFFSET));

        // Unglue fist from Hylian Shield
        const uintptr_t HYLIAN_SHIELD_HAND_START_DRAW_OFFSET = 0x22AC8;
        const uintptr_t HYLIAN_SHIELD_BACK_START_DRAW_OFFSET = 0x22C28;
        const uintptr_t HYLIAN_SHIELD_HAND2_START_DRAW_OFFSET = 0x22CB8;
        gSPBranchList(gLinkAdultOOT + HYLIAN_SHIELD_HAND_START_DRAW_OFFSET, SEGMENT_ADDR(6, HYLIAN_SHIELD_BACK_START_DRAW_OFFSET));
        gSPEndDisplayList(gLinkAdultOOT + HYLIAN_SHIELD_HAND2_START_DRAW_OFFSET);

        // Master Sword Blade
        GlobalObjects_rebaseDL((Gfx *)(gLinkAdultOOT + OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD), aLinkSegs);
        const uintptr_t MASTER_SWORD_BLADE_END = 0x22060;
        gSPEndDisplayList(gLinkAdultOOT + MASTER_SWORD_BLADE_END);

        // Master Sword Hilt
        const uintptr_t MASTER_SWORD_HILT_START = MASTER_SWORD_BLADE_END + sizeof(Gfx);
        const uintptr_t MASTER_SWORD_HILT_END = 0x22248;
        gSPBranchList(&sMasterSwordHilt[ARRAY_COUNT(sMasterSwordHilt) - 1], gLinkAdultOOT + MASTER_SWORD_HILT_START);
        gSPEndDisplayList(gLinkAdultOOT + MASTER_SWORD_HILT_END);

        // Hookshot (3rd Person)
        const uintptr_t HOOKSHOT_HAND_START = 0x24FC0;
        gSPEndDisplayList(gLinkAdultOOT + HOOKSHOT_HAND_START);

        // Hookshot (1st Person)
        const uintptr_t HOOKSHOT_FPS_HAND_START = 0x2AA28;
        gSPEndDisplayList(gLinkAdultOOT + HOOKSHOT_FPS_HAND_START);

        // Bow
        const uintptr_t BOW_HAND_START = 0x22F00;
        gSPEndDisplayList(gLinkAdultOOT + BOW_HAND_START);

        // Biggoron Sword Hilt
        GlobalObjects_rebaseDL((Gfx *)(gLinkAdultOOT + OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS), aLinkSegs);
        const uintptr_t BIGGORON_SWORD_HILT_END = 0x23A28;
        gSPEndDisplayList(gLinkAdultOOT + BIGGORON_SWORD_HILT_END);

        // Biggoron Sword Blade
        const uintptr_t BIGGORON_SWORD_BLADE_START = BIGGORON_SWORD_HILT_END + sizeof(Gfx);
        const uintptr_t BIGGORON_SWORD_HAND_START = 0x23AE0;
        gSPBranchList(&sBiggoronSwordBlade[ARRAY_COUNT(sBiggoronSwordBlade) - 1], gLinkAdultOOT + BIGGORON_SWORD_BLADE_START);
        gSPEndDisplayList(gLinkAdultOOT + BIGGORON_SWORD_HAND_START);

        Mtx hookshotChainHookOffset;
        guPosition(&hookshotChainHookOffset, 0, 0, 0, 1, 50, 840, 0);

        Mtx shieldBack;
        guPosition(&shieldBack, 0.f, 0.f, 180.f, 1.f, 935.f, 94.f, 29.f);

        Mtx swordBack;
        guPosition(&swordBack, 0.f, 0.f, 0.f, 1.f, -715.f, -310.f, 78.f);

#define REPOINT_SET_ADULT(dloffset, pmmdl) REPOINT_AND_SET(h, pmmdl, gLinkAdultOOT, dloffset, aLinkSegs)
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LFIST, PMM_DL_LFIST);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RFIST, PMM_DL_RFIST);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_BOTTLE_HAND, PMM_DL_LHAND_BOTTLE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_LEFT_HAND, PMM_DL_FPS_LHAND);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_RIGHT_HAND_AND_BOW, PMM_DL_FPS_RHAND);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_RIGHT_FOREARM, PMM_DL_FPS_RFOREARM);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_LEFT_FOREARM, PMM_DL_FPS_LFOREARM);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_AND_OCARINA, PMM_DL_OCARINA_TIME);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_AND_HYLIAN_SHIELD, PMM_DL_SHIELD_HERO);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_AND_MIRROR_SHIELD, PMM_DL_SHIELD_MIRROR);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD, PMM_DL_SWORD_GILDED_BLADE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD_GILDED_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GILDED_HILT, sMasterSwordHilt);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD, PMM_DL_SWORD_RAZOR_BLADE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD_RAZOR_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_RAZOR_HILT, sMasterSwordHilt);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD, PMM_DL_SWORD_KOKIRI_BLADE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD_KOKIRI_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_HILT, sMasterSwordHilt);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_RIGHT_HAND_HOLDING_HOOKSHOT, PMM_DL_FPS_HOOKSHOT);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_HOLDING_HOOKSHOT, PMM_DL_HOOKSHOT);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_HOOKSHOT_CHAIN, PMM_DL_HOOKSHOT_CHAIN);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_HOOKSHOT_HOOK, PMM_DL_HOOKSHOT_HOOK);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_HOLDING_BOW, PMM_DL_BOW);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_BOW_STRING, PMM_DL_BOW_STRING);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD4_SHEATH);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS, PMM_DL_SWORD_GREAT_FAIRY_HILT);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GREAT_FAIRY_BLADE, sBiggoronSwordBlade);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD5_SHEATH);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS, PMM_DL_SWORD_FIERCE_DEITY_HILT);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_FIERCE_DEITY_BLADE, sBiggoronSwordBlade);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_BOTTLE, PMM_DL_BOTTLE_GLASS);
        PlayerModelManager_setDisplayList(h, PMM_DL_BOTTLE_CONTENTS, gEmptyDL);
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &shieldBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_MIRROR_BACK, &shieldBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_GILDED_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_RAZOR_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_KOKIRI_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD4_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD5_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_HOOKSHOT_CHAIN_AND_HOOK, &hookshotChainHookOffset);

        // Fierce Deity
        h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_object_link_boy_fd", PMM_MODEL_TYPE_FIERCE_DEITY);
        sOoTAdultFierceDeityHandle = h;
        PlayerModelManager_setDisplayName(h, "Adult Link (OOT)");
        PlayerModelManager_setAuthor(h, "Nintendo");
        PlayerModelManager_setSkeleton(h, skel);
        setupFaceTextures(h, gLinkAdultOOT);

        REPOINT_SET_ADULT(OOT_LINK_ADULT_LFIST, PMM_DL_LFIST);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RFIST, PMM_DL_RFIST);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_BOTTLE_HAND, PMM_DL_LHAND_BOTTLE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_LEFT_HAND, PMM_DL_FPS_LHAND);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_RIGHT_HAND_AND_BOW, PMM_DL_FPS_RHAND);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_RIGHT_FOREARM, PMM_DL_FPS_RFOREARM);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_LEFT_FOREARM, PMM_DL_FPS_LFOREARM);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_AND_OCARINA, PMM_DL_OCARINA_TIME);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_AND_HYLIAN_SHIELD, PMM_DL_SHIELD_HERO);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_AND_MIRROR_SHIELD, PMM_DL_SHIELD_MIRROR);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD, PMM_DL_SWORD_GILDED_BLADE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD_GILDED_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GILDED_HILT, sMasterSwordHilt);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD, PMM_DL_SWORD_RAZOR_BLADE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD_RAZOR_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_RAZOR_HILT, sMasterSwordHilt);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD, PMM_DL_SWORD_KOKIRI_BLADE);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD_KOKIRI_SHEATH);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_HILT, sMasterSwordHilt);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_FPS_RIGHT_HAND_HOLDING_HOOKSHOT, PMM_DL_FPS_HOOKSHOT);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_HOLDING_HOOKSHOT, PMM_DL_HOOKSHOT);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_HOOKSHOT_CHAIN, PMM_DL_HOOKSHOT_CHAIN);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_HOOKSHOT_HOOK, PMM_DL_HOOKSHOT_HOOK);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_RIGHT_HAND_HOLDING_BOW, PMM_DL_BOW);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_BOW_STRING, PMM_DL_BOW_STRING);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD4_SHEATH);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS, PMM_DL_SWORD_GREAT_FAIRY_HILT);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GREAT_FAIRY_BLADE, sBiggoronSwordBlade);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_SHEATH, PMM_DL_SWORD5_SHEATH);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS, PMM_DL_SWORD_FIERCE_DEITY_HILT);
        PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_FIERCE_DEITY_BLADE, sBiggoronSwordBlade);
        REPOINT_SET_ADULT(OOT_LINK_ADULT_BOTTLE, PMM_DL_BOTTLE_GLASS);
        PlayerModelManager_setDisplayList(h, PMM_DL_BOTTLE_CONTENTS, gEmptyDL);
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &shieldBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_MIRROR_BACK, &shieldBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_GILDED_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_RAZOR_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_KOKIRI_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD4_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SWORD5_BACK, &swordBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_HOOKSHOT_CHAIN_AND_HOOK, &hookshotChainHookOffset);

        // Give child-sized adult equipment to child Link
        if (!sWasAdultRegisterAttempted) {
            registerChildLink();
        }

        if (sOoTChildHumanHandle) {
            PlayerModelManagerHandle h = sOoTChildHumanHandle;

#define APPLY_SMALL_DL(pmmDLId, adultDL, mtx)                           \
    {                                                                   \
        static Gfx _dl[] = {                                            \
            gsSPMatrix(mtx, G_MTX_PUSH | G_MTX_MUL | G_MTX_MODELVIEW), \
            gsDPNoOp(),                                                 \
            gsSPPopMatrix(G_MTX_MODELVIEW),                             \
            gsSPEndDisplayList(),                                       \
        };                                                              \
        gSPDisplayList(_dl + 1, adultDL);                               \
        PlayerModelManager_setDisplayList(h, pmmDLId, _dl);             \
    }                                                                   \
    (void)0

            static Mtx sShieldShrinker;
            guPosition(&sShieldShrinker, 0.f, 0.f, 0.f, 0.77f, 0.f, 0.f, 0.f);

            APPLY_SMALL_DL(PMM_DL_SHIELD_HERO, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_AND_HYLIAN_SHIELD, &sShieldShrinker);
            APPLY_SMALL_DL(PMM_DL_SHIELD_MIRROR, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_AND_MIRROR_SHIELD, &sShieldShrinker);

            static Mtx sHookshotShrinker;
            guPosition(&sHookshotShrinker, 0.f, 0.f, 0.f, 0.6f, 0.f, 0.f, 0.f);

            Mtx hookshotChainHookOffsetChild;
            guPosition(&hookshotChainHookOffsetChild, 0, 0, 0, 1, 0, 100, 0);

            APPLY_SMALL_DL(PMM_DL_HOOKSHOT, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_HOLDING_HOOKSHOT, &sHookshotShrinker);
            APPLY_SMALL_DL(PMM_DL_HOOKSHOT_HOOK, gLinkAdultOOT + OOT_LINK_ADULT_HOOKSHOT_HOOK, &sHookshotShrinker);
            PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT_CHAIN, gLinkAdultOOT + OOT_LINK_ADULT_HOOKSHOT_CHAIN);
            APPLY_SMALL_DL(PMM_DL_FPS_HOOKSHOT, gLinkAdultOOT + OOT_LINK_ADULT_FPS_RIGHT_HAND_HOLDING_HOOKSHOT, &sHookshotShrinker);
            PlayerModelManager_setMatrix(h, PMM_MTX_HOOKSHOT_CHAIN_AND_HOOK, &hookshotChainHookOffsetChild);

#undef APPLY_SMALL_DL
        }

#undef REPOINT_SET_ADULT
    } else {
        recomp_printf("PMM_FS: Could not load OOT adult Link from file system!\n");
        recomp_free(gLinkAdultOOT);
        gLinkAdultOOT = NULL;
    }

    sWasAdultRegisterAttempted = true;
}
