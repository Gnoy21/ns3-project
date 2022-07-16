#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/olsr-module.h"
#include "ns3/minyong-application.h"
#include "ns3/spectrum-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/opengym-module.h"
#include <iostream>
#include <fstream>

#include "ns3/netanim-module.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OpenGym");
    
void Run ();
void CreateNodes ();
void CreateDevices ();
void SetMobility ();
void InstallInternetStack ();
void InstallApplications ();
void Test ();

Ipv4Address GetIpv4Address (uint32_t index);

uint8_t nodeNum;
uint8_t uNodeNum = 25;
uint8_t hNodeNum = 30;
uint32_t seed = 5;
double duration = 600000;
double envStepTime = 80;
uint32_t openGymPort = 5555;
Ptr<MinyongApplication> app[56];

NodeContainer nodes;
NodeContainer uNodes;
NodeContainer hNodes;
NodeContainer bNodes;
NetDeviceContainer devices;
NetDeviceContainer uDevices;
NetDeviceContainer hDevices;
NetDeviceContainer bDevices;
Ipv4InterfaceContainer interfaces;

void
Run ()
{
    CreateNodes ();
    CreateDevices ();
    SetMobility ();
    InstallInternetStack ();
    InstallApplications ();
}

void
CreateNodes ()
{
    bNodes.Create (1);
    uNodes.Create (uNodeNum);
    hNodes.Create (hNodeNum);

    nodeNum = 1 + uNodeNum + hNodeNum;
    
    nodes.Add (bNodes);
    nodes.Add (uNodes);
    nodes.Add (hNodes);
}

void
CreateDevices ()
{
    std::string phyMode ("ErpOfdmRate54Mbps");
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211g);

    YansWifiPhyHelper wifiPhy;

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (150.0));
    wifiPhy.SetChannel (wifiChannel.Create ());

    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue (phyMode));

    wifiMac.SetType ("ns3::AdhocWifiMac");
    devices = wifi.Install (wifiPhy, wifiMac, nodes);
}

void
SetMobility ()
{
    RngSeedManager::SetSeed(seed);

    MobilityHelper bMobility;
    Ptr<ListPositionAllocator> bPositionAlloc = CreateObject<ListPositionAllocator> ();
    bPositionAlloc->Add (Vector (0.0, 0.0, 0.0));
    bMobility.SetPositionAllocator (bPositionAlloc);
    bMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    bMobility.Install (bNodes);

    MobilityHelper uMobility;
    Ptr<ListPositionAllocator> uPositionAlloc = CreateObject<ListPositionAllocator> ();
    for (uint8_t i = 0; i < 5; i++)
    {
        uPositionAlloc->Add (Vector (50.0 + (100.0 * i), 50.0, 0.0));
        uPositionAlloc->Add (Vector (50.0 + (100.0 * i), 150.0, 0.0));
        uPositionAlloc->Add (Vector (50.0 + (100.0 * i), 250.0, 0.0));
        uPositionAlloc->Add (Vector (50.0 + (100.0 * i), 350.0, 0.0));
        uPositionAlloc->Add (Vector (50.0 + (100.0 * i), 450.0, 0.0));
    }
    uMobility.SetPositionAllocator (uPositionAlloc);
    uMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    uMobility.Install (uNodes);

    MobilityHelper hMobility;
    hMobility.SetPositionAllocator (
        "ns3::RandomBoxPositionAllocator",
        "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=500]"),
        "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=500]"),
        "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]")
    );
    hMobility.SetMobilityModel (
        "ns3::RandomDirection2dMobilityModel",
        "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=3.0]"),
        "Bounds", StringValue ("0|500|0|500")
    );
    hMobility.Install (hNodes);
}

