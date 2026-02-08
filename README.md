# Traffic-Management-System---multi-process-Traffic-Simulator
# Traffic Management System - OS Project

A multi-process, multi-threaded traffic simulation system implemented in C++ that demonstrates inter-process communication (IPC), thread synchronization, and resource management concepts from Operating Systems.

## Project Overview

This project simulates a traffic management system with two intersections (F10 and F11) that communicate with each other using pipes. Each intersection runs as a separate process with its own traffic controller, managing vehicles, traffic lights, and parking lots using threads and synchronization primitives.

## Features

- **Multi-Process Architecture**: Separate processes for each intersection controller
- **Inter-Process Communication**: Pipe-based communication between controllers
- **Multi-Threading**: Each vehicle runs as an independent thread
- **Synchronization**: Uses mutexes and semaphores for thread-safe operations
- **Priority-Based Traffic Management**: Emergency vehicles get priority
- **Parking Management**: Semaphore-controlled parking lots with waiting areas
- **Traffic Light Control**: Automated traffic light scheduling

## File Descriptions

### Core Components

#### `main.cpp`
- **Purpose**: Entry point of the simulation
- **Functionality**: 
  - Creates and manages two intersection controller processes (F10 and F11)
  - Sets up pipe-based IPC between controllers
  - Initializes vehicles with different types, origins, destinations, and priorities
  - Spawns pipe listener threads to monitor inter-controller messages
  - Coordinates simulation lifecycle (start, run, cleanup)
- **Key Features**: Fork-based process creation, pipe management, vehicle thread coordination

#### `Vehicle.h` / `Vehicle.cpp`
- **Purpose**: Represents individual vehicles in the traffic system
- **Functionality**:
  - Stores vehicle attributes (ID, type, origin, destination, priority, arrival time)
  - Manages vehicle behavior as a separate thread
  - Handles parking reservation and operations
  - Determines emergency vehicle status (ambulances, firetrucks)
  - Requests intersection access based on traffic conditions
- **Key Features**: Thread-based execution, priority calculation, parking logic

#### `TrafficController.h` / `TrafficController.cpp`
- **Purpose**: Manages traffic flow at an intersection
- **Functionality**:
  - Controls four directional traffic lights (North, South, East, West)
  - Implements traffic light scheduling algorithm
  - Handles vehicle queue management and crossing permissions
  - Sends inter-controller messages for vehicles traveling between intersections
  - Provides priority handling for emergency vehicles
- **Key Features**: TrafficLight class, green duration management, message passing

#### `Intersection.h` / `Intersection.cpp`
- **Purpose**: Represents a physical intersection with multiple approach lanes
- **Functionality**:
  - Manages four directional vehicle lanes (North, South, East, West)
  - Thread-safe vehicle addition and removal operations
  - Provides access to next vehicle in each lane
  - Maintains reference to associated parking lot
  - Status reporting for debugging and monitoring
- **Key Features**: Thread-safe lane operations, parking lot integration

#### `VehileLane.h` / `VehileLane.cpp`
- **Purpose**: Implements a priority queue for vehicles in a single lane
- **Functionality**:
  - Maintains ordered queue based on vehicle priority and arrival time
  - Automatically reorders vehicles when new ones are added
  - Provides front/pop operations for vehicle processing
  - Fixed capacity with overflow handling
- **Key Features**: Priority-based ordering, array-based queue implementation

#### `ParkingLot.h` / `ParkingLot.cpp`
- **Purpose**: Manages parking resources using semaphores
- **Functionality**:
  - Controls access to limited parking spots using semaphores
  - Manages waiting area for vehicles when parking is full
  - Provides reservation and release mechanisms
  - Thread-safe parking operations
- **Key Features**: Semaphore-based synchronization, capacity management

### Additional Files

#### `controller_demo.cpp`
- **Purpose**: Standalone demo or test file for traffic controller functionality
- **Note**: Not included in the main simulation build

## System Requirements

- **Operating System**: Linux/Unix-based system
- **Compiler**: g++ with C++11 support or later
- **Libraries**: pthread (POSIX Threads), standard C++ libraries

## Compilation

To compile the project, use the following command:

```bash
g++ -o main_sim main.cpp Intersection.cpp TrafficController.cpp Vehicle.cpp ParkingLot.cpp VehileLane.cpp -pthread
```

**Explanation of flags:**
- `-o main_sim`: Specifies the output executable name
- `-pthread`: Links the pthread library for multi-threading support
- All `.cpp` files are compiled and linked together

## Running the Simulation

After successful compilation, run the executable:

```bash
./main_sim
```

**Expected Behavior:**
- Two controller processes will start (F10 and F11)
- Vehicles will be created and begin their journeys
- Traffic lights will cycle automatically
- Console output shows vehicle movements, parking operations, and controller messages
- Simulation runs until all vehicles complete their trips

## Compilation and Execution (One-liner)

Compile and run in a single command:

```bash
g++ -o main_sim main.cpp Intersection.cpp TrafficController.cpp Vehicle.cpp ParkingLot.cpp VehileLane.cpp -pthread && ./main_sim
```

## Project Architecture

```
┌─────────────────────────────────────────────────┐
│                   Main Process                   │
│                                                  │
│  ┌─────────────────┐      ┌─────────────────┐  │
│  │   F10 Process   │◄────►│   F11 Process   │  │
│  │                 │ Pipe │                 │  │
│  │  ┌───────────┐  │      │  ┌───────────┐  │  │
│  │  │Controller │  │      │  │Controller │  │  │
│  │  │  Thread   │  │      │  │  Thread   │  │  │
│  │  └───────────┘  │      │  └───────────┘  │  │
│  │                 │      │                 │  │
│  │  ┌───────────┐  │      │  ┌───────────┐  │  │
│  │  │ Listener  │  │      │  │ Listener  │  │  │
│  │  │  Thread   │  │      │  │  Thread   │  │  │
│  │  └───────────┘  │      │  └───────────┘  │  │
│  │                 │      │                 │  │
│  │  ┌───────────┐  │      │  ┌───────────┐  │  │
│  │  │Vehicle    │  │      │  │Vehicle    │  │  │
│  │  │Threads... │  │      │  │Threads... │  │  │
│  │  └───────────┘  │      │  └───────────┘  │  │
│  └─────────────────┘      └─────────────────┘  │
└─────────────────────────────────────────────────┘
```

## Key OS Concepts Demonstrated

1. **Process Management**
   - Process creation using `fork()`
   - Process synchronization using `wait()` and `waitpid()`
   - Process termination and cleanup

2. **Inter-Process Communication (IPC)**
   - Pipe-based message passing between processes
   - Custom message protocol for vehicle information sharing

3. **Thread Management**
   - POSIX threads (`pthread`) for concurrent vehicle execution
   - Thread creation, execution, and joining

4. **Synchronization**
   - Mutexes for protecting shared data structures
   - Semaphores for resource management (parking spots)
   - Thread-safe operations on queues and lanes

5. **Resource Management**
   - Limited parking resources with waiting queues
   - Traffic light scheduling and coordination
   - Priority-based vehicle processing

## Vehicle Types

The simulation supports various vehicle types with different priorities:
- **Emergency Vehicles**: Ambulance, Fire truck (highest priority)
- **Regular Vehicles**: Car, Bus, Bike, Tractor
- **Destination-based Parking**: Vehicles only park at their destination intersection

## Author

OS Course - Final Project (Semester 5)

## License

Educational project for academic purposes.
