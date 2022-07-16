/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This is an example script for AODV manet routing protocol. 
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */

#include <iostream>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/paparazzi-mobility-model.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

/**
 * \ingroup aodv-examples
 * \ingroup examples
 * \brief Test script.
 * 
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 * 
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.0.4]
 * 
 * ping 10.0.0.4
 *
 * When 1/3 of simulation time has elapsed, one of the nodes is moved out of
 * range, thereby breaking the topology.  By default, this will result in
 * only 34 of 100 pings being received.  If the step size is reduced
 * to cover the gap, then all pings can be received.
 */
class AodvExample 
{
public:
  AodvExample ();
  /**
   * \brief Configure script parameters
   * \param argc is the command line argument count
   * \param argv is the command line arguments
   * \return true on successful configuration
  */
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /**
   * Report results
   * \param os the output stream
   */
  void Report (std::ostream & os);

private:
  uint32_t nodeNum;
  double duration;
  uint32_t seed;
  double mapSize;
  bool useInterfer;
  
  NodeContainer nodes;
  NodeContainer interfererNodes;
  NetDeviceContainer devices;
  NetDeviceContainer interfererDevices;
  Ipv4InterfaceContainer interfaces;

private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
  static void generateTraffic(Ptr<Socket> socket, uint32_t pktSize, double pktCount, Time pktInterval);
};

int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

AodvExample::AodvExample () :
  nodeNum (100),
  duration (600),
  seed(100),
  mapSize(1000),
  useInterfer(false)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
  CommandLine cmd (__FILE__);

  cmd.AddValue ("nodeNum", "Number of nodes.", nodeNum);
  cmd.AddValue ("duration", "Simulation time, s.", duration);
  cmd.AddValue ("seed", "SetSeed", seed);
  cmd.AddValue ("mapSize", "Size of Map", mapSize);
  cmd.AddValue ("useInterfer", "use Interfer Node", useInterfer);

  cmd.Parse (argc, argv);
  return true;
}

void
AodvExample::Run ()
{
  SeedManager::SetSeed (seed);
  //Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << duration << " s ...\n";

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();
  Simulator::Destroy ();
}

void
AodvExample::Report (std::ostream &)
{
}

void
AodvExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)nodeNum << " nodes " << endl;
  nodes.Create (nodeNum);
  
  if (useInterfer)
  {
    interfererNodes.Create (4);
  }

  MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
			"X", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+to_string(mapSize)+"]"),
			"Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+to_string(mapSize)+"]"),
			"Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]"));

	mobility.SetMobilityModel ("ns3::PaparazziMobilityModel",
			"Bounds", BoxValue (Box (0, mapSize, 0, mapSize, 0, 0)),
			"Speed", DoubleValue (11)); // 40km/h
    
  mobility.Install (nodes);

  if (useInterfer)
  {
    MobilityHelper mobility2;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (200.0, 200.0, 0.0));
    positionAlloc->Add (Vector (800.0, 200.0, 0.0));
    positionAlloc->Add (Vector (200.0, 800.0, 0.0));
    positionAlloc->Add (Vector (800.0, 800.0, 0.0));
    mobility2.SetPositionAllocator (positionAlloc);
    mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility2.Install (interfererNodes);
  }

}

void
AodvExample::CreateDevices ()
{
  std::string phyMode ("DsssRate1Mbps");

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  
  YansWifiPhyHelper wifiPhy;
  
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  
  wifiPhy.Set ("TxGain", DoubleValue (9));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  if(useInterfer)
    {
        wifiPhy.Set ("TxGain", DoubleValue (-4));
        interfererDevices = wifi.Install (wifiPhy, wifiMac, interfererNodes);
    }
}

void
AodvExample::InstallInternetStack ()
{
  AodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (nodes);

  InternetStackHelper stack1;
  stack1.Install (interfererNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  if(useInterfer)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    for (int i = 0; i < 4; i++)
    {
      Ptr<Socket> interferer = Socket::CreateSocket (interfererNodes.Get(i), tid);
      InetSocketAddress interferingAddr = InetSocketAddress (Ipv4Address ("255.255.255.255"), 49000);
      interferer->SetAllowBroadcast (true);
      interferer->Connect (interferingAddr);

      Simulator::ScheduleWithContext (interferer->GetNode()->GetId(), Seconds(1), &AodvExample::generateTraffic, interferer, 1000, duration, Seconds(1.0));
    }
  }
}

void
AodvExample::InstallApplications ()
{
  V4PingHelper ping (interfaces.GetAddress (nodeNum - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));
  ping.SetAttribute ("Interval", TimeValue(Seconds(1)));

  ApplicationContainer p = ping.Install (nodes.Get (0));
  p.Start (Seconds (0));
  p.Stop (Seconds (duration) - Seconds (0.001));

  // move node away
  /*
  Ptr<Node> node = nodes.Get (size/2);
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
  */
}

void
AodvExample::generateTraffic(Ptr<Socket> socket, uint32_t pktSize, double pktCount, Time pktInterval)
{
    if (pktCount > 0)
    {
        socket->Send (Create<Packet> (pktSize));
        Simulator::Schedule (pktInterval, &AodvExample::generateTraffic, socket, pktSize, pktCount - 1, pktInterval);
    }
    else
    {
        socket->Close ();
    }
}

