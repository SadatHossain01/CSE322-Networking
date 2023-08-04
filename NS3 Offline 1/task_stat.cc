/*
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
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

#define PACKET_SIZE 1024 // in bytes

/*
Network Topology
    s0 ------           --------r0
    s1 ------           --------r1
    s2 ------x --p2--p y--------r2
    s3 ------           --------r3
    s4 ------           --------r4
*/

NS_LOG_COMPONENT_DEFINE("TaskStat");

using namespace ns3;

int
main(int argc, char* argv[])
{
    uint64_t nNodes = 20;
    uint64_t nLeftNodeCount = nNodes / 2;
    uint64_t nFlows = 50;
    uint64_t nPackets = 500; // per second
    uint64_t coverageArea = 5;
    uint64_t port = 9;

    LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);
    cmd.AddValue("nNodes", "Number of sender and receiver stations", nNodes);
    cmd.AddValue("nFlows", "Number of flows", nFlows);
    cmd.AddValue("nPackets", "Number of packets to be sent per second", nPackets);
    cmd.AddValue("coverageArea", "Set coverage area of the network", coverageArea);

    Time::SetResolution(Time::NS);

    NodeContainer p2pNodes;
    NodeContainer leftWifiStationNodes;
    NodeContainer rightWifiStationNodes;
    NodeContainer leftWifiApNode;
    NodeContainer rightWifiApNode;

    p2pNodes.Create(2);

    // left wifi
    leftWifiStationNodes.Create(nLeftNodeCount);
    leftWifiApNode = p2pNodes.Get(0);

    // right wifi
    rightWifiStationNodes.Create(nNodes - nLeftNodeCount);
    rightWifiApNode = p2pNodes.Get(1);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                               "MaxRange",
                               DoubleValue(coverageArea * 5));
    YansWifiPhyHelper phyLeft;
    phyLeft.SetChannel(channel.Create());

    YansWifiPhyHelper phyRight;
    phyRight.SetChannel(channel.Create());

    Ssid ssidLeft = Ssid("ssid-left");
    Ssid ssidRight = Ssid("ssid-right");

    WifiHelper wifi;
    WifiMacHelper mac;

    // left wifi
    mac.SetType("ns3::StaWifiMac",
                "Ssid",
                SsidValue(ssidLeft),
                "ActiveProbing",
                BooleanValue(false));
    NetDeviceContainer leftWifiStationDevices = wifi.Install(phyLeft, mac, leftWifiStationNodes);
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidLeft));
    NetDeviceContainer leftWifiApDevice = wifi.Install(phyLeft, mac, leftWifiApNode);

    // right wifi
    mac.SetType("ns3::StaWifiMac",
                "Ssid",
                SsidValue(ssidRight),
                "ActiveProbing",
                BooleanValue(false));
    NetDeviceContainer rightWifiStationDevices = wifi.Install(phyRight, mac, rightWifiStationNodes);
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidRight));
    NetDeviceContainer rightWifiApDevice = wifi.Install(phyRight, mac, rightWifiApNode);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(p2pNodes);
    p2pNodes.Get(0)->GetObject<ConstantPositionMobilityModel>()->SetPosition(
        Vector(2.0, nLeftNodeCount / 2, 0.0));
    p2pNodes.Get(1)->GetObject<ConstantPositionMobilityModel>()->SetPosition(
        Vector(4.0, nLeftNodeCount / 2, 0.0));

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(1.0),
                                  "DeltaX",
                                  DoubleValue(1.0),
                                  "DeltaY",
                                  DoubleValue(1.0),
                                  "GridWidth",
                                  UintegerValue(1),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.Install(leftWifiStationNodes);
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(6.0),
                                  "MinY",
                                  DoubleValue(1.0),
                                  "DeltaX",
                                  DoubleValue(1.0),
                                  "DeltaY",
                                  DoubleValue(1.0),
                                  "GridWidth",
                                  UintegerValue(1),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.Install(rightWifiStationNodes);

    InternetStackHelper stack;
    stack.Install(p2pNodes);
    stack.Install(leftWifiStationNodes);
    stack.Install(rightWifiStationNodes);

    Ipv4AddressHelper address;

    // p2p
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    // left wifi
    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(leftWifiApDevice);
    address.Assign(leftWifiStationDevices);

    // right wifi
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer rightWifiInterfaces = address.Assign(rightWifiStationDevices);
    address.Assign(rightWifiApDevice);

    ApplicationContainer senderApps;
    ApplicationContainer sinkApps;

    // server
    for (uint64_t i = 0; i < nFlows; ++i)
    {
        OnOffHelper senderHelper(
            "ns3::TcpSocketFactory",
            (InetSocketAddress(rightWifiInterfaces.GetAddress(i % (nNodes - nLeftNodeCount)),
                               port)));
        senderHelper.SetAttribute("PacketSize", UintegerValue(PACKET_SIZE));
        senderHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        senderHelper.SetAttribute("OffTime",
                                  StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        senderHelper.SetAttribute("DataRate", DataRateValue(DataRate(nPackets * PACKET_SIZE)));
        senderApps.Add(senderHelper.Install(leftWifiStationNodes.Get(i % nLeftNodeCount)));
    }

    // client
    for (uint64_t i = 0; i < nNodes - nLeftNodeCount; ++i)
    {
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                    (InetSocketAddress(Ipv4Address::GetAny(), port)));
        sinkApps.Add(sinkHelper.Install(rightWifiStationNodes.Get(i)));
    }

    std::string animFile = "task-stat.xml";
    AnimationInterface anim(animFile);
    anim.EnablePacketMetadata();                                // Optional
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional

    senderApps.Start(Seconds(3.0));
    sinkApps.Start(Seconds(0.0));
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
