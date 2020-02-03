Software for PN creation based on Regions theory (TStoPN)
========================

Part of code is based on PBLib (a fork of MiniSAT 2.2) and NetworkX.

Tested only on Ubuntu 18.04 LTS

### Required software:
- g++
```Bash
sudo apt install g++
```
- cmake
```Bash
sudo apt install cmake
```

- python 2.7 

- networkx
```Bash
pip install networkx
```

### Key principles:
- Creation of Petri Nets from Transition Systems
- Decomposition of Transition Systems into sets of  interacting State Machines

### Supported extensions

Input extensions: .g

Output extensions: .dot .g

Building
--------
Enter to the project folder and execute the following instructions:
```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make TS_splitter
```

Execution
---------

### a) TS to PN flow

Quick way (requirese graphviz library) with .ps file creation

```Bash
./execute.sh
```

Simple way

```Bash
./TS_splitter <file_path> <optional_flags>
```
Additional flags:

D: debug mode

S: verbose info mode

### b) TS to Synchronized SMs flow

```Bash
./TS_splitter <file_path> M <optional_flags>
```

Additional flags:

L: creation of a log file

D: debug info mode

O: write output SMs in .dot extension

G: write output SMs in .g extension

-ALL: execute the decomposition using an exact algorithm to find all minimal independent sets (not only the minimum required set to satisfy EC)

--INFO: base info are shown and the execution stops when all regions are created and the TS is excitation closed, otherwise label splitting is performed until reaching an ECTS

Independently by the used flags, after each execution the stats.csv file is updated.

#### Benchmarks on a set of files:

Execution of the decomposition on each file in ./auto_benchmark_dir/:

```Bash
./benchmark.sh
```

Execution of the decomposition on each file in ./benchmark_all_flag/ using -ALL flag:

```Bash
./benchmark-exact-alg.sh
```

### c) Additional tools

Creation of .dot file for TS

```Bash
./TS_splitter <file_path> TS
```

PN visualization
----------------

Graphviz library is required

```bash
dot -Tps filename.dot -o outfile.ps
```

## License ##

>BSD 3-Clause License
>
>TStoPN flow -- Copyright (c) 2018, Viktor Teren, Valentina Napoletani
TS decomposition flow -- Copyright (c) 2019, Viktor Teren
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