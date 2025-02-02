# Technical Test (client-server for metrics)

## Description

Provide a client-server software that delivers some key metrics about the host. You can choose 2-3 metrics you find the most relevant
(e.g.: CPU, RAM, ...).
### Features
1. Server: reads the metrics
2. Server: publish the metrics on the network
3. Client: connects to the server and reads the information published (CLI interface or logging)
NB: The implementation of the feature 3 is optional. We’re more interested in how you plan to implement it.
### Requirements
 - Client and server should be implemented with C++ (>= 17) and cmake
 - Select the techno for the interprocess communication and explain your choice
 - The program shall remain simple, but should be functional. Add the documentation you find necessary.
### Delivery
 - The project sources should be delivered by zip or accessible through a git repository.


## Tools and dependencies

 - C++ 20
 - Cmake >= 3.28
 - package libzmq3-dev


## Build instructions

```
mkdir build
cd build
cmake ..
make install
```

server and client installed in /install/bin

