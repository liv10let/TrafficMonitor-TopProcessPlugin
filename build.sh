#!/bin/bash

echo "=== Building TopProcessPlugin ==="

# Compile
g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 \
    -c TopProcessPlugin.cpp -o TopProcessPlugin.o

g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 \
    -c TopCpuProcessItem.cpp -o TopCpuProcessItem.o

g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 \
    -c TopMemoryProcessItem.cpp -o TopMemoryProcessItem.o

# Link
g++ -shared -static -s -o TopProcessPlugin.dll \
    TopProcessPlugin.o TopCpuProcessItem.o TopMemoryProcessItem.o \
    -lpsapi -lkernel32 -luser32

# Cleanup object files
rm -f *.o

if [ -f "TopProcessPlugin.dll" ]; then
    echo ""
    echo "=== Build successful! ==="
    echo "Output: TopProcessPlugin.dll"
    echo ""
    echo "To install:"
    echo "  Copy TopProcessPlugin.dll to TrafficMonitor/plugins/"
else
    echo ""
    echo "=== Build failed! ==="
fi
