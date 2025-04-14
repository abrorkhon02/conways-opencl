#pragma once
#include <fstream>
#include <string>

extern bool g_printAfterGeneration;
extern int g_delayMs;

void trace_log(const std::string& message);
void init_trace_log();