/*
 * PROJECT:         ReactOS win32 kernel mode subsystem
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            win32ss/gdi/ntgdi/freetype.c
 * PURPOSE:         FreeType font engine interface
 * PROGRAMMERS:     Copyright 2001 Huw D M Davies for CodeWeavers.
 *                  Copyright 2006 Dmitry Timoshkov for CodeWeavers.
 *                  Copyright 2016-2019 Katayama Hirofumi MZ.
 */

/** Includes ******************************************************************/

#include <win32k.h>

#include FT_GLYPH_H
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRIGONOMETRY_H
#include FT_BITMAP_H
#include FT_OUTLINE_H
#include FT_WINFONTS_H
#include FT_SFNT_NAMES_H
#include FT_SYNTHESIS_H
#include FT_TRUETYPE_IDS_H

#ifndef FT_INTERNAL_INTERNAL_H
    #define  FT_INTERNAL_INTERNAL_H  <freetype/internal/internal.h>
    #include FT_INTERNAL_INTERNAL_H
#endif
#include FT_INTERNAL_TRUETYPE_TYPES_H

#include <gdi/eng/floatobj.h>
#include "font.h"

#define NDEBUG
#include <debug.h>

/* TPMF_FIXED_PITCH is confusing; brain-dead api */
#ifndef _TMPF_VARIABLE_PITCH
    #define _TMPF_VARIABLE_PITCH    TMPF_FIXED_PITCH
#endif

/* Is bold emulation necessary? */
#define EMUBOLD_NEEDED(original, request) \
    ((request) != FW_DONTCARE) && ((request) - (original) >= FW_BOLD - FW_MEDIUM)

extern const MATRIX gmxWorldToDeviceDefault;
extern const MATRIX gmxWorldToPageDefault;
static const FT_Matrix identityMat = {(1 << 16), 0, 0, (1 << 16)};

/* HACK!! Fix XFORMOBJ then use 1:16 / 16:1 */
#define gmxWorldToDeviceDefault gmxWorldToPageDefault

FT_Library  g_FreeTypeLibrary;

/* registry */
static UNICODE_STRING g_FontRegPath =
    RTL_CONSTANT_STRING(L"\\REGISTRY\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");


/* The FreeType library is not thread safe, so we have
   to serialize access to it */
static PFAST_MUTEX      g_FreeTypeLock;

static LIST_ENTRY       g_FontListHead;
static PFAST_MUTEX      g_FontListLock;
static BOOL             g_RenderingEnabled = TRUE;

#define IntLockGlobalFonts() \
    ExEnterCriticalRegionAndAcquireFastMutexUnsafe(g_FontListLock)

#define IntUnLockGlobalFonts() \
    ExReleaseFastMutexUnsafeAndLeaveCriticalRegion(g_FontListLock)

#define ASSERT_GLOBALFONTS_LOCK_HELD() \
    ASSERT(g_FontListLock->Owner == KeGetCurrentThread())

#define IntLockFreeType() \
    ExEnterCriticalRegionAndAcquireFastMutexUnsafe(g_FreeTypeLock)

#define IntUnLockFreeType() \
    ExReleaseFastMutexUnsafeAndLeaveCriticalRegion(g_FreeTypeLock)

#define ASSERT_FREETYPE_LOCK_HELD() \
    ASSERT(g_FreeTypeLock->Owner == KeGetCurrentThread())

#define ASSERT_FREETYPE_LOCK_NOT_HELD() \
    ASSERT(g_FreeTypeLock->Owner != KeGetCurrentThread())

#define MAX_FONT_CACHE 256

static LIST_ENTRY g_FontCacheListHead;
static UINT g_FontCacheNumEntries;

static PWCHAR g_ElfScripts[32] =   /* These are in the order of the fsCsb[0] bits */
{
    L"Western", /* 00 */
    L"Central_European",
    L"Cyrillic",
    L"Greek",
    L"Turkish",
    L"Hebrew",
    L"Arabic",
    L"Baltic",
    L"Vietnamese", /* 08 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* 15 */
    L"Thai",
    L"Japanese",
    L"CHINESE_GB2312",
    L"Hangul",
    L"CHINESE_BIG5",
    L"Hangul(Johab)",
    NULL, NULL, /* 23 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    L"Symbol" /* 31 */
};

/*
 *  For TranslateCharsetInfo
 */
#define CP_SYMBOL   42
#define MAXTCIINDEX 32
static const CHARSETINFO g_FontTci[MAXTCIINDEX] =
{
    /* ANSI */
    { ANSI_CHARSET, 1252, {{0,0,0,0},{FS_LATIN1,0}} },
    { EASTEUROPE_CHARSET, 1250, {{0,0,0,0},{FS_LATIN2,0}} },
    { RUSSIAN_CHARSET, 1251, {{0,0,0,0},{FS_CYRILLIC,0}} },
    { GREEK_CHARSET, 1253, {{0,0,0,0},{FS_GREEK,0}} },
    { TURKISH_CHARSET, 1254, {{0,0,0,0},{FS_TURKISH,0}} },
    { HEBREW_CHARSET, 1255, {{0,0,0,0},{FS_HEBREW,0}} },
    { ARABIC_CHARSET, 1256, {{0,0,0,0},{FS_ARABIC,0}} },
    { BALTIC_CHARSET, 1257, {{0,0,0,0},{FS_BALTIC,0}} },
    { VIETNAMESE_CHARSET, 1258, {{0,0,0,0},{FS_VIETNAMESE,0}} },
    /* reserved by ANSI */
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    /* ANSI and OEM */
    { THAI_CHARSET, 874, {{0,0,0,0},{FS_THAI,0}} },
    { SHIFTJIS_CHARSET, 932, {{0,0,0,0},{FS_JISJAPAN,0}} },
    { GB2312_CHARSET, 936, {{0,0,0,0},{FS_CHINESESIMP,0}} },
    { HANGEUL_CHARSET, 949, {{0,0,0,0},{FS_WANSUNG,0}} },
    { CHINESEBIG5_CHARSET, 950, {{0,0,0,0},{FS_CHINESETRAD,0}} },
    { JOHAB_CHARSET, 1361, {{0,0,0,0},{FS_JOHAB,0}} },
    /* Reserved for alternate ANSI and OEM */
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    /* Reserved for system */
    { DEFAULT_CHARSET, 0, {{0,0,0,0},{FS_LATIN1,0}} },
    { SYMBOL_CHARSET, CP_SYMBOL, {{0,0,0,0},{FS_SYMBOL,0}} }
};

#ifndef CP_OEMCP
    #define CP_OEMCP  1
    #define CP_MACCP  2
#endif

/* Get charset from specified codepage.
   g_FontTci is used also in TranslateCharsetInfo. */
BYTE FASTCALL IntCharSetFromCodePage(UINT uCodePage)
{
    UINT i;

    if (uCodePage == CP_OEMCP)
        return OEM_CHARSET;

    if (uCodePage == CP_MACCP)
        return MAC_CHARSET;

    for (i = 0; i < MAXTCIINDEX; ++i)
    {
        if (g_FontTci[i].ciACP == 0)
            continue;

        if (g_FontTci[i].ciACP == uCodePage)
            return g_FontTci[i].ciCharset;
    }

    return DEFAULT_CHARSET;
}

/* list head */
static RTL_STATIC_LIST_HEAD(g_FontSubstListHead);

static void SharedMem_AddRef(PSHARED_MEM Ptr)
{
    ASSERT_FREETYPE_LOCK_HELD();

    ++Ptr->RefCount;
}

static void SharedFaceCache_Init(PSHARED_FACE_CACHE Cache)
{
    Cache->OutlineRequiredSize = 0;
    RtlInitUnicodeString(&Cache->FontFamily, NULL);
    RtlInitUnicodeString(&Cache->FullName, NULL);
}

static PSHARED_FACE SharedFace_Create(FT_Face Face, PSHARED_MEM Memory)
{
    PSHARED_FACE Ptr;
    Ptr = ExAllocatePoolWithTag(PagedPool, sizeof(SHARED_FACE), TAG_FONT);
    if (Ptr)
    {
        Ptr->Face = Face;
        Ptr->RefCount = 1;
        Ptr->Memory = Memory;
        SharedFaceCache_Init(&Ptr->EnglishUS);
        SharedFaceCache_Init(&Ptr->UserLanguage);

        SharedMem_AddRef(Memory);
        DPRINT("Creating SharedFace for %s\n", Face->family_name ? Face->family_name : "<NULL>");
    }
    return Ptr;
}

static PSHARED_MEM SharedMem_Create(PBYTE Buffer, ULONG BufferSize, BOOL IsMapping)
{
    PSHARED_MEM Ptr;
    Ptr = ExAllocatePoolWithTag(PagedPool, sizeof(SHARED_MEM), TAG_FONT);
    if (Ptr)
    {
        Ptr->Buffer = Buffer;
        Ptr->BufferSize = BufferSize;
        Ptr->RefCount = 1;
        Ptr->IsMapping = IsMapping;
        DPRINT("Creating SharedMem for %p (%i, %p)\n", Buffer, IsMapping, Ptr);
    }
    return Ptr;
}

