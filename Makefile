# TopProcessPlugin Makefile for MSYS2/MinGW

CXX = g++
CXXFLAGS = -std=c++17 -O2 -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0A00
LDFLAGS = -shared -static -s
LIBS = -lpsapi -lkernel32 -luser32

TARGET = TopProcessPlugin.dll
SOURCES = TopProcessPlugin.cpp TopCpuProcessItem.cpp TopMemoryProcessItem.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete: $(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	@echo "Copy $(TARGET) to TrafficMonitor plugins directory"
	@cp -v $(TARGET) "../TrafficMonitor/plugins/" 2>/dev/null || echo "Please manually copy $(TARGET) to TrafficMonitor/plugins/"
