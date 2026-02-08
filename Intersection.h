#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <iostream>
#include <string>
#include <mutex>
#include "VehileLane.h"
#include "ParkingLot.h"
#include "Vehicle.h"

using namespace std;

// Canonical textual directions used at the intersection.
struct Direction {
    static const string NORTH;
    static const string SOUTH;
    static const string EAST;
    static const string WEST;
};

// Thread-safe wrapper around four directional lanes and an optional
// attached parking lot.
class Intersection {
    VehicleLane northLane;
    VehicleLane southLane;
    VehicleLane eastLane;
    VehicleLane westLane;

    ParkingLot* parkingLot; // may be nullptr if no parking lot attached
    mutable mutex mtx;      // protects lane access and status prints

public:
    explicit Intersection(ParkingLot* lot = nullptr);

    // Add a vehicle to a directional lane based on its approach.
    void addVehicle(const string &direction, Vehicle* v);

    // Peek at the next vehicle from a direction without removing it.
    Vehicle* getNextVehicle(const string &direction) const;

    // Remove the front vehicle from a given direction.
    void removeVehicle(const string &direction);

    // Check if there is at least one vehicle on a given approach.
    bool hasVehicle(const string &direction) const;

    // Debug helper to dump the current queues.
    void printStatus() const;
};

#endif
