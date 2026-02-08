#ifndef VEHICLE_LANE_H
#define VEHICLE_LANE_H

#include <iostream>
#include <string>
#include "Vehicle.h"

using namespace std;

// A priority-ordered lane of vehicles. Highest priority and earliest arrival
// are always at the front of the lane.
class VehicleLane {
    static const int MAX_CAPACITY = 100;
    Vehicle* vehicles[MAX_CAPACITY];
    int count;

    // Reorder vehicles in-place by (priority, arrival_time).
    void reorder();

public:
    VehicleLane();

    // Insert vehicle according to its priority. Returns false if lane is full.
    bool push(Vehicle* v);

    // Peek at the next vehicle to cross, or nullptr if empty.
    Vehicle* front() const;

    // Remove the vehicle at the front (no-op if empty).
    void pop();

    int size() const;
    bool empty() const;

    void print() const;
};

#endif
