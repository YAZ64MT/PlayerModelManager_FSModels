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
Gfx *gOOTBowChild = NULL;
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
#define OOT_LINK_ADULT_BOW_STRING 0x2B108
#define OOT_LINK_ADULT_LEFT_HAND_HOLDING_BGS 0x238C8
#define OOT_LINK_ADULT_BOTTLE 0x2AD58

#define OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_SWORD 0x13F38
#define OOT_LINK_CHILD_LFIST_HOLDING_KOKIRI_TLUT 0x06A118
#define OOT_LINK_CHILD_SHEATH 0x15408
#define OOT_LINK_CHILD_BOTTLE_GLASS 0x18478
#define OOT_LINK_CHILD_DEKU_STICK 0x6CC0
#define OOT_LINK_CHILD_RFIST_HOLDING_DEKU_SHIELD 0x14440

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
    guPosition(&sBowChildResizer, 0.f, 0.f, 0.f, 0.8f, 0.f, 0.f, 0.f);
    gOOTBowChild = sBowChild;

    gSPBranchList(sCallDekuShield, gOOTDekuShield);
    guPosition(&sDekuShieldAdultResizer, 0.f, 0.f, 0.f, 1.35f, 0.f, 0.f, 0.f);
    gOOTDekuShieldAdult = sDekuShieldAdult;

    guPosition(&gOOTHookshotHookAndChainChildMtx, 0.f, 0.f, 0.f, 1.f, 0.f, 100.f, 0.f);
}

void initAdultEquipment() {
    if (sIsAdultEquipmentInitialized || !gLinkAdultOOT) {
        return;
    }

    sIsAdultEquipmentInitialized = true;

    GlobalObjectsSegmentMap aLinkSegs = {0};
    aLinkSegs[0x06] = gLinkAdultOOT;
    aLinkSegs[0x04] = gGameplayKeepOOT;

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

    guPosition(&gOOTHookshotHookAndChainMtx, 0, 0, 0, 1, 50, 840, 0);

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

    // clang-format off
    #define REPOINT_SET_CHILD(ptrToSet, dl) { ptrToSet = (Gfx *)(dl); GlobalObjects_rebaseDL(ptrToSet, cLinkSegs); } (void)0
    // clang-format on

    REPOINT_SET_CHILD(gOOTBottleChild, gLinkChildOOT + OOT_LINK_CHILD_BOTTLE_GLASS);
    REPOINT_SET_CHILD(gOOTDekuStick, gLinkChildOOT + OOT_LINK_CHILD_DEKU_STICK);
    REPOINT_SET_CHILD(gOOTKokiriSwordSheath, gLinkChildOOT + OOT_LINK_CHILD_SHEATH);
    REPOINT_SET_CHILD(gOOTKokiriSwordBlade, sKokiriSwordBlade);
    REPOINT_SET_CHILD(gOOTKokiriSwordHilt, sKokiriSwordHilt);
    REPOINT_SET_CHILD(gOOTDekuShield, gLinkChildOOT + OOT_LINK_CHILD_RFIST_HOLDING_DEKU_SHIELD);

    initCrossAgeEquipment();
}

typedef enum {
    OPT_SHIELD2_NONE,
    OPT_SHIELD2_DEKU,
    OPT_SHIELD2_HYLIAN,
    OPT_SHIELD2_MAX,
} Shield2Option;

typedef enum {
    OPT_SHIELD3_NONE,
    OPT_SHIELD3_MIRROR_OOT,
    OPT_SHIELD3_MAX,
} Shield3Option;

typedef enum {
    OPT_HOOKSHOT_NONE,
    OPT_HOOKSHOT_OOT,
    OPT_HOOKSHOT_MAX,
} HookshotOption;

typedef enum {
    OPT_DEKU_STICK_NONE,
    OPT_DEKU_STICK_OOT,
    OPT_DEKU_STICK_MAX,
} DekuStickOption;

typedef enum {
    OPT_BOTTLE_NONE,
    OPT_BOTTLE_OOT,
    OPT_BOTTLE_MAX,
} BottleOption;

typedef enum {
    OPT_SWORD1_NONE,
    OPT_SWORD1_KOKIRI,
    OPT_SWORD1_MASTER,
    OPT_SWORD1_MAX,
} Sword1Option;

typedef enum {
    OPT_SWORD2_NONE,
    OPT_SWORD2_MASTER,
    OPT_SWORD2_MAX,
} Sword2Option;

