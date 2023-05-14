#ifndef FC_SOLVE__FCS_DLLEXPORT_H
#define FC_SOLVE__FCS_DLLEXPORT_H

#ifdef _MSC_VER
#ifdef BUILDING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#define DLLLOCAL
#elif defined(__GNUC__)
#define DLLEXPORT __attribute__((visibility("default")))
#define DLLLOCAL __attribute__((visibility("hidden")))

/*
 * See: https://github.com/shlomif/black-hole-solitaire/issues/7
 *
 * dllimport cannot be applied to non-inline function definition · Issue #7
 *
 */
#if defined(WIN32) && defined(__clang__)
#undef DLLEXPORT
#define DLLEXPORT
#endif

#else
#define DLLEXPORT
#define DLLLOCAL
#endif

#endif