static void SharedFace_AddRef(PSHARED_FACE Ptr)
{
    ASSERT_FREETYPE_LOCK_HELD();

    ++Ptr->RefCount;
}

static void RemoveCachedEntry(PFONT_CACHE_ENTRY Entry)
{
    ASSERT_FREETYPE_LOCK_HELD();

    FT_Done_Glyph((FT_Glyph)Entry->BitmapGlyph);
    RemoveEntryList(&Entry->ListEntry);
    ExFreePoolWithTag(Entry, TAG_FONT);
    g_FontCacheNumEntries--;
    ASSERT(g_FontCacheNumEntries <= MAX_FONT_CACHE);
}

static void RemoveCacheEntries(FT_Face Face)
{
    PLIST_ENTRY CurrentEntry, NextEntry;
    PFONT_CACHE_ENTRY FontEntry;

    ASSERT_FREETYPE_LOCK_HELD();

    for (CurrentEntry = g_FontCacheListHead.Flink;
         CurrentEntry != &g_FontCacheListHead;
         CurrentEntry = NextEntry)
    {
        FontEntry = CONTAINING_RECORD(CurrentEntry, FONT_CACHE_ENTRY, ListEntry);
        NextEntry = CurrentEntry->Flink;

        if (FontEntry->Face == Face)
        {
            RemoveCachedEntry(FontEntry);
        }
    }
}

static void SharedMem_Release(PSHARED_MEM Ptr)
{
    ASSERT_FREETYPE_LOCK_HELD();
    ASSERT(Ptr->RefCount > 0);

    if (Ptr->RefCount <= 0)
        return;

    --Ptr->RefCount;
    if (Ptr->RefCount == 0)
    {
        DPRINT("Releasing SharedMem for %p (%i, %p)\n", Ptr->Buffer, Ptr->IsMapping, Ptr);
        if (Ptr->IsMapping)
            MmUnmapViewInSystemSpace(Ptr->Buffer);
        else
            ExFreePoolWithTag(Ptr->Buffer, TAG_FONT);
        ExFreePoolWithTag(Ptr, TAG_FONT);
    }
}

static void SharedFaceCache_Release(PSHARED_FACE_CACHE Cache)
{
    RtlFreeUnicodeString(&Cache->FontFamily);
    RtlFreeUnicodeString(&Cache->FullName);
}

static void SharedFace_Release(PSHARED_FACE Ptr)
{
    IntLockFreeType();
    ASSERT(Ptr->RefCount > 0);

    if (Ptr->RefCount <= 0)
        return;

    --Ptr->RefCount;
    if (Ptr->RefCount == 0)
    {
        DPRINT("Releasing SharedFace for %s\n", Ptr->Face->family_name ? Ptr->Face->family_name : "<NULL>");
        RemoveCacheEntries(Ptr->Face);
        FT_Done_Face(Ptr->Face);
        SharedMem_Release(Ptr->Memory);
        SharedFaceCache_Release(&Ptr->EnglishUS);
        SharedFaceCache_Release(&Ptr->UserLanguage);
        ExFreePoolWithTag(Ptr, TAG_FONT);
    }
    IntUnLockFreeType();
}


static VOID FASTCALL CleanupFontEntryEx(PFONT_ENTRY FontEntry, PFONTGDI FontGDI)
{
    // PFONTGDI FontGDI = FontEntry->Font;
    PSHARED_FACE SharedFace = FontGDI->SharedFace;

    if (FontGDI->Filename)
        ExFreePoolWithTag(FontGDI->Filename, GDITAG_PFF);

    if (FontEntry->StyleName.Buffer)
        RtlFreeUnicodeString(&FontEntry->StyleName);

    if (FontEntry->FaceName.Buffer)
        RtlFreeUnicodeString(&FontEntry->FaceName);

    EngFreeMem(FontGDI);
    SharedFace_Release(SharedFace);
    ExFreePoolWithTag(FontEntry, TAG_FONT);
}

static __inline VOID FASTCALL CleanupFontEntry(PFONT_ENTRY FontEntry)
{
    CleanupFontEntryEx(FontEntry, FontEntry->Font);
}


static __inline void FTVectorToPOINTFX(FT_Vector *vec, POINTFX *pt)
{
    pt->x.value = vec->x >> 6;
    pt->x.fract = (vec->x & 0x3f) << 10;
    pt->x.fract |= ((pt->x.fract >> 6) | (pt->x.fract >> 12));
    pt->y.value = vec->y >> 6;
    pt->y.fract = (vec->y & 0x3f) << 10;
    pt->y.fract |= ((pt->y.fract >> 6) | (pt->y.fract >> 12));
}

/*
   This function builds an FT_Fixed from a FIXED. It simply put f.value
   in the highest 16 bits and f.fract in the lowest 16 bits of the FT_Fixed.
*/
static __inline FT_Fixed FT_FixedFromFIXED(FIXED f)
{
    return (FT_Fixed)((long)f.value << 16 | (unsigned long)f.fract);
}

DBG VOID DumpFontEntry(PFONT_ENTRY FontEntry)
{
    const char *family_name;
    const char *style_name;
    FT_Face Face;
    PFONTGDI FontGDI = FontEntry->Font;

    if (!FontGDI)
    {
        DPRINT("FontGDI NULL\n");
        return;
    }

    Face = (FontGDI->SharedFace ? FontGDI->SharedFace->Face : NULL);
    if (Face)
    {
        family_name = Face->family_name;
        style_name = Face->style_name;
    }
    else
    {
        family_name = "<invalid>";
        style_name = "<invalid>";
    }

    DPRINT("family_name '%s', style_name '%s', FaceName '%wZ', StyleName '%wZ', FontGDI %p, "
           "FontObj %p, iUnique %lu, SharedFace %p, Face %p, CharSet %u, Filename '%S'\n",
           family_name,
           style_name,
           &FontEntry->FaceName,
           &FontEntry->StyleName,
           FontGDI,
           &FontGDI->FontObj,
           FontGDI->iUnique,
           FontGDI->SharedFace,
           Face,
           FontGDI->CharSet,
           FontGDI->Filename);
}

VOID DumpFontList(PLIST_ENTRY Head)
{
    PLIST_ENTRY Entry;
    PFONT_ENTRY CurrentEntry;

    DPRINT("## DumpFontList(%p)\n", Head);

    for (Entry = Head->Flink; Entry != Head; Entry = Entry->Flink)
    {
        CurrentEntry = CONTAINING_RECORD(Entry, FONT_ENTRY, ListEntry);
        DumpFontEntry(CurrentEntry);
    }
}

VOID DumpFontSubstEntry(PFONTSUBST_ENTRY pSubstEntry)
{
    DPRINT("%wZ,%u -> %wZ,%u\n",
        &pSubstEntry->FontNames[FONTSUBST_FROM],
        pSubstEntry->CharSets[FONTSUBST_FROM],
        &pSubstEntry->FontNames[FONTSUBST_TO],
        pSubstEntry->CharSets[FONTSUBST_TO]);
}

VOID DumpFontSubstList(VOID)
{
    PLIST_ENTRY         pHead = &g_FontSubstListHead;
    PLIST_ENTRY         pListEntry;
    PFONTSUBST_ENTRY    pSubstEntry;

    DPRINT("## DumpFontSubstList\n");

    for (pListEntry = pHead->Flink;
         pListEntry != pHead;
         pListEntry = pListEntry->Flink)
    {
        pSubstEntry =
            (PFONTSUBST_ENTRY)CONTAINING_RECORD(pListEntry, FONT_ENTRY, ListEntry);

        DumpFontSubstEntry(pSubstEntry);
    }
}

VOID DumpPrivateFontList(BOOL bDoLock)
{
    PPROCESSINFO Win32Process = PsGetCurrentProcessWin32Process();

    if (!Win32Process)
        return;

    if (bDoLock)
        IntLockProcessPrivateFonts(Win32Process);

    DumpFontList(&Win32Process->PrivateFontListHead);

    if (bDoLock)
        IntUnLockProcessPrivateFonts(Win32Process);
}

VOID DumpGlobalFontList(BOOL bDoLock)
{
    if (bDoLock)
        IntLockGlobalFonts();

    DumpFontList(&g_FontListHead);

    if (bDoLock)
        IntUnLockGlobalFonts();
}

VOID DumpFontInfo(BOOL bDoLock)
{
    DumpGlobalFontList(bDoLock);
    DumpPrivateFontList(bDoLock);
    DumpFontSubstList();
}
#endif

/*
 * IntLoadFontSubstList --- loads the list of font substitutes
 */
