#include <iostream>
#include <vector>
#include "Exception.h"
#include "Instance.h"
#include "ActivityImpl.h"

using namespace std;
using Fwk::Exception;

int main(int argc, char *argv[]) {
  try
  {
    Ptr<Instance::Manager> manager = shippingInstanceManager();
    
    Ptr<Instance> fleet = manager->instanceNew("fleet", "Fleet");
    fleet->attributeIs("Truck, speed", "50");
    fleet->attributeIs("Truck, cost", "1");
    fleet->attributeIs("Truck, capacity", "30");

    fleet->attributeIs("Plane, speed", "100");
    fleet->attributeIs("Plane, cost", "1");
    fleet->attributeIs("Plane, capacity", "50");
    
    Ptr<Instance> source = manager->instanceNew("source", "Customer");
    Ptr<Instance> otherCustomer = manager->instanceNew("other", "Customer");
    Ptr<Instance> dest = manager->instanceNew("dest", "Customer");
    Ptr<Instance> truckT = manager->instanceNew("truckT", "Truck terminal");
    Ptr<Instance> planeT = manager->instanceNew("planeT", "Plane terminal");

    // source <-seg1-> truck terminal <-seg2-> dest
    Ptr<Instance> seg1 = manager->instanceNew("seg1", "Truck segment");
    Ptr<Instance> rseg1 = manager->instanceNew("rseg1", "Truck segment");
    seg1->attributeIs("source", "source");
    rseg1->attributeIs("source", "truckT");
    seg1->attributeIs("return segment", "rseg1");

    seg1->attributeIs("length", "400");
    rseg1->attributeIs("length", "400");

    Ptr<Instance> seg2 = manager->instanceNew("seg2", "Truck segment");
    Ptr<Instance> rseg2 = manager->instanceNew("rseg2", "Truck segment");
    seg2->attributeIs("source", "truckT");
    rseg2->attributeIs("source", "dest");
    seg2->attributeIs("return segment", "rseg2");

    seg2->attributeIs("length", "400");
    rseg2->attributeIs("length", "400");

    // source <-seg3-> port terminal <-seg4-> dest 
    Ptr<Instance> seg3 = manager->instanceNew("seg3", "Plane segment");
    Ptr<Instance> rseg3 = manager->instanceNew("rseg3", "Plane segment");
    seg3->attributeIs("source", "source");
    rseg3->attributeIs("source", "planeT");
    seg3->attributeIs("return segment", "rseg3");

    seg3->attributeIs("length", "400");
    rseg3->attributeIs("length", "400");
    
    Ptr<Instance> seg4 = manager->instanceNew("seg4", "Plane segment");
    Ptr<Instance> rseg4 = manager->instanceNew("rseg4", "Plane segment");
    seg4->attributeIs("source", "planeT");
    rseg4->attributeIs("source", "dest");
    seg4->attributeIs("return segment", "rseg4");
    
    seg4->attributeIs("length", "400");
    rseg4->attributeIs("length", "400");

    //NOT TO BE ROUTED ALONG
    // source <-seg5-> other customer <-seg6-> dest
    Ptr<Instance> seg5 = manager->instanceNew("seg5", "Plane segment");
    Ptr<Instance> rseg5 = manager->instanceNew("rseg5", "Plane segment");
    seg5->attributeIs("source", "source");
    rseg5->attributeIs("source", "other");
    seg5->attributeIs("return segment", "rseg5");

    seg5->attributeIs("length", "200");
    rseg5->attributeIs("length", "200");

    Ptr<Instance> seg6 = manager->instanceNew("seg6", "Plane segment");
    Ptr<Instance> rseg6 = manager->instanceNew("rseg6", "Plane segment");
    seg6->attributeIs("source", "other");
    rseg6->attributeIs("source", "dest");
    seg6->attributeIs("return segment", "rseg6");
    
    seg6->attributeIs("length", "200");
    rseg6->attributeIs("length", "200");

    //Set up shipment source attributes
    source->attributeIs("Transfer Rate", "2");
    source->attributeIs("Shipment Size", "10");
    source->attributeIs("Destination", "dest");

    Activity::Manager::Ptr activityManager = activityManagerInstance();
    
    for (int i=0; i<10; i++){
        activityManager->nowIs(12.0 * i);
        //printSimStats();
    }
  }
  catch(Exception e)
  {
	cout << e.what() << endl;
  }
  catch(...)
  {
  }

  return 0;
}

