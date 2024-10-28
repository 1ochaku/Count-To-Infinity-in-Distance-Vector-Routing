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

struct RoutingTable
{
    vector<vector<int>> distances;
    vector<vector<bool>> neighbors;

    RoutingTable(int N) : distances(N, vector<int>(N, INT_MAX)),
                          neighbors(N, vector<bool>(N, false))
    {
        for (int i = 0; i < N; ++i)
        {
            distances[i][i] = 0;
        }
    }
};

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

RoutingTable handleLinkFailure(int N, RoutingTable &table, int failSrc, int failDest,
                               bool printTables = true)
{
    failSrc--; // Convert to 0-based indexing
    failDest--;

    cout << "\n=== Simulating Link Failure between Node " << (failSrc + 1)
         << " and Node " << (failDest + 1) << " ===\n";

    // Mark the failed link
    table.distances[failSrc][failDest] = INT_MAX;
    table.distances[failDest][failSrc] = INT_MAX;
    table.neighbors[failSrc][failDest] = false;
    table.neighbors[failDest][failSrc] = false;

    if (printTables)
    {
        cout << "\nState after link failure:";
        printRoutingTable(N, table, 0);
    }

    bool updated;
    int iterations = 0;
    const int MAX_ITERATIONS = 100;
    const int INFINITY_THRESHOLD = 100; // Threshold for count-to-infinity detection

    // Store node pairs showing count-to-infinity
    vector<pair<int, int>> countToInfinityPairs;

    do
    {
        updated = false;
        iterations++;
        vector<vector<int>> newDistances = table.distances;

        // For each node
        for (int node = 0; node < N; ++node)
        {
            // For each destination
            for (int dest = 0; dest < N; ++dest)
            {
                if (node != dest)
                {
                    int oldDist = table.distances[node][dest];
                    int minDist = INT_MAX;

                    // Check only direct neighbors
                    for (int neighbor = 0; neighbor < N; ++neighbor)
                    {
                        if (table.neighbors[node][neighbor])
                        {
                            if (table.distances[neighbor][dest] != INT_MAX)
                            {
                                int throughNeighbor = table.distances[node][neighbor] +
                                                      table.distances[neighbor][dest];
                                minDist = min(minDist, throughNeighbor);
                            }
                        }
                    }

                    if (minDist != oldDist)
                    {
                        newDistances[node][dest] = minDist;
                        updated = true;

                        // Print distance updates for debugging
                        if (minDist > oldDist && oldDist != INT_MAX)
                        {
                            cout << "\nNode " << (node + 1) << " updated its distance to Node "
                                 << (dest + 1) << ":\n";
                            cout << "Previous distance: " << oldDist << "\n";
                            cout << "New distance: " << minDist << "\n";
                        }
                    }
                }
            }
        }

        table.distances = newDistances;

        if (printTables)
        {
            printRoutingTable(N, table, iterations);
        }

        if (iterations >= MAX_ITERATIONS)
        {
            cout << "\nStopped after " << MAX_ITERATIONS << " iterations.\n";
            break;
        }

    } while (updated);

    // Analyze final distance vector table for count-to-infinity
    cout << "\n=== Count-to-Infinity Analysis ===\n";

    // Check each pair of nodes in the final distance table
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i != j && table.distances[i][j] != INT_MAX && table.distances[i][j] >= INFINITY_THRESHOLD)
            {
                countToInfinityPairs.push_back({i, j});
            }
        }
    }

    // Display results
    if (countToInfinityPairs.empty())
    {
        cout << "No count-to-infinity problems detected (threshold: " << INFINITY_THRESHOLD << ").\n";
    }
    else
    {
        cout << "Nodes showing count-to-infinity pattern (threshold: " << INFINITY_THRESHOLD << "):\n";
        for (const auto &pair : countToInfinityPairs)
        {
            cout << "Node " << (pair.first + 1) << " to Node " << (pair.second + 1)
                 << " (Current distance: " << table.distances[pair.first][pair.second] << ")\n";
        }
    }

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

    int failSrc, failDest;
    cout << "\nEnter the link to fail (source destination): ";
    cin >> failSrc >> failDest;

    RoutingTable finalTable = handleLinkFailure(N, routingTable, failSrc, failDest);

    return 0;
}