BOOL FASTCALL IntLoadFontSubstList(PLIST_ENTRY pHead)
{
    NTSTATUS                        Status;
    HANDLE                          KeyHandle;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    KEY_FULL_INFORMATION            KeyFullInfo;
    ULONG                           i, Length;
    UNICODE_STRING                  FromW, ToW;
    BYTE                            InfoBuffer[128];
    PKEY_VALUE_FULL_INFORMATION     pInfo;
    BYTE                            CharSets[FONTSUBST_FROM_AND_TO];
    LPWSTR                          pch;
    PFONTSUBST_ENTRY                pEntry;
    BOOLEAN                         Success;

    /* the FontSubstitutes registry key */
    static UNICODE_STRING FontSubstKey =
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\"
                            L"Microsoft\\Windows NT\\CurrentVersion\\"
                            L"FontSubstitutes");

    /* open registry key */
    InitializeObjectAttributes(&ObjectAttributes, &FontSubstKey,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL, NULL);
    Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwOpenKey failed: 0x%08X\n", Status);
        return FALSE;   /* failure */
    }

    /* query count of values */
    Status = ZwQueryKey(KeyHandle, KeyFullInformation,
                        &KeyFullInfo, sizeof(KeyFullInfo), &Length);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwQueryKey failed: 0x%08X\n", Status);
        ZwClose(KeyHandle);
        return FALSE;   /* failure */
    }

    /* for each value */
    for (i = 0; i < KeyFullInfo.Values; ++i)
    {
        /* get value name */
        Status = ZwEnumerateValueKey(KeyHandle, i, KeyValueFullInformation,
                                     InfoBuffer, sizeof(InfoBuffer), &Length);
        if (!NT_SUCCESS(Status))
        {
            DPRINT("ZwEnumerateValueKey failed: 0x%08X\n", Status);
            break;      /* failure */
        }

        /* create FromW string */
        pInfo = (PKEY_VALUE_FULL_INFORMATION)InfoBuffer;
        Length = pInfo->NameLength / sizeof(WCHAR);
        pInfo->Name[Length] = UNICODE_NULL;   /* truncate */
        Success = RtlCreateUnicodeString(&FromW, pInfo->Name);
        if (!Success)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DPRINT("RtlCreateUnicodeString failed\n");
            break;      /* failure */
        }

        /* query value */
        Status = ZwQueryValueKey(KeyHandle, &FromW, KeyValueFullInformation, 
                                 InfoBuffer, sizeof(InfoBuffer), &Length);
        pInfo = (PKEY_VALUE_FULL_INFORMATION)InfoBuffer;
        if (!NT_SUCCESS(Status) || !pInfo->DataLength)
        {
            DPRINT("ZwQueryValueKey failed: 0x%08X\n", Status);
            RtlFreeUnicodeString(&FromW);
            break;      /* failure */
        }

        /* create ToW string */
        pch = (LPWSTR)((PUCHAR)pInfo + pInfo->DataOffset);
        Length = pInfo->DataLength / sizeof(WCHAR);
        pch[Length] = UNICODE_NULL; /* truncate */
        Success = RtlCreateUnicodeString(&ToW, pch);
        if (!Success)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DPRINT("RtlCreateUnicodeString failed\n");
            RtlFreeUnicodeString(&FromW);
            break;      /* failure */
        }

        /* does charset exist? (from) */
        CharSets[FONTSUBST_FROM] = DEFAULT_CHARSET;
        pch = wcsrchr(FromW.Buffer, L',');
        if (pch)
        {
            /* truncate */
            *pch = UNICODE_NULL;
            FromW.Length = (pch - FromW.Buffer) * sizeof(WCHAR);
            /* parse charset number */
            CharSets[FONTSUBST_FROM] = (BYTE)_wtoi(pch + 1);
        }

        /* does charset exist? (to) */
        CharSets[FONTSUBST_TO] = DEFAULT_CHARSET;
        pch = wcsrchr(ToW.Buffer, L',');
        if (pch)
        {
            /* truncate */
            *pch = UNICODE_NULL;
            ToW.Length = (pch - ToW.Buffer) * sizeof(WCHAR);
            /* parse charset number */
            CharSets[FONTSUBST_TO] = (BYTE)_wtoi(pch + 1);
        }

        /* is it identical? */
        if (RtlEqualUnicodeString(&FromW, &ToW, TRUE) &&
            CharSets[FONTSUBST_FROM] == CharSets[FONTSUBST_TO])
        {
            RtlFreeUnicodeString(&FromW);
            RtlFreeUnicodeString(&ToW);
            continue;
        }

        /* allocate an entry */
        pEntry = ExAllocatePoolWithTag(PagedPool, sizeof(FONTSUBST_ENTRY), TAG_FONT);
        if (pEntry == NULL)
        {
            DPRINT("ExAllocatePoolWithTag failed\n");
            RtlFreeUnicodeString(&FromW);
            RtlFreeUnicodeString(&ToW);
            break;      /* failure */
        }

        /* store to *pEntry */
        pEntry->FontNames[FONTSUBST_FROM] = FromW;
        pEntry->FontNames[FONTSUBST_TO] = ToW;
        pEntry->CharSets[FONTSUBST_FROM] = CharSets[FONTSUBST_FROM];
        pEntry->CharSets[FONTSUBST_TO] = CharSets[FONTSUBST_TO];

        /* insert pEntry to *pHead */
        InsertTailList(pHead, &pEntry->ListEntry);
    }

    /* close now */
    ZwClose(KeyHandle);

    return NT_SUCCESS(Status);
}

BOOL FASTCALL InitFontSupport(VOID)
{
    ULONG ulError;

    InitializeListHead(&g_FontListHead);
    InitializeListHead(&g_FontCacheListHead);
    g_FontCacheNumEntries = 0;
    /* Fast Mutexes must be allocated from non paged pool */
    g_FontListLock = ExAllocatePoolWithTag(NonPagedPool, sizeof(FAST_MUTEX), TAG_INTERNAL_SYNC);
    if (g_FontListLock == NULL)
    {
        return FALSE;
    }

    ExInitializeFastMutex(g_FontListLock);
    g_FreeTypeLock = ExAllocatePoolWithTag(NonPagedPool, sizeof(FAST_MUTEX), TAG_INTERNAL_SYNC);
    if (g_FreeTypeLock == NULL)
    {
        return FALSE;
    }
    ExInitializeFastMutex(g_FreeTypeLock);

    ulError = FT_Init_FreeType(&g_FreeTypeLibrary);
    if (ulError)
    {
        DPRINT1("FT_Init_FreeType failed with error code 0x%x\n", ulError);
        return FALSE;
    }

    if (!IntLoadFontsInRegistry())
    {
        DPRINT1("Fonts registry is empty.\n");

        /* Load font(s) with writing registry */
        IntLoadSystemFonts();
    }

    IntLoadFontSubstList(&g_FontSubstListHead);

#if DBG
    DumpFontInfo(TRUE);
#endif

    return TRUE;
}

LONG FASTCALL IntWidthMatrix(FT_Face face, FT_Matrix *pmat, LONG lfWidth)
{
    LONG tmAveCharWidth;
    TT_OS2 *pOS2;
    FT_Fixed XScale;

    *pmat = identityMat;

    if (lfWidth == 0)
        return 0;

    ASSERT_FREETYPE_LOCK_HELD();
    pOS2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
    if (!pOS2)
        return 0;

    XScale = face->size->metrics.x_scale;
    tmAveCharWidth = (FT_MulFix(pOS2->xAvgCharWidth, XScale) + 32) >> 6;
    if (tmAveCharWidth == 0)
    {
        tmAveCharWidth = 1;
    }

    if (lfWidth == tmAveCharWidth)
        return 0;

    pmat->xx = INT_TO_FIXED(lfWidth) / tmAveCharWidth;
    pmat->xy = 0;
    pmat->yx = 0;
    pmat->yy = INT_TO_FIXED(1);
    return lfWidth;
}

VOID FASTCALL IntEscapeMatrix(FT_Matrix *pmat, LONG lfEscapement)
{
    FT_Vector vecAngle;
    /* Convert the angle in tenths of degrees into degrees as a 16.16 fixed-point value */
    FT_Angle angle = INT_TO_FIXED(lfEscapement) / 10;
    FT_Vector_Unit(&vecAngle, angle);
    pmat->xx = vecAngle.x;
    pmat->xy = -vecAngle.y;
    pmat->yx = -pmat->xy;
    pmat->yy = pmat->xx;
}

VOID FASTCALL FtMatrixFromMx(FT_Matrix *pmat, PMATRIX pmx)
{
    FLOATOBJ ef;

    /* Create a freetype matrix, by converting to 16.16 fixpoint format */
    ef = pmx->efM11;
    FLOATOBJ_MulLong(&ef, 0x00010000);
    pmat->xx = FLOATOBJ_GetLong(&ef);

    ef = pmx->efM21;
    FLOATOBJ_MulLong(&ef, 0x00010000);
    pmat->xy = FLOATOBJ_GetLong(&ef);

    ef = pmx->efM12;
    FLOATOBJ_MulLong(&ef, 0x00010000);
    pmat->yx = FLOATOBJ_GetLong(&ef);

    ef = pmx->efM22;
    FLOATOBJ_MulLong(&ef, 0x00010000);
    pmat->yy = FLOATOBJ_GetLong(&ef);
}

VOID FtSetCoordinateTransform(FT_Face face,PMATRIX pmx)
{
    FT_Matrix ftmatrix;
    FLOATOBJ efTemp;

    /* Create a freetype matrix, by converting to 16.16 fixpoint format */
    efTemp = pmx->efM11;
    FLOATOBJ_MulLong(&efTemp, 0x00010000);
    ftmatrix.xx = FLOATOBJ_GetLong(&efTemp);

    efTemp = pmx->efM12;
    FLOATOBJ_MulLong(&efTemp, 0x00010000);
    ftmatrix.xy = FLOATOBJ_GetLong(&efTemp);

    efTemp = pmx->efM21;
    FLOATOBJ_MulLong(&efTemp, 0x00010000);
    ftmatrix.yx = FLOATOBJ_GetLong(&efTemp);

    efTemp = pmx->efM22;
    FLOATOBJ_MulLong(&efTemp, 0x00010000);
    ftmatrix.yy = FLOATOBJ_GetLong(&efTemp);

    /* Set the transformation matrix */
    FT_Set_Transform(face, &ftmatrix, 0);
}

