#include "Intersection.h"

const string Direction::NORTH = "NORTH";
const string Direction::SOUTH = "SOUTH";
const string Direction::EAST  = "EAST";
const string Direction::WEST  = "WEST";

Intersection::Intersection(ParkingLot* lot)
    : parkingLot(lot) {}

void Intersection::addVehicle(const string &direction, Vehicle* v) {
    lock_guard<mutex> lock(mtx);

    if (direction == Direction::NORTH) {
        northLane.push(v);
    } else if (direction == Direction::SOUTH) {
        southLane.push(v);
    } else if (direction == Direction::EAST) {
        eastLane.push(v);
    } else if (direction == Direction::WEST) {
        westLane.push(v);
    } else {
        cout << "Invalid direction: " << direction << endl;
    }
}

Vehicle* Intersection::getNextVehicle(const string &direction) const {
    lock_guard<mutex> lock(mtx);

    if (direction == Direction::NORTH) {
        return northLane.front();
    } else if (direction == Direction::SOUTH) {
        return southLane.front();
    } else if (direction == Direction::EAST) {
        return eastLane.front();
    } else if (direction == Direction::WEST) {
        return westLane.front();
    } else {
        cout << "Invalid direction: " << direction << endl;
        return nullptr;
    }
}

void Intersection::removeVehicle(const string &direction) {
    lock_guard<mutex> lock(mtx);

    if (direction == Direction::NORTH) {
        northLane.pop();
    } else if (direction == Direction::SOUTH) {
        southLane.pop();
    } else if (direction == Direction::EAST) {
        eastLane.pop();
    } else if (direction == Direction::WEST) {
        westLane.pop();
    } else {
        cout << "Invalid direction: " << direction << endl;
    }
}

bool Intersection::hasVehicle(const string &direction) const {
    lock_guard<mutex> lock(mtx);

    if (direction == Direction::NORTH) {
        return !northLane.empty();
    } else if (direction == Direction::SOUTH) {
        return !southLane.empty();
    } else if (direction == Direction::EAST) {
        return !eastLane.empty();
    } else if (direction == Direction::WEST) {
        return !westLane.empty();
    } else {
        cout << "Invalid direction: " << direction << endl;
        return false;
    }
}

void Intersection::printStatus() const {
    lock_guard<mutex> lock(mtx);

    cout << "Intersection Status:" << endl;
    cout << "North Lane: "; northLane.print();
    cout << "South Lane: "; southLane.print();
    cout << "East Lane: ";  eastLane.print();
    cout << "West Lane: ";  westLane.print();
    cout << "-----------------------------" << endl;
}
