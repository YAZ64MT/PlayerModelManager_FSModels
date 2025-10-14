#include "global.h"
#include "recompconfig.h"
#include "recomputils.h"
#include "oot_objects.h"
#include "globalobjects_api.h"
#include "playermodelmanager_api.h"
#include "oot_equipment.h"

Gfx *gEmptyDLPtr = gEmptyDL;

Gfx *gOOTHookshot = NULL;
Gfx *gOOTHookshotChild = NULL;
Gfx *gOOTHookshotFirstPerson = NULL;
Gfx *gOOTHookshotFirstPersonChild = NULL;
Gfx *gOOTHookshotChain = NULL;
Gfx *gOOTHookshotHook = NULL;
Gfx *gOOTHookshotHookChild = NULL;
Gfx *gOOTBow = NULL;
Gfx *gOOTBowFirstPerson = NULL;
Gfx *gOOTBowChild = NULL;
Gfx *gOOTBowFirstPersonChild = NULL;
Gfx *gOOTBowString = NULL;
Gfx *gOOTMirrorShield = NULL;
Gfx *gOOTMirrorShieldChild = NULL;
Gfx *gOOTHylianShield = NULL;
Gfx *gOOTHylianShieldChild = NULL;
Gfx *gOOTDekuShield = NULL;
Gfx *gOOTDekuShieldAdult = NULL;
Gfx *gOOTMasterSwordHilt = NULL;
Gfx *gOOTMasterSwordBlade = NULL;
Gfx *gOOTMasterSwordSheath = NULL;
Gfx *gOOTKokiriSwordHilt = NULL;
Gfx *gOOTKokiriSwordBlade = NULL;
Gfx *gOOTKokiriSwordSheath = NULL;
Gfx *gOOTBiggoronSwordHilt = NULL;
Gfx *gOOTBiggoronSwordBlade = NULL;
Gfx *gOOTBottleAdult = NULL;
Gfx *gOOTBottleChild = NULL;
Gfx *gOOTDekuStick = NULL;
Gfx *gOOTFairyOcarina = NULL;

Mtx gOOTAdultShieldMtx;
Mtx gOOTHookshotHookAndChainMtx;
Mtx gOOTMasterSwordHiltMtx;

Mtx gOOTKokiriSwordHiltMtx;
Mtx gOOTHookshotHookAndChainChildMtx;
Mtx gOOTDekuShieldMtx;

static bool sIsAdultEquipmentInitialized = false;
static bool sIsChildEquipmentInitialized = false;
static bool sIsCrossEquipmentInitialized = false;

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
#define OOT_LINK_ADULT_FPS_RIGHT_HAND_AND_BOW 0x2A248
#define OOT_LINK_ADULT_BOW_STRING 0x2B108
#define OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS 0x238C8
#define OOT_LINK_ADULT_BOTTLE 0x2AD58

#define OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_SWORD 0x13F38
#define OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_TLUT 0x06A118
#define OOT_LINK_CHILD_SHEATH 0x15408
#define OOT_LINK_CHILD_BOTTLE_GLASS 0x18478
#define OOT_LINK_CHILD_DEKU_STICK 0x6CC0
#define OOT_LINK_CHILD_RFIST_HOLDING_DEKU_SHIELD 0x14440
#define OOT_LINK_CHILD_RFIST_HOLDING_FAIRY_OCARINA 0x15BA8

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

