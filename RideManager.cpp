#include "RideManager.h"
#include <cstring> // Include this header for strlen
#include <fstream> // Include this header for file operations

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

// Constructor to initialize socket
RideManager::RideManager() {
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8081 (changed from 8080)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8081); // Changed port number

    // Forcefully attaching socket to the port 8081
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

// Destructor to close socket
RideManager::~RideManager() {
    close(server_fd);
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

// Load drivers from file
void RideManager::loadDriversFromFile(const string& filename) {
    ifstream inFile(filename);
    string line;
    while (getline(inFile, line)) {
        istringstream iss(line);
        string driverID;
        double latitude, longitude;
        bool available;
        if (!(iss >> driverID >> latitude >> longitude >> available)) { break; }
        Drivers driver = {driverID, latitude, longitude, available};
        driverDatabase[driverID] = driver;
        if (driver.available) {
            availableDrivers.push({0.0, driverID});
        }
    }
    inFile.close();
    log("Drivers loaded from file successfully.");
}

// Load users from file
void RideManager::loadUsersFromFile(const string& filename) {
    ifstream inFile(filename);
    string line;
    while (getline(inFile, line)) {
        istringstream iss(line);
        string userID;
        double latitude, longitude;
        if (!(iss >> userID >> latitude >> longitude)) { break; }
        Users user = {userID, latitude, longitude};
        userDatabase[userID] = user;
    }
    inFile.close();
    log("Users loaded from file successfully.");
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

        // Notify the driver
        int clientSocket;
        struct sockaddr_in clientAddress;
        int clientAddrlen = sizeof(clientAddress);
        if ((clientSocket = accept(server_fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        handleRideRequest(clientSocket);
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

// Socket communication methods
void RideManager::startServer() {
    log("Server started, waiting for ride requests...");
    while (true) {
        int clientSocket;
        struct sockaddr_in clientAddress;
        int clientAddrlen = sizeof(clientAddress);
        if ((clientSocket = accept(server_fd, (struct sockaddr *)&clientAddress, (socklen_t*)&clientAddrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        handleRideRequest(clientSocket);
    }
}

void RideManager::handleRideRequest(int clientSocket) {
    char buffer[1024] = {0};
    read(clientSocket, buffer, 1024);
    cout << "Driver accepted the ride: " << buffer << endl;
    close(clientSocket);
}

void RideManager::acceptRide() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *message = "Ride accepted by driver";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "\n Socket creation error \n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081); // Changed port number

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cout << "\nInvalid address/ Address not supported \n";
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "\nConnection Failed \n";
        return;
    }
    send(sock, message, strlen(message), 0);
    cout << "Ride acceptance message sent\n";
    close(sock);
}

