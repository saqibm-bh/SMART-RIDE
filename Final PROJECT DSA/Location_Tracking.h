#ifndef SIMULATION_H
#define SIMULATION_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <climits>

// Constants and Grid Dimensions
const int GRID_SIZE = 20; // Increased grid size
extern char grid[GRID_SIZE][GRID_SIZE];
extern std::pair<int, int> userPos;
extern std::vector<std::pair<int, int>> driverPos;
extern std::vector<std::string> driverNames;
extern std::vector<std::string> carModels;
extern std::vector<int> driverFuel;
extern std::vector<std::pair<int, int>> obstacles;
extern std::vector<std::pair<int, int>> trafficSignals;
extern std::vector<std::pair<int, int>> fuelStations;
extern std::vector<std::pair<int, int>> congestionZones;

// Function Declarations
void clearConsole();
void printGrid();
void updateGrid();
int calculateDistance(std::pair<int, int> a, std::pair<int, int> b);
std::vector<std::pair<int, int>> findShortestPath(std::pair<int, int> start, std::pair<int, int> end);
void moveDriverToUser(int driverIndex);
void generateNonOverlappingPositions(std::vector<std::pair<int, int>> &positions, int count);
void displayDrivers();
void printLegend();
void startSimulation();
std::pair<int, int> findNearestFuelStation(std::pair<int, int> driver);

#endif // SIMULATION_H
