Software for PN creation based on Regions theory (TStoPN)
========================

Part of code is based on PBLib (a fork of MiniSAT 2.2), NetworkX and CUDD package.

Tested on Ubuntu 22.04.1 LTS

# Required software (and how install it):
- g++
```Bash
sudo apt install g++
```
- cmake
```Bash
sudo apt install cmake
```

- python 3.10
```Bash
sudo apt install python3.10
```

- pip
```Bash
sudo apt install python3-pip
```

- networkx
```Bash
pip install networkx
```

- boost (necessary for the installation of mCRL2)
```Bash
sudo apt-get install libboost-all-dev
```

- mCRL2 (Optional): install in order to allow to check the decomposition correctness for FCPNs
```Bash
sudo apt-get update
sudo apt-get install mcrl2
```

# Key principles:
- Creation of Petri Nets from Transition Systems
- Decomposition of Transition Systems into sets of  interacting State Machines
- Decomposition of Transition Systems into sets of interacting FCPNs

# Supported extensions

Input extensions: .g .ts

Output extensions: .dot .g .aut

# Building

Enter to the project folder and execute the following instructions:

```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make TS_splitter
```

# Execution



## a) TS to PN flow

[//]: # "Quick way (requirese graphviz library) with .ps file creation"

[//]: # "```Bash"
[//]: # "./execute.sh"
[//]: # "```"

[//]: # "### Simple way"

```Bash
./TS_splitter <file_path> PN <optional_flags>
```

#### Optional flags:

**D**: debug mode

**S**: verbose info mode

**O**: write output PN on file in .dot extension

**--INFO**: base info related to the time for the generation of the regions are shown, the same for N. of transitions, states, regions etc.

## b) TS to Synchronized SMs flow

```Bash
./TS_splitter <file_path> SM <optional_flags>
```

#### Optional flags:

[//]: # "L: creation of a log file"

**BDD**: usage of a BDD for the contemporary search of k SMs instead of sequentially search new SMs

**D**: debug info mode

**O**: write output SMs on file in .dot extension

**G**: write output SMs on file in .g extension

**-ALL**: execute the decomposition using an exact algorithm to find all minimal independent sets (not only the minimum required set to satisfy EC)

**COMPOSE**: perform the composition of PNs (actually FCPNs and SMs) and create an output file in .aut extension

**DOT**: combined with COMPOSE flag creates the output file in .dot extension instead of .aut

**GE**: perform an exact algorithm instead of greedy one during the removal of redundant SMs

**NOBOUNDS**: this flag can be used only with BDD flag and it allows the creation of unbounded SMs

**MS**: usage of a mixed strategy which chooses if use the exact algorithm or approximate one for the removal of redundant SMs

Statistics are stored into *'stats.csv'* file (in the folder from where the code is run).
The file contains the following data if BDD flag was **not** selected:
- decomposition type: SM/SM_exact_removal/SM_mixed_strategy
- file name
- time region generation
- time initial SM decomposition (without optimizations)
- time greedy algorithm
- time merge
- time entire decomposition (including greedy algorithm and merge)
- number of places after the initial creation of the set of SMs
- number of places after the execution of greedy algorithm
- final number of places
- number of transitions after the initial creation of the set of SMs
- number of transitions after the execution of greedy algorithm
- final number of transitions
- number of SMs
- max (place+transition) sum
- max alphabet (for the alphabet a and a' are the same)
- max transitions
- average number of places
- variance of the number of places
- average number of transitions
- variance of the number of transitions

If BDD flag was selected the *'stats.csv'* will contain the following data:
- decomposition type: SM_BDD/SM_BDD_DEBUG
- file name
- runtime
- time region generation
- time decomposition
- time greedy/exact SM removal
- time labels removal
- number of final places
- number of FCPNs
- number of places after the initial step of decomposition (before greedy algorithm)
- number of places after the greedy algorithm
- maximum alphabet
- average alphabet

### Execution using a script

In the root folder the following script can be executed, but firstly it has to be moved into execution folder (usually *'cmake-build-debug'*'), since it uses some dependencies of MIS solver:

```Bash
./Benchmark.sh
```

The script runs the decomposition in SMs on all benchmarks of *'auto_benchmark_dir'* folder. 

## c) TS to interacting FCPNs flow


### Approximated algorithm:

```Bash
./TS_splitter <file_path> FC <optional_flags>
```

Statistics are stored into *'stats.csv'* file (in the folder from where the code is run). The file contains the following data:

- decomposition type: FCPN/ACPN/FCPN_BDD/FCPN_DEBUG/ACPN_DEBUG/FCPN_BDD_DEBUG
- file name
- runtime
- time region generation
- time decomposition
- time greedy/exact SM removal
- time labels removal
- number of final places
- number of FCPNs
- number of places after the initial step of decomposition (before greedy algorithm)
- number of places after the greedy algorithm
- maximum alphabet
- average alphabet

#### Optional flags:

**D**: debug info mode

**B**: execute also SM decomposition combining the set of not minimized SMs to the FCPNs before performing the greedy
algorithm

**O**: write output FCPNs on file in .dot extension

**MIN**: once found a minimum number of FCPNs perform a minimization on the set of regions
**(not recommended, not compatible with BDD flag)**

**NOMERGE**: avoids the Merge step

**COMPOSE**: perform the composition of FCPNs and create an output file in .aut extension (does not works with k-FCPN decomposition)

**DOT**: combined with COMPOSE flag creates the output file in .dot extension instead of .aut

**I**: ignore incorrect decomposition in order to allow to produce the output PNs

**GE**: this flag is experimental and could not work: instead of executing greedy search perform a Pseudo-Boolean search 
in order to find the minimal number of FCPNs, indeed this search has very restricted constraints: at least one occurrence of each region in at least one FCPN, provides worst results compared to greedy algorithm **(not recommended and not compatible with BDD flag)**

**BDD**:  use a BDD to encode excitation closure constraint, enabling the direct decomposition into a set of FCPNs instead of iteratively search new FCPNs

**CHECK**: check if each derived FCPN/ACPN has a correct FCPN/ACPN structure, also in case of BDD usage check if excitation closure is satisfied

**SC**: check if the separate FCPNs are safe

**SAFE**: decompose allowing only standalone safe FCPNs

### Approximate algorithm using a script

In the root folder the following script can be executed:

```Bash
./FC-benchmark.sh
```

The script runs the approximate algorithm for FCPN decomposition on all benchmarks of *'auto_benchmark_dir'* folder.


## d) TS to interacting ACPNs flow

```Bash
./TS_splitter <file_path> AC <optional_flags>
```

#### Optional flags:

See FCPN flow (approximated algorithm).

## Combined flows

The flags M and AC/FC/KFC can be used together.


[//]: # "#### Benchmarks on a set of files (still present some issues, better avoid it):"

[//]: # "Execution of the decomposition on each file in ./auto_benchmark_dir/:"

[//]: # "```Bash"
[//]: # "cd cmake-build-debug"
[//]: # "./benchmark.sh"
[//]: # "```"

[//]: # "Execution of the decomposition on each file in ./benchmark_all_flag/ using -ALL flag:"

[//]: # "```Bash"
[//]: # "./benchmark-exact-alg.sh"
[//]: # "```"


## Additional tools

Creation of .dot extension TS/ECTS file starting from .g or .ts extension TS.

```Bash
./TS_splitter <file_path> (TS | ECTS) <optional_flag>
```

#### Optional flags:

**AUT**: instead of .dot extension export the resultant TS/ECTS in Aldebaran extension (.aut)

# PN visualization

**Graphviz library is required!**

Graphviz installation:
```bash
sudo apt install graphviz
```

Command for the creation of a PostScript PN/TS file.

```bash
dot -Tps filename.dot -o outfile.ps
```

# Decomposition check

In order to check the correctness of the decomposition mCLR2 tool can be used (https://www.mcrl2.org/web/user_manual/index.html).

Steps for the verification:

1) Generate the TS with AUT extension

```Bash
./TS_splitter <file_path> (TS | ECTS) AUT
```

2) Generate the FCPN/ACPN with COMPOSE flag:

```Bash
./TS_splitter <file_path> (FC | AC) COMPOSE
```

3) Verify the bisimulation between the initial TS (suppose example_TS.aut) and the composition of the PNs (suppose example_FCPN_composed.aut)
```Bash
ltscompare --equivalence=bisim <TS_aut_file> <composed_PN_aut_file> 
```

# Known restrictions


1) The parser for .ts files allow only the syntax with integers: the places and labels have to start from 0 and the maximum value have to corrspond to the number of places/labels - 1 (any index can be skipped).

2) There is any check on .ts inputs.

3) It is not possible to run two parallel instances of the program on the same input file since there is a file dependency used by the SAT solver therefore the results could result wrong.

[//]: # "Known issues"
[//]: # "------------------"

## License ##

>BSD 3-Clause License
>
>TStoPN flow -- Copyright (c) 2018, Viktor Teren, Valentina Napoletani All rights reserved.
>
>TS decomposition flow -- Copyright (c) 2019, Viktor Teren
All rights reserved.
>
>Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
>
>* Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
>
>* Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
>
>* Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
>
>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.