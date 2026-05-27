#!/bin/bash
cd "/d/TrafficMonitor 网速监控软件/TrafficMonitor_V1.86_x64/TopProcessPlugin"

echo "Compiling TopProcessPlugin.cpp..."
g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 -c TopProcessPlugin.cpp -o TopProcessPlugin.o

echo "Compiling TopCpuProcessItem.cpp..."
g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 -c TopCpuProcessItem.cpp -o TopCpuProcessItem.o

echo "Compiling TopMemoryProcessItem.cpp..."
g++ -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00 -c TopMemoryProcessItem.cpp -o TopMemoryProcessItem.o

echo "Linking..."
g++ -shared -static -s -o TopProcessPlugin.dll TopProcessPlugin.o TopCpuProcessItem.o TopMemoryProcessItem.o -lpsapi -lkernel32 -luser32

echo "Cleaning up..."
rm -f *.o

if [ -f "TopProcessPlugin.dll" ]; then
    echo ""
    echo "=== BUILD SUCCESSFUL ==="
    echo "TopProcessPlugin.dll created!"
    ls -la TopProcessPlugin.dll
else
    echo ""
    echo "=== BUILD FAILED ==="
fi
