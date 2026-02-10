#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ArduinoJson.h>

// Simple allocation tracker
size_t total_allocations = 0;

void* operator new(size_t size) {
    total_allocations++;
    return malloc(size);
}

void operator delete(void* ptr) noexcept {
    free(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    free(ptr);
}


void reset_metrics() {
    total_allocations = 0;
}

int main() {
    // DynamicJsonBuffer
    {
        reset_metrics();
        auto start = std::chrono::high_resolution_clock::now();

        DynamicJsonBuffer jsonBuffer;
        JsonArray& array = jsonBuffer.createArray();

        for (int i = 0; i < 1000; ++i) {
            JsonObject& obj = jsonBuffer.createObject();
            obj["type"] = "file";
            // Create a string long enough to defeat SSO on most platforms (usually 15-23 chars)
            std::string name = "file_very_long_name_" + std::to_string(i);
            obj["name"] = name;
            array.add(obj);
        }

        std::string output;
        array.printTo(output);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        std::cout << "DynamicJsonBuffer:" << std::endl;
        std::cout << "  Time: " << elapsed.count() << " ms" << std::endl;
        std::cout << "  Allocations: " << total_allocations << std::endl;
        std::cout << "  Output size: " << output.size() << std::endl;
    }

    // Manual String
    {
        reset_metrics();
        auto start = std::chrono::high_resolution_clock::now();

        std::string output = "[";

        bool first = true;
        for (int i = 0; i < 1000; ++i) {
            if (!first) output += ",";
            std::string name = "file_very_long_name_" + std::to_string(i);
            output += "{\"type\":\"file\",\"name\":\"" + name + "\"}";
            first = false;
        }
        output += "]";

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        std::cout << "Manual String:" << std::endl;
        std::cout << "  Time: " << elapsed.count() << " ms" << std::endl;
        std::cout << "  Allocations: " << total_allocations << std::endl;
        std::cout << "  Output size: " << output.size() << std::endl;
    }

    return 0;
}
