/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
SolvCreatePackageList(
    PSolvPackageList* ppSolvPackageList
    )
{
    uint32_t dwError = 0;
    PSolvPackageList pPkgList = NULL;

    if(!ppSolvPackageList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1,
                                 sizeof(SolvPackageList),
                                 (void **)&pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    queue_init(&pPkgList->queuePackages);

    *ppSolvPackageList = pPkgList;

cleanup:
    return dwError;

error:
    if(ppSolvPackageList)
    {
        *ppSolvPackageList = NULL;
    }
    SolvFreePackageList(pPkgList);
    goto cleanup;
}

void
SolvEmptyPackageList(
    PSolvPackageList  pPkgList
    )
{
    if(pPkgList)
    {
        queue_free(&pPkgList->queuePackages);
    }
}

void
SolvFreePackageList(
    PSolvPackageList pPkgList
    )
{
    if(pPkgList)
    {
        queue_free(&pPkgList->queuePackages);
        TDNF_SAFE_FREE_MEMORY(pPkgList);
    }
}

uint32_t
SolvQueueToPackageList(
    Queue* pQueue,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || !pQueue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_insertn(&pPkgList->queuePackages,
                  pPkgList->queuePackages.count,
                  pQueue->count,
                  pQueue->elements);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvGetListResult(
    PSolvQuery pQuery,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pQuery->queueResult.count == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_insertn(&pPkgList->queuePackages,
                  pPkgList->queuePackages.count,
                  pQuery->queueResult.count,
                  pQuery->queueResult.elements);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvGetSearchResult(
    PSolvQuery pQuery,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    queue_insertn(&pPkgList->queuePackages,
                  pPkgList->queuePackages.count,
                  pQuery->queueResult.count,
                  pQuery->queueResult.elements);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvGetPackageListSize(
    PSolvPackageList pPkgList,
    uint32_t* pdwSize
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    if(!pPkgList || !pdwSize)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwCount = pPkgList->queuePackages.count;
    *pdwSize = dwCount;

cleanup:
    return dwError;

error:
    if(pdwSize)
    {
        *pdwSize = 0;
    }
    goto cleanup;
}

uint32_t
SolvGetPkgInfoFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t dwWhichInfo,
    char** ppszInfo)
{
    uint32_t dwError = 0;
    const char* pszTemp = NULL;
    char* pszInfo = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !pSack->pPool || !ppszInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszTemp = solvable_lookup_str(pSolv, dwWhichInfo);
    if(!pszTemp)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszTemp, &pszInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszInfo = pszInfo;
cleanup:

    return dwError;
error:
    if(ppszInfo)
    {
        *ppszInfo = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszInfo);
    goto cleanup;

}

uint32_t
SolvGetPkgNameFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszName)
{
    uint32_t dwError = 0;
    const char* pszTemp = NULL;
    char* pszName = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !ppszName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszTemp = pool_id2str(pSack->pPool, pSolv->name);
    if(!pszTemp)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszTemp, &pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszName = pszName;
cleanup:
    return dwError;

error:
    if(ppszName)
    {
        *ppszName = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszName);
    goto cleanup;
}

uint32_t
SolvGetPkgArchFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszArch)
{

    uint32_t dwError = 0;
    char* pszArch = NULL;

    if(!pSack || !ppszArch)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgInfoFromId(pSack, dwPkgId, SOLVABLE_ARCH, &pszArch);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszArch = pszArch;
cleanup:

    return dwError;
error:
    if(ppszArch)
    {
        *ppszArch = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszArch);
    goto cleanup;

}

uint32_t
SolvGetPkgVersionFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszVersion)
{
    uint32_t dwError = 0;
    char* pszVersion = NULL;
    char* pszEpoch = NULL;
    char* pszRelease = NULL;
    const char* pszEvr = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !ppszVersion)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszEvr = solvable_lookup_str(pSolv, SOLVABLE_EVR);
    if(!pszEvr)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvSplitEvr(pSack,
                           pszEvr,
                           &pszEpoch,
                           &pszVersion,
                           &pszRelease);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszVersion = pszVersion;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszEpoch);
    TDNF_SAFE_FREE_MEMORY(pszRelease);
    return dwError;

error:
    if(ppszVersion)
    {
        *ppszVersion = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszVersion);
    goto cleanup;
}

uint32_t
SolvGetPkgReleaseFromId(
    PSolvSack pSack,
    uint32_t  dwPkgId,
    char** ppszRelease)
{
    uint32_t dwError = 0;
    char* pszVersion = NULL;
    char* pszEpoch = NULL;
    char* pszRelease = NULL;
    const char* pszEvr = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !ppszRelease)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszEvr = solvable_lookup_str(pSolv, SOLVABLE_EVR);
    if(!pszEvr)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvSplitEvr(pSack,
                           pszEvr,
                           &pszEpoch,
                           &pszVersion,
                           &pszRelease);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszRelease = pszRelease;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszEpoch);
    TDNF_SAFE_FREE_MEMORY(pszVersion);
    return dwError;

