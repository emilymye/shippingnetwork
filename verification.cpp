#include <iostream>
#include <vector>
#include "Exception.h"
#include "Instance.h"
#include "ActivityImpl.h"

using namespace std;
using Fwk::Exception;

static const string CAPACITY_TRUCK = "2";
static const string CAPACITY_PLANE = "5";
static const string TRANSFER_RATE = "48";
static const string SHIPMENT_SIZE = "50";
static const int TIME_STEP = 24;
static const int STEPS = 10;

int main(int argc, char *argv[]) {
    try
    {
        Ptr<Instance::Manager> manager = shippingInstanceManager();

        Ptr<Instance> fleet = manager->instanceNew("fleet", "Fleet");
        fleet->attributeIs("Truck, speed", "50");
        fleet->attributeIs("Truck, cost", "1");
        fleet->attributeIs("Truck, capacity", "50");

        fleet->attributeIs("Plane, speed", "100");
        fleet->attributeIs("Plane, cost", "2.0");
        fleet->attributeIs("Plane, capacity", "100");

        /********************************************************
        ******************* | NETWORK SETUP | ******************
        ********************************************************/

        Ptr<Instance> source = manager->instanceNew("source", "Customer");
        Ptr<Instance> otherCustomer = manager->instanceNew("other", "Customer");
        Ptr<Instance> dest = manager->instanceNew("dest", "Customer");
        source->attributeIs("Destination", "dest");
        Ptr<Instance> truckT = manager->instanceNew("truckT", "Truck terminal");
        Ptr<Instance> planeT = manager->instanceNew("planeT", "Plane terminal");

        // source <-seg1-> truck terminal <-seg2-> dest
        Ptr<Instance> seg1 = manager->instanceNew("seg1", "Truck segment");
        Ptr<Instance> rseg1 = manager->instanceNew("rseg1", "Truck segment");
        seg1->attributeIs("source", "source");
        rseg1->attributeIs("source", "truckT");
        seg1->attributeIs("return segment", "rseg1");
        Ptr<Instance> seg2 = manager->instanceNew("seg2", "Truck segment");
        Ptr<Instance> rseg2 = manager->instanceNew("rseg2", "Truck segment");
        seg2->attributeIs("source", "truckT");
        rseg2->attributeIs("source", "dest");
        seg2->attributeIs("return segment", "rseg2");
        seg1->attributeIs( "length", "400");
        rseg1->attributeIs("length", "400");
        seg2->attributeIs( "length", "400");
        rseg2->attributeIs("length", "400");

        // source <-seg3-> port terminal <-seg4-> dest 
        Ptr<Instance> seg3 = manager->instanceNew("seg3", "Plane segment");
        Ptr<Instance> rseg3 = manager->instanceNew("rseg3", "Plane segment");
        seg3->attributeIs("source", "source");
        rseg3->attributeIs("source", "planeT");
        seg3->attributeIs("return segment", "rseg3");
        Ptr<Instance> seg4 = manager->instanceNew("seg4", "Plane segment");
        Ptr<Instance> rseg4 = manager->instanceNew("rseg4", "Plane segment");
        seg4->attributeIs("source", "planeT");
        rseg4->attributeIs("source", "dest");
        seg4->attributeIs("return segment", "rseg4");
        seg3->attributeIs("length", "400");
        rseg3->attributeIs("length", "400");
        seg4->attributeIs("length", "400");
        rseg4->attributeIs("length", "400");

        //source <-seg5-> other customer <-seg6-> dest
        //to verify customers do not get routed to
        Ptr<Instance> seg5 = manager->instanceNew("seg5", "Plane segment");
        Ptr<Instance> rseg5 = manager->instanceNew("rseg5", "Plane segment");
        seg5->attributeIs("source", "source");
        rseg5->attributeIs("source", "other");
        seg5->attributeIs("return segment", "rseg5");
        Ptr<Instance> seg6 = manager->instanceNew("seg6", "Plane segment");
        Ptr<Instance> rseg6 = manager->instanceNew("rseg6", "Plane segment");
        seg6->attributeIs("return segment", "rseg6");
        seg6->attributeIs("source", "other");
        rseg6->attributeIs("source", "dest");

        seg5->attributeIs("length", "200");
        rseg5->attributeIs("length", "200");
        seg6->attributeIs("length", "200");
        rseg6->attributeIs("length", "200");

        /********************************************************
        ****************| END NETWORK SETUP | ******************
        ********************************************************/

        // TRUCK SEGMENTS
        seg1->attributeIs( "Capacity", CAPACITY_TRUCK);
        rseg1->attributeIs("Capacity", CAPACITY_TRUCK);
        seg2->attributeIs( "Capacity", CAPACITY_TRUCK);
        rseg2->attributeIs("Capacity", CAPACITY_TRUCK);

        // PLANE SEGMENTS
        seg3->attributeIs( "Capacity", CAPACITY_PLANE);
        rseg3->attributeIs("Capacity", CAPACITY_PLANE);
        seg4->attributeIs( "Capacity", CAPACITY_PLANE);
        rseg4->attributeIs("Capacity", CAPACITY_PLANE);

        //OTHER ATTRIBUTES 
        source->attributeIs("Transfer Rate", TRANSFER_RATE);
        source->attributeIs("Shipment Size", SHIPMENT_SIZE);

        Activity::Manager::Ptr activityManager = activityManagerInstance();
        // RealTimeManager::Ptr realTimeManager = realTimeManagerInstance();

        for ( int i = 1; i <= STEPS; i++){
            activityManager->nowIs(TIME_STEP * i);
            // realTimeManager->realTimePassedIs(TIME_STEP);

            cout << "AT TIME: " << TIME_STEP * i << endl;
            cout << "SOURCE->TRUCK: Capacity " << seg1->attribute("Capacity") << 
                "    recieved " << seg1->attribute("Shipments Received") << 
                "    refused " << seg1->attribute("Shipments Refused") << endl;
            cout << "TRUCK->DEST : Capacity " << seg2->attribute("Capacity") <<
                "    recieved " << seg2->attribute("Shipments Received") << 
                "    refused " << seg2->attribute("Shipments Refused") << endl;
            cout << "SOURCE->PLANE: Capacity " << seg3->attribute("Capacity") <<
                "    recieved " << seg3->attribute("Shipments Received") << 
                "    refused " << seg3->attribute("Shipments Refused") << endl;
            cout << "PLANE->DEST: Capacity " << seg4->attribute("Capacity") <<
                "    recieved " << seg4->attribute("Shipments Received") << 
                "    refused " << seg4->attribute("Shipments Refused") << endl << endl;

            cout << "DESTINATION: " << endl << 
                "   Recieved Shipments:" << dest->attribute("Shipments Received") << 
                "   Average latency: " << dest->attribute("Average Latency") << 
                "   Total cost: " << dest->attribute("Total Cost") << endl;
            cout << endl << endl;;
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

