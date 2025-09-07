/*
 * Project: FrameKit
 * File: SharedMemory.h
 * Author: George Gil
 * Date: 2025-08-12
 * License: MIT
 * Description:
 *   Cross-platform (Windows + Linux) shared memory utilities for FrameKit.
 *   Manager-free API with create/open/close/unlink and a typed mapping helper
 *   that runs constructors exactly once.
 *   Layout: [ControlBlock][Payload...]
 */

#pragma once


#include "FrameKit/Platform/PlatformDetection.h"

#include <cstdint>
#include <cstddef>
#include <new>        // placement new
#include <string>
#include <cstring>

#if defined(FK_PLATFORM_WINDOWS)
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #include <windows.h>
#elif defined(FK_PLATFORM_LINUX)
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #include <errno.h>
#else
  #if !defined(FK_ALLOW_UNKNOWN_PLATFORM)
  #error "SharedMemory.h: unsupported platform (expected Windows or Linux)."
  #endif
#endif

#include "SharedMemoryVersion.h"

// ---- Version packing helpers ------------------------------------------------

// Pack MAJOR.MINOR into a uint16_t: [major:8 | minor:8]
#ifndef FK_SHM_VERSION_MAJOR
#  error "FK_SHM_VERSION_MAJOR not defined. Include SharedMemoryVersion.h before using SharedMemory."
#endif
#ifndef FK_SHM_VERSION_MINOR
#  error "FK_SHM_VERSION_MINOR not defined. Include SharedMemoryVersion.h before using SharedMemory."
#endif

namespace FrameKit::IPC {

constexpr std::uint16_t EncodeVersion(std::uint16_t maj, std::uint16_t min) { return static_cast<std::uint16_t>(((maj & 0xFFu) << 8) | (min & 0xFFu)); }
constexpr std::uint8_t VersionMajor(std::uint16_t v) { return static_cast<std::uint8_t>((v >> 8) & 0xFFu); }
constexpr std::uint8_t VersionMinor(std::uint16_t v) { return static_cast<std::uint8_t>(v & 0xFFu); }

constexpr std::uint16_t kLocalVersion = EncodeVersion(FK_SHM_VERSION_MAJOR, FK_SHM_VERSION_MINOR);

// Optional: stringify a packed MAJOR.MINOR (useful for logs/UIs)
inline std::string ShmVersionString(std::uint16_t v) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%u.%u", static_cast<unsigned>(VersionMajor(v)),
                                     static_cast<unsigned>(VersionMinor(v)));
    return std::string(buf);
}

// ---- Control block & public types ------------------------------------------

struct ControlBlock {
    static constexpr std::uint32_t kMagic = 0xFD5A11ED; // "FD S-Map"
    std::uint32_t magic       = kMagic;
    std::uint16_t version     = kLocalVersion;  // set from SharedMemoryVersion.h
    std::uint16_t reserved    = 0;
    std::uint64_t totalSize   = 0;              // bytes: control + payload
    std::uint64_t payloadSize = 0;              // bytes: payload only (immediately after ControlBlock)
};

enum class OpenMode {
    CreateOnly,
    OpenOnly,
    OpenOrCreate
};

// Opaque mapping handle (RAII is manual: call CloseSharedMemory)
struct SharedMemory {
    void*        base = nullptr;  // mapped base (points to ControlBlock)
    std::size_t  size = 0;        // mapped size (total)
#if defined(FK_PLATFORM_WINDOWS)
    void*        hMap = nullptr;  // HANDLE
#elif defined(FK_PLATFORM_LINUX)
    int          fd   = -1;       // shm fd
#endif
    std::string  name;            // normalized name (e.g., "/FrameKit.EngineBus" on Linux)
};

// Low-level helpers to navigate the layout
inline ControlBlock* GetControl(void* base) noexcept {
    return base ? reinterpret_cast<ControlBlock*>(base) : nullptr;
}
inline void* GetPayload(ControlBlock* cb) noexcept {
    return cb ? reinterpret_cast<void*>(cb + 1) : nullptr;
}

// ---- Internal: platform helpers (header-only, inline) ----------------------

inline bool normalize_name(const char* in, std::string& out_norm) noexcept {
#if defined(FK_PLATFORM_LINUX)
    if (!in || !*in) return false;
    if (in[0] == '/') { out_norm = in; }
    else {
        out_norm.reserve(1 + std::strlen(in));
        out_norm = '/';
        out_norm += in;
    }
    return true;
#else
    if (!in || !*in) return false;
    out_norm = in;
    return true;
#endif
}

