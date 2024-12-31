#include "Location_Tracking.h"
#include "Traffic.h"
#include "RideManager.h"
#include "riderAndDriver.h"
#include <iostream>
#include <thread>
#include <cstdlib>   // For rand function
#include <ctime>     // For seeding rand
#include <iomanip>   // For setw and setfill
#include <vector>    // For std::vector in UserDriverHashTable
#include <limits>    // For std::numeric_limits
#include <string>    // For std::string
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// Define your Graph class
class Graph {
public:
    // Assuming these methods and members are already implemented:
    void addNode(int id, const string& location);
    void addEdge(int from, int to, double weight);
    void displayNodes();
    void dijkstra(int start, int end);
    
    //We can add more necessary methods here as per your requirement
};

// Function to print a centered text within a box
void printCenteredBox(const string& text) {
    int width = 80;
    int padding = (width - text.length()) / 2;
    cout << "+" << string(width, '-') << "+" << endl;
    cout << "|" << string(padding, ' ') << text << string(padding, ' ') << "|" << endl;
    cout << "+" << string(width, '-') << "+" << endl;
}

// Function to print a progress bar
void printProgressBar(int duration) {
    int width = 50;
    cout << "[";
    for (int i = 0; i < width; ++i) {
        cout << " ";
    }
    cout << "] 0%";
    cout.flush();
    for (int i = 0; i <= width; ++i) {
        this_thread::sleep_for(chrono::milliseconds(duration / width));
        cout << "\r[";
        for (int j = 0; j < i; ++j) {
            cout << "=";
        }
        for (int j = i; j < width; ++j) {
            cout << " ";
        }
        cout << "] " << setw(3) << (i * 100 / width) << "%";
        cout.flush();
    }
    cout << endl;
}

// Function to print a header
void printHeader(const string& header) {
    cout << "+" << string(78, '-') << "+" << endl;
    cout << "|" << setw(40 + header.length() / 2) << header << setw(39 - header.length() / 2) << "|" << endl;
    cout << "+" << string(78, '-') << "+" << endl;
}

// Function to print a footer
void printFooter() {
    cout << "+" << string(78, '-') << "+" << endl;
}

// Function to display users in table form
void displayUsersTable(const UserDriverHashTable& system) {
    cout << "+" << string(20, '-') << "+" << string(20, '-') << "+" << endl;
    cout << "| " << left << setw(18) << "Username" << " | " << setw(18) << "Phone Number" << " |" << endl;
    cout << "+" << string(20, '-') << "+" << string(20, '-') << "+" << endl;
    for (const auto &bucket : system.getUserTable()) {
        for (const auto &user : bucket) {
            cout << "| " << left << setw(18) << user.userName << " | " << setw(18) << user.userPhoneNo << " |" << endl;
        }
    }
    cout << "+" << string(20, '-') << "+" << string(20, '-') << "+" << endl;
}

// Function to display drivers in table form
void displayDriversTable(const UserDriverHashTable& system) {
    cout << "+" << string(20, '-') << "+" << string(20, '-') << "+" << string(20, '-') << "+" << endl;
    cout << "| " << left << setw(18) << "Driver Name" << " | " << setw(18) << "Phone Number" << " | " << setw(18) << "Vehicle Model" << " |" << endl;
    cout << "+" << string(20, '-') << "+" << string(20, '-') << "+" << string(20, '-') << "+" << endl;
    for (const auto &bucket : system.getDriverTable()) {
        for (const auto &driver : bucket) {
            cout << "| " << left << setw(18) << driver.driverName << " | " << setw(18) << driver.driverPhoneNo << " | " << setw(18) << driver.vehicleModel << " |" << endl;
        }
    }
    cout << "+" << string(20, '-') << "+" << string(20, '-') << "+" << string(20, '-') << "+" << endl;
}

// Function to handle real-time communication between user and driver
void realTimeCommunication(const string& userName, const string& driverName) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8082
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8082);

    // Forcefully attaching socket to the port 8082
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << "Waiting for driver to connect..." << endl;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    cout << "Driver connected. You can now chat in real-time." << endl;
    while (true) {
        cout << userName << ": ";
        string message;
        getline(cin, message);
        send(new_socket, message.c_str(), message.length(), 0);
        if (message == "exit") break;

        read(new_socket, buffer, 1024);
        cout << driverName << ": " << buffer << endl;
        if (string(buffer) == "exit") break;
    }

    close(new_socket);
    close(server_fd);
}

// Function to dynamically show the map/grid to the user and driver
void showDynamicMap() {
    while (true) {
        updateGrid();
        printGrid();
        this_thread::sleep_for(chrono::seconds(1)); // Update every second
    }
}

