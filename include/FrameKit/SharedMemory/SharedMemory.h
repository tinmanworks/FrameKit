// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/SharedMemory/SharedMemory.h
// Author       : George Gil
// Created      : 2025-09-11
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Public API for cross-platform shared memory
// =============================================================================
#pragma once

#include "FrameKit/SharedMemory/ShmVersion.h" // defines FK_SHM_VERSION_MAJOR/MINOR
#include "FrameKit/Engine/Defines.h"

#include <cstddef>
#include <cstdint>

// ---- Export / import --------------------------------------------------------
#if defined(FK_PLATFORM_WINDOWS)
  #if defined(FK_SHM_BUILD)
    #define FK_SHM_API __declspec(dllexport)
  #else
    #define FK_SHM_API __declspec(dllimport)
  #endif
#else
  #if defined(FK_SHM_BUILD)
    #define FK_SHM_API __attribute__((visibility("default")))
  #else
    #define FK_SHM_API
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ---- Opaque handle ----------------------------------------------------------
typedef struct FKShmHandle_t* FKShmHandle;

// ---- Open mode --------------------------------------------------------------
typedef enum FKShmOpenMode {
  FKSHM_CreateOnly   = 0,
  FKSHM_OpenOnly     = 1,
  FKSHM_OpenOrCreate = 2
} FKShmOpenMode;

// ---- Error codes (0 = OK) ---------------------------------------------------
enum {
  FKSHM_OK                    = 0,
  FKSHM_ERR_INVALID_ARG       = -1,
  FKSHM_ERR_UNSUPPORTED       = -2,
  FKSHM_ERR_SYS               = -3,
  FKSHM_ERR_EXISTS            = -4,
  FKSHM_ERR_NOT_FOUND         = -5,
  FKSHM_ERR_INCOMPATIBLE_VER  = -6,
  FKSHM_ERR_LAYOUT_MISMATCH   = -7,
  FKSHM_ERR_MAP_FAILED        = -8
};

// ---- Control block layout (read-only to callers) ----------------------------
typedef struct FKShmControlBlock {
  uint32_t magic;        // 0xFD5A11ED
  uint16_t version;      // [major:8 | minor:8]
  uint16_t reserved;
  uint64_t totalSize;    // control + payload
  uint64_t payloadSize;  // payload only
} FKShmControlBlock;

// ---- C API ------------------------------------------------------------------

// Create or open a mapping with payload_size bytes.
// Returns 0 on success. Sets *out_handle and *out_created (1 if newly created).
FK_SHM_API int fk_shm_create_or_open(const char* name,
                                     size_t      payload_size,
                                     FKShmOpenMode mode,
                                     FKShmHandle* out_handle,
                                     int*         out_created);

// Open an existing mapping with expected payload_size.
// Verifies version compatibility and payload size.
FK_SHM_API int fk_shm_open_typed(const char* name,
                                 size_t      payload_size,
                                 FKShmHandle* out_handle);

// Close/unmap a mapping. Safe to call with NULL.
FK_SHM_API void fk_shm_close(FKShmHandle handle);

// Remove named segment from namespace.
// Linux: shm_unlink. Windows: no-op that returns OK.
FK_SHM_API int fk_shm_unlink(const char* name);

// Accessors. Return NULL on error.
FK_SHM_API void*                       fk_shm_payload(FKShmHandle handle);
FK_SHM_API const FKShmControlBlock*    fk_shm_control(FKShmHandle handle);

// Version helpers.
FK_SHM_API uint16_t fk_shm_local_version(void);
FK_SHM_API uint8_t  fk_shm_ver_major(uint16_t packed);
FK_SHM_API uint8_t  fk_shm_ver_minor(uint16_t packed);

// ---- Constants --------------------------------------------------------------
enum { FKSHM_MAGIC = 0xFD5A11EDu };

#ifdef __cplusplus
} // extern "C"

// ---- Minimal C++ sugar ------------------------------------------------------
namespace FrameKit::SHM {

// Create/open typed payload T. Runs T() once if created.
template <class T>
inline T* CreateTyped(const char* name, FKShmOpenMode mode, FKShmHandle* out = nullptr) {
  FKShmHandle h = nullptr; int created = 0;
  if (fk_shm_create_or_open(name, sizeof(T), mode, &h, &created) != FKSHM_OK) return nullptr;
  const FKShmControlBlock* cb = fk_shm_control(h);
  if (!cb || cb->payloadSize != sizeof(T)) { fk_shm_close(h); return nullptr; }
  void* p = fk_shm_payload(h);
  if (created) new (p) T();
  if (out) *out = h; else fk_shm_close(h);
  return static_cast<T*>(p);
}

// Open typed payload T without constructing.
template <class T>
inline T* OpenTyped(const char* name, FKShmHandle* out = nullptr) {
  FKShmHandle h = nullptr;
  if (fk_shm_open_typed(name, sizeof(T), &h) != FKSHM_OK) return nullptr;
  void* p = fk_shm_payload(h);
  if (out) *out = h; else fk_shm_close(h);
  return static_cast<T*>(p);
}

inline void Close(FKShmHandle h) { fk_shm_close(h); }

} // namespace FrameKit::SHM
#endif // __cplusplus