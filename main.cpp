#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#include "Intersection.h"
#include "TrafficController.h"
#include "Vehicle.h"
#include "ParkingLot.h"

using namespace std;

struct PipeListenerArgs {
    string controllerName;
    int    readFd;
};

void* pipeListenerThread(void* arg)
{
    PipeListenerArgs* args = static_cast<PipeListenerArgs*>(arg);
    const string& name = args->controllerName;
    int readFd = args->readFd;

    cout << "[" << name << "-Listener] Pipe listener started." << endl;

    ControllerMessage msg{};
    while (true) {
        if (!TrafficController::receiveMessage(readFd, msg)) {
            break;
        }

        cout << "[" << name << "-Listener] Received message for vehicle "
             << msg.vehicleId
             << " (type=" << msg.type << ", emergency=" << (msg.isEmergency ? "yes" : "no")
             << ") from " << msg.origin << " to " << msg.destination
             << " via approach " << msg.approach
             << " movement " << msg.movement << endl;

        if (msg.isEmergency) {
            cout << "[" << name << "-Listener] Preparing for incoming emergency vehicle "
                 << msg.vehicleId << "." << endl;
        }
    }

    cout << "[" << name << "-Listener] Pipe listener exiting." << endl;
    delete args;
    return nullptr;
}


void createVehiclesForF10(vector<Vehicle*>& vehicles, map<Vehicle*, string>& laneMap)
{
    // 10 vehicles at F10, IDs 1..10
    struct VDef { int id; const char* type; const char* dest; int arr; std::string lane; };
    VDef defs[] = {
        {1,  "ambulance", "F11", 1,  Direction::NORTH},
        {2,  "firetruck", "F11", 2,  Direction::EAST},
        {3,  "firetruck", "F11", 3,  Direction::NORTH},
        {4,  "bike",      "F10", 2,  Direction::WEST},
        {5,  "car",       "F10", 4,  Direction::SOUTH},
        {6,  "firetruck", "F11", 5,  Direction::SOUTH},
        {7,  "bus",       "F11", 6,  Direction::EAST},
        {8,  "ambulance", "F11", 3,  Direction::SOUTH},
        {9,  "tractor",   "F10", 7,  Direction::WEST},
        {10, "car",       "F11", 8,  Direction::NORTH}
    };

    for (const auto& d : defs) {
        Vehicle* v = new Vehicle(d.id, d.type, "F10", d.dest, 0, d.arr);
        vehicles.push_back(v);
        laneMap[v] = d.lane;
    }
}

void createVehiclesForF11(vector<Vehicle*>& vehicles, map<Vehicle*, string>& laneMap)
{
    // 10 vehicles at F11, IDs 101..110
    struct VDef { int id; const char* type; const char* dest; int arr; std::string lane; };
    VDef defs[] = {
        {101, "ambulance", "F10", 1,  Direction::SOUTH},
        {102, "firetruck", "F11", 2,  Direction::EAST},
        {103, "bike",      "F11", 3,  Direction::WEST},
        {104, "firetruck", "F10", 2,  Direction::EAST},
        {105, "firetruck", "F10", 4,  Direction::SOUTH},
        {106, "firetruck", "F10", 5,  Direction::NORTH},
        {107, "ambulance", "F11", 6,  Direction::SOUTH},
        {108, "bus",       "F10", 7,  Direction::WEST},
        {109, "car",       "F10", 8,  Direction::NORTH},
        {110, "tractor",   "F11", 9,  Direction::EAST}
    };

    for (const auto& d : defs) {
        Vehicle* v = new Vehicle(d.id, d.type, "F11", d.dest, 0, d.arr);
        vehicles.push_back(v);
        laneMap[v] = d.lane;
    }
}


