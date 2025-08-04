#ifndef GEN_H
#define GEN_H

#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include <random>
#include <iostream>
#include "error_handler.hpp"
#include "random_utils.hpp"
#include <fstream>
#include <cstdint>
#include "assignment.hpp"
#include "boomwhacker.hpp"
#include "graph.hpp"
#include "note.hpp"
#include "player.hpp"
#include "config.hpp"
#include "globals.hpp"
#include <filesystem>
#include "json.hpp"

extern Config* cfg;
extern Assignment* assignment;

#endif  // GEN_H