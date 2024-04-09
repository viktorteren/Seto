/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include "../include/Label_splitting_module.h"
#include "../include/Merging_Minimal_Preregions_module.h"
#include "../include/Regions_generator.h"
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Merge.h"
#include "../include/PN_decomposition.h"
#include "../include/GreedyRemoval.h"
#include "../include/FCPN_Merge.h"
#include "../include/SM_composition.h"
#include <algorithm>
#include <iomanip>
#if __has_include(<Python.h>)
#include <Python.h>
#endif

#include <cuddObj.hh>

string parseArguments(int argc, vector<string> args);