typedef enum {
    OPT_SWORD3_NONE,
    OPT_SWORD3_MASTER,
    OPT_SWORD3_MAX,
} Sword3Option;

typedef enum {
    OPT_SWORD4_NONE,
    OPT_SWORD4_BIGGORON,
    OPT_SWORD4_MAX,
} Sword4Option;

typedef enum {
    OPT_SWORD5_NONE,
    OPT_SWORD5_BIGGORON,
    OPT_SWORD5_MAX,
} Sword5Option;

typedef enum {
    OPT_BOW_NONE,
    OPT_BOW_FAIRY,
    OPT_BOW_MAX,
} BowOption;

typedef enum {
    OOT_AGE_ADULT,
    OOT_AGE_CHILD,
    OOT_AGE_MAX,
} OOTLinkAge;

typedef struct {
    Gfx **dl[OOT_AGE_MAX];
} AgeDisplayList;

AgeDisplayList sShield2DLs[OPT_SHIELD2_MAX] = {
    {{NULL, NULL}},
    {{&gOOTDekuShieldAdult, &gOOTDekuShield}},
    {{&gOOTHylianShield, &gOOTHylianShieldChild}},
};

AgeDisplayList sShield3DLs[OPT_SHIELD3_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMirrorShield, &gOOTMirrorShieldChild}},
};

AgeDisplayList sSword1HiltDLs[OPT_SWORD1_MAX] = {
    {{NULL, NULL}},
    {{&gOOTKokiriSwordHilt, &gOOTKokiriSwordHilt}},
    {{&gOOTMasterSwordHilt, &gOOTMasterSwordHilt}},
};

AgeDisplayList sSword1BladeDLs[OPT_SWORD1_MAX] = {
    {{NULL, NULL}},
    {{&gOOTKokiriSwordBlade, &gOOTKokiriSwordBlade}},
    {{&gOOTMasterSwordBlade, &gOOTMasterSwordBlade}},
};

AgeDisplayList sSword1SheathDLs[OPT_SWORD1_MAX] = {
    {{NULL, NULL}},
    {{&gOOTKokiriSwordSheath, &gOOTKokiriSwordSheath}},
    {{&gOOTMasterSwordSheath, &gOOTMasterSwordSheath}},
};

AgeDisplayList sSword2HiltDLs[OPT_SWORD2_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordHilt, &gOOTMasterSwordHilt}},
};

AgeDisplayList sSword2BladeDLs[OPT_SWORD2_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordBlade, &gOOTMasterSwordBlade}},
};

AgeDisplayList sSword2SheathDLs[OPT_SWORD2_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordSheath, &gOOTMasterSwordSheath}},
};

AgeDisplayList sSword3HiltDLs[OPT_SWORD3_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordHilt, NULL}},
};

AgeDisplayList sSword3BladeDLs[OPT_SWORD3_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordBlade, NULL}},
};

AgeDisplayList sSword3SheathDLs[OPT_SWORD3_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordSheath, NULL}},
};

AgeDisplayList sSword4HiltDLs[OPT_SWORD4_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBiggoronSwordHilt, &gOOTBiggoronSwordHilt}},
};

AgeDisplayList sSword4BladeDLs[OPT_SWORD4_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBiggoronSwordBlade, &gOOTBiggoronSwordHilt}},
};

AgeDisplayList sSword4SheathDLs[OPT_SWORD4_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordSheath, &gOOTMasterSwordSheath}},
};

AgeDisplayList sSword5HiltDLs[OPT_SWORD5_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBiggoronSwordHilt, &gOOTBiggoronSwordHilt}},
};

AgeDisplayList sSword5BladeDLs[OPT_SWORD5_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBiggoronSwordBlade, &gOOTBiggoronSwordBlade}},
};

AgeDisplayList sSword5SheathDLs[OPT_SWORD5_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordSheath, NULL}},
};

AgeDisplayList sBottleGlassDLs[OPT_BOTTLE_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBottleAdult, &gOOTBottleChild}},
};

AgeDisplayList sBottleContentsDLs[OPT_BOTTLE_MAX] = {
    {{NULL, NULL}},
    {{&gEmptyDLPtr, &gEmptyDLPtr}},
};

AgeDisplayList sHookshotDLs[OPT_HOOKSHOT_MAX] = {
    {{NULL, NULL}},
    {{&gOOTHookshot, &gOOTHookshotChild}},
};

