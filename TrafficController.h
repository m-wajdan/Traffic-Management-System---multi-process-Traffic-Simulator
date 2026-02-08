#ifndef TRAFFIC_CONTROLLER_H
#define TRAFFIC_CONTROLLER_H

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string>

using namespace std;

class Intersection;
class Vehicle;

// Simple POD struct used for inter-controller IPC over pipes and
// to visualize lanes and traffic lights.
struct ControllerMessage {
    int  vehicleId;
    int  priority;
    bool isEmergency;
    char type[16];        // e.g., "car", "ambulance"
    char origin[8];       // intersection id, e.g., "F10"
    char destination[8];  // intersection id, e.g., "F11"
    char approach[8];     // lane direction at origin intersection: "N", "S", "E", "W" 
    char movement[16];    // intended movement: "STRAIGHT", "LEFT", "RIGHT"
};

class TrafficLight {
    string direction;
    bool green;

public:
    explicit TrafficLight(const string &dir);

    void setGreen(bool status);
    void setRed(bool status);
    bool isGreen() const;

    string getDirection() const;
};

class TrafficController {
    Intersection* intersection;
    TrafficLight northLight;
    TrafficLight southLight;
    TrafficLight eastLight;
    TrafficLight westLight;
    int greenDuration;
    bool running;

    pthread_t controllerThread;

public:
    explicit TrafficController(Intersection* inter, int greenTime = 5);

    // Check all approaches for an emergency vehicle, preferring the earliest one.
    Vehicle* checkEmergency() const;

    // Allow a single vehicle to cross and remove it from its lane.
    void crossVehicle(Vehicle* v);

    // Main controller loop: always serve emergencies first, then
    // cycle through the four directions.
    void runController();

    static void* runThread(void* arg);

    void startController();
    void stopController();

    // IPC helpers for use by controller processes.
    static bool sendMessage(int fd, const ControllerMessage &msg);
    static bool receiveMessage(int fd, ControllerMessage &msg);
};

#endif
