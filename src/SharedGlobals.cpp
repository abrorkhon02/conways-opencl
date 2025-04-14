#include "../include/SharedGlobals.h"
#include <iostream>
#include <chrono>

bool g_printAfterGeneration = false;
int g_delayMs = 0;

static std::ofstream g_trace_file;

void init_trace_log() {
    g_trace_file.open("opencl_debug_trace.log", std::ios::out | std::ios::trunc);
    if (!g_trace_file.is_open()) {
        std::cerr << "Failed to open debug trace file" << std::endl;
    }
    trace_log("Trace log initialized");
}

void trace_log(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch().count();
    
    if (g_trace_file.is_open()) {
        g_trace_file << "[" << value << "] " << message << std::endl;
        g_trace_file.flush();
    }
}