void
InstallInternetStack ()
{
    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("1.0.0.0", "255.0.0.0");
    interfaces = address.Assign (devices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

void
InstallApplications ()
{
    LogComponentEnable ("MinyongApplication", LOG_LEVEL_INFO);

    int* tmpLinkState = new int[40];

    for (int i = 0; i < 40; i++)
    {
        tmpLinkState[i] = (rand () % 30) + 1;
    }

    app[0] = CreateObject<MinyongApplication> ();
    app[0]->SetNodeType (BS);
    app[0]->SetPort (50001);
    app[0]->SetStartTime (Seconds (0.5));
    app[0]->SetStopTime (Seconds (duration));
    nodes.Get (0)->AddApplication (app[0]);

    for (int i = 1; i <= uNodeNum; i++)
    {
        app[i] = CreateObject<MinyongApplication> ();
        app[i]->SetNodeType (UAV);
        app[i]->SetPort (50001);
        app[i]->SetId (i+1);
        app[i]->SetLinkStateInfo (tmpLinkState);
        app[i]->SetStartTime (Seconds (0.5 + 0.05 * i));
        app[i]->SetStopTime (Seconds (duration));
        nodes.Get (i)->AddApplication (app[i]);
    }

    for (int i = (uNodeNum+1); i < nodeNum; i++)
    {
        app[i] = CreateObject<MinyongApplication> ();
        app[i]->SetNodeType (HUMAN);
        app[i]->SetPort (50001);
        app[i]->SetId (i+1);
        app[i]->SetStartTime (Seconds (0.5 + 0.05 * i));
        app[i]->SetStopTime (Seconds (duration));
        nodes.Get (i)->AddApplication (app[i]);
    }
}

Ipv4Address
GetIpv4Address (uint32_t index)
{
	return nodes.Get(index)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
}

Ptr<OpenGymSpace> 
MyGetObservationSpace ()
{
    uint32_t nodeNum = 122;
    float low = 0.0;
    float high = 500.0;
    std::vector<uint32_t> shape = {nodeNum,};
    std::string dtype = TypeNameGet<double> ();
    Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
    NS_LOG_UNCOND ("MyGetObservationSpace: " << space);
    
    return space;
}

Ptr<OpenGymSpace> 
MyGetActionSpace ()
{
    uint32_t nodeNum = 25;
    float low = 0.0;
    float high = 1.0;
    std::vector<uint32_t> shape = {nodeNum,};
    std::string dtype = TypeNameGet<uint32_t> ();
    Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
    NS_LOG_UNCOND ("MyGetActionSpace: " << space);

    return space;
}

bool 
MyGetGameOver ()
{
    bool isGameOver = false;
    bool test = false;
    static float stepCounter = 0.0;
    stepCounter += 1;
    if (stepCounter == 10 && test) {
        isGameOver = true;
    }
    NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
    
    return isGameOver;
}

Ptr<OpenGymDataContainer> 
MyGetObservation ()
{
    NS_LOG_UNCOND("MyGetObservation");
    uint32_t nodeNum = 122;
    //float low = 0.0;
    //float high = 500.0;
    Ptr<UniformRandomVariable> rngInt = CreateObject<UniformRandomVariable> ();

    std::vector<uint32_t> shape = {nodeNum,};
    Ptr<OpenGymBoxContainer<double> > box = CreateObject<OpenGymBoxContainer<double> >(shape);

    vector<PendingData> data;
    data = app[0]->GetPendingData ();

    double tmp_array[30][4];

    uint32_t usedChannel[40];

    for (int i = 0; i < 30; i++)
    {
        tmp_array[i][0] = -1;
        tmp_array[i][1] = -1;
        tmp_array[i][2] = -1;
        tmp_array[i][3] = -1;
    }

    for (int i = 0; i < 40; i++)
    {
        usedChannel[i] = 0;
    }

    double linkCount = 0;
    for (vector<PendingData>::iterator iter = data.begin (); iter != data.end (); iter++)
    {
        //NS_LOG_UNCOND (iter->humanId-27);
        tmp_array[iter->humanId-27][0] = iter->positionX;
        tmp_array[iter->humanId-27][1] = iter->positionY;
        tmp_array[iter->humanId-27][2] = iter->angle;
        tmp_array[iter->humanId-27][3] = iter->velocity;

        for (uint32_t i = 0; i < iter->routeSize; i++)
        {
            usedChannel[iter->route[i]] = 1;
        }
    }

    for (int i = 0; i < 40; i++)
    {
        if (usedChannel[i] == 1)
        {
            linkCount++;
        }
    }

    box->AddValue ((double)data.size ());
    box->AddValue (linkCount);

    for (int i = 0; i < 30; i++)
    {
        box->AddValue (tmp_array[i][0]);
        box->AddValue (tmp_array[i][1]);
        box->AddValue (tmp_array[i][2]);
        box->AddValue (tmp_array[i][3]);
    }

    NS_LOG_UNCOND ("MyGetObservation: " << box);

    return box;
}

float 
MyGetReward ()
{
    static float reward = 0.0;
    reward += 1;
    
    return reward;
}

string 
MyGetExtraInfo ()
{
    string myInfo = "testInfo";
    myInfo += "|123";
    NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);

    return myInfo;
}

bool 
MyExecuteActions (Ptr<OpenGymDataContainer> action)
{
    Ptr<OpenGymBoxContainer<uint32_t>> box = DynamicCast<OpenGymBoxContainer<uint32_t>> (action);
    if (box != nullptr)
    {
        for (int i = 1; i < 27; i++)
        {
            app[i]->ResetRoute (box->GetData ());
        }
    }

    NS_LOG_UNCOND ("MyExecuteActions: " << action);
    
    return true;
}

void 
ScheduleNextStateRead (double envStepTime, Ptr<OpenGymInterface> openGym)
{
    Simulator::Schedule (Seconds (envStepTime), &ScheduleNextStateRead, envStepTime, openGym);
    openGym->NotifyCurrentState ();
}

int
main (int argc, char *argv[])
{
    Run ();

    NS_LOG_UNCOND("Ns3Env parameters:");
    NS_LOG_UNCOND("--simulationTime: " << duration);
    NS_LOG_UNCOND("--openGymPort: " << openGymPort);
    NS_LOG_UNCOND("--envStepTime: " << envStepTime);
    NS_LOG_UNCOND("--seed: " << seed);
    
    Ptr<OpenGymInterface> openGym = CreateObject<OpenGymInterface> (openGymPort);
    openGym->SetGetActionSpaceCb( MakeCallback (&MyGetActionSpace) );
    openGym->SetGetObservationSpaceCb( MakeCallback (&MyGetObservationSpace) );
    openGym->SetGetGameOverCb( MakeCallback (&MyGetGameOver) );
    openGym->SetGetObservationCb( MakeCallback (&MyGetObservation) );
    openGym->SetGetRewardCb( MakeCallback (&MyGetReward) );
    openGym->SetGetExtraInfoCb( MakeCallback (&MyGetExtraInfo) );
    openGym->SetExecuteActionsCb( MakeCallback (&MyExecuteActions) );
    Simulator::Schedule (Seconds(envStepTime), &ScheduleNextStateRead, envStepTime, openGym);
    
    NS_LOG_UNCOND ("Simulation start");
    Simulator::Stop (Seconds (duration));
    Simulator::Run ();

    openGym->NotifySimulationEnd();
    Simulator::Destroy ();

    return 0;
}