#include <iostream>
#include <vector>
#include <map>
#include <climits>
#include <set>
#include <iomanip>

using namespace std;

struct Edge
{
    int src, dest, cost;
};

// Structure to represent a node's routing table
struct RoutingTable
{
    vector<vector<int>> distances;  // distances[i][j] is node i's estimate to reach j
    vector<vector<bool>> neighbors; // tracks direct neighbors

    RoutingTable(int N) : distances(N, vector<int>(N, INT_MAX)),
                          neighbors(N, vector<bool>(N, false))
    {
        for (int i = 0; i < N; ++i)
        {
            distances[i][i] = 0; // Distance to self is 0
        }
    }
};

// Prints the routing table for each node and incorporting INF for INT_MAX
void printRoutingTable(int N, const RoutingTable &table, int iteration)
{
    cout << "\n=== Iteration " << iteration << " ===\n";

    cout << "Node |";
    for (int i = 0; i < N; ++i)
    {
        cout << setw(5) << "N" << (i + 1);
    }
    cout << "\n-----";
    for (int i = 0; i < N; ++i)
    {
        cout << "-----";
    }
    cout << "\n";

    for (int i = 0; i < N; ++i)
    {
        cout << "N" << (i + 1) << "  |";
        for (int j = 0; j < N; ++j)
        {
            if (table.distances[i][j] == INT_MAX)
            {
                cout << setw(5) << "INF";
            }
            else if (table.distances[i][j] >= 100)
            {
                // Also handle large values more cleanly
                cout << setw(5) << "100+";
            }
            else
            {
                cout << setw(5) << table.distances[i][j];
            }
        }
        cout << "\n";
    }
    cout << "\n";
}

// Initialises the routing table at the start of the DVR algo
RoutingTable initializeRoutingTable(int N, const vector<Edge> &edges)
{
    RoutingTable table(N);

    // Initialize direct connections
    for (const auto &edge : edges)
    {
        int src = edge.src - 1;
        int dest = edge.dest - 1;
        table.distances[src][dest] = edge.cost;
        table.distances[dest][src] = edge.cost;
        table.neighbors[src][dest] = true;
        table.neighbors[dest][src] = true;
    }

    return table;
}

// Implements true distributed Bellman-Ford
RoutingTable distanceVectorRouting(int N, const vector<Edge> &edges, bool printTables = true)
{
    RoutingTable table = initializeRoutingTable(N, edges);

    if (printTables)
    {
        cout << "\nInitial State:";
        printRoutingTable(N, table, 0);
    }

    bool updated;
    int iterations = 0;
    const int MAX_ITERATIONS = 100;

    do
    {
        updated = false;
        iterations++;

        // Create a copy of current distances for parallel updates
        vector<vector<int>> newDistances = table.distances;

        // For each node
        for (int node = 0; node < N; ++node)
        {
            // For each destination
            for (int dest = 0; dest < N; ++dest)
            {
                if (node != dest)
                {
                    // Get minimum distance through neighbors only
                    int minDist = INT_MAX;

                    // Check only direct neighbors
                    for (int neighbor = 0; neighbor < N; ++neighbor)
                    {
                        if (table.neighbors[node][neighbor])
                        { // Only if directly connected
                            if (table.distances[neighbor][dest] != INT_MAX)
                            {
                                int throughNeighbor = table.distances[node][neighbor] +
                                                      table.distances[neighbor][dest];
                                minDist = min(minDist, throughNeighbor);
                            }
                        }
                    }

                    // Update only if we found a better path
                    if (minDist < table.distances[node][dest])
                    {
                        newDistances[node][dest] = minDist;
                        updated = true;
                    }
                }
            }
        }

        // Update all distances at once (simulating parallel updates)
        table.distances = newDistances;

        if (printTables)
        {
            printRoutingTable(N, table, iterations);
        }

        // We are limiting the computation of table till 100 only
        if (iterations >= MAX_ITERATIONS)
        {
            cout << "\nWarning: Stopped after " << MAX_ITERATIONS
                 << " iterations - possible count-to-infinity problem.\n";
            break;
        }

    } while (updated);

    cout << "\nConverged after " << iterations << " iterations.\n";
    return table;
}

int main()
{
    int N, M;
    cout << "Enter number of nodes (routers): ";
    cin >> N;
    cout << "Enter number of edges (links): ";
    cin >> M;

    vector<Edge> edges(M);
    cout << "Enter each edge as 'source destination cost':\n";
    for (int i = 0; i < M; ++i)
    {
        cin >> edges[i].src >> edges[i].dest >> edges[i].cost;
    }

    cout << "\n=== Initial Network State ===";
    RoutingTable routingTable = distanceVectorRouting(N, edges);

    return 0;
}