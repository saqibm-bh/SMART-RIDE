#include "Location_Tracking.h"
#include <windows.h> // For Sleep function
#include <cmath>     // For abs function
#include <cstdlib>   // For rand function
#include <ctime>     // For seeding rand
#include <algorithm> // For reverse function
#include <iostream>

// Initialize grid and other variables
char grid[GRID_SIZE][GRID_SIZE];
std::pair<int, int> userPos = {GRID_SIZE - 1, GRID_SIZE - 1};   // Initial user position
std::vector<std::pair<int, int>> driverPos = {{0, 0}, {0, GRID_SIZE - 1}, {GRID_SIZE - 1, 0}};
std::vector<std::string> driverNames = {"John Doe", "Alice Smith", "Bob Lee"};
std::vector<std::string> carModels = {"Toyota Corolla", "Honda Civic", "Ford Focus"};
std::vector<int> driverFuel = {30, 25, 40};
std::vector<std::pair<int, int>> obstacles;
std::vector<std::pair<int, int>> trafficSignals;
std::vector<std::pair<int, int>> fuelStations;
std::vector<std::pair<int, int>> congestionZones;

// Utility function to clear the console screen
void clearConsole() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = {0, 0};
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;
    DWORD written;

    // Get the console screen size
    GetConsoleScreenBufferInfo(hConsole, &screenInfo);
    DWORD consoleSize = screenInfo.dwSize.X * screenInfo.dwSize.Y;

    // Fill the console with blank spaces
    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, topLeft, &written);
    FillConsoleOutputAttribute(hConsole, screenInfo.wAttributes, consoleSize, topLeft, &written);
    SetConsoleCursorPosition(hConsole, topLeft);
#else
    std::cout << "\033[2J\033[1;1H"; // Use ANSI escape codes for Linux/Mac
#endif
}

// Function to generate random non-overlapping positions
void generateNonOverlappingPositions(std::vector<std::pair<int, int>> &positions, int count) {
    while (positions.size() < count) {
        std::pair<int, int> pos = {rand() % GRID_SIZE, rand() % GRID_SIZE};
        if (std::find(positions.begin(), positions.end(), pos) == positions.end() && pos != userPos) {
            positions.push_back(pos);
        }
    }
}

// Function to print the grid
void printGrid() {
    clearConsole();
    std::cout << "+" << std::string(GRID_SIZE * 2, '-') << "+" << std::endl;
    for (int i = 0; i < GRID_SIZE; i++) {
        std::cout << "|";
        for (int j = 0; j < GRID_SIZE; j++) {
            std::cout << grid[i][j] << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "+" << std::string(GRID_SIZE * 2, '-') << "+" << std::endl;
}

// Function to update the grid
void updateGrid() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '.'; // Empty space
        }
    }

    grid[userPos.first][userPos.second] = 'U'; // User position
    for (int i = 0; i < driverPos.size(); i++) {
        grid[driverPos[i].first][driverPos[i].second] = static_cast<char>('A' + i); // Driver positions
    }

    for (auto obstacle : obstacles) {
        grid[obstacle.first][obstacle.second] = '#'; // Obstacles
    }
    for (auto signal : trafficSignals) {
        grid[signal.first][signal.second] = 'T'; // Traffic signals
    }
    for (auto station : fuelStations) {
        grid[station.first][station.second] = 'F'; // Fuel stations
    }
    for (auto zone : congestionZones) {
        grid[zone.first][zone.second] = 'R'; // Congestion zones
    }
}

// Function to calculate the Manhattan distance between two points
int calculateDistance(std::pair<int, int> a, std::pair<int, int> b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

// Function to find the shortest path using BFS
std::vector<std::pair<int, int>> findShortestPath(std::pair<int, int> start, std::pair<int, int> end) {
    std::vector<std::vector<bool>> visited(GRID_SIZE, std::vector<bool>(GRID_SIZE, false));
    std::map<std::pair<int, int>, std::pair<int, int>> parent;
    std::queue<std::pair<int, int>> q;

    q.push(start);
    visited[start.first][start.second] = true;

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    while (!q.empty()) {
        std::pair<int, int> current = q.front();
        q.pop();

        if (current == end) {
            std::vector<std::pair<int, int>> path;
            while (current != start) {
                path.push_back(current);
                current = parent[current];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int i = 0; i < 4; i++) {
            int nx = current.first + dx[i];
            int ny = current.second + dy[i];
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE && !visited[nx][ny] && grid[nx][ny] != '#') {
                visited[nx][ny] = true;
                parent[{nx, ny}] = current;
                q.push({nx, ny});
            }
        }
    }
    return {}; // No path found
}

// Function to find the nearest fuel station
std::pair<int, int> findNearestFuelStation(std::pair<int, int> driver) {
    std::pair<int, int> nearestStation;
    int minDistance = INT_MAX;

    for (auto station : fuelStations) {
        int distance = calculateDistance(driver, station);
        if (distance < minDistance) {
            minDistance = distance;
            nearestStation = station;
        }
    }
    return nearestStation;
}

// Function to move the driver to the user
void moveDriverToUser(int driverIndex) {
    while (true) {
        std::vector<std::pair<int, int>> path = findShortestPath(driverPos[driverIndex], userPos);
        
        // If no path is found, stop the simulation
        if (path.empty()) {
            std::cout << "\nNo valid path found for " << driverNames[driverIndex] << ". Simulation ends.\n";
            break;
        }

        for (size_t i = 1; i < path.size(); i++) {
            driverPos[driverIndex] = path[i];
            driverFuel[driverIndex]--; // Decrease fuel as the driver moves

            updateGrid();
            printGrid();
            std::cout << "\nDriver " << driverNames[driverIndex] << " is en route...\n";
            Sleep(500); // Simulate movement delay

            // Check if the driver has reached the user
            if (driverPos[driverIndex] == userPos) {
                std::cout << "\nDriver " << driverNames[driverIndex] << " has arrived at your location!\n";
                std::cout << "Simulation ends.\n";
                return; // End the simulation once the driver reaches the user
            }
        }
    }
}

// Function to display available drivers
void displayDrivers() {
    std::cout << "Available Drivers:\n";
    for (size_t i = 0; i < driverPos.size(); i++) {
        std::cout << i + 1 << ". " << driverNames[i]
             << " (Car: " << carModels[i]
             << ", Fuel: " << driverFuel[i]
             << ", Location: (" << driverPos[i].first << ", " << driverPos[i].second << ")"
             << ", ETA: " << calculateDistance(driverPos[i], userPos) << ")\n";
    }
}

// Function to print the legend
void printLegend() {
    std::cout << "\nLegend:\n";
    std::cout << "U : User\n";
    for (size_t i = 0; i < driverNames.size(); i++) {
        std::cout << static_cast<char>('A' + i) << " : " << driverNames[i] << "\n";
    }
    std::cout << "# : Obstacles\n";
    std::cout << "T : Traffic Signal\n";
    std::cout << "F : Fuel Station\n";
    std::cout << "R : Congestion Zone\n";
    std::cout << "==================\n";
}

// Function to start the simulation
void startSimulation() {
    updateGrid();
    printGrid();
    printLegend();
    displayDrivers();

    int choice;
    std::cout << "Enter the number of the driver you want to select: ";
    std::cin >> choice;
    choice--;

    if (choice >= 0 && choice < driverPos.size()) {
        std::cout << "You selected " << driverNames[choice] << ".\n";
        moveDriverToUser(choice);
    } else {
        std::cout << "Invalid choice.\n";
    }
}