static Gfx sKokiriSwordHilt[] = {
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

static Gfx sKokiriSwordBlade[] = {
    gsDPPipeSync(),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsSPSetGeometryMode(G_CULL_BACK | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsDPPipeSync(),
    gsDPNoOp(), // branch goes here
};

#define DECLARE_STATIC_MATRIX_WRAPPED_DL(dlName, mtxName, dl)           \
    static Mtx mtxName;                                                 \
    static Gfx dlName[] = {                                             \
        gsSPMatrix(&mtxName, G_MTX_PUSH | G_MTX_MUL | G_MTX_MODELVIEW), \
        gsSPDisplayList(dl),                                            \
        gsSPPopMatrix(G_MTX_MODELVIEW),                                 \
        gsSPEndDisplayList(),                                           \
    }

static Gfx sCallHylianShield[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sHylianShieldChild, sHylianShieldResizer, sCallHylianShield);

static Gfx sCallMirrorShield[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sMirrorShieldChild, sMirrorShieldResizer, sCallMirrorShield);

static Gfx sCallHookshot[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sHookshotChild, sHookshotResizer, sCallHookshot);

static Gfx sCallHookshotHook[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sHookshotHookChild, sHookshotHookResizer, sCallHookshotHook);

static Gfx sCallHookshotFirstPerson[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sHookshotFirstPersonChild, sHookshotFirstPersonResizer, sCallHookshotFirstPerson);

static Gfx sCallBow[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sBowChild, sBowChildResizer, sCallBow);

static Gfx sCallBowFirstPerson[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sBowFirstPersonChild, sBowFirstPersonChildResizer, sCallBowFirstPerson);

static Gfx sCallDekuShield[] = {
    gsSPEndDisplayList(),
};

DECLARE_STATIC_MATRIX_WRAPPED_DL(sDekuShieldAdult, sDekuShieldAdultResizer, sCallDekuShield);

void initCrossAgeEquipment() {
    if (sIsCrossEquipmentInitialized || !sIsAdultEquipmentInitialized || !sIsChildEquipmentInitialized) {
        return;
    }

    sIsCrossEquipmentInitialized = true;

    gSPBranchList(sCallHylianShield, gOOTHylianShield);
    guPosition(&sHylianShieldResizer, 0.f, 0.f, 0.f, 0.8f, 0.f, 0.f, 0.f);
    gOOTHylianShieldChild = sHylianShieldChild;

    gSPBranchList(sCallMirrorShield, gOOTMirrorShield);
    guPosition(&sMirrorShieldResizer, 0.f, 0.f, 0.f, 0.77f, 0.f, 0.f, 0.f);
    gOOTMirrorShieldChild = sMirrorShieldChild;

    gSPBranchList(sCallHookshot, gOOTHookshot);
    guPosition(&sHookshotResizer, 0.f, 0.f, 0.f, 0.6f, 0.f, 0.f, 0.f);
    gOOTHookshotChild = sHookshotChild;

    gSPBranchList(sCallHookshotFirstPerson, gOOTHookshotFirstPerson);
    guPosition(&sHookshotFirstPersonResizer, 0.f, 0.f, 0.f, 0.6f, 0.f, 0.f, 0.f);
    gOOTHookshotFirstPersonChild = sHookshotFirstPersonChild;

    gSPBranchList(sCallHookshotHook, gOOTHookshotHook);
    guPosition(&sHookshotHookResizer, 0.f, 0.f, 0.f, 0.6f, 0.f, 0.f, 0.f);
    gOOTHookshotHookChild = sHookshotHookChild;

    gSPBranchList(sCallBow, gOOTBow);
    guPosition(&sBowChildResizer, 0.f, 0.f, 0.f, 0.65f, 0.f, -100.f, 0.f);
    gOOTBowChild = sBowChild;

    gSPBranchList(sCallBowFirstPerson, gOOTBowFirstPerson);
    guPosition(&sBowFirstPersonChildResizer, 0.f, 0.f, 0.f, 0.65f, 0.f, -100.f, 0.f);
    gOOTBowFirstPersonChild = sBowFirstPersonChild;

    gSPBranchList(sCallDekuShield, gOOTDekuShield);
    guPosition(&sDekuShieldAdultResizer, 0.f, 0.f, 0.f, 1.35f, 0.f, 0.f, 0.f);
    gOOTDekuShieldAdult = sDekuShieldAdult;

    guPosition(&gOOTHookshotHookAndChainChildMtx, 0.f, 0.f, 0.f, 1.f, 30.f, 150.f, 0.f);

    addEquipmentToModelManager();
}

void initAdultEquipment() {
    if (sIsAdultEquipmentInitialized || !gLinkAdultOOT) {
        return;
    }

    sIsAdultEquipmentInitialized = true;

    GlobalObjectsSegmentMap aLinkSegs = {0};
    aLinkSegs[0x06] = gLinkAdultOOT;
    aLinkSegs[0x04] = gGameplayKeepOOT;

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

    // Bow (First Person)
    const uintptr_t BOW_FPS_HAND_START = 0x2A3F8;
    gSPEndDisplayList(gLinkAdultOOT + BOW_FPS_HAND_START);

    // Biggoron Sword Hilt
    GlobalObjects_rebaseDL((Gfx *)(gLinkAdultOOT + OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS), aLinkSegs);
    const uintptr_t BIGGORON_SWORD_HILT_END = 0x23A28;
    gSPEndDisplayList(gLinkAdultOOT + BIGGORON_SWORD_HILT_END);

    // Biggoron Sword Blade
    const uintptr_t BIGGORON_SWORD_BLADE_START = BIGGORON_SWORD_HILT_END + sizeof(Gfx);
    const uintptr_t BIGGORON_SWORD_HAND_START = 0x23AE0;
    gSPBranchList(&sBiggoronSwordBlade[ARRAY_COUNT(sBiggoronSwordBlade) - 1], gLinkAdultOOT + BIGGORON_SWORD_BLADE_START);
    gSPEndDisplayList(gLinkAdultOOT + BIGGORON_SWORD_HAND_START);

    guPosition(&gOOTHookshotHookAndChainMtx, 0.f, 0.f, 0.f, 1.f, 50.f, 840.f, 0.f);

    guPosition(&gOOTAdultShieldMtx, 0.f, 0.f, 180.f, 1.f, 935.f, 94.f, 29.f);

    guPosition(&gOOTMasterSwordHiltMtx, 0.f, 0.f, 0.f, 1.f, -715.f, -310.f, 78.f);

    // clang-format off
    #define REPOINT_SET_ADULT(ptrToSet, dl) { ptrToSet = (Gfx *)(dl); GlobalObjects_rebaseDL(ptrToSet, aLinkSegs); } (void)0
    // clang-format on

    REPOINT_SET_ADULT(gOOTHylianShield, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_AND_HYLIAN_SHIELD);
    REPOINT_SET_ADULT(gOOTMirrorShield, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_AND_MIRROR_SHIELD);
    REPOINT_SET_ADULT(gOOTMasterSwordBlade, gLinkAdultOOT + OOT_LINK_ADULT_LEFT_HAND_AND_MASTER_SWORD);
    REPOINT_SET_ADULT(gOOTMasterSwordHilt, sMasterSwordHilt);
    REPOINT_SET_ADULT(gOOTMasterSwordSheath, gLinkAdultOOT + OOT_LINK_ADULT_SHEATH);
    REPOINT_SET_ADULT(gOOTHookshotFirstPerson, gLinkAdultOOT + OOT_LINK_ADULT_FPS_RIGHT_HAND_HOLDING_HOOKSHOT);
    REPOINT_SET_ADULT(gOOTHookshot, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_HOLDING_HOOKSHOT);
    REPOINT_SET_ADULT(gOOTHookshotChain, gLinkAdultOOT + OOT_LINK_ADULT_HOOKSHOT_CHAIN);
    REPOINT_SET_ADULT(gOOTHookshotHook, gLinkAdultOOT + OOT_LINK_ADULT_HOOKSHOT_HOOK);
    REPOINT_SET_ADULT(gOOTBow, gLinkAdultOOT + OOT_LINK_ADULT_RIGHT_HAND_HOLDING_BOW);
    REPOINT_SET_ADULT(gOOTBowFirstPerson, gLinkAdultOOT + OOT_LINK_ADULT_FPS_RIGHT_HAND_AND_BOW);
    REPOINT_SET_ADULT(gOOTBowString, gLinkAdultOOT + OOT_LINK_ADULT_BOW_STRING);
    REPOINT_SET_ADULT(gOOTBiggoronSwordHilt, gLinkAdultOOT + OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS);
    REPOINT_SET_ADULT(gOOTBiggoronSwordBlade, sBiggoronSwordBlade);
    REPOINT_SET_ADULT(gOOTBottleAdult, gLinkAdultOOT + OOT_LINK_ADULT_BOTTLE);

    initCrossAgeEquipment();
}

void initChildEquipment() {
    if (sIsChildEquipmentInitialized || !gLinkChildOOT) {
        return;
    }

    sIsChildEquipmentInitialized = true;

    GlobalObjectsSegmentMap cLinkSegs = {0};
    cLinkSegs[0x06] = gLinkChildOOT;
    cLinkSegs[0x04] = gGameplayKeepOOT;

    // Kokiri Sword (Hilt)
    GlobalObjects_rebaseDL((Gfx *)(gLinkChildOOT + OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_SWORD), cLinkSegs);
    const uintptr_t KOKIRI_SWORD_HILT_START = 0x14048;
    const uintptr_t KOKIRI_SWORD_HILT_END = 0x14110;
    gSPEndDisplayList(gLinkChildOOT + KOKIRI_SWORD_HILT_END);
    gSPBranchList(&sKokiriSwordHilt[ARRAY_COUNT(sKokiriSwordHilt) - 1], gLinkChildOOT + KOKIRI_SWORD_HILT_START);

    // Kokiri Sword (Blade)
    const uintptr_t KOKIRI_SWORD_BLADE_START = KOKIRI_SWORD_HILT_END + sizeof(Gfx);
    gSPBranchList(&sKokiriSwordBlade[ARRAY_COUNT(sKokiriSwordBlade) - 1], gLinkChildOOT + KOKIRI_SWORD_BLADE_START);

    // Deku Shield
    const uintptr_t DEKU_SHIELD_HAND_START = 0x14580;
    gSPEndDisplayList(gLinkChildOOT + DEKU_SHIELD_HAND_START);

    guPosition(&gOOTKokiriSwordHiltMtx, 0.f, 0.f, 0.f, 1.f, -440.f, -211.f, 0.f);

    guPosition(&gOOTDekuShieldMtx, 0.f, 0.f, 180.f, 1.f, 545.f, 0.f, 80.f);

    // Fairy Ocarina
    const uintptr_t FAIRY_OCARINA_HAND_START = 0x15CB8;
    gSPEndDisplayList(gLinkChildOOT + FAIRY_OCARINA_HAND_START);

    // clang-format off
    #define REPOINT_SET_CHILD(ptrToSet, dl) { ptrToSet = (Gfx *)(dl); GlobalObjects_rebaseDL(ptrToSet, cLinkSegs); } (void)0
    // clang-format on

    REPOINT_SET_CHILD(gOOTBottleChild, gLinkChildOOT + OOT_LINK_CHILD_BOTTLE_GLASS);
    REPOINT_SET_CHILD(gOOTDekuStick, gLinkChildOOT + OOT_LINK_CHILD_DEKU_STICK);
    REPOINT_SET_CHILD(gOOTKokiriSwordSheath, gLinkChildOOT + OOT_LINK_CHILD_SHEATH);
    REPOINT_SET_CHILD(gOOTKokiriSwordBlade, sKokiriSwordBlade);
    REPOINT_SET_CHILD(gOOTKokiriSwordHilt, sKokiriSwordHilt);
    REPOINT_SET_CHILD(gOOTDekuShield, gLinkChildOOT + OOT_LINK_CHILD_RFIST_HOLDING_DEKU_SHIELD);
    REPOINT_SET_CHILD(gOOTFairyOcarina, gLinkChildOOT + OOT_LINK_CHILD_RFIST_HOLDING_FAIRY_OCARINA);

    initCrossAgeEquipment();
}

extern Mtx gLinkHumanMirrorShieldMtx;
extern Mtx gLinkHumanHerosShieldMtx;

void addEquipmentToModelManager() {
    PlayerModelManagerHandle h;
    
    // Mirror Shield (OOT)
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_mirror_shield3_a", PMM_MODEL_TYPE_SHIELD_MIRROR);
    PlayerModelManager_setDisplayName(h, "Mirror Shield (OOT)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_MIRROR_BACK, &gOOTAdultShieldMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_MIRROR, gOOTMirrorShield);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_mirror_shield3_c", PMM_MODEL_TYPE_SHIELD_MIRROR);
    PlayerModelManager_setDisplayName(h, "Mirror Shield (OOT) (Child)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_MIRROR_BACK, SEGMENTED_TO_GLOBAL_PTR(GlobalObjects_getGlobalObject(OBJECT_LINK_CHILD), &gLinkHumanMirrorShieldMtx));
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_MIRROR, gOOTMirrorShieldChild);

    // Hylian Shield
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_hylian_shield2_a", PMM_MODEL_TYPE_SHIELD_HERO);
    PlayerModelManager_setDisplayName(h, "Hylian Shield (OOT)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &gOOTAdultShieldMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_HERO, gOOTHylianShield);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_hylian_shield2_c", PMM_MODEL_TYPE_SHIELD_HERO);
    PlayerModelManager_setDisplayName(h, "Hylian Shield (OOT) (Child)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, SEGMENTED_TO_GLOBAL_PTR(GlobalObjects_getGlobalObject(OBJECT_LINK_CHILD), &gLinkHumanHerosShieldMtx));
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_HERO, gOOTHylianShieldChild);

    // Deku Shield
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_deku_shield2_a", PMM_MODEL_TYPE_SHIELD_HERO);
    PlayerModelManager_setDisplayName(h, "Deku Shield (Adult)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &gOOTAdultShieldMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_HERO, gOOTDekuShieldAdult);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_deku_shield2_c", PMM_MODEL_TYPE_SHIELD_HERO);
    PlayerModelManager_setDisplayName(h, "Deku Shield");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &gOOTDekuShieldMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_HERO, gOOTDekuShield);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_deku_shield1_a", PMM_MODEL_TYPE_SHIELD_DEKU);
    PlayerModelManager_setDisplayName(h, "Deku Shield (Adult)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_DEKU_BACK, &gOOTAdultShieldMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_DEKU, gOOTDekuShieldAdult);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_deku_shield1_c", PMM_MODEL_TYPE_SHIELD_DEKU);
    PlayerModelManager_setDisplayName(h, "Deku Shield");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_DEKU_BACK, &gOOTDekuShieldMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SHIELD_DEKU, gOOTDekuShield);

    // Master Sword
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_master_sword1_a", PMM_MODEL_TYPE_SWORD_KOKIRI);
    PlayerModelManager_setDisplayName(h, "Master Sword");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_KOKIRI_BACK, &gOOTMasterSwordHiltMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_BLADE, gOOTMasterSwordBlade);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_HILT, gOOTMasterSwordHilt);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_SHEATH, gOOTMasterSwordSheath);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_master_sword2_a", PMM_MODEL_TYPE_SWORD_RAZOR);
    PlayerModelManager_setDisplayName(h, "Master Sword");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_RAZOR_BACK, &gOOTMasterSwordHiltMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_RAZOR_BLADE, gOOTMasterSwordBlade);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_RAZOR_HILT, gOOTMasterSwordHilt);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_RAZOR_SHEATH, gOOTMasterSwordSheath);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_master_sword3_a", PMM_MODEL_TYPE_SWORD_GILDED);
    PlayerModelManager_setDisplayName(h, "Master Sword");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_GILDED_BACK, &gOOTMasterSwordHiltMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GILDED_BLADE, gOOTMasterSwordBlade);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GILDED_HILT, gOOTMasterSwordHilt);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GILDED_SHEATH, gOOTMasterSwordSheath);

    // Kokiri Sword (OOT)
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_kokiri_sword1_c", PMM_MODEL_TYPE_SWORD_KOKIRI);
    PlayerModelManager_setDisplayName(h, "Kokiri Sword (OoT)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SWORD_KOKIRI_BACK, &gOOTKokiriSwordHiltMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_BLADE, gOOTKokiriSwordBlade);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_HILT, gOOTKokiriSwordHilt);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_KOKIRI_SHEATH, gOOTKokiriSwordSheath);

    // Biggoron Sword
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_biggoron_sword5_a", PMM_MODEL_TYPE_SWORD_GREAT_FAIRY);
    PlayerModelManager_setDisplayName(h, "Biggoron Sword");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SWORD5_BACK, &gOOTMasterSwordHiltMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GREAT_FAIRY_BLADE, gOOTBiggoronSwordBlade);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_GREAT_FAIRY_HILT, gOOTBiggoronSwordHilt);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD5_SHEATH, gOOTMasterSwordSheath);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_biggoron_sword4_a", PMM_MODEL_TYPE_SWORD_FIERCE_DIETY);
    PlayerModelManager_setDisplayName(h, "Biggoron Sword");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_SWORD4_BACK, &gOOTMasterSwordHiltMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_FIERCE_DEITY_BLADE, gOOTBiggoronSwordBlade);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD_FIERCE_DEITY_HILT, gOOTBiggoronSwordHilt);
    PlayerModelManager_setDisplayList(h, PMM_DL_SWORD4_SHEATH, gOOTMasterSwordSheath);

    // Hookshot (OOT)
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_hookshot_a", PMM_MODEL_TYPE_HOOKSHOT);
    PlayerModelManager_setDisplayName(h, "Hookshot (OoT)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_HOOKSHOT_CHAIN_AND_HOOK, &gOOTHookshotHookAndChainMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT, gOOTHookshot);
    PlayerModelManager_setDisplayList(h, PMM_DL_FPS_HOOKSHOT, gOOTHookshotFirstPerson);
    PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT_CHAIN, gOOTHookshotChain);
    PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT_HOOK, gOOTHookshotHook);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_hookshot_c", PMM_MODEL_TYPE_HOOKSHOT);
    PlayerModelManager_setDisplayName(h, "Hookshot (OoT) (Child)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setMatrix(h, PMM_MTX_HOOKSHOT_CHAIN_AND_HOOK, &gOOTHookshotHookAndChainChildMtx);
    PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT, gOOTHookshotChild);
    PlayerModelManager_setDisplayList(h, PMM_DL_FPS_HOOKSHOT, gOOTHookshotFirstPersonChild);
    PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT_CHAIN, gOOTHookshotChain);
    PlayerModelManager_setDisplayList(h, PMM_DL_HOOKSHOT_HOOK, gOOTHookshotHookChild);

    // Bottle (OOT)
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_bottle_a", PMM_MODEL_TYPE_BOTTLE);
    PlayerModelManager_setDisplayName(h, "Bottle (OoT) (Adult)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setDisplayList(h, PMM_DL_BOTTLE_GLASS, gOOTBottleAdult);
    PlayerModelManager_setDisplayList(h, PMM_DL_BOTTLE_CONTENTS, gEmptyDL);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_bottle_c", PMM_MODEL_TYPE_BOTTLE);
    PlayerModelManager_setDisplayName(h, "Bottle (OoT) (Child)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setDisplayList(h, PMM_DL_BOTTLE_GLASS, gOOTBottleChild);
    PlayerModelManager_setDisplayList(h, PMM_DL_BOTTLE_CONTENTS, gEmptyDL);

    // Bow (OOT)
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_bow_a", PMM_MODEL_TYPE_BOW);
    PlayerModelManager_setDisplayName(h, "Bow (OoT)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setDisplayList(h, PMM_DL_BOW, gOOTBow);
    PlayerModelManager_setDisplayList(h, PMM_DL_FPS_BOW, gOOTBowFirstPerson);
    PlayerModelManager_setDisplayList(h, PMM_DL_BOW_STRING, gOOTBowString);

    // Fairy Ocarina
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_ocarina1_fairy_c", PMM_MODEL_TYPE_OCARINA_FAIRY);
    PlayerModelManager_setDisplayName(h, "Fairy Ocarina");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setDisplayList(h, PMM_DL_OCARINA_FAIRY, gOOTFairyOcarina);

    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_ocarina2_fairy_c", PMM_MODEL_TYPE_OCARINA_TIME);
    PlayerModelManager_setDisplayName(h, "Fairy Ocarina");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setDisplayList(h, PMM_DL_OCARINA_TIME, gOOTFairyOcarina);

    // Deku Stick (OOT)
    h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_deku_stick_c", PMM_MODEL_TYPE_DEKU_STICK);
    PlayerModelManager_setDisplayName(h, "Deku Stick (OoT)");
    PlayerModelManager_setAuthor(h, "Nintendo");
    PlayerModelManager_setDisplayList(h, PMM_DL_DEKU_STICK, gOOTDekuStick);
}
