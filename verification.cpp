#include <string>
#include <ostream>
#include <iostream>
#include <string>
#include "Instance.h"
#include "ActivityImpl.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

int main(int argc, char *argv[]) {
    Ptr<Instance::Manager> m = shippingInstanceManager();
    Activity::Manager::Ptr activityManager = activityManagerInstance();
    activityManager->nowIs(6.0);
    activityManager->nowIs(24.0);

    // RealTimeManager::Ptr realTimeManager = realTimeManagerInstance();
    // realTimeManager->realTimePassedIs(6.0);
    //  realTimeManager->realTimePassedIs(18.0);
    return 0;
}