#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompdata.h"
#include "defines_ooto.h"
#include "ml64compat.h"
#include "globalobjects_api.h"

#define OOTO_CHILD_SKEL_PTR 0x060053A8
#define OOTO_ADULT_SKEL_PTR 0x06005380

#define READ_U32(arr, offset) (*(u32 *)(arr + offset))

void OotoFixHeaderSkelPtr(void *obj) {
    u8 *zobj = obj;

    uintptr_t *skelHeaderPtrPtr = (uintptr_t *)(zobj + Z64O_SKELETON_HEADER_POINTER);

    if (*skelHeaderPtrPtr == 0xFFFFFFFF) {
        switch (zobj[Z64O_FORM_BYTE]) {
            case OOTO_FORM_BYTE_CHILD:
                *skelHeaderPtrPtr = OOTO_CHILD_SKEL_PTR;
                break;

            case OOTO_FORM_BYTE_ADULT:
                *skelHeaderPtrPtr = OOTO_ADULT_SKEL_PTR;
                break;

            default:
                break;
        }
    }
}

void OotoFixChildLeftShoulder(void *obj) {
    u8 *zobj = obj;

    if (zobj[Z64O_FORM_BYTE] == OOTO_FORM_BYTE_CHILD) {
        Gfx *lShoulderLUT = (Gfx *)(zobj + OOTO_CHILD_LUT_DL_LSHOULDER);
        Gfx *lForearmLUT = (Gfx *)(zobj + OOTO_CHILD_LUT_DL_LFOREARM);

        Gfx *lShoulder = (Gfx *)(lShoulderLUT->words.w1);
        Gfx *lForearm = (Gfx *)(lForearmLUT->words.w1);

        if (lShoulder == lForearm) {
            FlexSkeletonHeader *skel = (FlexSkeletonHeader *)(zobj + SEGMENT_OFFSET(*(uintptr_t *)(zobj + Z64O_SKELETON_HEADER_POINTER)));

            LodLimb **limbs = (LodLimb **)(zobj + SEGMENT_OFFSET(skel->sh.segment));

            LodLimb *lShoulderLimb = (LodLimb *)(zobj + SEGMENT_OFFSET(limbs[PLAYER_LIMB_LEFT_SHOULDER - 1]));

            lShoulderLUT->words.w1 = (uintptr_t)lShoulderLimb->dLists[0];
        }
    }
}

bool isSegmentedPtr(void *p) {
    return (uintptr_t)p >> 24 <= 0xF;
}

U32ValueHashmapHandle gSegmentPointersRemap;

// remap specific pointers
// useful for gameplay keep textures
void remapPtrs(void *obj, Gfx *dlPtr, U32ValueHashmapHandle oldPtrs2New) {
    if (isSegmentedPtr(obj)) {
        recomp_printf("remapPtrs: Incorrectly passed in non-global pointer 0x%X as zobj\n", obj);
        return;
    }

    if (!isSegmentedPtr(dlPtr)) {
        recomp_printf("remapPtrs: Incorrectly passed in global pointer 0x%X as dlPtr\n", obj);
        return;
    }

    unsigned targetSegment = SEGMENT_NUMBER(dlPtr);

    Gfx *globalPtr = SEGMENTED_TO_GLOBAL_PTR(obj, dlPtr);

    unsigned opcode;

    unsigned currentSegment;

    bool isEndDl = false;

    while (!isEndDl) {
        opcode = globalPtr->words.w0 >> 24;

        currentSegment = SEGMENT_NUMBER(globalPtr->words.w1);

        switch (opcode) {
            case G_ENDDL:
                isEndDl = true;
                break;

            case G_DL:
                if (currentSegment == targetSegment) {
                    remapPtrs(obj, (Gfx *)globalPtr->words.w1, oldPtrs2New);
                }

                if ((globalPtr->words.w0 >> 16 & 0xFF) == G_DL_NOPUSH) {
                    isEndDl = true;
                }
                // FALL THROUGH
            case G_VTX:
            case G_MTX:
            case G_SETTIMG:
            case G_SETZIMG:
            case G_SETCIMG:
            case G_MOVEMEM:
                recomputil_u32_value_hashmap_get(oldPtrs2New, globalPtr->words.w1, &globalPtr->words.w1);
                break;

            default:
                break;
        }

        globalPtr = (Gfx *)((uintptr_t)globalPtr + sizeof(Gfx));
    }
}

void applySegmentPtrRemaps(void *obj, Gfx *dl) {
    remapPtrs(obj, dl, gSegmentPointersRemap);
}

void setSegmentPtrRemap(u32 key, u32 value) {
    recomputil_u32_value_hashmap_insert(gSegmentPointersRemap, key, value);
}

RECOMP_CALLBACK(".", _internal_initHashObjects)
void initSegmentPtrsRemap() {
    gSegmentPointersRemap = recomputil_create_u32_value_hashmap();
}
