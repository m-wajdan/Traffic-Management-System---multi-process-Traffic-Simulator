#include "Vehicle.h"
#include "ParkingLot.h"

Vehicle::Vehicle(int id, const string &type, const string &origin, 
                 const string &destination, int priority, int arr_time)
{
    this->id = id;
    this->type = type;
    this->origin = origin;
    this->destination = destination;
    this->arrival_time = arr_time;
    this->parking_reserved = false;

    if(type == "ambulance" || type == "firetruck")
        this->priority = 1;
    else if(type == "bus")
        this->priority = 2;
    else
        this->priority = 3;

    if(type == "car" || type == "bus" || type == "tractor" || type == "bike")
        can_park = true;
    else
        can_park = false;
}

int Vehicle::getId() const { return id; }
string Vehicle::getType() const { return type; }
string Vehicle::getOrigin() const { return origin; }
string Vehicle::getDestination() const { return destination; }
int Vehicle::getPriority() const { return priority; }
int Vehicle::getArrivalTime() const { return arrival_time; }
bool Vehicle::canPark() const { return can_park; }
bool Vehicle::isEmergency() const { return (type == "ambulance" || type == "firetruck"); }

void Vehicle::setRequestIntersectionAccessFunction(function<void(Vehicle*)> func)
{
    this->requestIntersectionAccess = func;
}

bool Vehicle::parkingVehicle(ParkingLot &lot)
{
    if(!can_park || isEmergency()) return false;

    bool ok = lot.tryReserveWaitingSlot(this);
    if(ok) parking_reserved = true;
    return ok;
}

void Vehicle::occupyReservedParking(ParkingLot &lot)
{
    if(!parking_reserved) return;

    if(lot.aquireParkingSpot(this))
    {
        cout << "[Vehicle] " << origin << " -> Vehicle " << id
             << " (" << type << ") has parked at parking lot "
             << lot.getParkingLotID() << endl;

        sleep(5);
        lot.leaveParking(this);
    }
    else
    {
        lot.releaseWaitingSlot(this);
    }
    parking_reserved = false;
}

void Vehicle::cancelParkingReservation(ParkingLot &lot)
{
    if(parking_reserved)
    {
        lot.releaseWaitingSlot(this);
        parking_reserved = false;
    }
}

void Vehicle::crossingIntersection()
{
    cout << "[Vehicle] " << origin << " -> Vehicle " << id
         << " (" << type << ") is crossing intersection "
         << destination << "." << endl;
    cout << "[Vehicle] Vehicle " << id << " (" << type
         << ") has crossed intersection " << destination << "." << endl;
}

void Vehicle::runVehicle(ParkingLot &F10, ParkingLot &F11)
{
    sleep(arrival_time);

    cout << "[Vehicle] " << origin << " -> Vehicle " << id
         << " (" << type << ") has arrived at its intersection." << endl;

    // Try parking
    if(origin == "F10") parkingVehicle(F10);
    else if(origin == "F11") parkingVehicle(F11);

    // Request intersection
    if(requestIntersectionAccess) 
        requestIntersectionAccess(this);
    else 
        crossingIntersection();

    // If parking was reserved, actually park
    if(parking_reserved)
    {
        if(origin == "F10") occupyReservedParking(F10);
        else if(origin == "F11") occupyReservedParking(F11);
    }
}

void* Vehicle::threadStart(void* arg)
{
    ThreadArg* ta = static_cast<ThreadArg*>(arg);
    ta->v->runVehicle(*ta->f10, *ta->f11);
    delete ta;
    return nullptr;
}

bool Vehicle::start(ParkingLot &F10, ParkingLot &F11)
{
    ThreadArg* ta = new ThreadArg{this, &F10, &F11};

    int rc = pthread_create(&thread_id, nullptr, threadStart, ta);
    if(rc != 0)
    {
        cout << "pthread_create failed for vehicle " << id << endl;
        delete ta;
        return false;
    }
    return true;
}

void Vehicle::wait(){
    pthread_join(thread_id, nullptr);
}