static BOOL SubstituteFontByList(PLIST_ENTRY        pHead,
                     PUNICODE_STRING    pOutputName,
                     PUNICODE_STRING    pInputName,
                     BYTE               RequestedCharSet,
                     BYTE               CharSetMap[FONTSUBST_FROM_AND_TO])
{
    PLIST_ENTRY         pListEntry;
    PFONTSUBST_ENTRY    pSubstEntry;
    BYTE                CharSets[FONTSUBST_FROM_AND_TO];

    CharSetMap[FONTSUBST_FROM] = DEFAULT_CHARSET;
    CharSetMap[FONTSUBST_TO] = RequestedCharSet;

    /* for each list entry */
    for (pListEntry = pHead->Flink;
         pListEntry != pHead;
         pListEntry = pListEntry->Flink)
    {
        pSubstEntry =
            (PFONTSUBST_ENTRY)CONTAINING_RECORD(pListEntry, FONT_ENTRY, ListEntry);

        CharSets[FONTSUBST_FROM] = pSubstEntry->CharSets[FONTSUBST_FROM];

        if (CharSets[FONTSUBST_FROM] != DEFAULT_CHARSET &&
            CharSets[FONTSUBST_FROM] != RequestedCharSet)
        {
            continue;   /* not matched */
        }

        /* does charset number exist? (to) */
        if (pSubstEntry->CharSets[FONTSUBST_TO] != DEFAULT_CHARSET)
        {
            CharSets[FONTSUBST_TO] = pSubstEntry->CharSets[FONTSUBST_TO];
        }
        else
        {
            CharSets[FONTSUBST_TO] = RequestedCharSet;
        }

        /* does font name match? */
        if (!RtlEqualUnicodeString(&pSubstEntry->FontNames[FONTSUBST_FROM],
                                   pInputName, TRUE))
        {
            continue;   /* not matched */
        }

        /* update *pOutputName */
        *pOutputName = pSubstEntry->FontNames[FONTSUBST_TO];

        if (CharSetMap[FONTSUBST_FROM] == DEFAULT_CHARSET)
        {
            /* update CharSetMap */
            CharSetMap[FONTSUBST_FROM]  = CharSets[FONTSUBST_FROM];
            CharSetMap[FONTSUBST_TO]    = CharSets[FONTSUBST_TO];
        }
        return TRUE;   /* success */
    }

    return FALSE;
}

static VOID IntUnicodeStringToBuffer(LPWSTR pszBuffer, SIZE_T cbBuffer, const UNICODE_STRING *pString)
{
    SIZE_T cbLength = pString->Length;

    if (cbBuffer < sizeof(UNICODE_NULL))
        return;

    if (cbLength > cbBuffer - sizeof(UNICODE_NULL))
        cbLength = cbBuffer - sizeof(UNICODE_NULL);

    RtlCopyMemory(pszBuffer, pString->Buffer, cbLength);
    pszBuffer[cbLength / sizeof(WCHAR)] = UNICODE_NULL;
}

static NTSTATUS DuplicateUnicodeString(PUNICODE_STRING Source, PUNICODE_STRING Destination)
{
    NTSTATUS Status = STATUS_NO_MEMORY;
    UNICODE_STRING Tmp;

    Tmp.Buffer = ExAllocatePoolWithTag(PagedPool, Source->MaximumLength, TAG_USTR);
    if (Tmp.Buffer)
    {
        Tmp.MaximumLength = Source->MaximumLength;
        Tmp.Length = 0;
        RtlCopyUnicodeString(&Tmp, Source);

        Destination->MaximumLength = Tmp.MaximumLength;
        Destination->Length = Tmp.Length;
        Destination->Buffer = Tmp.Buffer;

        Status = STATUS_SUCCESS;
    }

    return Status;
}

static BOOL SubstituteFontRecurse(LOGFONTW* pLogFont)
{
    UINT            RecurseCount = 5;
    UNICODE_STRING  OutputNameW = { 0 };
    BYTE            CharSetMap[FONTSUBST_FROM_AND_TO];
    BOOL            Found;
    UNICODE_STRING  InputNameW;

    if (pLogFont->lfFaceName[0] == UNICODE_NULL)
        return FALSE;

    RtlInitUnicodeString(&InputNameW, pLogFont->lfFaceName);

    while (RecurseCount-- > 0)
    {
        Found = SubstituteFontByList(&g_FontSubstListHead,
                                     &OutputNameW, &InputNameW,
                                     pLogFont->lfCharSet, CharSetMap);
        if (!Found)
            break;

        IntUnicodeStringToBuffer(pLogFont->lfFaceName, sizeof(pLogFont->lfFaceName), &OutputNameW);

        if (CharSetMap[FONTSUBST_FROM] == DEFAULT_CHARSET ||
            CharSetMap[FONTSUBST_FROM] == pLogFont->lfCharSet)
        {
            pLogFont->lfCharSet = CharSetMap[FONTSUBST_TO];
        }
    }

    return TRUE;    /* success */
}

/*
 * IntLoadSystemFonts
 *
 * Search the system font directory and adds each font found.
 */
VOID FASTCALL IntLoadSystemFonts(VOID)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Directory, FileName, TempString;
    IO_STATUS_BLOCK Iosb;
    HANDLE hDirectory;
    BYTE *DirInfoBuffer;
    PFILE_DIRECTORY_INFORMATION DirInfo;
    BOOLEAN bRestartScan = TRUE;
    NTSTATUS Status;
    INT i;
    static UNICODE_STRING SearchPatterns[] =
    {
        RTL_CONSTANT_STRING(L"*.ttf"),
        RTL_CONSTANT_STRING(L"*.ttc"),
        RTL_CONSTANT_STRING(L"*.otf"),
        RTL_CONSTANT_STRING(L"*.otc"),
        RTL_CONSTANT_STRING(L"*.fon"),
        RTL_CONSTANT_STRING(L"*.fnt")
    };

    RtlInitUnicodeString(&Directory, L"\\SystemRoot\\Fonts\\");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &Directory,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    Status = ZwOpenFile(
                 &hDirectory,
                 SYNCHRONIZE | FILE_LIST_DIRECTORY,
                 &ObjectAttributes,
                 &Iosb,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE);

    if (NT_SUCCESS(Status))
    {
        for (i = 0; i < _countof(SearchPatterns); ++i)
        {
            DirInfoBuffer = ExAllocatePoolWithTag(PagedPool, 0x4000, TAG_FONT);
            if (DirInfoBuffer == NULL)
            {
                ZwClose(hDirectory);
                return;
            }

            FileName.Buffer = ExAllocatePoolWithTag(PagedPool, MAX_PATH * sizeof(WCHAR), TAG_FONT);
            if (FileName.Buffer == NULL)
            {
                ExFreePoolWithTag(DirInfoBuffer, TAG_FONT);
                ZwClose(hDirectory);
                return;
            }
            FileName.Length = 0;
            FileName.MaximumLength = MAX_PATH * sizeof(WCHAR);

            while (1)
            {
                Status = ZwQueryDirectoryFile(
                             hDirectory,
                             NULL,
                             NULL,
                             NULL,
                             &Iosb,
                             DirInfoBuffer,
                             0x4000,
                             FileDirectoryInformation,
                             FALSE,
                             &SearchPatterns[i],
                             bRestartScan);

                if (!NT_SUCCESS(Status) || Status == STATUS_NO_MORE_FILES)
                {
                    break;
                }

                DirInfo = (PFILE_DIRECTORY_INFORMATION)DirInfoBuffer;
                while (1)
                {
                    TempString.Buffer = DirInfo->FileName;
                    TempString.Length =
                        TempString.MaximumLength = DirInfo->FileNameLength;
                    RtlCopyUnicodeString(&FileName, &Directory);
                    RtlAppendUnicodeStringToString(&FileName, &TempString);
                    IntGdiAddFontResourceEx(&FileName, 0, AFRX_WRITE_REGISTRY);
                    if (DirInfo->NextEntryOffset == 0)
                        break;
                    DirInfo = (PFILE_DIRECTORY_INFORMATION)((ULONG_PTR)DirInfo + DirInfo->NextEntryOffset);
                }

                bRestartScan = FALSE;
            }

            ExFreePoolWithTag(FileName.Buffer, TAG_FONT);
            ExFreePoolWithTag(DirInfoBuffer, TAG_FONT);
        }
        ZwClose(hDirectory);
    }
}

static FT_Error IntRequestFontSize(PDC dc, PFONTGDI FontGDI, LONG lfWidth, LONG lfHeight);

