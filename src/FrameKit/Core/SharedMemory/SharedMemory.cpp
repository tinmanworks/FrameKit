// =============================================================================
// Project      : FrameKit
// File         : src/SharedMemory.cpp
// Author       : George Gil
// Created      : 2025-09-11
// Updated      : 2025-09-11
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Implementation of shared memory C API for FrameKit
// =============================================================================

#define FK_SHM_BUILD
#include "FrameKit/SharedMemory/SharedMemory.h"
#include "FrameKit/Engine/Defines.h"

#include <cstring>
#include <string>

#if defined(FK_PLATFORM_WINDOWS)
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #include <windows.h>
#elif defined(FK_PLATFORM_LINUX)
  #include <cerrno>
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <unistd.h>
#else
  // macOS or unknown platforms currently unsupported
  #define FK_SHM_UNSUPPORTED_PLATFORM
#endif

// ---- Version helpers (internal) --------------------------------------------
static inline uint16_t fk_pack_version(uint16_t maj, uint16_t min) {
  return static_cast<uint16_t>(((maj & 0xFFu) << 8) | (min & 0xFFu));
}

extern "C" {

FK_SHM_API uint8_t  fk_shm_ver_major(uint16_t packed) { return static_cast<uint8_t>((packed >> 8) & 0xFFu); }
FK_SHM_API uint8_t  fk_shm_ver_minor(uint16_t packed) { return static_cast<uint8_t>(packed & 0xFFu); }
FK_SHM_API uint16_t fk_shm_local_version(void) {
  return fk_pack_version(FK_SHM_VERSION_MAJOR, FK_SHM_VERSION_MINOR);
}

} // extern "C"

// ---- Opaque handle definition ----------------------------------------------
struct FKShmHandle_t {
  void*      base = nullptr;   // mapped base (points to FKShmControlBlock)
  size_t     size = 0;         // total mapping size
#if defined(FK_PLATFORM_WINDOWS)
  HANDLE     hMap = nullptr;
#elif defined(FK_PLATFORM_LINUX)
  int        fd   = -1;
#endif
  std::string name;            // normalized name
};

// ---- Internal helpers -------------------------------------------------------
static inline FKShmControlBlock* get_control(void* base) noexcept {
  return base ? reinterpret_cast<FKShmControlBlock*>(base) : nullptr;
}
static inline void* get_payload(FKShmControlBlock* cb) noexcept {
  return cb ? reinterpret_cast<void*>(cb + 1) : nullptr;
}

static bool normalize_name(const char* in, std::string& out_norm) noexcept {
#if defined(FK_PLATFORM_LINUX)
  if (!in || !*in) return false;
  if (in[0] == '/') out_norm = in;
  else { out_norm.reserve(std::strlen(in) + 1); out_norm = '/'; out_norm += in; }
  return true;
#else
  if (!in || !*in) return false;
  out_norm = in;
  return true;
#endif
}

static void close_mapping(FKShmHandle h) noexcept {
  if (!h) return;
#if defined(FK_PLATFORM_WINDOWS)
  if (h->base) { UnmapViewOfFile(h->base); h->base = nullptr; }
  if (h->hMap) { CloseHandle(h->hMap); h->hMap = nullptr; }
#elif defined(FK_PLATFORM_LINUX)
  if (h->base) { munmap(h->base, h->size); h->base = nullptr; }
  if (h->fd >= 0) { close(h->fd); h->fd = -1; }
#endif
  h->size = 0;
}

static int unlink_name(const char* name) noexcept {
#if defined(FK_PLATFORM_WINDOWS)
  (void)name;
  return FKSHM_OK; // Windows: object disappears when last handle closes
#elif defined(FK_PLATFORM_LINUX)
  std::string norm;
  if (!normalize_name(name, norm)) return FKSHM_ERR_INVALID_ARG;
  return (shm_unlink(norm.c_str()) == 0) ? FKSHM_OK : FKSHM_ERR_SYS;
#else
  (void)name;
  return FKSHM_ERR_UNSUPPORTED;
#endif
}