inline void close_mapping(SharedMemory& shm) noexcept {
#if defined(FK_PLATFORM_WINDOWS)
    if (shm.base) { UnmapViewOfFile(shm.base); shm.base = nullptr; }
    if (shm.hMap) { CloseHandle(reinterpret_cast<HANDLE>(shm.hMap)); shm.hMap = nullptr; }
#elif defined(FK_PLATFORM_LINUX)
    if (shm.base) { munmap(shm.base, shm.size); shm.base = nullptr; }
    if (shm.fd >= 0) { close(shm.fd); shm.fd = -1; }
#endif
    shm.size = 0;
}

inline bool unlink_mapping_name(const char* name) noexcept {
#if defined(FK_PLATFORM_WINDOWS)
    (void)name; // Windows: no unlink; object vanishes when last handle closes
    return true;
#elif defined(FK_PLATFORM_LINUX)
    std::string norm;
    if (!normalize_name(name, norm)) return false;
    return shm_unlink(norm.c_str()) == 0;
#else
    (void)name; return true;
#endif
}

/**
 * Create or open a mapping of exactly totalSize bytes.
 * Sets `created=true` iff this call created the underlying named object.
 */
[[nodiscard]] inline bool create_or_open_mapping(const char* name,
                                   std::size_t totalSize,
                                   OpenMode mode,
                                   SharedMemory& out,
                                   bool& created) noexcept
{
    created = false;
    out = SharedMemory{};
    if (!normalize_name(name, out.name)) return false;

#if defined(FK_PLATFORM_WINDOWS)
    DWORD sizeLow  = static_cast<DWORD>( totalSize        & 0xFFFFFFFFULL );
    DWORD sizeHigh = static_cast<DWORD>((totalSize >> 32) & 0xFFFFFFFFULL );

    HANDLE hMap = nullptr;

    if (mode == OpenMode::CreateOnly) {
        hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                  sizeHigh, sizeLow, out.name.c_str());
        if (!hMap) return false;
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(hMap);
            return false; // caller insisted on CreateOnly
        }
        created = true;
    } else if (mode == OpenMode::OpenOnly) {
        hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, out.name.c_str());
        if (!hMap) return false;
    } else { // OpenOrCreate
        hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                  sizeHigh, sizeLow, out.name.c_str());
        if (!hMap) return false;
        created = (GetLastError() != ERROR_ALREADY_EXISTS);
    }

    void* base = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, totalSize);
    if (!base) { CloseHandle(hMap); return false; }

    out.base = base;
    out.size = totalSize;
    out.hMap = hMap;
    return true;

#elif defined(FK_PLATFORM_LINUX)
    int flags = O_RDWR;
    int fd = -1;

    if (mode == OpenMode::CreateOnly) {
        fd = shm_open(out.name.c_str(), flags | O_CREAT | O_EXCL, 0666);
        if (fd < 0) return false;
        if (ftruncate(fd, static_cast<off_t>(totalSize)) != 0) {
            close(fd); shm_unlink(out.name.c_str()); return false;
        }
        created = true;
    } else if (mode == OpenMode::OpenOnly) {
        fd = shm_open(out.name.c_str(), flags, 0666);
        if (fd < 0) return false;
        // creator defines size; we map expected size and validate via ControlBlock
    } else { // OpenOrCreate
        fd = shm_open(out.name.c_str(), flags | O_CREAT, 0666);
        if (fd < 0) return false;

        struct stat st{};
        if (fstat(fd, &st) != 0) { close(fd); return false; }
        if (st.st_size == 0) {
            if (ftruncate(fd, static_cast<off_t>(totalSize)) != 0) {
                close(fd); shm_unlink(out.name.c_str()); return false;
            }
            created = true;
        }
    }

    void* base = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) {
        if (created) shm_unlink(out.name.c_str());
        close(fd);
        return false;
    }

    out.base = base;
    out.size = totalSize;
    out.fd   = fd;
    return true;
#else
    (void)totalSize; (void)mode; (void)out;
    return false;
#endif
}

// Forward Declarations
inline void CloseSharedMemory(SharedMemory& shm) noexcept;

// ---- Public API -------------------------------------------------------------

