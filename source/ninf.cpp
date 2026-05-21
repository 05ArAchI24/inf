#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

using namespace std;

string readFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return "";
    string content;
    getline(file, content);
    return content;
}

string formatSpeed(unsigned long long bytes) {
    unsigned long long bits = bytes * 8;
    if (bits >= 1024 * 1024 * 1024) {
        return to_string(bits / (1024 * 1024 * 1024)) + " Gbps";
    } else if (bits >= 1024 * 1024) {
        return to_string(bits / (1024 * 1024)) + " Mbps";
    } else if (bits >= 1024) {
        return to_string(bits / 1024) + " Kbps";
    }
    return to_string(bits) + " bps";
}

string getPublicIP() {
    FILE* pipe = popen("curl -s ifconfig.me", "r");
    if (!pipe) return "Unknown";
    char buffer[128];
    string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result.empty() ? "Unknown" : result;
}

string drawBar(int percent, int width = 30) {
    string bar;
    int filled = (percent * width) / 100;
    for (int i = 0; i < width; i++) {
        if (i < filled) bar += "#";
        else bar += "-";
    }
    return bar;
}

int main() {
    struct ifaddrs *ifaddr, *ifa;
    int family;
    
    cout << "===========- NETWORK INFORMATION -===========" << endl;
    
    if (getifaddrs(&ifaddr) == -1) {
        cerr << "Error: can't get network interfaces" << endl;
        return -1;
    }
    
    vector<pair<string, string>> ipv4List;
    vector<pair<string, string>> ipv6List;
    string macAddress;
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        family = ifa->ifa_addr->sa_family;
        
        if (family == AF_INET) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                ipv4List.push_back({ifa->ifa_name, ip});
            }
        } else if (family == AF_INET6) {
            char ip[INET6_ADDRSTRLEN];
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            if (strcmp(ifa->ifa_name, "lo") != 0 && strstr(ip, "fe80") == nullptr) {
                ipv6List.push_back({ifa->ifa_name, ip});
            }
        } else if (family == AF_PACKET && macAddress.empty()) {
            struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
            if (s->sll_halen == 6 && strcmp(ifa->ifa_name, "lo") != 0) {
                unsigned char *mac = (unsigned char*)s->sll_addr;
                char macStr[18];
                snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                macAddress = macStr;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    
    cout << "\n[IPv4 ADDRESSES]" << endl;
    for (auto& iface : ipv4List) {
        cout << "  " << iface.first << ": " << iface.second << endl;
    }
    
    if (!ipv6List.empty()) {
        cout << "\n[IPv6 ADDRESSES]" << endl;
        for (auto& iface : ipv6List) {
            cout << "  " << iface.first << ": " << iface.second << endl;
        }
    }
    
    if (!macAddress.empty()) {
        cout << "\n[MAC ADDRESS]" << endl;
        cout << "  " << macAddress << endl;
    }
    
    ifstream routeFile("/proc/net/route");
    if (routeFile.is_open()) {
        string line;
        string gateway;
        getline(routeFile, line);
        while (getline(routeFile, line)) {
            char iface[32];
            unsigned long dest, gw;
            sscanf(line.c_str(), "%s %lx %lx", iface, &dest, &gw);
            if (dest == 0) {
                struct in_addr addr;
                addr.s_addr = gw;
                gateway = inet_ntoa(addr);
                break;
            }
        }
        routeFile.close();
        if (!gateway.empty()) {
            cout << "\n[DEFAULT GATEWAY]" << endl;
            cout << "  " << gateway << endl;
        }
    }
    
    string publicIP = getPublicIP();
    if (publicIP != "Unknown") {
        cout << "\n[PUBLIC IP]" << endl;
        cout << "  " << publicIP << endl;
    }
    
    ifstream netdev("/proc/net/dev");
    if (netdev.is_open()) {
        string line;
        getline(netdev, line);
        getline(netdev, line);
        
        cout << "\n[TRAFFIC STATISTICS]" << endl;
        
        while (getline(netdev, line)) {
            size_t pos = line.find(':');
            if (pos != string::npos) {
                string iface = line.substr(0, pos);
                iface.erase(0, iface.find_first_not_of(" \t"));
                if (iface != "lo") {
                    unsigned long long rxBytes, txBytes;
                    sscanf(line.c_str() + pos + 1, "%llu %*u %*u %*u %*u %*u %*u %*u %llu",
                           &rxBytes, &txBytes);
                    
                    cout << "  " << iface << ":" << endl;
                    cout << "    RX: " << formatSpeed(rxBytes) << endl;
                    cout << "    TX: " << formatSpeed(txBytes) << endl;
                }
            }
        }
        netdev.close();
    }
    
    cout << "\n=============================================" << endl;
    
    return 0;
}