/* NOTE: If nIndex < 0 then return the number of charsets. */
UINT FASTCALL IntGetCharSet(INT nIndex, FT_ULong CodePageRange1)
{
    UINT BitIndex, CharSet;
    UINT nCount = 0;

    if (CodePageRange1 == 0)
    {
        return (nIndex < 0) ? 1 : DEFAULT_CHARSET;
    }

    for (BitIndex = 0; BitIndex < MAXTCIINDEX; ++BitIndex)
    {
        if (CodePageRange1 & (1 << BitIndex))
        {
            CharSet = g_FontTci[BitIndex].ciCharset;
            if ((nIndex >= 0) && (nCount == (UINT)nIndex))
            {
                return CharSet;
            }
            ++nCount;
        }
    }

    return (nIndex < 0) ? nCount : ANSI_CHARSET;
}

/* pixels to points */
#define PX2PT(pixels) FT_MulDiv((pixels), 72, 96)

static INT FASTCALL IntGdiLoadFontsFromMemory(PGDI_LOAD_FONT pLoadFont)
{
    FT_Error            Error;
    PFONT_ENTRY         Entry;
    PFONT_ENTRY_MEM     PrivateEntry;
    PFONTGDI            FontGDI;
    FT_Face             Face;
    NTSTATUS            Status;
    ANSI_STRING         AnsiString;
    FT_WinFNT_HeaderRec WinFNT;
    PUNICODE_STRING     pFileName       = pLoadFont->pFileName;
    DWORD               Characteristics = pLoadFont->Characteristics;
    PUNICODE_STRING     pValueName = &pLoadFont->RegValueName;
    TT_OS2 *            pOS2;
    FT_ULong            os2_ulCodePageRange1;
    PSHARED_FACE        SharedFace;
    INT                 iCharSet, CharSetCount;
    FT_Long             iFace, FaceCount;
    LIST_ENTRY          LoadedFontList;
    USHORT              NameLength;
    SIZE_T              Length;
    PWCHAR              pszBuffer;
    UNICODE_STRING      NewString;
    WCHAR               szSize[32];

    /* Retrieve the number of faces */
    IntLockFreeType();
    Error = FT_New_Memory_Face(g_FreeTypeLibrary,
                               pLoadFont->Memory->Buffer,
                               pLoadFont->Memory->BufferSize,
                               -1,
                               &Face);
    if (!Error)
    {
        FaceCount = Face->num_faces;
        FT_Done_Face(Face);
    }
    IntUnLockFreeType();

    if (Error)
    {
        UNICODE_STRING MemoryFont = RTL_CONSTANT_STRING(L"MemoryFont");
        PUNICODE_STRING PrintFile = pFileName ? pFileName : &MemoryFont;
        if (Error == FT_Err_Unknown_File_Format)
            DPRINT1("Unknown font file format (%wZ)\n", PrintFile);
        else
            DPRINT1("Error reading font (FT_Error: %d, %wZ)\n", Error, PrintFile);
        return 0;   /* failure */
    }

    /*
     * Initialize the temporary font list that needs to be appended to the
     * global or per-process font table, in case font enumeration successes.
     * If an error happens while loading and enumerating the fonts, this list
     * is used to cleanup the allocated resources.
     */
    InitializeListHead(&LoadedFontList);

    /*
     * Enumerate each typeface in the font.
     */
    for (iFace = 0; iFace < FaceCount; ++iFace)
    {
        Face = NULL;
        SharedFace = NULL;

        IntLockFreeType();
        Error = FT_New_Memory_Face(g_FreeTypeLibrary,
                                   pLoadFont->Memory->Buffer,
                                   pLoadFont->Memory->BufferSize,
                                   iFace,
                                   &Face);
        if (!Error)
        {
            SharedFace = SharedFace_Create(Face, pLoadFont->Memory);
        }
        IntUnLockFreeType();

        if (Error || !SharedFace)
        {
            DPRINT1("Error reading font (FT_Error: %d)\n", Error);
            goto Finish; /* failure */
        }

        /* os2_ulCodePageRange1 and CharSetCount and IsTrueType */
        os2_ulCodePageRange1 = 0;
        if (FT_IS_SFNT(Face))
        {
            IntLockFreeType();
            pOS2 = (TT_OS2 *)FT_Get_Sfnt_Table(Face, FT_SFNT_OS2);
            if (pOS2)
            {
                os2_ulCodePageRange1 = pOS2->ulCodePageRange1;
            }
            IntUnLockFreeType();

            CharSetCount = IntGetCharSet(-1, os2_ulCodePageRange1);
            pLoadFont->IsTrueType = TRUE;
        }
        else
        {
            CharSetCount = 1;
            pLoadFont->IsTrueType = FALSE;
        }

        /*
         * Enumerate all supported character sets for the selected typeface.
         */
        for (iCharSet = 0; iCharSet < CharSetCount; ++iCharSet)
        {
            /*
             * Add a reference to SharedFace only when iCharSet is > 0,
             * since the first reference has been already done by the
             * SharedFace_Create() call above.
             */
            if (iCharSet > 0)
            {
                IntLockFreeType();
                SharedFace_AddRef(SharedFace);
                IntUnLockFreeType();
            }

            /* Allocate a FONT_ENTRY */
            Entry = ExAllocatePoolWithTag(PagedPool, sizeof(FONT_ENTRY), TAG_FONT);
            if (!Entry)
            {
                DPRINT1("Failed to allocate FONT_ENTRY\n");
                SharedFace_Release(SharedFace);
                EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Finish; /* failure */
            }

            /* Allocate a FONTGDI */
            FontGDI = EngAllocMem(FL_ZERO_MEMORY, sizeof(FONTGDI), GDITAG_RFONT);
            if (!FontGDI)
            {
                DPRINT1("Failed to allocate FontGDI\n");
                SharedFace_Release(SharedFace);
                ExFreePoolWithTag(Entry, TAG_FONT);
                EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Finish; /* failure */
            }

            /* Set face */
            FontGDI->SharedFace = SharedFace;
            FontGDI->CharSet = ANSI_CHARSET;
            FontGDI->OriginalItalic = FALSE;
            FontGDI->RequestItalic = FALSE;
            FontGDI->OriginalWeight = FW_NORMAL;
            FontGDI->RequestWeight = FW_NORMAL;

            IntLockFreeType();
            if (FT_IS_SFNT(Face))
            {
                pOS2 = (TT_OS2 *)FT_Get_Sfnt_Table(Face, FT_SFNT_OS2);
                if (pOS2)
                {
                    FontGDI->OriginalItalic = !!(pOS2->fsSelection & 0x1);
                    FontGDI->OriginalWeight = pOS2->usWeightClass;
                }
            }
            else
            {
                Error = FT_Get_WinFNT_Header(Face, &WinFNT);
                if (!Error)
                {
                    FontGDI->OriginalItalic = !!WinFNT.italic;
                    FontGDI->OriginalWeight = WinFNT.weight;
                }
            }
            IntUnLockFreeType();

            /* Entry->FaceName */
            RtlInitAnsiString(&AnsiString, Face->family_name);
            Status = RtlAnsiStringToUnicodeString(&Entry->FaceName, &AnsiString, TRUE);
            if (!NT_SUCCESS(Status))
            {
                DPRINT1("Failed to allocate Entry->FaceName\n");
                CleanupFontEntryEx(Entry, FontGDI);
                EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Finish; /* failure */
            }

            /* Entry->StyleName */
            RtlInitUnicodeString(&Entry->StyleName, NULL);
            if (Face->style_name && Face->style_name[0] &&
                strcmp(Face->style_name, "Regular") != 0)
            {
                RtlInitAnsiString(&AnsiString, Face->style_name);
                Status = RtlAnsiStringToUnicodeString(&Entry->StyleName, &AnsiString, TRUE);
                if (!NT_SUCCESS(Status))
                {
                    DPRINT1("Failed to allocate Entry->StyleName\n");
                    CleanupFontEntryEx(Entry, FontGDI);
                    EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto Finish; /* failure */
                }
            }

            /* FontGDI->CharSet */
            if (FT_IS_SFNT(Face))
            {
                FontGDI->CharSet = IntGetCharSet(iCharSet, os2_ulCodePageRange1);
            }
            else
            {
                IntLockFreeType();
                Error = FT_Get_WinFNT_Header(Face, &WinFNT);
                if (!Error)
                {
                    FontGDI->CharSet = WinFNT.charset;
                    pLoadFont->CharSet = WinFNT.charset;
                }
                IntUnLockFreeType();
            }

            /* Set the file name */
            if (pFileName)
            {
                // TODO: Since this Filename is common to all the faces+charsets
                // inside the given font, it may be worth to somehow cache it
                // only once and share it amongst all these faces+charsets.

                Length = pFileName->Length + sizeof(UNICODE_NULL);
                FontGDI->Filename = ExAllocatePoolWithTag(PagedPool, Length, GDITAG_PFF);
                if (FontGDI->Filename == NULL)
                {
                    DPRINT1("Failed to allocate FontGDI->Filename\n");
                    CleanupFontEntryEx(Entry, FontGDI);
                    EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto Finish; /* failure */
                }
                IntUnicodeStringToBuffer(FontGDI->Filename, Length, pFileName);
            }
            else
            {
                /* This is a memory font, initialize a suitable entry */

                FontGDI->Filename = NULL;

                PrivateEntry = ExAllocatePoolWithTag(PagedPool, sizeof(FONT_ENTRY_MEM), TAG_FONT);
                if (!PrivateEntry)
                {
                    DPRINT1("Failed to allocate PrivateEntry\n");
                    CleanupFontEntryEx(Entry, FontGDI);
                    EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto Finish; /* failure */
                }

                PrivateEntry->Entry = Entry;
                if (pLoadFont->PrivateEntry)
                {
                    InsertTailList(&pLoadFont->PrivateEntry->ListEntry, &PrivateEntry->ListEntry);
                }
                else
                {
                    InitializeListHead(&PrivateEntry->ListEntry);
                    pLoadFont->PrivateEntry = PrivateEntry;
                }
            }

            /* Add this font resource to the font table */
            Entry->Font = FontGDI;
            Entry->NotEnum = (Characteristics & FR_NOT_ENUM);
            InsertTailList(&LoadedFontList, &Entry->ListEntry);

            DPRINT("Font loaded: %s (%s), CharSet %u, Num glyphs %d\n",
                   Face->family_name, Face->style_name, FontGDI->CharSet, Face->num_glyphs);
        }

        IntLockFreeType();
        /* Error = */ IntRequestFontSize(NULL, FontGDI, 0, 0);
        IntUnLockFreeType();

        /*
         * Initialize and build the registry font value entry,
         * only in the case we load fonts from a file and not from memory.
         */
        if (!pFileName)
            continue;
        NameLength = Entry->FaceName.Length;
        if (pValueName->Length == 0)
        {
            if (FT_IS_SFNT(Face))
            {
                // L"Name StyleName\0"
                Length = NameLength + sizeof(L' ') + Entry->StyleName.Length + sizeof(UNICODE_NULL);
                pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
                if (pszBuffer)
                {
                    RtlInitEmptyUnicodeString(pValueName, pszBuffer, Length);
                    RtlCopyUnicodeString(pValueName, &Entry->FaceName);
                    if (Entry->StyleName.Length > 0)
                    {
                        RtlAppendUnicodeToString(pValueName, L" ");
                        RtlAppendUnicodeStringToString(pValueName, &Entry->StyleName);
                    }
                }
                else
                {
                    break;  /* failure */
                }
            }
            else
            {
                szSize[0] = L' ';
                _itow(PX2PT(FontGDI->EmHeight), szSize+1, 10);

                Length = NameLength + (wcslen(szSize) + 1) * sizeof(WCHAR);
                pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
                if (pszBuffer)
                {
                    RtlInitEmptyUnicodeString(pValueName, pszBuffer, Length);
                    RtlCopyUnicodeString(pValueName, &Entry->FaceName);
                    RtlAppendUnicodeToString(pValueName, szSize);
                }
                else
                {
                    break;  /* failure */
                }
            }
        }
        else
        {
            if (FT_IS_SFNT(Face))
            {
                // L"... & Name StyleName\0"
                Length = pValueName->Length + 3 * sizeof(WCHAR) + Entry->FaceName.Length +
                         sizeof(L' ') + Entry->StyleName.Length + sizeof(UNICODE_NULL);
                pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
                if (pszBuffer)
                {
                    RtlInitEmptyUnicodeString(&NewString, pszBuffer, Length);
                    RtlCopyUnicodeString(&NewString, pValueName);
                    RtlAppendUnicodeToString(&NewString, L" & ");
                    RtlAppendUnicodeStringToString(&NewString, &Entry->FaceName);
                    if (Entry->StyleName.Length > 0)
                    {
                        RtlAppendUnicodeToString(&NewString, L" ");
                        RtlAppendUnicodeStringToString(&NewString, &Entry->StyleName);
                    }
                }
                else
                {
                    RtlFreeUnicodeString(pValueName);
                    break;  /* failure */
                }
            }
            else
            {
                szSize[0] = L',';
                _itow(PX2PT(FontGDI->EmHeight), szSize+1, 10);

                Length = pValueName->Length + (wcslen(szSize) + 1) * sizeof(WCHAR);
                pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
                if (pszBuffer)
                {
                    RtlInitEmptyUnicodeString(&NewString, pszBuffer, Length);
                    RtlCopyUnicodeString(&NewString, pValueName);
                    RtlAppendUnicodeToString(&NewString, szSize);
                }
                else
                {
                    RtlFreeUnicodeString(pValueName);
                    break;  /* failure */
                }
            }

            RtlFreeUnicodeString(pValueName);
            *pValueName = NewString;
        }
    }

Finish:
    if (iFace == FaceCount)
    {
        /*
         * We succeeded, append the created font entries into the correct font table.
         */
        PLIST_ENTRY ListToAppend;

        /* No typefaces were present */
        if (FaceCount == 0)
        {
            ASSERT(IsListEmpty(&LoadedFontList));
            return 0;
        }

        ASSERT(!IsListEmpty(&LoadedFontList));

        /*
         * Remove the temporary font list' head and reinitialize it.
         * This effectively empties the list and at the same time transforms
         * 'ListToAppend' into a headless list, ready to be appended to the
         * suitable font table.
         */
        ListToAppend = LoadedFontList.Flink;
        RemoveEntryList(&LoadedFontList);
        InitializeListHead(&LoadedFontList);

        if (Characteristics & FR_PRIVATE)
        {
            /* Private font */
            PPROCESSINFO Win32Process = PsGetCurrentProcessWin32Process();
            IntLockProcessPrivateFonts(Win32Process);
            AppendTailList(&Win32Process->PrivateFontListHead, ListToAppend);
            IntUnLockProcessPrivateFonts(Win32Process);
        }
        else
        {
            /* Global font */
            IntLockGlobalFonts();
            AppendTailList(&g_FontListHead, ListToAppend);
            IntUnLockGlobalFonts();
        }

        return FaceCount;   /* Number of loaded faces */
    }
    else
    {
        /* We failed, cleanup the resources */
        PLIST_ENTRY ListEntry;

        if (pLoadFont->PrivateEntry)
        {
            while (!IsListEmpty(&pLoadFont->PrivateEntry->ListEntry))
            {
                ListEntry = RemoveHeadList(&pLoadFont->PrivateEntry->ListEntry);
                PrivateEntry = CONTAINING_RECORD(ListEntry, FONT_ENTRY_MEM, ListEntry);
                ExFreePoolWithTag(PrivateEntry, TAG_FONT);
            }
            ExFreePoolWithTag(pLoadFont->PrivateEntry, TAG_FONT);
            pLoadFont->PrivateEntry = NULL;
        }

        while (!IsListEmpty(&LoadedFontList))
        {
            ListEntry = RemoveHeadList(&LoadedFontList);
            Entry = CONTAINING_RECORD(ListEntry, FONT_ENTRY, ListEntry);
            CleanupFontEntry(Entry);
        }

        return 0;   /* No faces have been added */
    }
}