AgeDisplayList sHookshotChainDLs[OPT_HOOKSHOT_MAX] = {
    {{NULL, NULL}},
    {{&gOOTHookshotChain, &gOOTHookshotChain}},
};

AgeDisplayList sHookshotHookDLs[OPT_HOOKSHOT_MAX] = {
    {{NULL, NULL}},
    {{&gOOTHookshotHook, &gOOTHookshotHookChild}},
};

AgeDisplayList sHookshotFirstPersonDLs[OPT_HOOKSHOT_MAX] = {
    {{NULL, NULL}},
    {{&gOOTHookshotFirstPerson, &gOOTHookshotFirstPersonChild}},
};

AgeDisplayList sDekuStickDLs[OPT_DEKU_STICK_MAX] = {
    {{NULL, NULL}},
    {{&gOOTDekuStick, &gOOTDekuStick}},
};

AgeDisplayList sBowDLs[OPT_BOW_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBow, &gOOTBowChild}},
};

AgeDisplayList sBowStringDLs[OPT_BOW_MAX] = {
    {{NULL, NULL}},
    {{&gOOTBowString, NULL}},
};

typedef struct {
    Mtx *mtx[OOT_AGE_MAX];
} AgeDisplayListMatrix;

AgeDisplayListMatrix sShield2Mtxs[OPT_SHIELD2_MAX] = {
    {{NULL, NULL}},
    {{&gOOTAdultShieldMtx, &gOOTDekuShieldMtx}},
    {{&gOOTAdultShieldMtx, NULL}},
};

AgeDisplayListMatrix sShield3Mtxs[OPT_SHIELD3_MAX] = {
    {{NULL, NULL}},
    {{&gOOTAdultShieldMtx, NULL}},
};

AgeDisplayListMatrix sSword1HiltMtxs[OPT_SWORD1_MAX] = {
    {{NULL, NULL}},
    {{&gOOTKokiriSwordHiltMtx, &gOOTKokiriSwordHiltMtx}},
    {{&gOOTMasterSwordHiltMtx, &gOOTMasterSwordHiltMtx}},
};

AgeDisplayListMatrix sSword2HiltMtxs[OPT_SWORD2_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordHiltMtx, &gOOTMasterSwordHiltMtx}},
};

AgeDisplayListMatrix sSword3HiltMtxs[OPT_SWORD3_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordHiltMtx, &gOOTMasterSwordHiltMtx}},
};

AgeDisplayListMatrix sSword4HiltMtxs[OPT_SWORD4_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordHiltMtx, &gOOTMasterSwordHiltMtx}},
};

AgeDisplayListMatrix sSword5HiltMtxs[OPT_SWORD5_MAX] = {
    {{NULL, NULL}},
    {{&gOOTMasterSwordHiltMtx, &gOOTMasterSwordHiltMtx}},
};

AgeDisplayListMatrix sHookshotHookAndChainMtxs[OPT_HOOKSHOT_MAX] = {
    {{NULL, NULL}},
    {{&gOOTHookshotHookAndChainMtx, &gOOTHookshotHookAndChainChildMtx}},
};

typedef enum {
    MOD_CFG_SWORD1,
    MOD_CFG_SWORD2,
    MOD_CFG_SWORD3,
    MOD_CFG_SWORD4,
    MOD_CFG_SWORD5,
    MOD_CFG_SHIELD1,
    MOD_CFG_SHIELD2,
    MOD_CFG_SHIELD3,
    MOD_CFG_BOTTLE,
    MOD_CFG_DEKU_STICK,
    MOD_CFG_HOOKSHOT,
    MOD_CFG_BOW,
    MOD_CFG_MAX,
} ModConfigOption;

typedef struct {
    const char *id;
    u32 selected;
    bool isChanged;
} ModConfigEntry;

typedef struct {
    ModConfigEntry entries[OOT_AGE_MAX];
} ModConfigAgeEntry;

#define DECLARE_CONFIG_AGE_ENTRY(adultCfgId, childCfgId)     \
    {                                                        \
        {                                                    \
            {adultCfgId, 0, false}, { childCfgId, 0, false } \
        }                                                    \
    }

