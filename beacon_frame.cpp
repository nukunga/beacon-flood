// beacon_frame.cpp
#include "beacon_frame.h"
#include <cstring>
#include <cstdint>

// 데이터를 벡터에 추가하는 헬퍼 함수
static inline void appendData(std::vector<uint8_t> &vec, const void* data, size_t len) {
    const uint8_t *d = reinterpret_cast<const uint8_t*>(data);
    vec.insert(vec.end(), d, d + len);
}

// Radiotap 헤더, 802.11 Beacon 헤더, 고정 파라미터, Tagged IE들을 포함하는 Beacon 프레임 구성
bool createBeaconFrame(const std::string &ssid, std::vector<uint8_t> &packet) {
    packet.clear();

    // Radiotap Header (8바이트)
    RadiotapHeader rt_hdr;
    rt_hdr.it_version = 0;
    rt_hdr.it_pad = 0;
    rt_hdr.it_len = 8;      // 옵션 필드 없음 -> 8바이트
    rt_hdr.it_present = 0;  // no optional fields
    appendData(packet, &rt_hdr, sizeof(rt_hdr));

    // 802.11 Beacon Header (24바이트)
    uint8_t beaconHdr[24] = {0};
    uint16_t fc = BeaconConstants::FRAME_CONTROL_BEACON;
    std::memcpy(beaconHdr, &fc, sizeof(uint16_t));
    // Duration: 0 (bytes[2,3]) - 이미 0 초기화됨.
    // Addr1
    uint8_t bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    std::memcpy(beaconHdr + 4, bcast, 6);
    // Addr2
    uint8_t src_mac[6] = {0x02,0x03,0x04,0x05,0x06,0x01};
    std::memcpy(beaconHdr + 10, src_mac, 6);
    // Addr3
    std::memcpy(beaconHdr + 16, src_mac, 6);
    // Sequence Control (bytes[22~23]): 0
    appendData(packet, beaconHdr, sizeof(beaconHdr));

    // 고정 파라미터 (12바이트)
    uint8_t fixedParams[12] = {0};
    // timestamp (8바이트) => 0으로 초기화
    uint16_t interval = BeaconConstants::BEACON_INTERVAL;
    std::memcpy(fixedParams + 8, &interval, sizeof(uint16_t));
    uint16_t cap = BeaconConstants::CAPABILITY_INFO;
    std::memcpy(fixedParams + 10, &cap, sizeof(uint16_t));
    appendData(packet, fixedParams, sizeof(fixedParams));

    // Tagged IE, SSID
    uint8_t tag = 0;  // SSID
    uint8_t len = static_cast<uint8_t>(ssid.length());
    packet.push_back(tag);
    packet.push_back(len);
    packet.insert(packet.end(), ssid.begin(), ssid.end());

    return true;
}