error:
    if(ppszRelease)
    {
        *ppszRelease = NULL;
    }
    goto cleanup;

}

uint32_t
SolvGetPkgRepoNameFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszRepoName)
{
    uint32_t dwError = 0;
    char* pszRepoName = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !ppszRepoName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pSolv->repo->name)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pSolv->repo->name, &pszRepoName);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszRepoName = pszRepoName;
cleanup:
    return dwError;

error:
    if(ppszRepoName)
    {
        *ppszRepoName = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszRepoName);
    goto cleanup;

}

uint32_t
SolvGetPkgInstallSizeFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t* pdwSize)
{
    uint32_t dwError = 0;
    uint32_t installSize = 0;
    Solvable *pSolv = NULL;

    if(!pSack || !pdwSize)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    installSize = solvable_lookup_num(pSolv, SOLVABLE_INSTALLSIZE, 0);
    *pdwSize = installSize;

cleanup:
    return dwError;

error:
    if(pdwSize)
    {
        *pdwSize = 0;
    }
    goto cleanup;;
}

uint32_t
SolvGetPkgSummaryFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszSummary)
{

    uint32_t dwError = 0;
    char* pszSummary = NULL;

    if(!pSack || !ppszSummary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgInfoFromId(pSack, dwPkgId, SOLVABLE_SUMMARY, &pszSummary);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszSummary = pszSummary;

cleanup:
    return dwError;

error:
    if(ppszSummary)
    {
        *ppszSummary = NULL;
    }
    goto cleanup;

}

uint32_t
SolvGetPkgLicenseFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszLicense)
{

    uint32_t dwError = 0;
    char* pszLicense = NULL;

    if(!pSack || !ppszLicense)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgInfoFromId(pSack, dwPkgId, SOLVABLE_LICENSE, &pszLicense);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszLicense = pszLicense;

cleanup:
    return dwError;

error:
    if(ppszLicense)
    {
        *ppszLicense = NULL;
    }
    goto cleanup;
}

uint32_t
SolvGetPkgDescriptionFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszDescription
    )
{

    uint32_t dwError = 0;
    char* pszDescription = NULL;

    if(!pSack || !ppszDescription)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgInfoFromId(pSack,
                                   dwPkgId,
                                   SOLVABLE_DESCRIPTION,
                                   &pszDescription);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszDescription = pszDescription;

cleanup:
    return dwError;

error:
    if(ppszDescription)
    {
        *ppszDescription = NULL;
    }
    goto cleanup;
}

uint32_t
SolvGetPkgUrlFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszUrl)
{

    uint32_t dwError = 0;
    char* pszUrl = NULL;

    if(!pSack || !ppszUrl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetPkgInfoFromId(pSack, dwPkgId, SOLVABLE_URL, &pszUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszUrl = pszUrl;

cleanup:

    return dwError;
error:
    if(ppszUrl)
    {
        *ppszUrl = NULL;
    }
    goto cleanup;
}

uint32_t
SolvGetPkgNevrFromId(
    PSolvSack pSack,
    uint32_t  dwPkgId,
    char** ppszNevr)
{
    uint32_t dwError = 0;
    const char* pszTemp = NULL;
    char* pszNevr = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !pSack->pPool || !ppszNevr)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszTemp = pool_solvable2str(pSack->pPool, pSolv);
    if(!pszNevr)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszTemp, &pszNevr);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszNevr = pszNevr;

cleanup:
    return dwError;

error:
    if(ppszNevr)
    {
        *ppszNevr = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszNevr);
    goto cleanup;
}

uint32_t
SolvGetPkgLocationFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszLocation)
{
    uint32_t dwError = 0;
    const char* pszTemp = NULL;
    char* pszLocation = NULL;
    Solvable *pSolv = NULL;

    if(!pSack || !ppszLocation)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, dwPkgId);
    if(!pSolv)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszTemp = solvable_get_location(pSolv, NULL);
    if(!pszTemp)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszTemp, &pszLocation);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszLocation = pszLocation;

cleanup:
    return dwError;

error:
    if(ppszLocation)
    {
        ppszLocation = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszLocation);
    goto cleanup;
}

