#include "gen.hpp"
#include "graph.hpp"

/**
 * @brief Constructor for player adjacency graph
 */
Graph::Graph() {
  nodes = std::vector<Node>(cfg->num_players);
  for (int i = 0; i < cfg->num_players; i++) {
    nodes[i].player = i;
    nodes[i].edges = std::vector<Edge>();
  }
}

/**
 * @brief Flattens the graph to an array of player indices representing the player seating chart.
 */
std::vector<int> Graph::flatten() {
  std::vector<int> flattened_graph(cfg->num_players);
  for (int i = 0; i < cfg->num_players; i++)
  {
    flattened_graph[i] = random_player();
  }
  return flattened_graph;
}