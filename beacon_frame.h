// beacon_frame.h
#pragma once
#include <string>
#include <vector>
#include <cstdint>

// 최소한의 radioTap 헤더 - 8바이트, 옵션 x
struct RadiotapHeader {
    uint8_t  it_version; // General: 0
    uint8_t  it_pad;    // General: 0
    uint16_t it_len;    // 헤더 길이, 옵션 x => 8 바이트
    uint32_t it_present;    // 옵션 x => 0
} __attribute__((packed));

// Beacon 프레임 관련 상수
namespace BeaconConstants {
    constexpr uint16_t FRAME_CONTROL_BEACON = 0x0080;   // Beacon Frame Control, Management, Type: 0, Subtype: 0x08
    constexpr uint16_t BEACON_INTERVAL = 0x0064;    // Beacon Interval (100 TU) => 기본값
    constexpr uint16_t CAPABILITY_INFO = 0x0000;
}

bool createBeaconFrame(const std::string &ssid, std::vector<uint8_t> &packet);
