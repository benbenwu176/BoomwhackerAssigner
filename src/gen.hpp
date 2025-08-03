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
#include "assignment.hpp"
#include "boomwhacker.hpp"
#include "graph.hpp"
#include "note.hpp"
#include "player.hpp"
#include "config.hpp"
#include "globals.hpp"

extern Config* cfg;
extern Assignment* assignment;

#endif  // GEN_H