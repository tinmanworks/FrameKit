#if 0
// Shared type:
struct EngineBus {
    uint32_t seq{0};
    double   temperatureC{0.0};
    char     status[32]{"idle"};
};

// Producer
#include "SharedMemory.h"
using namespace FrameKit::IPC;

int main() {
    SharedMemory shm{};
    auto* bus = CreateTypedSharedMemory<EngineBus>("FrameKit.EngineBus",
                                                   OpenMode::OpenOrCreate, &shm);
    if (!bus) return 1;

    bus->temperatureC = 42.5;
    bus->seq++;
    std::strncpy(bus->status, "running", sizeof(bus->status)-1);

    CloseSharedMemory(shm);
}

// Consumer
#include "SharedMemory.h"
using namespace FrameKit::IPC;

int main() {
    SharedMemory shm{};
    auto* bus = OpenTypedSharedMemory<EngineBus>("FrameKit.EngineBus", &shm);
    if (!bus) return 1;

    // read
    // printf("seq=%u temp=%.2f status=%s\n", bus->seq, bus->temperatureC, bus->status);

    CloseSharedMemory(shm);
}

#endif