#include <string>
#include <set>
#include <ostream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include "Instance.h"
#include "ActivityImpl.h"
#include "Exception.h"

using namespace std;
using Fwk::Exception;

bool useRandom = true;

static const int TIME_STEP = 1000;
static const int NSTEPS = 20;

//windows testing does not a boost-happy experience make
static const string numToStr  = "0123456789";


int main(int argc, char *argv[]) {
    try
    {
        Ptr<Instance::Manager> m = shippingInstanceManager();
        Ptr<Instance> fleet = m->instanceNew("fleet", "Fleet");
        fleet->attributeIs("Truck, speed", "50");
        fleet->attributeIs("Truck, cost", "1");
        fleet->attributeIs("Truck, capacity", "100");

        //DESTINATION
        Ptr<Instance> dest = m->instanceNew("dest", "Customer");
        Ptr<Instance> destT = m->instanceNew("destT","Truck terminal");
        Ptr<Instance> bridge = m->instanceNew("bridge", "Truck segment");
        Ptr<Instance> rbridge = m->instanceNew("rbridge", "Truck segment");
        bridge->attributeIs("source","destT");
        rbridge->attributeIs("source","dest");
        bridge->attributeIs("return segment","rbridge");
        bridge->attributeIs("length", "20");
        bridge->attributeIs("length", "20");

        Ptr<Instance> terminals[10];
        Ptr<Instance> terminalSegments[10];
        Ptr<Instance> sources[100];
        Ptr<Instance> sourceSegments[100];

        for (int i = 0; i < 10; i++){
            string termString = "term";
            termString.append(1,numToStr[i]);
            Ptr<Instance> term = m->instanceNew(termString, "Truck terminal");
            Ptr<Instance> seg = m->instanceNew("seg" + termString, "Truck segment");
            Ptr<Instance> rseg = m->instanceNew("rseg" + termString, "Truck segment");
            seg->attributeIs("source",termString);
            rseg->attributeIs("source", "destT");
            seg->attributeIs("return segment","rseg" + termString);
            seg->attributeIs("length", "20");
            rseg->attributeIs("length", "20");

            terminals[i] = term;
            terminalSegments[i] = seg;


            for (int j = 0; j < 10; j++){
                string sourceStr = termString + "cust";
                sourceStr.append(1,numToStr[j]);

                Ptr<Instance> source = m->instanceNew(sourceStr, "Customer");
                Ptr<Instance> cseg = m->instanceNew("seg" + sourceStr, "Truck segment");
                Ptr<Instance> rcseg = m->instanceNew("rseg" + sourceStr, "Truck segment");
                cseg->attributeIs("source",sourceStr);
                rcseg->attributeIs("source", termString);
                cseg->attributeIs("return segment","rseg" + sourceStr);
                cseg->attributeIs("length", "20");
                rcseg->attributeIs("length", "20");

                int idx = i * 10 + j;
                sources[idx] = source;
                sourceSegments[idx] = cseg;

                string size = "100";
                if (useRandom) {
                    stringstream ss; 
                    int i = (rand() % 1000) + 1;
                    ss << i;
                    size = ss.str();
                }

                source->attributeIs("Transfer Rate", "10");
                source->attributeIs("Shipment Size", size);
                source->attributeIs("Destination", "dest");
            }
        }

        Activity::Manager::Ptr activityManager = activityManagerInstance();
        // RealTimeManager::Ptr realTimeManager = realTimeManagerInstance();

        for (int step = 1; step <= NSTEPS; step++){
            activityManager->nowIs(step * TIME_STEP);
            // realTimeManager->realTimePassedIs(TIME_STEP);
            cout <<"AT STEP " << step << endl;
            cout << "DESTINATION:   Recieved Shipments:" << dest->attribute("Shipments Received") << 
                "   Average latency: " << dest->attribute("Average Latency") << 
                "   Total cost: " << dest->attribute("Total Cost") << endl;
            cout << "Bridge:   Recieved " << bridge->attribute("Shipments Received") << " Refused " + bridge->attribute("Shipments Refused") << endl;

                float innerReceived = 0;
                float innerRejected = 0;
                for (int ts = 0; ts < 10; ts++){
                    innerReceived += atoi(terminalSegments[ts]->attribute("Shipments Received").c_str());
                    innerRejected += atoi(terminalSegments[ts]->attribute("Shipments Refused").c_str());

                    float refusedC = 0;
                    float receivedC = 0;
                    for (int tc = 0 ; tc < 100; tc++ ){
                        receivedC += atoi(sourceSegments[tc]->attribute("Shipments Received").c_str());
                        refusedC += atoi(sourceSegments[tc]->attribute("Shipments Refused").c_str());
                    }

                    cout << "CUSTOMERS to TERMINAL " << ts << " AVERAGES: Received " << receivedC/10.0 << " refused " << refusedC/10.0 << endl;
                }
                cout << "Terminal to Central Terminal AVERAGES: Received " << innerReceived/10.0 << " refused " << innerRejected/10.0<< endl;
            
        }
    } catch(Exception e)
    {
        cout << e.what() << endl;
    }
    catch(...)
    {
    }

    return 0;
}

