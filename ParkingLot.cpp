#include "ParkingLot.h"
#include "Vehicle.h"

ParkingLot::ParkingLot(const string &lotID, int parking_cap, int waiting_cap)
    : parking_capacity(parking_cap),
      waiting_capacity(waiting_cap),
      parkingLotID(lotID)
{
    sem_init(&parking_spots, 0, parking_capacity);
    sem_init(&waiting_spots, 0, waiting_capacity);

    mtx.lock();
    cout << "[ParkingLot] " << lotID << " initialized with:" << endl;
    cout << "  Parking spots: " << parking_capacity << endl;
    cout << "  Waiting spots: " << waiting_capacity << endl;
    mtx.unlock();
}

bool ParkingLot::tryReserveWaitingSlot(Vehicle* v)
{
    if(!v) return false;

    if(v->isEmergency())
    {
        lock_guard<mutex> lock(mtx);
        cout << "Emergency vehicles cannot request parking." << endl;
        return false;
    }

    if(!v->canPark())
    {
        lock_guard<mutex> lock(mtx);
        cout << "[ParkingLot] " << parkingLotID << " -> Vehicle "
             << v->getId() << " (" << v->getType() << ") cannot park here." << endl;
        return false;
    }

    if(sem_trywait(&waiting_spots) != 0)
    {
        lock_guard<mutex> lock(mtx);
        cout << "[ParkingLot] " << parkingLotID
             << " waiting queue full. Vehicle " << v->getId()
             << " (" << v->getType() << ") cannot enter waiting queue." << endl;
        return false;
    }

    lock_guard<mutex> lock(mtx);
    cout << "[ParkingLot] " << parkingLotID << " -> Vehicle "
         << v->getId() << " (" << v->getType()
         << ") obtained a waiting spot in waiting queue." << endl;

    return true;
}

bool ParkingLot::aquireParkingSpot(Vehicle* v)
{
    if(!v) return false;

    if(sem_trywait(&parking_spots) != 0)
    {
        lock_guard<mutex> lock(mtx);
        cout << "[ParkingLot] " << parkingLotID << " -> Vehicle "
             << v->getId() << " (" << v->getType()
             << ") could not obtain parking spot." << endl;
        return false;
    }

    sem_post(&waiting_spots);

    lock_guard<mutex> lock(mtx);
    cout << "[ParkingLot] " << parkingLotID << " -> Vehicle "
         << v->getId() << " (" << v->getType()
         << ") obtained a parking spot." << endl;

    return true;
}

void ParkingLot::releaseWaitingSlot(Vehicle* v)
{
    if(!v) return;
    sem_post(&waiting_spots);

    lock_guard<mutex> lock(mtx);
    cout << "[ParkingLot] " << parkingLotID << " -> Vehicle "
         << v->getId() << " (" << v->getType()
         << ") released a waiting spot." << endl;
}

void ParkingLot::leaveParking(Vehicle* v)
{
    if(!v) return;
    sem_post(&parking_spots);

    lock_guard<mutex> lock(mtx);
    cout << "[ParkingLot] " << parkingLotID << " -> Vehicle "
         << v->getId() << " (" << v->getType()
         << ") has left the parking lot." << endl;
}

ParkingLot::~ParkingLot()
{
    sem_destroy(&parking_spots);
    sem_destroy(&waiting_spots);
}
