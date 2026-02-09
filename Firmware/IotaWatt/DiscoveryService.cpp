#include "DiscoveryService.h"

DiscoveryService discoveryService;

DiscoveryService::DiscoveryService() {
    state = DS_IDLE;
    candidateChannel = -1;
    peakWatts = 0;
    startTime = 0;
    stateTime = 0;
    mainsMask = 0;
}

void DiscoveryService::start(String _id, uint16_t _mainsMask) {
    id = _id;
    mainsMask = _mainsMask;

    // Capture baseline
    for(int i=0; i<maxInputs; i++) {
        if(inputChannel[i]->isActive() && inputChannel[i]->_type == channelTypePower) {
            baselineWatts[i] = inputChannel[i]->dataBucket.watts;
        } else {
            baselineWatts[i] = 0;
        }
    }

    state = DS_SCANNING_UP;
    startTime = millis();
    stateTime = millis();
    candidateChannel = -1;

    // Schedule immediately
    NewService(discoveryServiceLoop);
}

void DiscoveryService::stop() {
    state = DS_IDLE;
}

String DiscoveryService::status() {
    String json = "{";
    json += "\"state\":";
    switch(state) {
        case DS_IDLE: json += "\"idle\""; break;
        case DS_SCANNING_UP: json += "\"scanning\""; break;
        case DS_WAITING_DOWN: json += "\"waiting_down\""; break;
        case DS_CONFIRMED: json += "\"confirmed\""; break;
        case DS_MISC: json += "\"misc\""; break;
        case DS_TIMEOUT: json += "\"timeout\""; break;
        case DS_ERROR: json += "\"error\""; break;
    }

    if (state == DS_WAITING_DOWN || state == DS_CONFIRMED) {
        json += ",\"candidate\":";
        json += candidateChannel;
        json += ",\"power\":";
        json += String(peakWatts - baselineWatts[candidateChannel], 0);
    }

    json += "}";
    return json;
}

uint32_t DiscoveryService::loop() {
    if (state == DS_IDLE) return 0; // Stop service

    uint32_t now = millis();

    if (state == DS_SCANNING_UP) {
        if (now - startTime > 60000) { // 60s timeout
            state = DS_TIMEOUT;
            return 0;
        }

        int potentialCandidate = -1;
        bool mainsJumped = false;
        int jumpCount = 0;

        for(int i=0; i<maxInputs; i++) {
            if(!inputChannel[i]->isActive() || inputChannel[i]->_type != channelTypePower) continue;

            double current = inputChannel[i]->dataBucket.watts;
            double delta = current - baselineWatts[i];

            if (delta >= 600 && delta <= 1200) { // Slightly wider range to account for noise
                jumpCount++;
                if ((mainsMask >> i) & 1) {
                    mainsJumped = true;
                } else {
                    potentialCandidate = i;
                }
            }
        }

        if (jumpCount > 0) {
            if (potentialCandidate != -1) {
                // Found a specific circuit
                candidateChannel = potentialCandidate;
                peakWatts = inputChannel[candidateChannel]->dataBucket.watts;
                state = DS_WAITING_DOWN;
                stateTime = now;
            } else if (mainsJumped) {
                // Only mains jumped -> Misc
                state = DS_MISC; // Or DS_MISC_WAITING?
                // The prompt says "If it's Misc, an alert should pop up so we can measure it again later".
                // It doesn't explicitly say we need to wait for it to turn off to confirm Misc,
                // but for consistency with the flow "alert to remove power draw", we should probably wait.
                // However, without a specific channel to monitor, we just monitor mains drop?
                // For simplicity, let's just report MISC immediately.
                // Wait, if we report MISC immediately, the user might still have the device ON.
                // The prompt says "when it sees the spike send a message to stop...".
                // So for Misc, we should also transition to a state where we tell the user "Found on Mains, turn off".
                // But without a specific channel, tracking the "Drop" is harder (Mains fluctuates more).
                // Let's just report MISC found. The user can turn it off.
                return 0;
            }
        }
        return 100; // Check every 100ms
    }

    if (state == DS_WAITING_DOWN) {
        if (now - stateTime > 30000) { // 30s timeout to turn off
            state = DS_TIMEOUT;
            return 0;
        }

        double current = inputChannel[candidateChannel]->dataBucket.watts;
        // Check if it dropped back near baseline
        if (current < peakWatts - 500) { // Allow some hysteresis
             state = DS_CONFIRMED;
             return 0;
        }
        return 100;
    }

    return 0;
}

uint32_t discoveryServiceLoop(struct serviceBlock* sb) {
    return discoveryService.loop();
}