static LPCWSTR FASTCALL NameFromCharSet(BYTE CharSet)
{
    switch (CharSet)
    {
        case ANSI_CHARSET: return L"ANSI";
        case DEFAULT_CHARSET: return L"Default";
        case SYMBOL_CHARSET: return L"Symbol";
        case SHIFTJIS_CHARSET: return L"Shift_JIS";
        case HANGUL_CHARSET: return L"Hangul";
        case GB2312_CHARSET: return L"GB 2312";
        case CHINESEBIG5_CHARSET: return L"Chinese Big5";
        case OEM_CHARSET: return L"OEM";
        case JOHAB_CHARSET: return L"Johab";
        case HEBREW_CHARSET: return L"Hebrew";
        case ARABIC_CHARSET: return L"Arabic";
        case GREEK_CHARSET: return L"Greek";
        case TURKISH_CHARSET: return L"Turkish";
        case VIETNAMESE_CHARSET: return L"Vietnamese";
        case THAI_CHARSET: return L"Thai";
        case EASTEUROPE_CHARSET: return L"Eastern European";
        case RUSSIAN_CHARSET: return L"Russian";
        case MAC_CHARSET: return L"Mac";
        case BALTIC_CHARSET: return L"Baltic";
        default: return L"Unknown";
    }
}

/*
 * IntGdiAddFontResource
 *
 * Adds the font resource from the specified file to the system.
 */

