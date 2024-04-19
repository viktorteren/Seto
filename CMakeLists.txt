cmake_minimum_required(VERSION 3.1)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

set(CUDD_DIR ${CMAKE_SOURCE_DIR}/libs/cudd)
if(MSVC OR MINGW)
    if (CMAKE_CL_64)
        set(CUDD_CONFIG_DIR ${CMAKE_SOURCE_DIR}/libs/cudd-config/windows)
    else()
        set(CUDD_CONFIG_DIR ${CMAKE_SOURCE_DIR}/libs/cudd-config/windows-32)
    endif()
else()
    set(CUDD_CONFIG_DIR ${CMAKE_SOURCE_DIR}/libs/cudd-config/unix)
endif()
set(UNISTD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/unistd)
set(GETOPT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/getopt)

set(SOURCES
        ${CUDD_DIR}/cudd/cuddAddAbs.c
        ${CUDD_DIR}/cudd/cuddAddApply.c
        ${CUDD_DIR}/cudd/cuddAddFind.c
        ${CUDD_DIR}/cudd/cuddAddInv.c
        ${CUDD_DIR}/cudd/cuddAddIte.c
        ${CUDD_DIR}/cudd/cuddAddNeg.c
        ${CUDD_DIR}/cudd/cuddAddWalsh.c
        ${CUDD_DIR}/cudd/cuddAndAbs.c
        ${CUDD_DIR}/cudd/cuddAnneal.c
        ${CUDD_DIR}/cudd/cuddApa.c
        ${CUDD_DIR}/cudd/cuddAPI.c
        ${CUDD_DIR}/cudd/cuddApprox.c
        ${CUDD_DIR}/cudd/cuddBddAbs.c
        ${CUDD_DIR}/cudd/cuddBddCorr.c
        ${CUDD_DIR}/cudd/cuddBddIte.c
        ${CUDD_DIR}/cudd/cuddBridge.c
        ${CUDD_DIR}/cudd/cuddCache.c
        ${CUDD_DIR}/cudd/cuddCheck.c
        ${CUDD_DIR}/cudd/cuddClip.c
        ${CUDD_DIR}/cudd/cuddCof.c
        ${CUDD_DIR}/cudd/cuddCompose.c
        ${CUDD_DIR}/cudd/cuddDecomp.c
        ${CUDD_DIR}/cudd/cuddEssent.c
        ${CUDD_DIR}/cudd/cuddExact.c
        ${CUDD_DIR}/cudd/cuddExport.c
        ${CUDD_DIR}/cudd/cuddGenCof.c
        ${CUDD_DIR}/cudd/cuddGenetic.c
        ${CUDD_DIR}/cudd/cuddGroup.c
        ${CUDD_DIR}/cudd/cuddHarwell.c
        ${CUDD_DIR}/cudd/cuddInit.c
        ${CUDD_DIR}/cudd/cuddInteract.c
        ${CUDD_DIR}/cudd/cuddLCache.c
        ${CUDD_DIR}/cudd/cuddLevelQ.c
        ${CUDD_DIR}/cudd/cuddLinear.c
        ${CUDD_DIR}/cudd/cuddLiteral.c
        ${CUDD_DIR}/cudd/cuddMatMult.c
        ${CUDD_DIR}/cudd/cuddPriority.c
        ${CUDD_DIR}/cudd/cuddRead.c
        ${CUDD_DIR}/cudd/cuddRef.c
        ${CUDD_DIR}/cudd/cuddReorder.c
        ${CUDD_DIR}/cudd/cuddSat.c
        ${CUDD_DIR}/cudd/cuddSign.c
        ${CUDD_DIR}/cudd/cuddSolve.c
        ${CUDD_DIR}/cudd/cuddSplit.c
        ${CUDD_DIR}/cudd/cuddSubsetHB.c
        ${CUDD_DIR}/cudd/cuddSubsetSP.c
        ${CUDD_DIR}/cudd/cuddSymmetry.c
        ${CUDD_DIR}/cudd/cuddTable.c
        ${CUDD_DIR}/cudd/cuddUtil.c
        ${CUDD_DIR}/cudd/cuddWindow.c
        ${CUDD_DIR}/cudd/cuddZddCount.c
        ${CUDD_DIR}/cudd/cuddZddFuncs.c
        ${CUDD_DIR}/cudd/cuddZddGroup.c
        ${CUDD_DIR}/cudd/cuddZddIsop.c
        ${CUDD_DIR}/cudd/cuddZddLin.c
        ${CUDD_DIR}/cudd/cuddZddMisc.c
        ${CUDD_DIR}/cudd/cuddZddPort.c
        ${CUDD_DIR}/cudd/cuddZddReord.c
        ${CUDD_DIR}/cudd/cuddZddSetop.c
        ${CUDD_DIR}/cudd/cuddZddSymm.c
        ${CUDD_DIR}/cudd/cuddZddUtil.c
        ${CUDD_DIR}/util/cpu_stats.c
        ${CUDD_DIR}/util/cpu_time.c
        ${CUDD_DIR}/util/cstringstream.c
        ${CUDD_DIR}/util/datalimit.c
        ${CUDD_DIR}/util/pathsearch.c
        ${CUDD_DIR}/util/pipefork.c
        ${CUDD_DIR}/util/prtime.c
        ${CUDD_DIR}/util/strsav.c
        ${CUDD_DIR}/util/texpand.c
        ${CUDD_DIR}/util/ucbqsort.c
        ${CUDD_DIR}/util/safe_mem.c
        ${CUDD_DIR}/st/st.c
        ${CUDD_DIR}/epd/epd.c
        ${CUDD_DIR}/mtr/mtrBasic.c
        ${CUDD_DIR}/mtr/mtrGroup.c
        ${CUDD_DIR}/cplusplus/cuddObj.cc
)