ModConfigAgeEntry gModOptions[MOD_CFG_MAX] = {
    DECLARE_CONFIG_AGE_ENTRY("sword1_a", "sword1_c"),         // MOD_CFG_SWORD1
    DECLARE_CONFIG_AGE_ENTRY("sword2_a", ""),                 // MOD_CFG_SWORD2
    DECLARE_CONFIG_AGE_ENTRY("sword3_a", ""),                 // MOD_CFG_SWORD3
    DECLARE_CONFIG_AGE_ENTRY("sword4_a", ""),                 // MOD_CFG_SWORD4
    DECLARE_CONFIG_AGE_ENTRY("sword5_a", "sword5_c"),         // MOD_CFG_SWORD5
    DECLARE_CONFIG_AGE_ENTRY("", ""),                         // MOD_CFG_SHIELD1
    DECLARE_CONFIG_AGE_ENTRY("shield2_a", "shield2_c"),       // MOD_CFG_SHIELD2
    DECLARE_CONFIG_AGE_ENTRY("shield3_a", "shield3_c"),       // MOD_CFG_SHIELD3
    DECLARE_CONFIG_AGE_ENTRY("bottla_a", "bottle_c"),         // MOD_CFG_BOTTLE
    DECLARE_CONFIG_AGE_ENTRY("deku_stick_a", "deku_stick_c"), // MOD_CFG_DEKU_STICK
    DECLARE_CONFIG_AGE_ENTRY("hookshot_a", "hookshot_c"),     // MOD_CFG_HOOKSHOT
    DECLARE_CONFIG_AGE_ENTRY("bow_a", "bow_c"),               // MOD_CFG_BOW
};

typedef struct {
    PlayerModelManagerDisplayListId dlId;
    PlayerModelManagerMatrixId mtxId;
    AgeDisplayList *ageDLs;
    AgeDisplayListMatrix *ageMtxs;
    ModConfigAgeEntry *cfgEntry;
} DLConfigEntry;

#define NO_MTX PMM_MTX_MAX

DLConfigEntry gOptionsToDLs[] = {
    {PMM_DL_SWORD1_HILT, PMM_MTX_SWORD1_BACK, sSword1HiltDLs, sSword1HiltMtxs, &gModOptions[MOD_CFG_SWORD1]},
    {PMM_DL_SWORD1_BLADE, NO_MTX, sSword1BladeDLs, NULL, &gModOptions[MOD_CFG_SWORD1]},
    {PMM_DL_SWORD1_SHEATH, NO_MTX, sSword1SheathDLs, NULL, &gModOptions[MOD_CFG_SWORD1]},
    {PMM_DL_SWORD2_HILT, PMM_MTX_SWORD2_BACK, sSword2HiltDLs, sSword2HiltMtxs, &gModOptions[MOD_CFG_SWORD2]},
    {PMM_DL_SWORD2_BLADE, NO_MTX, sSword2BladeDLs, NULL, &gModOptions[MOD_CFG_SWORD2]},
    {PMM_DL_SWORD2_SHEATH, NO_MTX, sSword2SheathDLs, NULL, &gModOptions[MOD_CFG_SWORD2]},
    {PMM_DL_SWORD3_HILT, PMM_MTX_SWORD3_BACK, sSword3HiltDLs, sSword3HiltMtxs, &gModOptions[MOD_CFG_SWORD3]},
    {PMM_DL_SWORD3_BLADE, NO_MTX, sSword3BladeDLs, NULL, &gModOptions[MOD_CFG_SWORD3]},
    {PMM_DL_SWORD3_SHEATH, NO_MTX, sSword3SheathDLs, NULL, &gModOptions[MOD_CFG_SWORD3]},
    {PMM_DL_SWORD4_HILT, PMM_MTX_SWORD4_BACK, sSword4HiltDLs, sSword4HiltMtxs, &gModOptions[MOD_CFG_SWORD4]},
    {PMM_DL_SWORD4_BLADE, NO_MTX, sSword4BladeDLs, NULL, &gModOptions[MOD_CFG_SWORD4]},
    {PMM_DL_SWORD4_SHEATH, NO_MTX, sSword4SheathDLs, NULL, &gModOptions[MOD_CFG_SWORD4]},
    {PMM_DL_SWORD5_HILT, PMM_MTX_SWORD5_BACK, sSword5HiltDLs, sSword5HiltMtxs, &gModOptions[MOD_CFG_SWORD5]},
    {PMM_DL_SWORD5_BLADE, NO_MTX, sSword5BladeDLs, NULL, &gModOptions[MOD_CFG_SWORD5]},
    {PMM_DL_SWORD5_SHEATH, NO_MTX, sSword5SheathDLs, NULL, &gModOptions[MOD_CFG_SWORD5]},
    {PMM_DL_SHIELD2, PMM_MTX_SHIELD2_BACK, sShield2DLs, sShield2Mtxs, &gModOptions[MOD_CFG_SHIELD2]},
    {PMM_DL_SHIELD3, PMM_MTX_SHIELD3_BACK, sShield3DLs, sShield3Mtxs, &gModOptions[MOD_CFG_SHIELD3]},
    {PMM_DL_BOTTLE_GLASS, NO_MTX, sBottleGlassDLs, NULL, &gModOptions[MOD_CFG_BOTTLE]},
    {PMM_DL_BOTTLE_CONTENTS, NO_MTX, sBottleContentsDLs, NULL, &gModOptions[MOD_CFG_BOTTLE]},
    {PMM_DL_DEKU_STICK, NO_MTX, sDekuStickDLs, NULL, &gModOptions[MOD_CFG_DEKU_STICK]},
    {PMM_DL_HOOKSHOT, PMM_MTX_HOOKSHOT_CHAIN_AND_HOOK, sHookshotDLs, sHookshotHookAndChainMtxs, &gModOptions[MOD_CFG_HOOKSHOT]},
    {PMM_DL_FPS_HOOKSHOT, NO_MTX, sHookshotFirstPersonDLs, NULL, &gModOptions[MOD_CFG_HOOKSHOT]},
    {PMM_DL_HOOKSHOT_CHAIN, NO_MTX, sHookshotChainDLs, NULL, &gModOptions[MOD_CFG_HOOKSHOT]},
    {PMM_DL_HOOKSHOT_HOOK, NO_MTX, sHookshotHookDLs, NULL, &gModOptions[MOD_CFG_HOOKSHOT]},
    {PMM_DL_BOW, NO_MTX, sBowDLs, NULL, &gModOptions[MOD_CFG_BOW]},
    {PMM_DL_BOW_STRING, NO_MTX, sBowStringDLs, NULL, &gModOptions[MOD_CFG_BOW]},
};

