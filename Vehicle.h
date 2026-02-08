#ifndef VEHICLE_H
#define VEHICLE_H

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <mutex>
#include <functional>
#include <unistd.h>
using namespace std;

class ParkingLot;

class Vehicle
{
    int id; 
    string type; 
    string origin; 
    string destination; 
    int priority; 
    int arrival_time; 
    bool can_park;
    
    bool parking_reserved;
    mutex mtx;
    pthread_t thread_id;

    function<void(Vehicle*)> requestIntersectionAccess;
    
public:

    Vehicle(int id, const string &type, const string &origin, 
            const string &destination, int priority, int arr_time);

    Vehicle(const Vehicle&) = delete;
    Vehicle& operator=(const Vehicle&) = delete;

    //Getters
    int getId() const;
    string getType() const;
    string getOrigin() const;
    string getDestination() const;
    int getPriority() const;
    int getArrivalTime() const;
    bool canPark() const;
    bool isEmergency() const;

    void setRequestIntersectionAccessFunction(function<void(Vehicle*)> func);

    bool parkingVehicle(ParkingLot &lot);
    void occupyReservedParking(ParkingLot &lot);
    void cancelParkingReservation(ParkingLot &lot);

    void crossingIntersection();

    void runVehicle(ParkingLot &F10 , ParkingLot &F11);

    struct ThreadArg {
        Vehicle* v;
        ParkingLot* f10;
        ParkingLot* f11;
    };

    static void* threadStart(void* arg);

    bool start(ParkingLot &F10, ParkingLot &F11);
    void wait();
};

#endif
