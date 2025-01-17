# Makefile for Beacon Flooding Attack

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LIBS = 

TARGET = beacon-flood

SRCS = beaconflood.cpp beacon_frame.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