uint32_t
SolvGetPackageId(
    PSolvPackageList pPkgList,
    int pkgIndex,
    Id* pkgId
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || pkgIndex < 0 || pkgIndex >= pPkgList->queuePackages.count)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    *pkgId = pPkgList->queuePackages.elements[pkgIndex];

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCmpEvr(
    PSolvSack pSack,
    Id pkg1,
    Id pkg2,
    int* result)
{
    uint32_t    dwError = 0;
    Solvable    *pSolv1 = NULL;
    Solvable    *pSolv2 = NULL;
    const char  *pszEvr1 = NULL;
    const char  *pszEvr2 = NULL;

    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv1 = pool_id2solvable(pSack->pPool, pkg1);
    pSolv2 = pool_id2solvable(pSack->pPool, pkg2);
    if(!pSolv1 || !pSolv2)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszEvr1 = solvable_lookup_str(pSolv1, SOLVABLE_EVR);
    pszEvr2 = solvable_lookup_str(pSolv2, SOLVABLE_EVR);

    *result = pool_evrcmp_str(pSack->pPool, pszEvr1, pszEvr2, EVRCMP_COMPARE);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvGetLatest(
    PSolvSack pSack,
    Queue* pPkgList,
    Id pkg,
    Id* result)
{
    uint32_t dwError = 0;
    Solvable    *pSolv1 = NULL;
    Solvable    *pSolv2 = NULL;
    const char  *pszEvr1 = NULL;
    const char  *pszEvr2 = NULL;
    const char  *pszName1  = NULL;
    const char  *pszName2  = NULL;
    int pkgIter  = 0;
    int compareResult = 0;
    *result = pkg;

    if(!pSack || pkg <= 0 || !pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv1 = pool_id2solvable(pSack->pPool, pkg);
    if(!pSolv1)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszName1 = pool_id2str(pSack->pPool, pSolv1->name);
    pszEvr1 = solvable_lookup_str(pSolv1, SOLVABLE_EVR);
    for( ; pkgIter < pPkgList->count;  pkgIter++)
    {
        pSolv2 = pool_id2solvable(pSack->pPool, pPkgList->elements[pkgIter]);
        if(!pSolv2)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
        }

        pszName2 = pool_id2str(pSack->pPool, pSolv2->name);

        if(!strcmp(pszName1, pszName2))
        {
            pszEvr2 = solvable_lookup_str(pSolv2, SOLVABLE_EVR);

            compareResult = pool_evrcmp_str(pSack->pPool, pszEvr2, pszEvr1, EVRCMP_COMPARE);
            if(compareResult == 1)
            {
                *result = pPkgList->elements[pkgIter];
                pSolv1 = pSolv2;
                pszEvr1 = pszEvr2;
            }
        }
    }
cleanup:
    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvFindAllInstalled(
    PSolvSack pSack,
    PSolvPackageList pPkgList)
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack || !pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreateQuery(pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvAddSystemRepoFilter(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvCountPkgByName(
    PSolvSack pSack,
    const char* pszName,
    uint32_t* dwCount
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;

    if(!pSack || !pszName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreateQuery(pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySinglePackageFilter(pQuery, pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvCreatePackageList(&pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetPackageListSize(pPkgList, dwCount);
    BAIL_ON_TDNF_ERROR(dwError);
cleanup:
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvFindInstalledPkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack || !pszName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreateQuery(pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvAddSystemRepoFilter(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySinglePackageFilter(pQuery, pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvFindAvailablePkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreateQuery(pSack, &pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvAddAvailableRepoFilter(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySinglePackageFilter(pQuery, pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;

error:
    goto cleanup;;
}

uint32_t
SolvGetTransResultsWithType(
    Transaction *trans,
    Id type,
    PSolvPackageList pPkgList
    )
{
    uint32_t  dwError = 0;
    Id pkg = 0;
    Id pkgType = 0;
    Queue solvedPackages;
    queue_init(&solvedPackages);
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if (!trans)
    {
        dwError = ERROR_TDNF_SOLV_NO_SOLUTION;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (int i = 0; i < trans->steps.count; ++i)
    {
        pkg = trans->steps.elements[i];

        switch (type)
        {
            case SOLVER_TRANSACTION_OBSOLETED:
                pkgType =  transaction_type(trans, pkg, SOLVER_TRANSACTION_SHOW_OBSOLETES);
                break;
            default:
                pkgType  = transaction_type(trans, pkg,
                     SOLVER_TRANSACTION_SHOW_ACTIVE|
                     SOLVER_TRANSACTION_SHOW_ALL);
                break;
        }

        if (type == pkgType)
            queue_push(&solvedPackages, pkg);
    }
    queue_insertn(&pPkgList->queuePackages, pPkgList->queuePackages.count,
             solvedPackages.count, solvedPackages.elements);
cleanup:
    queue_free(&solvedPackages);
    return dwError;

error:
    goto cleanup;
}
