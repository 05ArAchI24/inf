#include <iostream>
#include <string>
#include <fstream>
using namespace std;

string readFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return "";
    string content;
    getline(file, content);
    return content;
}

double readEnergyFull() {
    string full = readFile("/sys/class/power_supply/BAT1/energy_full");
    if (full.empty()) return 0;
    return stod(full) / 100000;
    
}

double readEnergyFullDesign() {
    string design = readFile("/sys/class/power_supply/BAT1/energy_full_design");
    if (design.empty()) return 0;
    return stod(design) / 1000000;
}

string formatTime(int min) {
    if (min < 0) return "Unknown";
    int hr = min / 60;
    int mins = min % 60;
    return to_string(hr) + "h " + to_string(mins) + "m ";
}

string drawBar(int precent, int width = 20) {
    string bar;
    int filled = (precent * width) / 100;
    for (int i = 0; i < width; i++) {
        if(i<filled) bar+="#";
        else bar += "-";
    }
    return bar;
}

int main() {
    string capacityPath = "/sys/class/power_supply/BAT1/capacity";
    string statusPath = "/sys/class/power_supply/BAT1/status";
    string voltagePath = "/sys/class/power_supply/BAT1/voltage_now";
    string currentPath = "/sys/class/power_supply/BAT1/current_now";
    string energyPath = "/sys/class/power_supply/BAT1/charge_now";

    ifstream test(capacityPath);
    if (!test.is_open()) {
        cerr << "Error 404: battery didn't find." << endl;
        cerr << "check path: '/sys/class/power_supply/BAT1/'" << endl;
        return -1;
    }

    string capacityStr = readFile(capacityPath);
    int precent = stoi(capacityStr);
    string status = readFile(statusPath);
    string voltageStr = readFile(voltagePath);
    string currentStr = readFile(currentPath);
    string energyStr = readFile(energyPath);
    
    double voltage = stod(voltageStr) / 1000000;
    double current = stod(currentStr) / 1000000;
    double energy = stod(energyStr) / 1000000;
    
    string timeLeft = "Unknown";
    if (current > 0) {
        if (status == "Discharging") {
            double hoursLeft = energy / (voltage * current);
            timeLeft = formatTime(hoursLeft * 60);
        } else if (status == "Charging") {
            double energyFull = readEnergyFull();
            double hoursLeft = (energyFull - energy) / (voltage * current);
            timeLeft = formatTime(hoursLeft * 60);
        }
    }

    double healthPercent = 100.0;
    double fullDesign = readEnergyFullDesign();
    double fullNow = readEnergyFull();
    if (fullDesign > 0) {
        healthPercent = (fullNow / fullDesign) * 100;
    }

    string bar = drawBar(precent, 30);
    
    cout << "===========- BATTERY INFORMATION -===========" << endl;
    cout << "Charge: " << precent << "% " << bar << endl;
    cout << "Status: " << status << endl;
    if(timeLeft != "Unknown") {
        cout << "Time left: " << timeLeft << endl;
    }
    cout << "Health battery: " << (int)healthPercent << endl;

    cout << "\n=============- TECHNICAL  DATA -=============" << endl;

    cout << "Voltage: " << voltage << "V" << endl;
    if (current > 0) {
        cout << "Current: " << current << "A" << endl;
        cout << "Power: " << voltage * current << "W" << endl;

    }

    cout << "Energy: " << energy << " / " << fullNow << "Wh" << endl;
    return 0;
}