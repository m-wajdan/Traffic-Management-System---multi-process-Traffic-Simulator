#include "TrafficController.h"
#include "Intersection.h"
#include "Vehicle.h"

TrafficLight::TrafficLight(const string &dir)
    : direction(dir), green(false) {}

void TrafficLight::setGreen(bool status) {
    green = status;
}

void TrafficLight::setRed(bool status) {
    green = !status;
}

bool TrafficLight::isGreen() const {
    return green;
}

string TrafficLight::getDirection() const {
    return direction;
}

TrafficController::TrafficController(Intersection* inter, int greenTime)
    : intersection(inter),
      northLight(Direction::NORTH),
      southLight(Direction::SOUTH),
      eastLight(Direction::EAST),
      westLight(Direction::WEST),
      greenDuration(greenTime),
      running(true) {}

Vehicle* TrafficController::checkEmergency() const {
    if (intersection->hasVehicle(Direction::NORTH)) {
        Vehicle* v = intersection->getNextVehicle(Direction::NORTH);
        if (v && v->isEmergency()) return v;
    }

    if (intersection->hasVehicle(Direction::SOUTH)) {
        Vehicle* v = intersection->getNextVehicle(Direction::SOUTH);
        if (v && v->isEmergency()) return v;
    }

    if (intersection->hasVehicle(Direction::EAST)) {
        Vehicle* v = intersection->getNextVehicle(Direction::EAST);
        if (v && v->isEmergency()) return v;
    }

    if (intersection->hasVehicle(Direction::WEST)) {
        Vehicle* v = intersection->getNextVehicle(Direction::WEST);
        if (v && v->isEmergency()) return v;
    }

    return nullptr;
}

void TrafficController::crossVehicle(Vehicle* v) {
    if (!v) {
        return;
    }

    cout << "[TrafficController] Vehicle " << v->getId() << " (" << v->getType()
         << ") is crossing from " << v->getOrigin() << " to " << v->getDestination() << endl;

    // Determine from which lane this vehicle is crossing by checking
    // which directional lane has it at the front. This keeps all
    // lane and priority management inside the existing abstractions.
    string dir;
    if (intersection->hasVehicle(Direction::NORTH) &&
        intersection->getNextVehicle(Direction::NORTH) == v) {
        dir = Direction::NORTH;
    } else if (intersection->hasVehicle(Direction::SOUTH) &&
               intersection->getNextVehicle(Direction::SOUTH) == v) {
        dir = Direction::SOUTH;
    } else if (intersection->hasVehicle(Direction::EAST) &&
               intersection->getNextVehicle(Direction::EAST) == v) {
        dir = Direction::EAST;
    } else if (intersection->hasVehicle(Direction::WEST) &&
               intersection->getNextVehicle(Direction::WEST) == v) {
        dir = Direction::WEST;
    }

    if (!dir.empty()) {
        intersection->removeVehicle(dir);
    } else {
        cout << "[TrafficController] Warning: vehicle " << v->getId()
             << " not found at the front of any lane; skipping removal." << endl;
    }

    sleep(2); // simulate crossing time
}

void TrafficController::runController() {
    int cycle = 1;
    while (running) {
        // Always serve emergencies first.
        Vehicle* emergencyVehicle = checkEmergency();
        if (emergencyVehicle) {
            cout << "\n[TrafficController] EMERGENCY phase: giving priority to vehicle "
                 << emergencyVehicle->getId() << " (" << emergencyVehicle->getType() << ")" << endl;

            // For visualization, briefly turn all lights red during emergency.
            northLight.setRed(true);
            southLight.setRed(true);
            eastLight.setRed(true);
            westLight.setRed(true);

            crossVehicle(emergencyVehicle);
            cout << endl;
            continue;
        }

        cout << "\n[TrafficController] === Traffic light cycle " << cycle << " ===" << endl;

        // ---------------- NORTH lane phase ----------------
        cout << "[TrafficController] Phase: NORTH lane GREEN" << endl;
        northLight.setGreen(true);
        southLight.setRed(true);
        eastLight.setRed(true);
        westLight.setRed(true);

        if (intersection->hasVehicle(Direction::NORTH)) {
            Vehicle* v = intersection->getNextVehicle(Direction::NORTH);
            crossVehicle(v);
        }

        sleep(greenDuration);
        northLight.setRed(true);
        cout << "[TrafficController] Phase: NORTH lane RED" << endl;

        // ---------------- SOUTH lane phase ----------------
        cout << "\n[TrafficController] Phase: SOUTH lane GREEN" << endl;
        northLight.setRed(true);
        southLight.setGreen(true);
        eastLight.setRed(true);
        westLight.setRed(true);

        if (intersection->hasVehicle(Direction::SOUTH)) {
            Vehicle* v = intersection->getNextVehicle(Direction::SOUTH);
            crossVehicle(v);
        }

        sleep(greenDuration);
        southLight.setRed(true);
        cout << "[TrafficController] Phase: SOUTH lane RED" << endl;

        // ---------------- EAST lane phase ----------------
        cout << "\n[TrafficController] Phase: EAST lane GREEN" << endl;
        northLight.setRed(true);
        southLight.setRed(true);
        eastLight.setGreen(true);
        westLight.setRed(true);

        if (intersection->hasVehicle(Direction::EAST)) {
            Vehicle* v = intersection->getNextVehicle(Direction::EAST);
            crossVehicle(v);
        }

        sleep(greenDuration);
        eastLight.setRed(true);
        cout << "[TrafficController] Phase: EAST lane RED" << endl;

        // ---------------- WEST lane phase ----------------
        cout << "\n[TrafficController] Phase: WEST lane GREEN" << endl;
        northLight.setRed(true);
        southLight.setRed(true);
        eastLight.setRed(true);
        westLight.setGreen(true);

        if (intersection->hasVehicle(Direction::WEST)) {
            Vehicle* v = intersection->getNextVehicle(Direction::WEST);
            crossVehicle(v);
        }

        sleep(greenDuration);
        westLight.setRed(true);
        cout << "[TrafficController] Phase: WEST lane RED" << endl;

        cout << "[TrafficController] === End of cycle " << cycle << " ===" << endl;
        ++cycle;
    }
}

void* TrafficController::runThread(void* arg) {
    TrafficController* controller = static_cast<TrafficController*>(arg);
    controller->runController();
    return nullptr;
}

void TrafficController::startController() {
    pthread_create(&controllerThread, nullptr, runThread, this);
}

void TrafficController::stopController() {
    running = false;
    pthread_join(controllerThread, nullptr);
}

bool TrafficController::sendMessage(int fd, const ControllerMessage &msg) {
    ssize_t written = write(fd, &msg, sizeof(msg));
    return written == static_cast<ssize_t>(sizeof(msg));
}

bool TrafficController::receiveMessage(int fd, ControllerMessage &msg) {
    ssize_t readBytes = read(fd, &msg, sizeof(msg));
    return readBytes == static_cast<ssize_t>(sizeof(msg));
}
