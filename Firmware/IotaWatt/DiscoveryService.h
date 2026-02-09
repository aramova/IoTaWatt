#ifndef DiscoveryService_h
#define DiscoveryService_h

#include "IotaWatt.h"

enum DiscoveryState {
    DS_IDLE,
    DS_SCANNING_UP,
    DS_WAITING_DOWN,
    DS_CONFIRMED,
    DS_MISC,
    DS_TIMEOUT,
    DS_ERROR
};

class DiscoveryService {
public:
    DiscoveryService();
    void start(String id, uint16_t mainsMask);
    void stop();
    String status();
    uint32_t loop();

    DiscoveryState state;
    int candidateChannel;
    String candidateName;

private:
    double baselineWatts[15];
    double peakWatts;
    uint32_t startTime;
    uint32_t stateTime;
    uint16_t mainsMask;
    String id;
};

extern DiscoveryService discoveryService;
uint32_t discoveryServiceLoop(struct serviceBlock* sb);

#endif