add_library(cudd STATIC ${SOURCES})
target_include_directories(cudd PRIVATE ${CUDD_DIR}/cudd ${CUDD_DIR}/util ${CUDD_DIR}/st ${CUDD_DIR}/epd ${CUDD_DIR}/mtr ${CUDD_DIR} ${CUDD_CONFIG_DIR})
if (MSVC)
    target_include_directories(cudd PRIVATE ${UNISTD_INCLUDE_DIR} ${GETOPT_INCLUDE_DIR})
endif()
#add_dependencies(cudd CUDD_DOWNLOAD)

if (MSVC)
    set(CUDD_INCLUDE_DIR ${CUDD_DIR}/cplusplus ${CUDD_DIR}/cudd ${CUDD_DIR}/util ${CUDD_CONFIG_DIR} ${UNISTD_INCLUDE_DIR} ${GETOPT_INCLUDE_DIR} PARENT_SCOPE)
else()
    set(CUDD_INCLUDE_DIR ${CUDD_DIR}/cplusplus ${CUDD_DIR}/cudd ${CUDD_DIR}/util ${CUDD_CONFIG_DIR} PARENT_SCOPE)
endif()

set(PBLIB_DIR ${CMAKE_SOURCE_DIR}/libs/pblib/pblib)

set(PBLIB_SOURCE_FILES
        ${PBLIB_DIR}/auxvarmanager.cpp
        ${PBLIB_DIR}/clausedatabase.cpp
        ${PBLIB_DIR}/formula.cpp
        ${PBLIB_DIR}/helper.cpp
        ${PBLIB_DIR}/incpbconstraint.cpp
        ${PBLIB_DIR}/IncrementalData.cpp
        ${PBLIB_DIR}/IncSimplePBConstraint.cpp
        ${PBLIB_DIR}/pb2cnf.cpp
        ${PBLIB_DIR}/PBConfig.cpp
        ${PBLIB_DIR}/pbconstraint.cpp
        ${PBLIB_DIR}/PBFuzzer.cpp
        ${PBLIB_DIR}/PBParser.cpp
        ${PBLIB_DIR}/preencoder.cpp
        ${PBLIB_DIR}/SimplePBConstraint.cpp
        ${PBLIB_DIR}/VectorClauseDatabase.cpp
        ${PBLIB_DIR}/encoder/adderencoding.cpp
        ${PBLIB_DIR}/encoder/amo.cpp
        ${PBLIB_DIR}/encoder/bdd.cpp
        ${PBLIB_DIR}/encoder/BDD_Seq_Amo.cpp
        ${PBLIB_DIR}/encoder/bimander_amo_encoding.cpp
        ${PBLIB_DIR}/encoder/binary_amo.cpp
        ${PBLIB_DIR}/encoder/BinaryMerge.cpp
        ${PBLIB_DIR}/encoder/cardencoding.cpp
        ${PBLIB_DIR}/encoder/commander_encoding.cpp
        ${PBLIB_DIR}/encoder/Encoder.cpp
        ${PBLIB_DIR}/encoder/k-product.cpp
        ${PBLIB_DIR}/encoder/naive_amo_encoder.cpp
        ${PBLIB_DIR}/encoder/sorting_merging.cpp
        ${PBLIB_DIR}/encoder/SortingNetworks.cpp
        ${PBLIB_DIR}/encoder/SWC.cpp
)

add_library(pblib STATIC ${PBLIB_SOURCE_FILES})
