# RAD Flow

![CI Status](https://github.com/andrewboutros/rad-flow/.github/workflows/rad_sim_ci.yml/badge.svg)

## Introduction
The RAD flow is an open source academic architecture exploration and evaluation flow for novel beyond-FPGA reconfigurable acceleration devices (RADs).
These devices incorporate conventional FPGA fabrics, several coarse-grained domain-specialized accelerator blocks, and high-performance packet-switched networks-on-chip (NoCs) for system-level communication.
The flow consists of the following tools:
- **RAD-Sim** for rapid design space exploration and architecture-application co-design
- **RAD-Gen** for silicon area/timing/power implementation results of hard (ASIC) RAD components **(Under development)**

<img src="https://drive.google.com/uc?export=view&id=1m8ChChTQlDjKXf8buyGt05LwFYkPiHV6" width="1000"/>

## Documentation
The flow's full documentation can be found [here](https://www.notion.so/RAD-Sim-Documentation-7adde413148a4286bc5cc85648564672).

## How to Cite
The following paper may be used as a general citation for RAD-Sim:

A. Boutros, E. Nurvitadhi, and V. Betz, "RAD-Sim: Rapid Architecture Exploration for Novel Reconfigurable Acceleration Devices", International Conference on Field Programmable Logic and Applications (FPL), 2022

Bibtex:
```
@inproceedings{radsim,
  title={{RAD-Sim: Rapid Architecture Exploration for Novel Reconfigurable Acceleration Devices}},
  author={Boutros, Andrew and Nurvitadhi, Eriko and Betz, Vaughn},
  booktitle={International Conference on Field Programmable Logic and Applications (FPL)},
  year={2022},
  organization={IEEE}
}
```

## Contributors
**Professors:** Vaughn Betz

**Graduate Students:** Andrew Boutros

**Summer Students:**

**Companies:** Intel, VMWare

**Funding Agencies:** NSERC

## License

Unless otherwise noted (in particular for Booksim, the example designs or some libraries), all software, documents and scripts follow the standard MIT license below
> The MIT License (MIT)
>
> Copyright 2022 RAD Flow Developers
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Terms and conditions for Booksim 2.0 can be found here and are copied below for your convenience.
> Copyright (c) 2007-2015, Trustees of The Leland Stanford Junior University
> 
> All rights reserved.
> 
> Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
> 
> Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
> 
> Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
> 
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