INT FASTCALL IntGdiAddFontResourceEx(PUNICODE_STRING FileName, DWORD Characteristics, DWORD dwFlags)
{
    NTSTATUS Status;
    HANDLE FileHandle;
    PVOID Buffer = NULL;
    IO_STATUS_BLOCK Iosb;
    PVOID SectionObject;
    SIZE_T ViewSize = 0, Length;
    LARGE_INTEGER SectionSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    GDI_LOAD_FONT LoadFont;
    INT FontCount;
    HANDLE KeyHandle;
    UNICODE_STRING PathName;
    LPWSTR pszBuffer;
    PFILE_OBJECT FileObject;
    static const UNICODE_STRING TrueTypePostfix = RTL_CONSTANT_STRING(L" (TrueType)");
    static const UNICODE_STRING DosPathPrefix = RTL_CONSTANT_STRING(L"\\??\\");

    /* Build PathName */
    if (dwFlags & AFRX_DOS_DEVICE_PATH)
    {
        Length = DosPathPrefix.Length + FileName->Length + sizeof(UNICODE_NULL);
        pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
        if (!pszBuffer)
            return 0;   /* failure */

        RtlInitEmptyUnicodeString(&PathName, pszBuffer, Length);
        RtlAppendUnicodeStringToString(&PathName, &DosPathPrefix);
        RtlAppendUnicodeStringToString(&PathName, FileName);
    }
    else
    {
        Status = DuplicateUnicodeString(FileName, &PathName);
        if (!NT_SUCCESS(Status))
            return 0;   /* failure */
    }

    /* Open the font file */
    InitializeObjectAttributes(&ObjectAttributes, &PathName,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    Status = ZwOpenFile(
                 &FileHandle,
                 FILE_GENERIC_READ | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,
                 FILE_SHARE_READ,
                 FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Could not load font file: %wZ\n", &PathName);
        RtlFreeUnicodeString(&PathName);
        return 0;
    }

    Status = ObReferenceObjectByHandle(FileHandle, FILE_READ_DATA, NULL,
                                       KernelMode, (PVOID*)&FileObject, NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("ObReferenceObjectByHandle failed.\n");
        ZwClose(FileHandle);
        RtlFreeUnicodeString(&PathName);
        return 0;
    }

    SectionSize.QuadPart = 0LL;
    Status = MmCreateSection(&SectionObject,
                             STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ,
                             NULL, &SectionSize, PAGE_READONLY,
                             SEC_COMMIT, FileHandle, FileObject);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Could not map file: %wZ\n", &PathName);
        ZwClose(FileHandle);
        ObDereferenceObject(FileObject);
        RtlFreeUnicodeString(&PathName);
        return 0;
    }
    ZwClose(FileHandle);

    Status = MmMapViewInSystemSpace(SectionObject, &Buffer, &ViewSize);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Could not map file: %wZ\n", &PathName);
        ObDereferenceObject(SectionObject);
        ObDereferenceObject(FileObject);
        RtlFreeUnicodeString(&PathName);
        return 0;
    }

    LoadFont.pFileName          = &PathName;
    LoadFont.Memory             = SharedMem_Create(Buffer, ViewSize, TRUE);
    LoadFont.Characteristics    = Characteristics;
    RtlInitUnicodeString(&LoadFont.RegValueName, NULL);
    LoadFont.IsTrueType         = FALSE;
    LoadFont.CharSet            = DEFAULT_CHARSET;
    LoadFont.PrivateEntry       = NULL;
    FontCount = IntGdiLoadFontsFromMemory(&LoadFont);

    /* Release our copy */
    IntLockFreeType();
    SharedMem_Release(LoadFont.Memory);
    IntUnLockFreeType();

    ObDereferenceObject(SectionObject);

    ObDereferenceObject(FileObject);

    /* Save the loaded font name into the registry */
    if (FontCount > 0 && (dwFlags & AFRX_WRITE_REGISTRY))
    {
        UNICODE_STRING NewString;
        SIZE_T Length;
        PWCHAR pszBuffer;
        LPCWSTR CharSetName;
        if (LoadFont.IsTrueType)
        {
            /* Append " (TrueType)" */
            Length = LoadFont.RegValueName.Length + TrueTypePostfix.Length + sizeof(UNICODE_NULL);
            pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
            if (pszBuffer)
            {
                RtlInitEmptyUnicodeString(&NewString, pszBuffer, Length);
                NewString.Buffer[0] = UNICODE_NULL;
                RtlAppendUnicodeStringToString(&NewString, &LoadFont.RegValueName);
                RtlAppendUnicodeStringToString(&NewString, &TrueTypePostfix);
                RtlFreeUnicodeString(&LoadFont.RegValueName);
                LoadFont.RegValueName = NewString;
            }
            else
            {
                // FIXME!
            }
        }
        else if (LoadFont.CharSet != DEFAULT_CHARSET)
        {
            /* Append " (CharSetName)" */
            CharSetName = NameFromCharSet(LoadFont.CharSet);
            Length = LoadFont.RegValueName.Length +
                     (wcslen(CharSetName) + 3) * sizeof(WCHAR) +
                     sizeof(UNICODE_NULL);

            pszBuffer = ExAllocatePoolWithTag(PagedPool, Length, TAG_USTR);
            if (pszBuffer)
            {
                RtlInitEmptyUnicodeString(&NewString, pszBuffer, Length);
                NewString.Buffer[0] = UNICODE_NULL;
                RtlAppendUnicodeStringToString(&NewString, &LoadFont.RegValueName);
                RtlAppendUnicodeToString(&NewString, L" (");
                RtlAppendUnicodeToString(&NewString, CharSetName);
                RtlAppendUnicodeToString(&NewString, L")");
                RtlFreeUnicodeString(&LoadFont.RegValueName);
                LoadFont.RegValueName = NewString;
            }
            else
            {
                // FIXME!
            }
        }

        InitializeObjectAttributes(&ObjectAttributes, &g_FontRegPath,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL, NULL);
        Status = ZwOpenKey(&KeyHandle, KEY_WRITE, &ObjectAttributes);
        if (NT_SUCCESS(Status))
        {
            SIZE_T DataSize;
            LPWSTR pFileName;

            if (dwFlags & AFRX_ALTERNATIVE_PATH)
            {
                pFileName = PathName.Buffer;
            }
            else
            {
                pFileName = wcsrchr(PathName.Buffer, L'\\');
            }

            if (pFileName)
            {
                if (!(dwFlags & AFRX_ALTERNATIVE_PATH))
                {
                    pFileName++;
                }
                DataSize = (wcslen(pFileName) + 1) * sizeof(WCHAR);
                ZwSetValueKey(KeyHandle, &LoadFont.RegValueName, 0, REG_SZ,
                              pFileName, DataSize);
            }
            ZwClose(KeyHandle);
        }
    }
    RtlFreeUnicodeString(&LoadFont.RegValueName);

    RtlFreeUnicodeString(&PathName);
    return FontCount;
}

INT FASTCALL IntGdiAddFontResource(PUNICODE_STRING FileName, DWORD Characteristics)
{
    return IntGdiAddFontResourceEx(FileName, Characteristics, 0);
}

/* Borrowed from shlwapi!PathIsRelativeW */
BOOL WINAPI PathIsRelativeW(LPCWSTR lpszPath)
{
    if (!lpszPath || !*lpszPath)
        return TRUE;
    if (*lpszPath == L'\\' || (*lpszPath && lpszPath[1] == L':'))
        return FALSE;
    return TRUE;
}

