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
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//  s1                          r1
//      n0 -------------- n1
//  s2    point-to-point        r2
//

using namespace ns3;

#define PACKET_SIZE 1024

NS_LOG_COMPONENT_DEFINE("Task1");

class TutorialApp : public Application
{
  public:
    TutorialApp();
    ~TutorialApp() override;

    /**
     * Register this type.
     * \return The TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Setup the socket.
     * \param socket The socket.
     * \param address The destination address.
     * \param packetSize The packet size to transmit.
     * \param nPackets The number of packets to transmit.
     * \param dataRate the data rate to use.
     */
    void Setup(Ptr<Socket> socket,
               Address address,
               uint32_t packetSize,
               uint32_t simultime,
               DataRate dataRate);

  private:
    void StartApplication() override;
    void StopApplication() override;

    /// Schedule a new transmission.
    void ScheduleTx();
    /// Send a packet.
    void SendPacket();

    Ptr<Socket> m_socket;   //!< The transmission socket.
    Address m_peer;         //!< The destination address.
    uint32_t m_packetSize;  //!< The packet size.
    uint32_t m_simultime;   //!< The simulation time.
    DataRate m_dataRate;    //!< The data rate to use.
    EventId m_sendEvent;    //!< Send event.
    bool m_running;         //!< True if the application is running.
    uint32_t m_packetsSent; //!< The number of packets sent.
};

TutorialApp::TutorialApp()
    : m_socket(nullptr),
      m_peer(),
      m_packetSize(0),
      m_simultime(0),
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0)
{
}

TutorialApp::~TutorialApp()
{
    m_socket = nullptr;
}

/* static */
TypeId
TutorialApp::GetTypeId()
{
    static TypeId tid = TypeId("TutorialApp")
                            .SetParent<Application>()
                            .SetGroupName("Tutorial")
                            .AddConstructor<TutorialApp>();
    return tid;
}

void
TutorialApp::Setup(Ptr<Socket> socket,
                   Address address,
                   uint32_t packetSize,
                   uint32_t simultime,
                   DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_simultime = simultime;
    m_dataRate = dataRate;
}

void
TutorialApp::StartApplication()
{
    m_running = true;
    m_packetsSent = 0;
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        m_socket->Bind();
    }
    else
    {
        m_socket->Bind6();
    }
    m_socket->Connect(m_peer);
    SendPacket();
}

void
TutorialApp::StopApplication()
{
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void
TutorialApp::SendPacket()
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (Simulator::Now().GetSeconds() < m_simultime)
        ScheduleTx();
}

void
TutorialApp::ScheduleTx()
{
    if (m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &TutorialApp::SendPacket, this);
    }
}

static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
}

int
main(int argc, char* argv[])
{
    uint64_t payloadSize = 1472;
    std::string algorithm1 = "ns3::TcpNewReno";
    std::string algorithm2 = "ns3::TcpWestwoodPlus";
    const uint64_t nLeaf = 2;
    const std::string senderDataRate = "1Gbps";
    const std::string senderDelay = "1ms";
    double simulationTime = 10;         // seconds
    uint64_t bottleneckDataRate = 50;   // Mbps
    const uint64_t bottleneckDelay = 1; // ms
    const std::string bottleneckDelayString = std::to_string(bottleneckDelay) + "ms";
    uint64_t packetLossExponent = 6;

    CommandLine cmd(__FILE__);
    cmd.AddValue("bottleneckBW", "Bottleneck bandwidth in Mbps", bottleneckDataRate);
    cmd.AddValue("packetLossExponent", "Packet Loss Rate Exponent", packetLossExponent);
    cmd.Parse(argc, argv);

    const int nFlows = nLeaf;

    double packetLossRate = (1.0 / std::pow(10, packetLossExponent));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    PointToPointHelper bottleNeckLink;
    std::string dataRateString = std::to_string(bottleneckDataRate) + "Mbps";
    bottleNeckLink.SetDeviceAttribute("DataRate", StringValue(dataRateString));
    bottleNeckLink.SetChannelAttribute("Delay", StringValue(bottleneckDelayString));

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(senderDataRate));
    p2p.SetChannelAttribute("Delay", StringValue(senderDelay));
    p2p.SetQueue("ns3::DropTailQueue",
                 "MaxSize",
                 StringValue(std::to_string(bottleneckDataRate * bottleneckDelay) + "p"));

    PointToPointDumbbellHelper dumbbell(nLeaf, p2p, nLeaf, p2p, bottleNeckLink);

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(packetLossRate));
    dumbbell.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(algorithm1));
    InternetStackHelper stack1;

    for (uint32_t i = 0; i < dumbbell.LeftCount(); i += 2)
    {
        stack1.Install(dumbbell.GetLeft(i));  // ith left side leaf node
        stack1.Install(dumbbell.GetRight(i)); // ith right side leaf node
    }
    stack1.Install(dumbbell.GetLeft());  // left side bottleneck router
    stack1.Install(dumbbell.GetRight()); // right side bottleneck router

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(algorithm2));
    InternetStackHelper stack2;
    for (uint32_t i = 1; i < dumbbell.LeftCount(); i += 2)
    {
        stack2.Install(dumbbell.GetLeft(i));  // ith left side leaf node
        stack2.Install(dumbbell.GetRight(i)); // ith right side leaf node
    }

    // IP Address Assignment
    Ipv4AddressHelper left("10.1.1.0", "255.255.255.0");
    Ipv4AddressHelper right("10.2.1.0", "255.255.255.0");
    Ipv4AddressHelper center("10.3.1.0", "255.255.255.0");
    dumbbell.AssignIpv4Addresses(left, right, center);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    FlowMonitorHelper flowMonitor;
    flowMonitor.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(2.0)));
    Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();

    int port = 8080;
    ApplicationContainer sinkApps;
    for (int i = 0; i < nFlows; i++)
    {
        PacketSinkHelper sink("ns3::TcpSocketFactory",
                              InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer sinkApps = sink.Install(dumbbell.GetRight(i));
        sinkApps.Start(Seconds(0.0));
        sinkApps.Stop(Seconds(simulationTime + 2.0));

        Ptr<Socket> ns3TcpSocket =
            Socket::CreateSocket(dumbbell.GetLeft(i), TcpSocketFactory::GetTypeId());
        Ptr<TutorialApp> app = CreateObject<TutorialApp>();
        app->Setup(ns3TcpSocket,
                   InetSocketAddress(dumbbell.GetRightIpv4Address(i), port),
                   payloadSize,
                   simulationTime,
                   DataRate(senderDataRate));
        dumbbell.GetLeft(i)->AddApplication(app);
        app->SetStartTime(Seconds(1.0));
        app->SetStopTime(Seconds(simulationTime));
        ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
    }

    Simulator::Stop(Seconds(simulationTime + 2));
    Simulator::Run();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowMonitor.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();
    double throughput = 0;
    for (auto it = stats.begin(); it != stats.end(); it++)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(it->first);
        throughput =
            it->second.rxBytes * 8.0 /
            (it->second.timeLastRxPacket.GetSeconds() - it->second.timeFirstTxPacket.GetSeconds()) /
            1000000.0;
    }

    double startTime = stats.begin()->second.timeFirstTxPacket.GetSeconds();
    double endTime = stats.begin()->second.timeLastRxPacket.GetSeconds();
    double totalTime = endTime - startTime;
    std::cout << throughput << std::endl;

    Simulator::Destroy();

    return 0;
}