static int create_or_open_mapping(const char* name,
                                  size_t total_size,
                                  FKShmOpenMode mode,
                                  FKShmHandle& out,
                                  int& created) noexcept
{
  created = 0;
  out = nullptr;

#if defined(FK_SHM_UNSUPPORTED_PLATFORM)
  (void)name; (void)total_size; (void)mode;
  return FKSHM_ERR_UNSUPPORTED;
#else
  std::string norm;
  if (!normalize_name(name, norm) || total_size == 0)
    return FKSHM_ERR_INVALID_ARG;

  FKShmHandle h = new(std::nothrow) FKShmHandle_t();
  if (!h) return FKSHM_ERR_SYS;
  h->name = std::move(norm);

#if defined(FK_PLATFORM_WINDOWS)
  DWORD sizeLow  = static_cast<DWORD>( total_size        & 0xFFFFFFFFULL );
  DWORD sizeHigh = static_cast<DWORD>((total_size >> 32) & 0xFFFFFFFFULL );

  HANDLE map = nullptr;
  if (mode == FKSHM_CreateOnly) {
    map = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                             sizeHigh, sizeLow, h->name.c_str());
    if (!map) { delete h; return FKSHM_ERR_SYS; }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      CloseHandle(map); delete h; return FKSHM_ERR_EXISTS;
    }
    created = 1;
  } else if (mode == FKSHM_OpenOnly) {
    map = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, h->name.c_str());
    if (!map) { delete h; return FKSHM_ERR_NOT_FOUND; }
  } else { // OpenOrCreate
    map = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                             sizeHigh, sizeLow, h->name.c_str());
    if (!map) { delete h; return FKSHM_ERR_SYS; }
    created = (GetLastError() != ERROR_ALREADY_EXISTS) ? 1 : 0;
  }

  void* base = MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, total_size);
  if (!base) { CloseHandle(map); delete h; return FKSHM_ERR_MAP_FAILED; }

  h->hMap = map;
  h->base = base;
  h->size = total_size;

#elif defined(FK_PLATFORM_LINUX)
  int flags = O_RDWR;
  int fd = -1;

  if (mode == FKSHM_CreateOnly) {
    fd = shm_open(h->name.c_str(), flags | O_CREAT | O_EXCL, 0666);
    if (fd < 0) { delete h; return (errno == EEXIST) ? FKSHM_ERR_EXISTS : FKSHM_ERR_SYS; }
    if (ftruncate(fd, static_cast<off_t>(total_size)) != 0) {
      int e = errno; close(fd); shm_unlink(h->name.c_str()); delete h;
      (void)e; return FKSHM_ERR_SYS;
    }
    created = 1;
  } else if (mode == FKSHM_OpenOnly) {
    fd = shm_open(h->name.c_str(), flags, 0666);
    if (fd < 0) { delete h; return FKSHM_ERR_NOT_FOUND; }
    // map with expected size and validate later
  } else {
    fd = shm_open(h->name.c_str(), flags | O_CREAT, 0666);
    if (fd < 0) { delete h; return FKSHM_ERR_SYS; }
    struct stat st{};
    if (fstat(fd, &st) != 0) { int e = errno; (void)e; close(fd); delete h; return FKSHM_ERR_SYS; }
    if (st.st_size == 0) {
      if (ftruncate(fd, static_cast<off_t>(total_size)) != 0) {
        int e = errno; (void)e; close(fd); shm_unlink(h->name.c_str()); delete h; return FKSHM_ERR_SYS;
      }
      created = 1;
    }
  }

  void* base = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (base == MAP_FAILED) {
    int was_created = created;
    if (was_created) shm_unlink(h->name.c_str());
    close(fd);
    delete h;
    return FKSHM_ERR_MAP_FAILED;
  }

  h->fd   = fd;
  h->base = base;
  h->size = total_size;
#endif

  out = h;
  return FKSHM_OK;
#endif
}