BOOL FASTCALL IntLoadFontsInRegistry(VOID)
{
    NTSTATUS                        Status;
    HANDLE                          KeyHandle;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    KEY_FULL_INFORMATION            KeyFullInfo;
    ULONG                           i, Length;
    UNICODE_STRING                  FontTitleW, FileNameW;
    SIZE_T                          InfoSize;
    LPBYTE                          InfoBuffer;
    PKEY_VALUE_FULL_INFORMATION     pInfo;
    LPWSTR                          pchPath;
    BOOLEAN                         Success;
    WCHAR                           szPath[MAX_PATH];
    INT                             nFontCount = 0;
    DWORD                           dwFlags;

    /* open registry key */
    InitializeObjectAttributes(&ObjectAttributes, &g_FontRegPath,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL, NULL);
    Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("ZwOpenKey failed: 0x%08X\n", Status);
        return FALSE;   /* failure */
    }

    /* query count of values */
    Status = ZwQueryKey(KeyHandle, KeyFullInformation,
                        &KeyFullInfo, sizeof(KeyFullInfo), &Length);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("ZwQueryKey failed: 0x%08X\n", Status);
        ZwClose(KeyHandle);
        return FALSE;   /* failure */
    }

    /* allocate buffer */
    InfoSize = (MAX_PATH + 256) * sizeof(WCHAR);
    InfoBuffer = ExAllocatePoolWithTag(PagedPool, InfoSize, TAG_FONT);
    if (!InfoBuffer)
    {
        DPRINT1("ExAllocatePoolWithTag failed\n");
        ZwClose(KeyHandle);
        return FALSE;
    }

    /* for each value */
    for (i = 0; i < KeyFullInfo.Values; ++i)
    {
        /* get value name */
        Status = ZwEnumerateValueKey(KeyHandle, i, KeyValueFullInformation,
                                     InfoBuffer, InfoSize, &Length);
        if (Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL)
        {
            /* too short buffer */
            ExFreePoolWithTag(InfoBuffer, TAG_FONT);
            InfoSize *= 2;
            InfoBuffer = ExAllocatePoolWithTag(PagedPool, InfoSize, TAG_FONT);
            if (!InfoBuffer)
            {
                DPRINT1("ExAllocatePoolWithTag failed\n");
                break;
            }
            /* try again */
            Status = ZwEnumerateValueKey(KeyHandle, i, KeyValueFullInformation,
                                         InfoBuffer, InfoSize, &Length);
        }
        if (!NT_SUCCESS(Status))
        {
            DPRINT1("ZwEnumerateValueKey failed: 0x%08X\n", Status);
            break;      /* failure */
        }

        /* create FontTitleW string */
        pInfo = (PKEY_VALUE_FULL_INFORMATION)InfoBuffer;
        Length = pInfo->NameLength / sizeof(WCHAR);
        pInfo->Name[Length] = UNICODE_NULL;   /* truncate */
        Success = RtlCreateUnicodeString(&FontTitleW, pInfo->Name);
        if (!Success)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DPRINT1("RtlCreateUnicodeString failed\n");
            break;      /* failure */
        }

        /* query value */
        Status = ZwQueryValueKey(KeyHandle, &FontTitleW, KeyValueFullInformation, 
                                 InfoBuffer, InfoSize, &Length);
        if (Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL)
        {
            /* too short buffer */
            ExFreePoolWithTag(InfoBuffer, TAG_FONT);
            InfoSize *= 2;
            InfoBuffer = ExAllocatePoolWithTag(PagedPool, InfoSize, TAG_FONT);
            if (!InfoBuffer)
            {
                DPRINT1("ExAllocatePoolWithTag failed\n");
                break;
            }
            /* try again */
            Status = ZwQueryValueKey(KeyHandle, &FontTitleW, KeyValueFullInformation, 
                                     InfoBuffer, InfoSize, &Length);
        }
        pInfo = (PKEY_VALUE_FULL_INFORMATION)InfoBuffer;
        if (!NT_SUCCESS(Status) || !pInfo->DataLength)
        {
            DPRINT1("ZwQueryValueKey failed: 0x%08X\n", Status);
            RtlFreeUnicodeString(&FontTitleW);
            break;      /* failure */
        }

        /* Build pchPath */
        pchPath = (LPWSTR)((PUCHAR)pInfo + pInfo->DataOffset);
        Length = pInfo->DataLength / sizeof(WCHAR);
        pchPath[Length] = UNICODE_NULL; /* truncate */

        /* Load font(s) without writing registry */
        if (PathIsRelativeW(pchPath))
        {
            dwFlags = 0;
            Status = RtlStringCbPrintfW(szPath, sizeof(szPath),
                                        L"\\SystemRoot\\Fonts\\%s", pchPath);
        }
        else
        {
            dwFlags = AFRX_ALTERNATIVE_PATH | AFRX_DOS_DEVICE_PATH;
            Status = RtlStringCbCopyW(szPath, sizeof(szPath), pchPath);
        }

        if (NT_SUCCESS(Status))
        {
            RtlCreateUnicodeString(&FileNameW, szPath);
            nFontCount += IntGdiAddFontResourceEx(&FileNameW, 0, dwFlags);
            RtlFreeUnicodeString(&FileNameW);
        }

        RtlFreeUnicodeString(&FontTitleW);
    }

    /* close now */
    ZwClose(KeyHandle);

    /* free memory block */
    if (InfoBuffer)
    {
        ExFreePoolWithTag(InfoBuffer, TAG_FONT);
    }

    return (KeyFullInfo.Values != 0 && nFontCount != 0);
}

HANDLE FASTCALL IntGdiAddFontMemResource(PVOID Buffer, DWORD dwSize, PDWORD pNumAdded)
{
    HANDLE Ret = NULL;
    GDI_LOAD_FONT LoadFont;
    PFONT_ENTRY_COLL_MEM EntryCollection;
    INT FaceCount;

    PVOID BufferCopy = ExAllocatePoolWithTag(PagedPool, dwSize, TAG_FONT);
    if (!BufferCopy)
    {
        *pNumAdded = 0;
        return NULL;
    }
    RtlCopyMemory(BufferCopy, Buffer, dwSize);

    LoadFont.pFileName          = NULL;
    LoadFont.Memory             = SharedMem_Create(BufferCopy, dwSize, FALSE);
    LoadFont.Characteristics    = FR_PRIVATE | FR_NOT_ENUM;
    RtlInitUnicodeString(&LoadFont.RegValueName, NULL);
    LoadFont.IsTrueType         = FALSE;
    LoadFont.PrivateEntry       = NULL;
    FaceCount = IntGdiLoadFontsFromMemory(&LoadFont);

    RtlFreeUnicodeString(&LoadFont.RegValueName);

    /* Release our copy */
    IntLockFreeType();
    SharedMem_Release(LoadFont.Memory);
    IntUnLockFreeType();

    if (FaceCount > 0)
    {
        EntryCollection = ExAllocatePoolWithTag(PagedPool, sizeof(FONT_ENTRY_COLL_MEM), TAG_FONT);
        if (EntryCollection)
        {
            PPROCESSINFO Win32Process = PsGetCurrentProcessWin32Process();
            EntryCollection->Entry = LoadFont.PrivateEntry;
            IntLockProcessPrivateFonts(Win32Process);
            EntryCollection->Handle = ULongToHandle(++Win32Process->PrivateMemFontHandleCount);
            InsertTailList(&Win32Process->PrivateMemFontListHead, &EntryCollection->ListEntry);
            IntUnLockProcessPrivateFonts(Win32Process);
            Ret = EntryCollection->Handle;
        }
    }
    *pNumAdded = FaceCount;

    return Ret;
}

// FIXME: Add RemoveFontResource

VOID FASTCALL IntGdiCleanupMemEntry(PFONT_ENTRY_MEM Head)
{
    PLIST_ENTRY Entry;
    PFONT_ENTRY_MEM FontEntry;

    while (!IsListEmpty(&Head->ListEntry))
    {
        Entry = RemoveHeadList(&Head->ListEntry);
        FontEntry = CONTAINING_RECORD(Entry, FONT_ENTRY_MEM, ListEntry);

        CleanupFontEntry(FontEntry->Entry);
        ExFreePoolWithTag(FontEntry, TAG_FONT);
    }

    CleanupFontEntry(Head->Entry);
    ExFreePoolWithTag(Head, TAG_FONT);
}

static VOID FASTCALL UnlinkFontMemCollection(PFONT_ENTRY_COLL_MEM Collection)
{
    PFONT_ENTRY_MEM FontMemEntry = Collection->Entry;
    PLIST_ENTRY ListEntry;
    RemoveEntryList(&Collection->ListEntry);

    do {
        /* Also unlink the FONT_ENTRY stuff from the PrivateFontListHead */
        RemoveEntryList(&FontMemEntry->Entry->ListEntry);

        ListEntry = FontMemEntry->ListEntry.Flink;
        FontMemEntry = CONTAINING_RECORD(ListEntry, FONT_ENTRY_MEM, ListEntry);

    } while (FontMemEntry != Collection->Entry);
}

BOOL FASTCALL IntGdiRemoveFontMemResource(HANDLE hMMFont)
{
    PLIST_ENTRY Entry;
    PFONT_ENTRY_COLL_MEM CurrentEntry;
    PFONT_ENTRY_COLL_MEM EntryCollection = NULL;
    PPROCESSINFO Win32Process = PsGetCurrentProcessWin32Process();

    IntLockProcessPrivateFonts(Win32Process);
    for (Entry = Win32Process->PrivateMemFontListHead.Flink;
         Entry != &Win32Process->PrivateMemFontListHead;
         Entry = Entry->Flink)
    {
        CurrentEntry = CONTAINING_RECORD(Entry, FONT_ENTRY_COLL_MEM, ListEntry);

        if (CurrentEntry->Handle == hMMFont)
        {
            EntryCollection = CurrentEntry;
            UnlinkFontMemCollection(CurrentEntry);
            break;
        }
    }
    IntUnLockProcessPrivateFonts(Win32Process);

    if (EntryCollection)
    {
        IntGdiCleanupMemEntry(EntryCollection->Entry);
        ExFreePoolWithTag(EntryCollection, TAG_FONT);
        return TRUE;
    }
    return FALSE;
}


VOID FASTCALL IntGdiCleanupPrivateFontsForProcess(VOID)
{
    PPROCESSINFO Win32Process = PsGetCurrentProcessWin32Process();
    PLIST_ENTRY Entry;
    PFONT_ENTRY_COLL_MEM EntryCollection;

    DPRINT("IntGdiCleanupPrivateFontsForProcess()\n");
    do {
        Entry = NULL;
        EntryCollection = NULL;

        IntLockProcessPrivateFonts(Win32Process);
        if (!IsListEmpty(&Win32Process->PrivateMemFontListHead))
        {
            Entry = Win32Process->PrivateMemFontListHead.Flink;
            EntryCollection = CONTAINING_RECORD(Entry, FONT_ENTRY_COLL_MEM, ListEntry);
            UnlinkFontMemCollection(EntryCollection);
        }
        IntUnLockProcessPrivateFonts(Win32Process);

        if (EntryCollection)
        {
            IntGdiCleanupMemEntry(EntryCollection->Entry);
            ExFreePoolWithTag(EntryCollection, TAG_FONT);
        }
        else
        {
            /* No Mem fonts anymore, see if we have any other private fonts left */
            Entry = NULL;
            IntLockProcessPrivateFonts(Win32Process);
            if (!IsListEmpty(&Win32Process->PrivateFontListHead))
            {
                Entry = RemoveHeadList(&Win32Process->PrivateFontListHead);
            }
            IntUnLockProcessPrivateFonts(Win32Process);

            if (Entry)
            {
                CleanupFontEntry(CONTAINING_RECORD(Entry, FONT_ENTRY, ListEntry));
            }
        }

    } while (Entry);
}