/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-adaptive-reno.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpAdaptiveReno")
            .SetParent<TcpNewReno>()
            .SetGroupName("Internet")
            .AddConstructor<TcpAdaptiveReno>()
            .AddAttribute(
                "FilterType",
                "Use this to choose no filter or Tustin's approximation filter",
                EnumValue(TcpAdaptiveReno::TUSTIN),
                MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
            .AddTraceSource("EstimatedBW",
                            "The estimated bandwidth",
                            MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                            "ns3::TracedValueCallback::DataRate");
    return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno()
    : TcpWestwoodPlus(),
      m_minRtt(Seconds(0.0)),
      m_currentRtt(Seconds(0.0)),
      m_congRtt(Seconds(0.0)),
      m_prevCongRtt(Seconds(0.0)),
      m_jthLossRtt(Seconds(0.0)),
      m_baseWnd(0.0),
      m_probeWnd(0.0),
      m_incWnd(0.0)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& sock)
    : TcpWestwoodPlus(sock),
      m_minRtt(sock.m_minRtt),
      m_currentRtt(sock.m_currentRtt),
      m_congRtt(sock.m_congRtt),
      m_prevCongRtt(sock.m_prevCongRtt),
      m_jthLossRtt(sock.m_jthLossRtt),
      m_baseWnd(sock.m_baseWnd),
      m_probeWnd(sock.m_probeWnd),
      m_incWnd(sock.m_incWnd)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::~TcpAdaptiveReno()
{
}

// This function is called each time a packet is Acked. It will increase the
// count of acked segments and update the current estimated bandwidth.
void
TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

    if (m_minRtt.IsZero() || rtt < m_minRtt)
        m_minRtt = rtt;
    m_currentRtt = rtt;

    m_ackedSegments += packetsAcked;
    TcpWestwoodPlus::EstimateBW(rtt, tcb);
}

// Estimates the current congestion level using Round
// Trip Time (RTT).
double
TcpAdaptiveReno::EstimateCongestionLevel()
{
    NS_LOG_FUNCTION(this);
    double a = 0.85; // Exponential Soothing Factor

    double congRtt = a * m_prevCongRtt.GetSeconds() + (1 - a) * m_congRtt.GetSeconds();
    m_congRtt = Seconds(congRtt);

    // Congestion Level
    double c = std::min((m_currentRtt.GetSeconds() - m_minRtt.GetSeconds()) /
                            (m_congRtt.GetSeconds() - m_minRtt.GetSeconds()),
                        1.0);
    return c;
}

// Estimates the increase in window size.
// Calculate maximum windows increase and update the value of m_incWnd.
void
TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this << tcb);

    double c = EstimateCongestionLevel(); // Congestion Level
    int m_scalingFactor = 1000; // Paper page 2 says, M is a scaling factor we set at 10Mbps based
                                // on our Internet measurements so far.
    // Current Bandwidth has been calculated in TcpWestWoodPlus’s
    // EstimateBW() function which is called in PktsAcked()
    double maxSegmentSize = tcb->m_segmentSize * tcb->m_segmentSize;

    double m_maxIncWnd = m_currentBW.Get().GetBitRate() / m_scalingFactor * maxSegmentSize;

    double alpha = 10;
    double beta = 2 * m_maxIncWnd * (1 / alpha - (1 / alpha + 1) / std::exp(alpha));
    double gamma = 1 - 2 * m_maxIncWnd * (1 / alpha - (1 / alpha + 1 / 2.0) / std::exp(alpha));

    m_incWnd = m_maxIncWnd / std::exp(c * alpha) + c * beta + gamma;
}

// This function increases congestion window in congestion avoidance phase.
// In TCP-AReno, congestion window W is managed in two parts;
// a base part, W_base, and a fast probing part, W_probe.
// The base part maintains a congestion window size equivalent to TCP-Reno
// and the probing part is introduced to quickly fill the bottleneck link.
// The base part is always increased like TCP-Reno, i.e. 1MSS/RTT, while
// the increase of the probing part, W_inc, is dynamically adjusted.
void
TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);

    if (segmentsAcked <= 0)
        return;

    EstimateIncWnd(tcb);

    // Base Window
    double maxSegmentSize = tcb->m_segmentSize * tcb->m_segmentSize;
    m_baseWnd += std::max(maxSegmentSize / tcb->m_cWnd.Get(), 1.0);

    // Probe Window
    m_probeWnd = std::max((double)m_probeWnd + (double)m_incWnd / tcb->m_cWnd.Get(), 0.00);

    tcb->m_cWnd = m_baseWnd + m_probeWnd;
}

// This function is called when a packet is lost due to congestion.
// It will update the value of m_congRtt and m_prevCongRtt.
uint32_t
TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
    m_prevCongRtt = m_congRtt; // since a loss event has occured, set the previous congestion RTT
    m_jthLossRtt = m_currentRtt;

    double c = EstimateCongestionLevel(); // Congestion Level

    m_baseWnd = std::max(2.0 * tcb->m_segmentSize, (double)tcb->m_cWnd.Get() / (1.0 + c));
    m_probeWnd = 0.0;

    return m_baseWnd;
}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork()
{
    return CreateObject<TcpAdaptiveReno>(*this);
}

} // namespace ns3