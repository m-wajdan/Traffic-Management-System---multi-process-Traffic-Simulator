#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

#include "Intersection.h"
#include "TrafficController.h"
#include "Vehicle.h"

using namespace std;

// Simple demo showing two controller processes (F10 and F11)
// coordinating an emergency vehicle via pipes.

void runControllerProcess(const string &name, int readFd, int writeFd) {
    Intersection intersection(nullptr);
    TrafficController controller(&intersection, 2);

    if (name == "F10") {
        // Create a local emergency vehicle at F10 heading to F11.
        Vehicle ambulance(1, "ambulance", "F10", "F11", 1, 0);
        intersection.addVehicle(Direction::NORTH, &ambulance);

        // Announce intention to F11.
        ControllerMessage msg{};
        msg.vehicleId = ambulance.getId();
        msg.priority = ambulance.getPriority();
        msg.isEmergency = ambulance.isEmergency();
        strncpy(msg.origin, "F10", sizeof(msg.origin) - 1);
        strncpy(msg.destination, "F11", sizeof(msg.destination) - 1);

        cout << "[" << name << "] Sending emergency intent for vehicle "
             << msg.vehicleId << " from " << msg.origin << " to " << msg.destination << endl;
        TrafficController::sendMessage(writeFd, msg);

        // Locally preempt and cross the emergency vehicle.
        controller.crossVehicle(&ambulance);

        cout << "[" << name << "] Emergency vehicle has crossed. Exiting controller." << endl;
    } else if (name == "F11") {
        // Wait for F10's message.
        ControllerMessage incoming{};
        if (TrafficController::receiveMessage(readFd, incoming)) {
            cout << "[" << name << "] Received intent: vehicle " << incoming.vehicleId
                 << ", emergency=" << (incoming.isEmergency ? "yes" : "no")
                 << ", from " << incoming.origin << " to " << incoming.destination << endl;

            if (incoming.isEmergency) {
                cout << "[" << name << "] Clearing paths for incoming emergency from "
                     << incoming.origin << " to " << incoming.destination << endl;
            }
        } else {
            cerr << "[" << name << "] Failed to receive message." << endl;
        }

        cout << "[" << name << "] Controller exiting." << endl;
    }
}

int main() {
    int f10ToF11[2];
    int f11ToF10[2];

    if (pipe(f10ToF11) == -1 || pipe(f11ToF10) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pidF10 = fork();
    if (pidF10 == 0) {
        // Child: F10 controller process
        close(f10ToF11[0]); // will only write to F11
        close(f11ToF10[1]); // will only read from F11 (unused in this simple demo)

        runControllerProcess("F10", f11ToF10[0], f10ToF11[1]);
        close(f10ToF11[1]);
        close(f11ToF10[0]);
        _exit(0);
    }

    pid_t pidF11 = fork();
    if (pidF11 == 0) {
        // Child: F11 controller process
        close(f10ToF11[1]); // will read from F10
        close(f11ToF10[0]); // will write to F10 (unused in this simple demo)

        runControllerProcess("F11", f10ToF11[0], f11ToF10[1]);
        close(f10ToF11[0]);
        close(f11ToF10[1]);
        _exit(0);
    }

    // Parent: close all pipe ends and wait for children.
    close(f10ToF11[0]);
    close(f10ToF11[1]);
    close(f11ToF10[0]);
    close(f11ToF10[1]);

    int status = 0;
    waitpid(pidF10, &status, 0);
    waitpid(pidF11, &status, 0);

    cout << "Controller demo completed." << endl;
    return 0;
}
