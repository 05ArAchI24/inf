#include <iostream>
#include <string>
#include <sys/statvfs.h>

using namespace std;

string formatSize(unsigned long long bytes) {
    const unsigned long long KB = 1024;
    const unsigned long long MB = KB * 1024;
    const unsigned long long GB = MB * 1024;

    if (bytes >= GB) {
        return to_string(bytes/GB) + "." + to_string((bytes % GB) / (GB / 10)) + "Gb";
    } else if (bytes >= MB) {
        return to_string(bytes/MB) + "." + to_string((bytes % MB) / (MB / 10)) + "Mb";
    } else {
        return to_string(bytes) + " B";
    }
}

int main () {
    string path = "/";

    struct statvfs stat;

    if (statvfs(path.c_str(), &stat) != 0) {
        cerr << "Error: didn't find information about disk" << endl;
        return -1;
    }

    unsigned long long blockSize = stat.f_frsize;
    unsigned long long totalBlocks = stat.f_blocks;
    unsigned long long freeBlocks = stat.f_bfree;
    unsigned long long availableBlocks = stat.f_bavail;
    unsigned long long totalBytes = totalBlocks * blockSize;
    unsigned long long freeBytes = freeBlocks * blockSize;
    unsigned long long availableBytes = availableBlocks * blockSize;
    unsigned long long usedBytes = totalBytes - freeBytes;

    int percentUsed = (int)((double)usedBytes / totalBytes * 100);
    int percentFree = 100 - percentUsed;

    cout << "================================" << endl;
    cout << "    DISK INFORMATION" << path << endl;
    cout << "================================" << endl;
    cout << "All: " << formatSize(totalBytes) << endl;
    cout << "Use: " << formatSize(usedBytes) <<endl;
    cout << "Free: " << formatSize(freeBytes) <<endl;
    cout << "Available" << formatSize(availableBytes) <<endl;
    cout << "================================" << endl;

    cout << "\n[";
    int barLength = 50;
    int filled = (percentUsed * barLength) / 100;
    for (int i = 0; i < barLength; i++) {
        if (i < filled) cout << "#";
        else cout << ".";
    }
    cout << "] " << percentUsed << "%" << " used" << endl;
    
    return 0;
}