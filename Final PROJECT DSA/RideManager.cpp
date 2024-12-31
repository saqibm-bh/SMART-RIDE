#include "RideManager.h"

// Haversine formula to calculate distance between two coordinates
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth's radius in kilometers
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
               sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

// Comparator for Min-Heap
bool CompareDistance::operator()(const pair<double, string>& a, const pair<double, string>& b) {
    return a.first > b.first; // Min-Heap: smallest distance first
}

// Logging function
void RideManager::log(const string& message) {
    lock_guard<mutex> lock(mtx);
    cout << "[LOG]: " << message << endl;
}

// Load drivers into the system
void RideManager::loadDrivers(const vector<Drivers>& drivers) {
    for (const auto& driver : drivers) {
        driverDatabase[driver.driverID] = driver;
        if (driver.available) {
            availableDrivers.push({0.0, driver.driverID}); // Initial distance set to 0.0
        }
    }
    log("Drivers loaded successfully.");
}

// Load users into the system
void RideManager::loadUsers(const vector<Users>& users) {
    for (const auto& user : users) {
        userDatabase[user.userID] = user;
    }
    log("Users loaded successfully.");
}

// Request a ride
void RideManager::requestRide(const string& userID) {
    log("Entering requestRide for user: " + userID);  // Log when the function is entered

    if (userDatabase.find(userID) == userDatabase.end()) {
        log("User not found: " + userID);
        return;
    }

    Users user = userDatabase[userID];
    log("User " + userID + " requesting a ride at location (" + to_string(user.latitude) + ", " + to_string(user.longitude) + ")...");

    if (availableDrivers.empty()) {
        log("No available drivers at the moment. Adding user " + userID + " to the ride request queue.");
        rideRequestQueue.push(userID);  // Log if no drivers are available
        return;
    }

    // Recalculate distances and find the nearest driver
    log("Recalculating distances to find the nearest driver for user " + userID + "...");
    priority_queue<pair<double, string>, vector<pair<double, string>>, CompareDistance> tempDrivers;

    while (!availableDrivers.empty()) {
        string driverID = availableDrivers.top().second;
        availableDrivers.pop();

        Drivers driver = driverDatabase[driverID];
        double distance = calculateDistance(user.latitude, user.longitude, driver.latitude, driver.longitude);

        // Log the distance calculation for each driver
        log("Calculated distance to driver " + driverID + ": " + to_string(distance) + " km.");

        tempDrivers.push({distance, driverID});
    }

    availableDrivers = tempDrivers;  // Update the available drivers with recalculated distances
    log("Finished recalculating distances.");

    // Assign the nearest driver
    if (!availableDrivers.empty()) {
        string nearestDriverID = availableDrivers.top().second;
        availableDrivers.pop();

        driverDatabase[nearestDriverID].available = false;
        log("Driver " + nearestDriverID + " assigned to user " + userID);  // Log the driver assignment
    } else {
        log("No drivers available after recalculating distances. Adding user " + userID + " to the queue.");
        rideRequestQueue.push(userID);  // If no drivers are available after recalculating
    }
}


void RideManager::processPendingRequests() {
    log("Processing pending requests...");

    while (!rideRequestQueue.empty() && !availableDrivers.empty()) {
        string userID = rideRequestQueue.front();
        rideRequestQueue.pop();

        log("Attempting to process ride request for user " + userID);

        requestRide(userID);  // Call requestRide to match and assign a driver

        // Log when we process each request
        log("Processed ride request for user " + userID);
    }

    if (rideRequestQueue.empty()) {
        log("No more pending requests to process.");
    } else {
        log("Some requests are still pending. Waiting for available drivers.");
    }
}


void RideManager::markDriverAvailable(const string& driverID) {
    log("Marking driver " + driverID + " as available.");

    if (driverDatabase.find(driverID) != driverDatabase.end()) {
        driverDatabase[driverID].available = true;
        availableDrivers.push({0.0, driverID});  // Add the driver back to the heap with a 0.0 distance for simplicity
        log("Driver " + driverID + " is now available.");
    } else {
        log("Driver not found: " + driverID);
    }
}