// Create/open a mapping whose payload is T; constructs T with placement-new exactly once.
template <typename T>
[[nodiscard]] inline T* CreateTypedSharedMemory(const char* name,
                                  OpenMode mode = OpenMode::OpenOrCreate,
                                  SharedMemory* outMapping = nullptr)
{
    const std::size_t totalSize = sizeof(ControlBlock) + sizeof(T);

    SharedMemory shm{};
    bool created = false;
    if (!create_or_open_mapping(name, totalSize, mode, shm, created)) {
        return nullptr;
    }

    auto* cb = GetControl(shm.base);

    if (created) {
        *cb = ControlBlock{};
        cb->version     = kLocalVersion;
        cb->totalSize   = totalSize;
        cb->payloadSize = sizeof(T);
        void* payload = GetPayload(cb);
        new (payload) T; // run constructor once
    } else {
        if (!cb || cb->magic != ControlBlock::kMagic) {
            close_mapping(shm);
            return nullptr;
        }
        // Version policy: same MAJOR, opener minor >= stored minor
        const auto localMaj  = VersionMajor(kLocalVersion);
        const auto localMin  = VersionMinor(kLocalVersion);
        const auto remoteMaj = VersionMajor(cb->version);
        const auto remoteMin = VersionMinor(cb->version);
        if (localMaj != remoteMaj || localMin < remoteMin) {
            close_mapping(shm);
            return nullptr; // incompatible versions
        }
        if (cb->payloadSize != sizeof(T)) {
            close_mapping(shm);
            return nullptr; // layout mismatch
        }
    }

    if (outMapping) *outMapping = shm; else CloseSharedMemory(shm);
    return reinterpret_cast<T*>(GetPayload(cb));
}

// Open an existing mapping whose payload is T (no constructors run).
template <typename T>
[[nodiscard]] inline T* OpenTypedSharedMemory(const char* name,
                                SharedMemory* outMapping = nullptr)
{
    const std::size_t totalSize = sizeof(ControlBlock) + sizeof(T);

    SharedMemory shm{};
    bool created = false;
    if (!create_or_open_mapping(name, totalSize, OpenMode::OpenOnly, shm, created)) {
        return nullptr;
    }

    auto* cb = GetControl(shm.base);
    if (!cb || cb->magic != ControlBlock::kMagic) {
        close_mapping(shm);
        return nullptr;
    }

    // Version policy: same MAJOR, opener minor >= stored minor
    const auto localMaj  = VersionMajor(kLocalVersion);
    const auto localMin  = VersionMinor(kLocalVersion);
    const auto remoteMaj = VersionMajor(cb->version);
    const auto remoteMin = VersionMinor(cb->version);
    if (localMaj != remoteMaj || localMin < remoteMin) {
        close_mapping(shm);
        return nullptr; // incompatible versions
    }

    if (cb->payloadSize != sizeof(T)) {
        close_mapping(shm);
        return nullptr; // layout mismatch
    }

    if (outMapping) *outMapping = shm; else CloseSharedMemory(shm);
    return reinterpret_cast<T*>(GetPayload(cb));
}

// Close/unmap a mapping (does NOT call T::~T()).
inline void CloseSharedMemory(SharedMemory& shm) noexcept {
    close_mapping(shm);
}

// Remove a named segment from the namespace (Linux only; no-op on Windows).
inline bool UnlinkSharedMemory(const char* name) noexcept {
    return unlink_mapping_name(name);
}

// Create a bigger mapping with the same name + ".v2" and copy overlapping bytes.
// Returns pointer to new T; on success, caller should CloseSharedMemory(oldShm).
template <typename T>
[[nodiscard]] inline T* RecreateBigger(const char* name,
                         const SharedMemory& oldShm,
                         std::size_t newPayloadSize,
                         SharedMemory* outNewShm = nullptr)
{
    std::string newName = std::string(name) + ".v2";
    const std::size_t newTotal = sizeof(ControlBlock) + newPayloadSize;

    SharedMemory newShm{};
    bool created = false;
    if (!create_or_open_mapping(newName.c_str(), newTotal, OpenMode::CreateOnly, newShm, created) || !created) {
        return nullptr;
    }

    auto* newCb = GetControl(newShm.base);
    *newCb = ControlBlock{};
    newCb->version     = kLocalVersion;
    newCb->totalSize   = newTotal;
    newCb->payloadSize = newPayloadSize;

    void* dst = GetPayload(newCb);
    new (dst) T;

    auto* oldCb = GetControl(oldShm.base);
    if (oldCb && oldCb->magic == ControlBlock::kMagic) {
        const std::size_t toCopy = (oldCb->payloadSize < newPayloadSize)
            ? static_cast<std::size_t>(oldCb->payloadSize)
            : newPayloadSize;
        std::memcpy(dst, GetPayload(oldCb), toCopy);
    }

    if (outNewShm) *outNewShm = newShm; else CloseSharedMemory(newShm);
    return reinterpret_cast<T*>(dst);
}

} // namespace FrameKit::IPC