void runControllerProcess(const string &name, int readFd, int writeFd)
{
    cout << "\n[" << name << "] Controller process starting." << endl;

    // Each controller process owns a single parking lot matching its intersection name.
    ParkingLot localLot(name, 10, 15);
    Intersection intersection(&localLot);

    // Traffic controller for this intersection.
    TrafficController controller(&intersection, 5); // 3s green duration for demo cycles.

    // Start the controller main loop in its own thread.
    controller.startController();

    // Start a listener thread for incoming IPC messages from the peer controller.
    pthread_t listenerTid;
    {
        PipeListenerArgs* args = new PipeListenerArgs{ name, readFd };
        int rc = pthread_create(&listenerTid, nullptr, pipeListenerThread, args);
        if (rc != 0) {
            cerr << "[" << name << "] Failed to create pipe listener thread." << endl;
            delete args;
        }
    }

    // Generate vehicles local to this intersection (hard-coded, 10 each).
    vector<Vehicle*> vehicles;
    map<Vehicle*, string> laneMap; // Maps each vehicle to its assigned lane direction.

    if (name == "F10") {
        createVehiclesForF10(vehicles, laneMap);
    } else {
        createVehiclesForF11(vehicles, laneMap);
    }

    // Install per-vehicle requestIntersectionAccess callback.
    for (Vehicle* v : vehicles) {
        v->setRequestIntersectionAccessFunction(
            [&, name](Vehicle* veh) {
                auto it = laneMap.find(veh);
                string laneDir = (it != laneMap.end()) ? it->second : Direction::NORTH;

                cout << "[" << name << "] Vehicle " << veh->getId()
                     << " (" << veh->getType() << ") requesting intersection access via lane "
                     << laneDir << "." << endl;

                // Enqueue the vehicle into the appropriate lane.
                intersection.addVehicle(laneDir, veh);

                // Notify peer controller about emergencies moving to the neighboring intersection.
                if (veh->isEmergency() && veh->getOrigin() != veh->getDestination()) {
                    ControllerMessage msg{};
                    msg.vehicleId   = veh->getId();
                    msg.priority    = veh->getPriority();
                    msg.isEmergency = veh->isEmergency();

                    strncpy(msg.type, veh->getType().c_str(), sizeof(msg.type) - 1);
                    strncpy(msg.origin, veh->getOrigin().c_str(), sizeof(msg.origin) - 1);
                    strncpy(msg.destination, veh->getDestination().c_str(), sizeof(msg.destination) - 1);

                    // Short lane notation for approach (N/S/E/W).
                    string approachShort;
                    if (laneDir == Direction::NORTH)      approachShort = "N";
                    else if (laneDir == Direction::SOUTH) approachShort = "S";
                    else if (laneDir == Direction::EAST)  approachShort = "E";
                    else if (laneDir == Direction::WEST)  approachShort = "W";
                    else                                  approachShort = "N";

                    strncpy(msg.approach, approachShort.c_str(), sizeof(msg.approach) - 1);
                    // For this driver, treat all as straight movements.
                    strncpy(msg.movement, "STRAIGHT", sizeof(msg.movement) - 1);

                    cout << "[" << name << "] Notifying peer controller about emergency vehicle "
                         << msg.vehicleId << " from " << msg.origin << " to "
                         << msg.destination << "." << endl;

                    TrafficController::sendMessage(writeFd, msg);
                }
            }
        );
    }

    // Start vehicle threads.
    cout << "\n[" << name << "] Spawning vehicle threads." << endl;
    for (Vehicle* v : vehicles) {
        if (!v->start(localLot, localLot)) {
            cerr << "[" << name << "] Failed to start thread for vehicle "
                 << v->getId() << "." << endl;
        }
    }

    // Wait for all vehicle threads to finish.
    for (Vehicle* v : vehicles) {
        v->wait();
    }
    cout << "\n[" << name << "] All vehicle threads have finished." << endl;

    // Print final intersection state at this controller.
    cout << "\n[" << name << "] Final intersection state:" << endl;
    intersection.printStatus();

    // Allow some time for the controller to finish serving any remaining vehicles.
    sleep(5);

    // Stop controller loop and join its thread.
    controller.stopController();

    // Close our pipe ends to unblock the listener, then join the listener thread.
    close(readFd);
    close(writeFd);

    pthread_join(listenerTid, nullptr);

    // Cleanup vehicle objects.
    for (Vehicle* v : vehicles) {
        delete v;
    }

    cout << "\n[" << name << "] Controller process exiting cleanly." << endl;
}


int main()
{
    cout << "\n[Main] Starting dual-intersection traffic simulation (F10, F11)." << endl;

    int f10ToF11[2];
    int f11ToF10[2];

    if (pipe(f10ToF11) == -1 || pipe(f11ToF10) == -1) {
        perror("pipe");
        return 1;
    }

    // Fork controller process for F10.
    pid_t pidF10 = fork();
    if (pidF10 == -1) {
        perror("fork F10");
        return 1;
    }

    if (pidF10 == 0) {
        // Child: F10 controller process.
        close(f10ToF11[0]); // F10 will write to F11.
        close(f11ToF10[1]); // F10 will read from F11.

        runControllerProcess("F10", f11ToF10[0], f10ToF11[1]);
        _exit(0);
    }

    // Fork controller process for F11.
    pid_t pidF11 = fork();
    if (pidF11 == -1) {
        perror("fork F11");
        // Best effort: terminate F10 child.
        kill(pidF10, SIGTERM);
        return 1;
    }

    if (pidF11 == 0) {
        // Child: F11 controller process.
        close(f10ToF11[1]); // F11 will read from F10.
        close(f11ToF10[0]); // F11 will write to F10.

        runControllerProcess("F11", f10ToF11[0], f11ToF10[1]);
        _exit(0);
    }

    // Parent: close all pipe ends, only children use them.
    close(f10ToF11[0]);
    close(f10ToF11[1]);
    close(f11ToF10[0]);
    close(f11ToF10[1]);

    cout << "[Main] Controller processes started: F10 PID=" << pidF10
         << ", F11 PID=" << pidF11 << "." << endl;

    // Wait for both child processes to finish.
    int status = 0;
    waitpid(pidF10, &status, 0);
    cout << "\n[Main] Child process " << pidF10 << " exited with status " << status << "." << endl;

    waitpid(pidF11, &status, 0);
    cout << "[Main] Child process " << pidF11 << " exited with status " << status << "." << endl;

    cout << "\n[Main] Simulation finished. Exiting." << endl;
    return 0;
}