int main()
{
    printCenteredBox("Welcome to Smart Ride");
    printCenteredBox("Smarter Rides");

    printProgressBar(2000);

    UserDriverHashTable system;
    RideManager manager;
    Graph2 g;

    system.loadFromFile("users.txt", "drivers.txt");

    bool userLoggedIn = false;  // Flag to check if the user has logged in
    bool driverLoggedIn = false; // Flag to check if the driver has logged in
    string loggedInUserName;     // Store the logged-in username
    string loggedInDriverName;   // Store the logged-in driver's name

    int choice;
    do
    {
        printHeader("Main Menu");
        std::cout << "\n1. Register User\n2. Register Driver\n3. Log in User\n4. Log in Driver\n5. Display Users\n6. Display Drivers\n7. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        printFooter();

        // Check for invalid input
        if (std::cin.fail()) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the invalid input
            std::cout << "Invalid input. Please enter a number between 1 and 7." << std::endl;
            continue; // Ask for input again
        }

        switch (choice)
        {
        case 1:
            system.insertUser();
            system.saveToFile("users.txt", "drivers.txt"); // Save data to file
            break;
        case 2:
            system.insertDriver();
            system.saveToFile("users.txt", "drivers.txt"); // Save data to file
            break;
        case 3:
        {
            std::string userName, password;
            std::cout << "Enter username: ";
            std::cin >> userName;
            std::cout << "Enter password: ";
            std::cin >> password;

            if (system.logInUser(userName, password))  // Assuming logInUser returns a bool indicating success
            {
                userLoggedIn = true;
                loggedInUserName = userName;  // Store the logged-in user's name
                std::cout << "Login successful!" << std::endl;

                // Exit the loop once the user is logged in
                choice = 7;  // Set choice to 7 to break out of the loop
            }
            else
            {
                std::cout << "Invalid username or password. Please try again." << std::endl;
            }
            break;
        }
        case 4:
        {
            std::string driverName, password;
            std::cout << "Enter driver username: ";
            std::cin >> driverName;
            std::cout << "Enter driver password: ";
            std::cin >> password;

            if (system.logInDriver(driverName, password))  // Assuming logInDriver returns a bool indicating success
            {
                driverLoggedIn = true;
                loggedInDriverName = driverName;  // Store the logged-in driver's name
                std::cout << "Driver login successful!" << std::endl;

                // Exit the loop once the driver is logged in
                choice = 7;  // Set choice to 7 to break out of the loop
            }
            else
            {
                std::cout << "Invalid driver username or password. Please try again." << std::endl;
            }
            break;
        }
        case 5:
            displayUsersTable(system);
            break;
        case 6:
            displayDriversTable(system);
            break;
        case 7:
            system.saveToFile("users.txt", "drivers.txt");
            std::cout << "Exiting system." << std::endl;
            break;
        default:
            std::cout << "Invalid choice, please try again." << std::endl;
            break;
        }

    } while (choice != 7 && !userLoggedIn && !driverLoggedIn);  // Loop continues until the user or driver is logged in or 'Exit' is selected

    // Once the user is logged in successfully, continue with user options
    if (userLoggedIn)
    {
        bool userSessionActive = true;  // Flag to keep the user session active
        bool rideInProgress = false;    // Flag to check if a ride is in progress

        // While loop for user options after login
        while (userSessionActive)
        {
            printHeader("User Menu");
            std::cout << "\n1. View Profile\n2. Request Ride\n3. Update Profile\n4. Back to Main Menu\n";
            std::cout << "Enter your choice: ";
            std::cin >> choice;
            printFooter();

            // Check for invalid input
            if (std::cin.fail()) {
                std::cin.clear(); // Clear the error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the invalid input
                std::cout << "Invalid input. Please enter a number between 1 and 4." << std::endl;
                continue; // Ask for input again
            }

            switch (choice)
            {
            case 1:
                // View Profile
                for (auto &bucket : system.getUserTable())
                {
                    for (auto &user : bucket)
                    {
                        if (user.userName == loggedInUserName)
                        {
                            user.displayUser();
                            break;
                        }
                    }
                }
                break;
            case 2:
                // Request Ride
                {
                    // Adding some nodes (locations)
                    g.addNode(1, "NUST Hostels");
                    g.addNode(2, "NUST Gate 1");
                    g.addNode(3, "NUST Gate 2");
                    g.addNode(4, "Bus Stop 26");
                    g.addNode(5, "F-6 Markaz");
                    g.addNode(6, "F-10 Markaz");

                    // Adding weighted edges (from, to, weight)
                    g.addEdge(1, 2, 3.0);
                    g.addEdge(1, 3, 2.0);
                    g.addEdge(2, 1, 3.0);
                    g.addEdge(2, 3, 2.0);
                    g.addEdge(2, 4, 20.0);
                    g.addEdge(2, 5, 25.0);
                    g.addEdge(2, 6, 15.0);
                    g.addEdge(3, 1, 2.0);
                    g.addEdge(3, 2, 2.0);
                    g.addEdge(3, 4, 15.0);
                    g.addEdge(3, 5, 35.0);
                    g.addEdge(3, 6, 25.0);
                    g.addEdge(4, 2, 20.0);
                    g.addEdge(4, 3, 17.0);
                    g.addEdge(4, 5, 45.0);
                    g.addEdge(4, 6, 35.0);
                    g.addEdge(5, 2, 25.0);
                    g.addEdge(5, 3, 28.0);
                    g.addEdge(5, 4, 45.0);
                    g.addEdge(5, 6, 15.0);
                    g.addEdge(6, 2, 15.0);
                    g.addEdge(6, 3, 18.0);
                    g.addEdge(6, 4, 35.0);
                    g.addEdge(6, 5, 15.0);

                    // Displaying the list of locations with IDs
                    g.displayNodes();

                    // Getting start and end locations from the user
                    int start, end;
                    cout << "Enter pick-up location: ";
                    cin >> start;
                    cout << "Enter your destination: ";
                    cin >> end;

                    srand(time(0));
                    generateNonOverlappingPositions(obstacles, GRID_SIZE / 2);
                    generateNonOverlappingPositions(trafficSignals, GRID_SIZE / 4);
                    generateNonOverlappingPositions(fuelStations, GRID_SIZE / 5);
                    generateNonOverlappingPositions(congestionZones, GRID_SIZE / 4);
                    startSimulation();

                    // Find and display the shortest route
                    g.dijkstra(start, end);
                    rideInProgress = true;  // Set ride in progress flag

                    // Request ride through RideManager
                    manager.requestRide(loggedInUserName);

                    // Start a thread to show the dynamic map
                    thread mapThread(showDynamicMap);
                    mapThread.detach();
                }
                break;
            case 3:
                // Update Profile
                for (auto &bucket : system.getUserTable())
                {
                    for (auto &user : bucket)
                    {
                        if (user.userName == loggedInUserName)
                        {
                            const_cast<User&>(user).editUserInfo(system.getUserTable());
                            system.saveToFile("users.txt", "drivers.txt"); // Save data to file
                            break;
                        }
                    }
                }
                break;
            case 4:
                std::cout << "Returning to main menu." << std::endl;
                userSessionActive = false;  // End the session
                userLoggedIn = false;       // Set userLoggedIn to false to go back to main menu
                break;
            default:
                std::cout << "Invalid choice, please try again." << std::endl;
                break;
            }
        }
    }

    // Once the driver is logged in successfully, continue with driver options
    if (driverLoggedIn)
    {
        std::cout << loggedInDriverName << ", you are now waiting for ride requests.\n";
        
        bool driverSessionActive = true;  // Flag to keep the driver's session active
        
        // Driver options after login
        while (driverSessionActive)
        {
            printHeader("Driver Menu");
            std::cout << "\n1. View Profile\n2. Update Profile\n3. Back to Main Menu\n";
            std::cout << "Enter your choice: ";
            std::cin >> choice;
            printFooter();

            // Check for invalid input
            if (std::cin.fail()) {
                std::cin.clear(); // Clear the error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the invalid input
                std::cout << "Invalid input. Please enter a number between 1 and 3." << std::endl;
                continue; // Ask for input again
            }

            switch (choice)
            {
            case 1:
                // View Profile
                for (auto &bucket : system.getDriverTable())
                {
                    for (auto &driver : bucket)
                    {
                        if (driver.driverName == loggedInDriverName)
                        {   
                            driver.displayDriver();
                            break;
                        }
                    }
                }
                break;
            case 2:
                // Update Profile
                for (auto &bucket : system.getDriverTable())
                {
                    for (auto &driver : bucket)
                    {
                        if (driver.driverName == loggedInDriverName)
                        {
                            const_cast<Driver&>(driver).editDriverInfo(system.getDriverTable());
                            system.saveToFile("users.txt", "drivers.txt"); // Save data to file
                            break;
                        }
                    }
                }
                break;
            case 3:
                std::cout << "Returning to main menu." << std::endl;
                driverSessionActive = false;  // End the driver's session
                driverLoggedIn = false;       // Set driverLoggedIn to false to go back to main menu
                break;
            default:
                std::cout << "Invalid choice, please try again." << std::endl;
                break;
            }
        }
    }

    if (userLoggedIn && driverLoggedIn) {
        realTimeCommunication(loggedInUserName, loggedInDriverName);
        thread mapThread(showDynamicMap);
        mapThread.detach();
    }

    system.saveToFile("users.txt", "drivers.txt");
    return 0;
}
