#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <climits>
#include <stack>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// Node structure to represent each location
struct Node {
    string name;
};

// Edge structure to represent a connection between two nodes
struct Edge {
    int to;        // Destination node
    double weight; // Travel time or distance
    double congestion; // Congestion factor for adjusting the weight
};

// Graph class to represent the graph and its functionalities
class Graph2 {
public:
    unordered_map<int, Node> nodes;               // Map of nodes (id -> Node)
    unordered_map<int, vector<Edge>> adj_list;     // Adjacency list for the graph

    void addNode(int id, string name);            // Add a node
    void addEdge(int from, int to, double weight); // Add an edge
    void updateCongestion(int from, int to, double congestion); // Update congestion
    void dijkstra(int start, int end);             // Dijkstra's algorithm to find the shortest path
    void displayNodes();                          // Display all nodes (locations)
    void notifyDriver();                          // Notify driver
};

#endif
