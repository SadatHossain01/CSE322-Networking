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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 * and Greeshma Umapathi
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

#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-congestion-ops.h"
#include "tcp-westwood-plus.h"

#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/traced-value.h"

namespace ns3
{

class Time;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP Adaptive Reno
 *
 *  TCP-AReno adaptively tunes TCP-Reno’s response function in
 *  such a way that it improves efficiency in high-speed networks
 *  as well as friendliness to TCP-Reno. TCP-AReno is based on
 *  TCP-Westwood-BBE (Buffer and Bandwidth Estimation) , which is
 *  proposed to enhance friendliness of TCP-Westwood even in networks
 *  with varying buffer capacities, flows with varying RTTs. It estimates
 *  congestion level via RTT to determine whether a packet loss is due to
 *  congestion or not. Basically, any multi-bit congestion indicators can
 *  be used for this purpose if they detect congestion only when coexisting
 *  TCP-Reno flows detect congestion, i.e. only when a packet loss is likely
 *  to happen. RTT has been employed as it is proven to be a good congestion
 *  estimator and does not require any network supports. If a loss is not
 *  recognized as a congestion event, the congestion window is reduced according
 *  to TCPW’s eligible bandwidth estimation. Otherwise, the loss is associated
 *  to congestion and it adjusts the window reduction so that the reduction
 *  matches TCP-Reno’s behavior, i.e., halving the congestion window. Our
 *  Internet measurements have shown  that this loss discrimination helps
 *  to improve throughputs without losing friendliness to TCP-Reno. TCP-AReno
 *  introduces a fast window expansion mechanism to quickly increase congestion
 *  window whenever it finds network underutilization. It dynamically adjusts
 *  the TCP response function based on the congestion measurement introduced by
 *  TCPW-BBE, whereas HS-TCP adjusts the function based on congestion window size.
 *  TCP-AReno increases the congestion window much faster than TCP-Reno when it
 *  detects no congestion, i.e. when RTT is close to its minimum value, and thus
 *  achieves higher efficiency than TCP-Reno. When a TCP-AReno flow competes with
 *  TCP-Reno flows, it detects increased RTT and thus increased congestion level.
 *  In this case, it recognizes that packet losses are likely to happen and adopts
 *  a response function that matches TCP-Reno’s behavior, i.e. increasing the congestion
 * window by 1MSS/RTT.
 *
 * The TcpAdaptiveReno class has inherited from the TcpWestwoodPlus class. It overrides
 * the existing PktsAcked, GetSsThresh & CongestionAvoidance functions and implements
 * two new ones. Necessary variables are also added to the class.
 *
 * WARNING: this TCP model lacks validation and regression tests; use with caution.
 */

class TcpAdaptiveReno : public TcpWestwoodPlus
{
  public:
    /**
    \brief Get the type ID.
    \return the object TypeId
    */
    static TypeId GetTypeId();

    TcpAdaptiveReno();

    /**
     * \brief Copy constructor
     * \param sock the object to copy
     */
    TcpAdaptiveReno(const TcpAdaptiveReno& sock);
    ~TcpAdaptiveReno() override;

    /**
     * \brief Filter type (None or Tustin)
     */
    enum FilterType
    {
        NONE,
        TUSTIN
    };

    uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) override;

    void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt) override;

    Ptr<TcpCongestionOps> Fork() override;

  private:
    void EstimateBW(const Time& rtt, Ptr<TcpSocketState> tcb);

    double EstimateCongestionLevel();

    void EstimateIncWnd(Ptr<TcpSocketState> tcb);

  protected:
    void CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

    Time m_congRtt;     //!< RTT's estimated value
                        // when jth packet loss occurs
    Time m_prevCongRtt; //!< Previous Congestion RTT (j-1th event)
    Time m_jthLossRtt;  // jth Packet Loss RTT
    Time m_minRtt;      //!< Minimum RTT
    Time m_currentRtt;  //!< Current RTT

    // Congestion Window Stuffs
    int32_t m_incWnd;   //!< Increase in window size
    uint32_t m_baseWnd; //!< Base Part of Window
    int32_t m_probeWnd; //!< Probe Part of Window
};

} // namespace ns3

#endif /* TCP_ADAPTIVERENO_H */