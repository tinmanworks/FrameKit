#pragma once
#include <cstdint>
#include <unordered_map>

namespace FrameKit {

    /*
        ServiceRegistry

        Non-template, ABI-stable registry:
          (service_id, version) -> void*

        Use this when you want engine/app-owned services (TransmissionService, etc.)
        to be accessible from layers without introducing templated APIs.
    */

    struct ServiceKey {
        uint64_t id = 0;
        uint32_t version = 0;

        bool operator==(const ServiceKey& o) const noexcept {
            return id == o.id && version == o.version;
        }
    };

    struct ServiceKeyHash {
        size_t operator()(const ServiceKey& k) const noexcept {
            // Simple stable hash (good enough for small counts of services)
            const uint64_t v = (static_cast<uint64_t>(k.version) << 32);
            return static_cast<size_t>(k.id ^ v);
        }
    };

    class ServiceRegistry {
    public:
        // Registers or replaces a service pointer for (id, version).
        void Register(uint64_t id, uint32_t version, void* service) {
            m_Map[ServiceKey{ id, version }] = service;
        }

        // Returns nullptr if not found.
        void* Get(uint64_t id, uint32_t version) const {
            auto it = m_Map.find(ServiceKey{ id, version });
            return (it == m_Map.end()) ? nullptr : it->second;
        }

        void Unregister(uint64_t id, uint32_t version) {
            m_Map.erase(ServiceKey{ id, version });
        }

        void Clear() { m_Map.clear(); }

    private:
        std::unordered_map<ServiceKey, void*, ServiceKeyHash> m_Map;
    };

} // namespace FrameKit
