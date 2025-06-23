#pragma once

#include <vector>
#include "note.hpp"

class Edge{
public:
  int dst; // The destination node
  Note* note; // The note that this edge is connected to
};

class Node {
public:
  int player; // The player this node is assigned to
  std::vector<Edge> edges; // The edges that are connected to this node
};

class Graph {
public:
  std::vector<Node> nodes; // Array of nodes

  Graph();
  std::vector<int> flatten();
};