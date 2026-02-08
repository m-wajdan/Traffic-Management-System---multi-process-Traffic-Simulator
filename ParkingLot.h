#ifndef PARKINGLOT_H
#define PARKINGLOT_H

#include <iostream>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <mutex>
#include <unistd.h>

using namespace std;

class Vehicle;

class ParkingLot
{
    sem_t parking_spots;
    sem_t waiting_spots;
    mutex mtx;
    string parkingLotID;
    int parking_capacity;
    int waiting_capacity;

public:

    ParkingLot(const string &lotID, int parking_cap = 10, int waiting_cap = 15);

    ParkingLot(const ParkingLot&) = delete;
    ParkingLot& operator=(const ParkingLot&) = delete;

    bool tryReserveWaitingSlot(Vehicle* v);
    bool aquireParkingSpot(Vehicle* v);
    void releaseWaitingSlot(Vehicle* v);
    void leaveParking(Vehicle* v);

    int getParkingCapacity() const { return parking_capacity; }
    int getWaitingCapacity() const { return waiting_capacity; }
    string getParkingLotID() const { return parkingLotID; }

    ~ParkingLot();
};


#endif
