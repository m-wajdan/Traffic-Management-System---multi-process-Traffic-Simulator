#include "VehileLane.h"

VehicleLane::VehicleLane() : count(0) {
    for (int i = 0; i < MAX_CAPACITY; ++i) {
        vehicles[i] = nullptr;
    }
}

void VehicleLane::reorder() {
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            Vehicle* v1 = vehicles[j];
            Vehicle* v2 = vehicles[j + 1];

            if (!v1 || !v2) {
                continue;
            }

            if (v1->getPriority() > v2->getPriority() ||
                (v1->getPriority() == v2->getPriority() &&
                 v1->getArrivalTime() > v2->getArrivalTime())) {
                Vehicle* temp = vehicles[j];
                vehicles[j] = vehicles[j + 1];
                vehicles[j + 1] = temp;
            }
        }
    }
}

bool VehicleLane::push(Vehicle* v) {
    if (count >= MAX_CAPACITY) {
        cout << "VehicleLane is full. Cannot add more vehicles." << endl;
        return false;
    }
    vehicles[count++] = v;
    reorder();
    return true;
}

Vehicle* VehicleLane::front() const {
    if (count == 0) {
        return nullptr;
    }
    return vehicles[0];
}

void VehicleLane::pop() {
    if (count == 0) {
        return;
    }
    for (int i = 1; i < count; ++i) {
        vehicles[i - 1] = vehicles[i];
    }
    vehicles[count - 1] = nullptr;
    --count;
}

int VehicleLane::size() const {
    return count;
}

bool VehicleLane::empty() const {
    return count == 0;
}

void VehicleLane::print() const {
    cout << "Lane: \n";
    for (int i = 0; i < count; ++i) {
        if (vehicles[i]) {
            cout << vehicles[i]->getType() << "(" << vehicles[i]->getPriority() << ") ";
        }
    }
    cout << endl;
}
