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

#include <iomanip>
#include <iostream>

// Default Network Topology
//
//  s1---                        ---r1
//         n0 -------------- n1
//  s2---    point-to-point      ---r2
//

using namespace ns3;

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
     * \param dataRate the data rate to use.
     */
    void Setup(Ptr<Socket> socket,
               Address address,
               uint32_t packetSize,
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
    DataRate m_dataRate;    //!< The data rate to use.
    EventId m_sendEvent;    //!< Send event.
    bool m_running;         //!< True if the application is running.
    uint32_t m_packetsSent; //!< The number of packets sent.
};

TutorialApp::TutorialApp()
    : m_socket(nullptr),
      m_peer(),
      m_packetSize(0),
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
                   DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_dataRate = dataRate;
}

void
TutorialApp::StartApplication()
{
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind();
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

    m_packetsSent++;
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

std::ofstream out1, out2, out3;

static void
CwndChangeAlgo1(uint32_t oldCwnd, uint32_t newCwnd)
{
    // NS_LOG_UNCOND(Simulator::Now().GetSeconds() << " " << newCwnd / 1024.0 << std::endl);
    out1 << std::fixed << std::setprecision(6) << Simulator::Now().GetSeconds() << " "
         << newCwnd / 1024.0 << std::endl;
}

static void
CwndChangeAlgo2(uint32_t oldCwnd, uint32_t newCwnd)
{
    // NS_LOG_UNCOND(Simulator::Now().GetSeconds() << " " << newCwnd / 1024.0 << std::endl);
    out2 << std::fixed << std::setprecision(6) << Simulator::Now().GetSeconds() << " "
         << newCwnd / 1024.0 << std::endl;
}

int
main(int argc, char* argv[])
{
    out1.open("what1.dat", std::ios::app); // Algorithm 1 Throughput or Congestion Window
    out2.open("what2.dat", std::ios::app); // Algorithm 2 Throughput
    out3.open("what3.dat", std::ios::app); // Jain's Fairness Index

    std::string algorithm1 = "TcpNewReno"; // it will always be this
    std::string algorithm2 = "TcpAdaptiveReno";
    uint64_t payloadSize = 1024;
    const std::string senderDataRate = "1Gbps";
    const std::string senderDelay = "1ms";
    double simulationTime = 10;         // seconds
    uint64_t bottleneckDataRate = 50;   // Mbps
    const uint64_t bottleneckDelay = 100; // ms
    int packetLossExponent = -6;
    int basePort = 8080;
    bool congestionDataEnabled = false;

    std::string experimentName; // "loss" for packet loss rate, "data" for data rate

    enum ExperimentType
    {
        LOSS_EXPONENT, // for both throughput and Jain's Fairness Index
        DATA_RATE,     // for both throughput and Jain's Fairness Index
        CONGESTION_TIME,
    };

    ExperimentType experimentType;

    // Command Line Parameters
    CommandLine cmd(__FILE__);
    cmd.AddValue("bottleneckBW", "Bottleneck bandwidth in Mbps", bottleneckDataRate);
    cmd.AddValue("packetLossExponent", "Packet Loss Exponent", packetLossExponent);
    cmd.AddValue("algorithm1", "Congestion Control Algorithm 1", algorithm1);
    cmd.AddValue("algorithm2", "Congestion Control Algorithm 2", algorithm2);
    cmd.AddValue("experimentName", "Experiment Name (loss/data/congestion)", experimentName);
    cmd.Parse(argc, argv);

    double packetLossRate = pow(10.00, packetLossExponent); // given exponent is negative
    if (experimentName == "loss")
        experimentType = LOSS_EXPONENT;
    else if (experimentName == "data")
        experimentType = DATA_RATE;
    else if (experimentName == "congestion")
    {
        experimentType = CONGESTION_TIME;
        congestionDataEnabled = true;
    }
    else
    {
        std::cout << "Invalid experiment name. Exiting..." << std::endl;
        return 0;
    }

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    // Bottleneck Link
    PointToPointHelper bottleNeckLink;
    std::string str = std::to_string(bottleneckDataRate) + "Mbps";
    bottleNeckLink.SetDeviceAttribute("DataRate", StringValue(str));
    str = std::to_string(bottleneckDelay) + "ms";
    bottleNeckLink.SetChannelAttribute("Delay", StringValue(str));

    // PointToPointHelper for both sides of leaves
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(senderDataRate));
    p2p.SetChannelAttribute("Delay", StringValue(senderDelay));
    p2p.SetQueue("ns3::DropTailQueue",
                 "MaxSize",
                 StringValue(std::to_string(bottleneckDataRate * bottleneckDelay) + "p"));

    // The Dumbbell Topology
    PointToPointDumbbellHelper dumbbell(2, p2p, 2, p2p, bottleNeckLink);
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(packetLossRate));
    dumbbell.m_routerDevices.Get(1)->SetAttribute(
        "ReceiveErrorModel",
        PointerValue(em)); // right router device is the receiver

    // Congestion Control Algorithm 1 : the upper flow (from top left to top right)
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + algorithm1));
    InternetStackHelper stack1;
    stack1.Install(dumbbell.GetLeft(0));  // left side leaf node
    stack1.Install(dumbbell.GetRight(0)); // right side leaf node

    // Setting Algorithm 1 (TcpNewReno) to the bottleneck routers
    stack1.Install(dumbbell.GetLeft());  // left side bottleneck router
    stack1.Install(dumbbell.GetRight()); // right side bottleneck router

    // Congestion Control Algorithm 2 : the bottom flow (from bottom left to bottom right)
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + algorithm2));
    InternetStackHelper stack2;
    stack2.Install(dumbbell.GetLeft(1));  // left side leaf node
    stack2.Install(dumbbell.GetRight(1)); // right side leaf node

    // IP Address Assignment
    Ipv4AddressHelper left, right, center;
    left.SetBase("10.1.1.0", "255.255.255.0");   // left network
    right.SetBase("10.2.1.0", "255.255.255.0");  // right network
    center.SetBase("10.3.1.0", "255.255.255.0"); // center network
    dumbbell.AssignIpv4Addresses(left, right, center);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Flow Monitor Installation
    FlowMonitorHelper flowMonitor;
    // The maximum per-hop delay that should be considered
    flowMonitor.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(2.0)));
    Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();

    ApplicationContainer sinkApps, senderApps;

    // Receiver Apps
    PacketSinkHelper sink1("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), basePort));
    PacketSinkHelper sink2("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), basePort + 1));
    sinkApps.Add(sink1.Install(dumbbell.GetRight(0)));
    sinkApps.Add(sink2.Install(dumbbell.GetRight(1)));

    // Sender Apps
    Ptr<Socket> ns3TcpSocket1 =
        Socket::CreateSocket(dumbbell.GetLeft(0), TcpSocketFactory::GetTypeId());
    Ptr<TutorialApp> app1 = CreateObject<TutorialApp>();
    app1->Setup(ns3TcpSocket1,
                InetSocketAddress(dumbbell.GetRightIpv4Address(0), basePort),
                payloadSize,
                DataRate(senderDataRate));
    Ptr<Socket> ns3TcpSocket2 =
        Socket::CreateSocket(dumbbell.GetLeft(1), TcpSocketFactory::GetTypeId());
    Ptr<TutorialApp> app2 = CreateObject<TutorialApp>();
    app2->Setup(ns3TcpSocket2,
                InetSocketAddress(dumbbell.GetRightIpv4Address(1), basePort + 1),
                payloadSize,
                DataRate(senderDataRate));
    dumbbell.GetLeft(0)->AddApplication(app1);
    dumbbell.GetLeft(1)->AddApplication(app2);
    senderApps.Add(app1);
    senderApps.Add(app2);

    if (congestionDataEnabled)
    {
        ns3TcpSocket1->TraceConnectWithoutContext("CongestionWindow",
                                                  MakeCallback(&CwndChangeAlgo1));
        ns3TcpSocket2->TraceConnectWithoutContext("CongestionWindow",
                                                  MakeCallback(&CwndChangeAlgo2));
    }

    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(simulationTime + 1.0));
    senderApps.Start(Seconds(1.0));
    senderApps.Stop(Seconds(simulationTime + 1.0));
    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowMonitor.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    std::vector<double> throughputs;
    for (auto it = stats.begin(); it != stats.end(); it++)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(it->first);
        double throughput = (it->second.rxBytes * 8.0) / (simulationTime * 1024);
        NS_LOG_UNCOND("Flow ID: " << it->first << " Source: " << t.sourceAddress
                                  << " Destination: " << t.destinationAddress);
        NS_LOG_UNCOND("Throughput: " << throughput << " kbps");
        throughputs.push_back(throughput);
    }

    if (!congestionDataEnabled)
    {
        double finalThroughputAlgo1 = 0;       // The upper flow
        double finalThroughputAlgo2 = 0;       // The bottom flow
        double numerator = 0, denominator = 0; // Jain's Fairness Index

        for (uint32_t i = 0; i < throughputs.size(); i++)
        {
            if (i & 1)
                finalThroughputAlgo2 += throughputs[i];
            else
                finalThroughputAlgo1 += throughputs[i];

            numerator += throughputs[i];
            denominator += throughputs[i] * throughputs[i];
        }

        double JainIndex = (numerator * numerator) / (throughputs.size() * denominator);

        NS_LOG_UNCOND("Jain's Fairness Index: " << JainIndex);

        if (experimentType == ExperimentType::DATA_RATE)
        {
            out1 << std::fixed << std::setprecision(6) << bottleneckDataRate << " "
                 << finalThroughputAlgo1 << std::endl;
            out2 << std::fixed << std::setprecision(6) << bottleneckDataRate << " "
                 << finalThroughputAlgo2 << std::endl;
            out3 << std::fixed << std::setprecision(6) << bottleneckDataRate << " " << JainIndex
                 << std::endl;
        }
        else if (experimentType == ExperimentType::LOSS_EXPONENT)
        {
            out1 << std::fixed << std::setprecision(6) << packetLossExponent << " "
                 << finalThroughputAlgo1 << std::endl;
            out2 << std::fixed << std::setprecision(6) << packetLossExponent << " "
                 << finalThroughputAlgo2 << std::endl;
            out3 << std::fixed << std::setprecision(6) << packetLossExponent << " " << JainIndex
                 << std::endl;
        }
    }

    Simulator::Destroy();

    out1.close();
    out2.close();
    out3.close();

    return 0;
}