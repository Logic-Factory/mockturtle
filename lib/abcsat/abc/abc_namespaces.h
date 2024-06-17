/**CFile****************************************************************

  FileName    [abc_namespaces.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Namespace logic.]

  Synopsis    []

  Author      [Alan Mishchenko]

  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - Nov 20, 2015.]

  Revision    []

***********************************************************************/

#pragma once


////////////////////////////////////////////////////////////////////////
///                         NAMESPACES                               ///
////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
#  ifdef ABC_ABCSAT_NAMESPACE
#    define ABC_ABCSAT_NAMESPACE_HEADER_START namespace ABC_ABCSAT_NAMESPACE {
#    define ABC_ABCSAT_NAMESPACE_HEADER_END }
#    define ABC_ABCSAT_NAMESPACE_CXX_HEADER_START ABC_ABCSAT_NAMESPACE_HEADER_START
#    define ABC_ABCSAT_NAMESPACE_CXX_HEADER_END ABC_ABCSAT_NAMESPACE_HEADER_END
#    define ABC_ABCSAT_NAMESPACE_IMPL_START namespace ABC_ABCSAT_NAMESPACE {
#    define ABC_ABCSAT_NAMESPACE_IMPL_END }
#    define ABC_ABCSAT_NAMESPACE_PREFIX ABC_ABCSAT_NAMESPACE::
#    define ABC_ABCSAT_NAMESPACE_USING_NAMESPACE using namespace ABC_ABCSAT_NAMESPACE;
#  else
#    define ABC_ABCSAT_NAMESPACE_HEADER_START extern "C" {
#    define ABC_ABCSAT_NAMESPACE_HEADER_END }
#    define ABC_ABCSAT_NAMESPACE_CXX_HEADER_START
#    define ABC_ABCSAT_NAMESPACE_CXX_HEADER_END
#    define ABC_ABCSAT_NAMESPACE_IMPL_START
#    define ABC_ABCSAT_NAMESPACE_IMPL_END
#    define ABC_ABCSAT_NAMESPACE_PREFIX
#    define ABC_ABCSAT_NAMESPACE_USING_NAMESPACE
#  endif // #ifdef ABC_ABCSAT_NAMESPACE
#ifdef SATOKO_ABCSAT_NAMESPACE
    #define SATOKO_ABCSAT_NAMESPACE_HEADER_START namespace SATOKO_ABCSAT_NAMESPACE {
    #define SATOKO_ABCSAT_NAMESPACE_HEADER_END }
    #define SATOKO_ABCSAT_NAMESPACE_CXX_HEADER_START ABC_ABCSAT_NAMESPACE_HEADER_START
    #define SATOKO_ABCSAT_NAMESPACE_CXX_HEADER_END ABC_ABCSAT_NAMESPACE_HEADER_END
    #define SATOKO_ABCSAT_NAMESPACE_IMPL_START namespace SATOKO_ABCSAT_NAMESPACE {
    #define SATOKO_ABCSAT_NAMESPACE_IMPL_END }
    #define SATOKO_ABCSAT_NAMESPACE_PREFIX SATOKO_ABCSAT_NAMESPACE::
    #define SATOKO_ABCSAT_NAMESPACE_USING_NAMESPACE using namespace SATOKO_ABCSAT_NAMESPACE;
#endif
#else
#  define ABC_ABCSAT_NAMESPACE_HEADER_START
#  define ABC_ABCSAT_NAMESPACE_HEADER_END
#  define ABC_ABCSAT_NAMESPACE_CXX_HEADER_START
#  define ABC_ABCSAT_NAMESPACE_CXX_HEADER_END
#  define ABC_ABCSAT_NAMESPACE_IMPL_START
#  define ABC_ABCSAT_NAMESPACE_IMPL_END
#  define ABC_ABCSAT_NAMESPACE_PREFIX
#  define ABC_ABCSAT_NAMESPACE_USING_NAMESPACE
#endif // #ifdef __cplusplus

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
