#ifndef RIDEMANAGER_H
#define RIDEMANAGER_H

#include <iostream>
#include <queue>
#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <limits>
#include <tuple>
#include <mutex>
#include <thread>
#include <chrono>

using namespace std;

// Structure for User
struct Users {
    string userID;
    double latitude;
    double longitude;
};

// Structure for Driver
struct Drivers {
    string driverID;
    double latitude;
    double longitude;
    bool available;
};

// Distance calculation using Haversine Formula
double calculateDistance(double lat1, double lon1, double lat2, double lon2);

// Comparator for Min-Heap (Priority Queue) based on distance
struct CompareDistance {
    bool operator()(const pair<double, string>& a, const pair<double, string>& b);
};

// RideManager Class
class RideManager {
private:
    map<string, Drivers> driverDatabase; // DriverID -> Driver details
    map<string, Users> userDatabase;     // UserID -> User details

    priority_queue<pair<double, string>, vector<pair<double, string>>, CompareDistance> availableDrivers;
    queue<string> rideRequestQueue; // Queue to store userIDs for pending ride requests

    mutex mtx; // Mutex for thread-safe operations

    void log(const string& message);

public:
    // Load drivers and users
    void loadDrivers(const vector<Drivers>& drivers);
    void loadUsers(const vector<Users>& users);

    // Request and process rides
    void requestRide(const string& userID);
    void processPendingRequests();
    void markDriverAvailable(const string& driverID);
};

#endif // RIDEMANAGER_H