RECOMP_CALLBACK("*", recomp_on_play_main)
void updateConfigDLs(PlayState *play) {
    if (sIsCrossEquipmentInitialized) {
        // update mod options
        for (int i = 0; i < MOD_CFG_MAX; ++i) {
            ModConfigAgeEntry *currAgeEntry = &gModOptions[i];
            for (int j = 0; j < OOT_AGE_MAX; ++j) {
                ModConfigEntry *curr = &currAgeEntry->entries[j];
                if (curr->id[0] != '\0') {
                    u32 newVal = recomp_get_config_u32(curr->id);
                    curr->isChanged = newVal != curr->selected;
                    curr->selected = newVal;
                }
            }
        }

        typedef struct {
            PlayerModelManagerHandle handle;
            OOTLinkAge age;
        } HandleAndAge;

        HandleAndAge handlesAndAges[] = {
            {getChildLinkHumanHandle(), OOT_AGE_CHILD},
            {getAdultLinkHumanHandle(), OOT_AGE_ADULT},
            {getAdultLinkFierceDeityHandle(), OOT_AGE_ADULT},
        };

        // apply any changes made
        for (int i = 0; i < ARRAY_COUNT(handlesAndAges); ++i) {
            HandleAndAge *ha = &handlesAndAges[i];
            if (ha->handle) {
                for (int j = 0; j < ARRAY_COUNT(gOptionsToDLs); ++j) {
                    DLConfigEntry *curr = &gOptionsToDLs[j];
                    ModConfigEntry *currAgeCfg = &curr->cfgEntry->entries[ha->age];
                    if (currAgeCfg->isChanged) {
                        Gfx **dlPtr = curr->ageDLs[currAgeCfg->selected].dl[ha->age];
                        Gfx *dl = dlPtr ? *dlPtr : NULL;
                        PlayerModelManager_setDisplayList(ha->handle, curr->dlId, dl);

                        if (curr->mtxId < PMM_MTX_MAX && curr->ageMtxs) {
                            Mtx *mtx = curr->ageMtxs[currAgeCfg->selected].mtx[ha->age];
                            PlayerModelManager_setMatrix(ha->handle, curr->mtxId, mtx);
                        }
                    }
                }
            }
        }
    }
}
