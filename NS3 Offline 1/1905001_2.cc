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

#include <fstream>
#include <string>

#define PACKET_SIZE 1024 // in bytes
#define RANGE 5

/*
Network Topology
    s0 ------           --------r0
    s1 ------           --------r1
    s2 ------x --p2p-- y--------r2
    s3 ------           --------r3
    s4 ------           --------r4
    All nodes except x and y are mobile nodes
*/

NS_LOG_COMPONENT_DEFINE("TaskMobile");

using namespace ns3;

double packetReceived = 0;
double packetSent = 0;
uint64_t receivedBits = 0;

void
CalculateReceived(Ptr<const Packet> packet, const Address& address)
{
    packetReceived += packet->GetSize() / PACKET_SIZE;
    receivedBits += packet->GetSize() * 8;
    // NS_LOG_UNCOND("Received " << packet->GetSize() << " bytes from " << address);
}

void
CalculateSent(Ptr<const Packet> packet)
{
    packetSent += packet->GetSize() / PACKET_SIZE;
    // NS_LOG_UNCOND("Sent " << packet->GetSize() << " bytes");
}

int
main(int argc, char* argv[])
{
    uint64_t nNodes = 20;
    uint64_t nFlows = 10;
    uint64_t nPackets = 100; // per second
    uint64_t speed = 5.0;   // in m/s
    uint64_t port = 8082;
    double simulationTime = 10.0;

    // LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes", "Number of sender and receiver stations", nNodes);
    cmd.AddValue("nFlows", "Number of flows", nFlows);
    cmd.AddValue("nPackets", "Number of packets sent per second", nPackets);
    cmd.AddValue("speed", "Speed of the mobile nodes", speed);
    cmd.Parse(argc, argv);

    uint64_t nLeftNodeCount = nNodes / 2;
    uint64_t nRightNodeCount = nNodes - nLeftNodeCount;

    if (nFlows < nLeftNodeCount)
        nFlows = nLeftNodeCount;

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(PACKET_SIZE));

    Time::SetResolution(Time::NS);

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    // left wifi
    NodeContainer leftWifiStationNodes;
    leftWifiStationNodes.Create(nLeftNodeCount);
    NodeContainer leftWifiApNode;
    leftWifiApNode = p2pNodes.Get(0);

    // right wifi
    NodeContainer rightWifiStationNodes;
    rightWifiStationNodes.Create(nRightNodeCount);
    NodeContainer rightWifiApNode;
    rightWifiApNode = p2pNodes.Get(1);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
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

    // for left wifi
    double midYPosition;
    if (nLeftNodeCount % 2 == 0)
        midYPosition = (nLeftNodeCount - 1) / 2.0;
    else
        midYPosition = nLeftNodeCount / 2.0;
    double diffY = sqrt(RANGE) / midYPosition;
    p2pNodes.Get(0)->GetObject<ConstantPositionMobilityModel>()->SetPosition(
        Vector(2.0, diffY * midYPosition, 0.0));
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(1.0),
                                  "MinY",
                                  DoubleValue(0),
                                  "DeltaX",
                                  DoubleValue(1.0),
                                  "DeltaY",
                                  DoubleValue(diffY),
                                  "GridWidth",
                                  UintegerValue(1),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.SetMobilityModel(
        "ns3::RandomWalk2dMobilityModel",
        "Speed",
        StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(speed) + "]"),
        "Bounds",
        RectangleValue(Rectangle(-100, 100, -100, 100)));
    mobility.Install(leftWifiStationNodes);

    // for right wifi
    if (nRightNodeCount % 2 == 0)
        midYPosition = (nRightNodeCount - 1) / 2.0;
    else
        midYPosition = nRightNodeCount / 2.0;
    diffY = sqrt(RANGE - 1) / midYPosition;
    p2pNodes.Get(1)->GetObject<ConstantPositionMobilityModel>()->SetPosition(
        Vector(4.0, diffY * midYPosition, 0.0));
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(5.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(1.0),
                                  "DeltaY",
                                  DoubleValue(diffY),
                                  "GridWidth",
                                  UintegerValue(1),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.SetMobilityModel(
        "ns3::RandomWalk2dMobilityModel",
        "Speed",
        StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(speed) + "]"),
        "Bounds",
        RectangleValue(Rectangle(-100, 100, -100, 100)));
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
            (InetSocketAddress(rightWifiInterfaces.GetAddress(i % nRightNodeCount), port)));
        senderHelper.SetAttribute("PacketSize", UintegerValue(PACKET_SIZE));
        senderHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        senderHelper.SetAttribute("OffTime",
                                  StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        senderHelper.SetAttribute("DataRate", DataRateValue(DataRate(nPackets * PACKET_SIZE * 8)));
        senderApps.Add(senderHelper.Install(leftWifiStationNodes.Get(i % nLeftNodeCount)));

        // Tracing
        Ptr<OnOffApplication> onOff =
            StaticCast<OnOffApplication>(senderApps.Get(i % nLeftNodeCount));
        onOff->TraceConnectWithoutContext("Tx", MakeCallback(&CalculateSent));
    }

    // client
    for (uint64_t i = 0; i < nRightNodeCount; ++i)
    {
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                    (InetSocketAddress(Ipv4Address::GetAny(), port)));
        sinkApps.Add(sinkHelper.Install(rightWifiStationNodes.Get(i)));

        // Tracing
        Ptr<PacketSink> sink = StaticCast<PacketSink>(sinkApps.Get(i));
        sink->TraceConnectWithoutContext("Rx", MakeCallback(&CalculateReceived));
    }

    std::string animFile = "1905001_2.xml";
    AnimationInterface anim(animFile);
    anim.SetMaxPktsPerTraceFile(5000000);

    senderApps.Start(Seconds(1.0));
    sinkApps.Start(Seconds(0.0));
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();

    double throughput = receivedBits / 1000.0 / 1000.0 / simulationTime;
    double deliveryRatio = packetReceived / packetSent;

    std::string out = "what2.dat";

    std::ofstream outfile;
    outfile.open(out);
    outfile << throughput << std::endl;
    outfile << deliveryRatio << std::endl;
    outfile.close();

    NS_LOG_UNCOND(throughput << " " << deliveryRatio);
    return 0;
}