// ---- Public API -------------------------------------------------------------
extern "C" {

FK_SHM_API int fk_shm_create_or_open(const char* name,
                                     size_t payload_size,
                                     FKShmOpenMode mode,
                                     FKShmHandle* out_handle,
                                     int* out_created)
{
  if (!out_handle || !out_created || !name || payload_size == 0)
    return FKSHM_ERR_INVALID_ARG;

  const size_t total = sizeof(FKShmControlBlock) + payload_size;

  FKShmHandle h = nullptr; int created = 0;
  int rc = create_or_open_mapping(name, total, mode, h, created);
  if (rc != FKSHM_OK) return rc;

  auto* cb = get_control(h->base);
  if (created) {
    // Initialize control block
    cb->magic       = FKSHM_MAGIC;
    cb->version     = fk_shm_local_version();
    cb->reserved    = 0;
    cb->totalSize   = static_cast<uint64_t>(total);
    cb->payloadSize = static_cast<uint64_t>(payload_size);
    // payload left uninitialized; higher layer may placement-new
  } else {
    // Validate magic
    if (!cb || cb->magic != FKSHM_MAGIC) {
      close_mapping(h); delete h; return FKSHM_ERR_SYS;
    }
    // Version policy: same MAJOR, opener MINOR >= stored MINOR
    const uint8_t localMaj  = fk_shm_ver_major(fk_shm_local_version());
    const uint8_t localMin  = fk_shm_ver_minor(fk_shm_local_version());
    const uint8_t remoteMaj = fk_shm_ver_major(cb->version);
    const uint8_t remoteMin = fk_shm_ver_minor(cb->version);
    if (localMaj != remoteMaj || localMin < remoteMin) {
      close_mapping(h); delete h; return FKSHM_ERR_INCOMPATIBLE_VER;
    }
    // Layout check
    if (cb->payloadSize != static_cast<uint64_t>(payload_size)) {
      close_mapping(h); delete h; return FKSHM_ERR_LAYOUT_MISMATCH;
    }
  }

  *out_handle  = h;
  *out_created = created;
  return FKSHM_OK;
}

FK_SHM_API int fk_shm_open_typed(const char* name,
                                 size_t payload_size,
                                 FKShmHandle* out_handle)
{
  if (!out_handle || !name || payload_size == 0)
    return FKSHM_ERR_INVALID_ARG;

  const size_t total = sizeof(FKShmControlBlock) + payload_size;

  FKShmHandle h = nullptr; int created = 0;
  int rc = create_or_open_mapping(name, total, FKSHM_OpenOnly, h, created);
  if (rc != FKSHM_OK) return rc;

  auto* cb = get_control(h->base);
  if (!cb || cb->magic != FKSHM_MAGIC) { close_mapping(h); delete h; return FKSHM_ERR_SYS; }

  const uint8_t localMaj  = fk_shm_ver_major(fk_shm_local_version());
  const uint8_t localMin  = fk_shm_ver_minor(fk_shm_local_version());
  const uint8_t remoteMaj = fk_shm_ver_major(cb->version);
  const uint8_t remoteMin = fk_shm_ver_minor(cb->version);
  if (localMaj != remoteMaj || localMin < remoteMin) {
    close_mapping(h); delete h; return FKSHM_ERR_INCOMPATIBLE_VER;
  }

  if (cb->payloadSize != static_cast<uint64_t>(payload_size)) {
    close_mapping(h); delete h; return FKSHM_ERR_LAYOUT_MISMATCH;
  }

  *out_handle = h;
  return FKSHM_OK;
}

FK_SHM_API void fk_shm_close(FKShmHandle handle) {
  if (!handle) return;
  close_mapping(handle);
  delete handle;
}

FK_SHM_API int fk_shm_unlink(const char* name) {
  if (!name) return FKSHM_ERR_INVALID_ARG;
  return unlink_name(name);
}

FK_SHM_API void* fk_shm_payload(FKShmHandle handle) {
  if (!handle || !handle->base) return nullptr;
  return get_payload(get_control(handle->base));
}

FK_SHM_API const FKShmControlBlock* fk_shm_control(FKShmHandle handle) {
  if (!handle) return nullptr;
  return get_control(handle->base);
}

} // extern "C"
