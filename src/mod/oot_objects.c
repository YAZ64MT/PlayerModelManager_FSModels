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
#define OOT_LINK_ADULT_SKELETON 0x060377F4

#define OOT_LINK_CHILD_SIZE 0x2CF80
#define OOT_LINK_ADULT_SIZE 0x37800

#define OOT_LINK_CHILD_LFIST 0x13E18
#define OOT_LINK_ADULT_LFIST 0x21CE8

#define OOT_LINK_CHILD_RFIST 0x14320
#define OOT_LINK_ADULT_RFIST 0x226E0

#define OOT_LINK_CHILD_BOTTLE_HAND 0x15FD0
#define OOT_LINK_ADULT_BOTTLE_HAND 0x24B58

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

void registerChildLink() {

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
        PlayerModelManager_setDisplayName(h, "Young Link (OOT)");
        PlayerModelManager_setAuthor(h, "Nintendo");
        PlayerModelManager_setSkeleton(h, skel);
        setupFaceTextures(h, gLinkChildOOT);

#define REPOINT_SET_CHILD(dloffset, pmmdl) REPOINT_AND_SET(h, pmmdl, gLinkChildOOT, dloffset, cLinkSegs)
        REPOINT_SET_CHILD(OOT_LINK_CHILD_LFIST, PMM_DL_LFIST);
        REPOINT_SET_CHILD(OOT_LINK_CHILD_RFIST, PMM_DL_RFIST);
        REPOINT_SET_CHILD(OOT_LINK_CHILD_BOTTLE_HAND, PMM_DL_LHAND_BOTTLE);

#undef REPOINT_SET_CHILD
    } else {
        recomp_printf("PMM_FS: Could not load OOT child Link from file system!\n");
        recomp_free(gLinkChildOOT);
        gLinkChildOOT = NULL;
    }
}

#define OOT_LINK_ADULT_FPS_RIGHT_FOREARM 0x29FA0
#define OOT_LINK_ADULT_FPS_LEFT_FOREARM 0x29918
#define OOT_LINK_ADULT_FPS_LEFT_HAND 0x29C20
#define OOT_LINK_ADULT_FPS_RIGHT_HAND_AND_BOW 0x2A248
#define OOT_LINK_ADULT_RIGHT_HAND_AND_OCARINA 0x24698
#define OOT_LINK_ADULT_RIGHT_HAND_AND_MIRROR_SHIELD 0x241C0
#define OOT_LINK_ADULT_RIGHT_HAND_AND_HYLIAN_SHIELD 0x22970

void registerAdultLink() {
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
        PlayerModelManager_setDisplayName(h, "Adult Link (OOT)");
        PlayerModelManager_setAuthor(h, "Nintendo");
        PlayerModelManager_setSkeleton(h, skel);
        setupFaceTextures(h, gLinkAdultOOT);

        // Unglue FPS right hand from bow
        const int BOW_START_DRAW_OFFSET = 0x2A2C0;
        const int HAND_START_DRAW_OFFSET = 0x2A3F8;
        gSPBranchList(gLinkAdultOOT + BOW_START_DRAW_OFFSET, SEGMENT_ADDR(6, HAND_START_DRAW_OFFSET));

        // Unglue Ocarina of Time from hand
        const int OCARINA_HAND_START_DRAW_OFFSET = 0x24740;
        const int OCARINA_START_DRAW_OFFSET = 0x248D8;
        gSPBranchList(gLinkAdultOOT + OCARINA_HAND_START_DRAW_OFFSET, SEGMENT_ADDR(6, OCARINA_START_DRAW_OFFSET));

        // Unglue fist from Mirror Shield
        const int MIRROR_SHIELD_HAND_START_DRAW_OFFSET = 0x24378;
        const int MIRROR_SHIELD_BODY_START_DRAW_OFFSET = 0x245A8;
        gSPBranchList(gLinkAdultOOT + MIRROR_SHIELD_HAND_START_DRAW_OFFSET, SEGMENT_ADDR(6, MIRROR_SHIELD_BODY_START_DRAW_OFFSET));

        // Unglue fist from Hylian Shield
        const int HYLIAN_SHIELD_HAND_START_DRAW_OFFSET = 0x22AC8;
        const int HYLIAN_SHIELD_BACK_START_DRAW_OFFSET = 0x22C28;
        const int HYLIAN_SHIELD_HAND2_START_DRAW_OFFSET = 0x22CB8;

        gSPBranchList(gLinkAdultOOT + HYLIAN_SHIELD_HAND_START_DRAW_OFFSET, SEGMENT_ADDR(6, HYLIAN_SHIELD_BACK_START_DRAW_OFFSET));
        gSPEndDisplayList(gLinkAdultOOT + HYLIAN_SHIELD_HAND2_START_DRAW_OFFSET);

        Mtx shieldBack;
        guPosition(&shieldBack, 0.f, 0.f, 180.f, 1.f, 935.f, 94.f, 29.f);

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
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &shieldBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_MIRROR_BACK, &shieldBack);

        // Fierce Deity
        h = PLAYERMODELMANAGER_REGISTER_MODEL("oot_object_link_boy_fd", PMM_MODEL_TYPE_FIERCE_DEITY);
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
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_HERO_BACK, &shieldBack);
        PlayerModelManager_setMatrix(h, PMM_MTX_SHIELD_MIRROR_BACK, &shieldBack);

#undef REPOINT_SET_ADULT
    } else {
        recomp_printf("PMM_FS: Could not load OOT adult Link from file system!\n");
        recomp_free(gLinkAdultOOT);
        gLinkAdultOOT = NULL;
    }
}
