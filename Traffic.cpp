/*
Ali Safdar Saeed
Task 08 of Smart Ride: Real-time Traffic Updates and Route Optimization

In this task, I will add a graph to represent the real world locations and routes. At
each edge, I included the properties like way, average time from start to end (weight),
and congestion that will affect the weight of the edge.
I used Dijkstra's algorithm to find the shortest path.

*/

#include "Traffic.h"
#include <iostream>
#include <stack>
#include <queue>
#include <climits>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring> // Include this header for strlen

using namespace std;

// Add a node with a given id and name
void Graph2::addNode(int id, string name) {
    nodes[id] = {name};
}

// Add an edge from one node to another with a specified weight
void Graph2::addEdge(int from, int to, double weight) {
    adj_list[from].push_back({to, weight, 1.0}); // Default congestion is 1.0
}

// Update the congestion on a specific edge and adjust the weight accordingly
void Graph2::updateCongestion(int from, int to, double congestion) {
    for (auto &edge : adj_list[from]) {
        if (edge.to == to) {
            edge.congestion = congestion;
            edge.weight *= congestion; // Adjust weight based on congestion
        }
    }
}

// Dijkstra's algorithm to find the shortest path from start to end
void Graph2::dijkstra(int start, int end) {
    unordered_map<int, double> dist; // Distance from start to each node
    unordered_map<int, int> prev;    // Predecessor for path reconstruction
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq; // Min-heap

    // Initialize distances to infinity and predecessors to -1
    for (const auto& node : nodes) {
        dist[node.first] = INT_MAX;
        prev[node.first] = -1;
    }
    dist[start] = 0; // Distance to start node is 0
    pq.push({0, start}); // Start with the source node

    while (!pq.empty()) {
        int u = pq.top().second; // Current node
        double u_dist = pq.top().first;
        pq.pop();

        if (u_dist > dist[u]) continue; // Skip if this distance is not optimal

        // Explore all neighbors of the current node
        for (const auto& edge : adj_list[u]) {
            int v = edge.to;
            double weight = edge.weight;
            // Relaxation step: update distance and predecessor if a shorter path is found
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    // Reconstruct the path from start to end
    stack<int> path;
    for (int at = end; at != -1; at = prev[at]) {
        path.push(at);
    }

    // Display the result
    if (dist[end] == INT_MAX) {
        cout << "No path found from " << nodes[start].name << " to " << nodes[end].name << endl;
        return;
    }
    cout << "Shortest path from " << nodes[start].name << " to " << nodes[end].name << " is: ";
    while (!path.empty()) {
        cout << nodes[path.top()].name;
        path.pop();
        if (!path.empty()) cout << " -> ";
    }
    cout << endl;
    cout << "Expected Time: " << dist[end] << " Minutes" << endl;
}

// Display all nodes (locations) in the graph
void Graph2::displayNodes() {
    cout << "Available Locations where we are operating:" << endl;
    for (const auto& node : nodes) {
        cout << "    " << node.first << ": " << node.second.name << endl;
    }
}

// Notify driver with real-time traffic updates
void Graph2::notifyDriver() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *message = "Traffic update: Congestion ahead";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8082);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        return;
    }
    send(sock, message, strlen(message), 0);
    std::cout << "Traffic update message sent\n";
    close(sock);
}
