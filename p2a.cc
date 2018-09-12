#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/application-container.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RedTests");

uint32_t checkTimes;
double avgQueueSize;

double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;


NodeContainer n1n5;
NodeContainer n2n5;
NodeContainer n3n5;
NodeContainer n4n5;
NodeContainer n5n6;

Ipv4InterfaceContainer i1i5;
Ipv4InterfaceContainer i2i5;
Ipv4InterfaceContainer i3i5;
Ipv4InterfaceContainer i4i5;
Ipv4InterfaceContainer i5i6;

std::stringstream filePlotQueue;
std::stringstream filePlotQueueAvg;
std::stringstream filePlotEnqueue;
std::stringstream filePlotDequeue;
std::stringstream filePlotDropped;


void CheckQueueSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = StaticCast<RedQueueDisc> (queue)->GetQueueSize ();

  avgQueueSize += qSize;
  checkTimes++;

  Simulator::Schedule (Seconds (0.01), &CheckQueueSize, queue);

  std::ofstream fPlotQueue (filePlotQueue.str ().c_str (), std::ios::out|std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();

  std::ofstream fPlotQueueAvg (filePlotQueueAvg.str ().c_str (), std::ios::out|std::ios::app);
  fPlotQueueAvg << Simulator::Now ().GetSeconds () << " " << avgQueueSize / checkTimes << std::endl;
  fPlotQueueAvg.close ();
}

void EnqueueAtRed(Ptr<const QueueItem> item) {
    TcpHeader tcp;
    Ptr<Packet> pkt = item->GetPacket();
    pkt->PeekHeader(tcp);

    std::ofstream outfile;
    outfile.open("end.csv",std::ios::app);
    outfile<<Simulator::Now().GetSeconds()<<","<<tcp.GetDestinationPort()<<","<<tcp.GetSequenceNumber()<<std::endl;
}

void DequeueAtRed(Ptr<const QueueItem> item) {
    TcpHeader tcp;
    Ptr<Packet> pkt = item->GetPacket();
    pkt->PeekHeader(tcp);

    std::ofstream outfile;
    outfile.open("deq.csv",std::ios::app);
    outfile<<Simulator::Now().GetSeconds()<<","<<tcp.GetDestinationPort()<<","<<tcp.GetSequenceNumber()<<std::endl;   

}

void DroppedAtRed(Ptr<const QueueItem> item) {
    TcpHeader tcp;
    Ptr<Packet> pkt = item->GetPacket();
    pkt->PeekHeader(tcp);

    std::ofstream outfile;
    outfile.open("drop.csv",std::ios::app);
    outfile<<Simulator::Now().GetSeconds()<<","<<tcp.GetDestinationPort()<<","<<tcp.GetSequenceNumber()<<std::endl;
  
}



void
BuildAppsTest ()
{
      uint16_t port1 = 50001;
     Address sinkLocalAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress1);
  ApplicationContainer sinkApp = sinkHelper.Install (n5n6.Get (1));
      sinkApp.Start (Seconds (sink_start_time));
      sinkApp.Stop (Seconds (sink_stop_time));

      uint16_t port2 = 50002;
      Address sinkLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port2));
      PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
      sinkApp.Add(sinkHelper2.Install (n5n6.Get (1)));
  
      uint16_t port3 = 50003;
      Address sinkLocalAddress3 (InetSocketAddress (Ipv4Address::GetAny (), port3));
      PacketSinkHelper sinkHelper3 ("ns3::TcpSocketFactory", sinkLocalAddress3);
      sinkApp.Add(sinkHelper3.Install (n5n6.Get (1)));
   
     uint16_t port4 = 50004;
      Address sinkLocalAddress4 (InetSocketAddress (Ipv4Address::GetAny (), port4));
      PacketSinkHelper sinkHelper4 ("ns3::TcpSocketFactory", sinkLocalAddress4);
      sinkApp.Add (sinkHelper4.Install (n5n6.Get (1)));

      
      OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
      clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      clientHelper1.SetAttribute
        ("DataRate", DataRateValue (DataRate ("100Mb/s")));
      clientHelper1.SetAttribute
        ("PacketSize", UintegerValue (1000));

      ApplicationContainer clientApps1;
      AddressValue remoteAddress1
        (InetSocketAddress (i5i6.GetAddress (1), port1));
      clientHelper1.SetAttribute ("Remote", remoteAddress1);
      clientApps1.Add (clientHelper1.Install (n1n5.Get (0)));
      clientApps1.Start (Seconds(0.0));
      clientApps1.Stop (Seconds (client_stop_time));

     
      OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
      clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      clientHelper2.SetAttribute
        ("DataRate", DataRateValue (DataRate ("100Mb/s")));
      clientHelper2.SetAttribute
        ("PacketSize", UintegerValue (1000));

      ApplicationContainer clientApps2;
      AddressValue remoteAddress2
        (InetSocketAddress (i5i6.GetAddress (1), port2));
      clientHelper2.SetAttribute ("Remote", remoteAddress2);
      clientApps2.Add (clientHelper2.Install (n2n5.Get (0)));
      clientApps2.Start (Seconds (0.2));
      clientApps2.Stop (Seconds (client_stop_time));

    
      OnOffHelper clientHelper3 ("ns3::TcpSocketFactory", Address ());
      clientHelper3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      clientHelper3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      clientHelper3.SetAttribute
        ("DataRate", DataRateValue (DataRate ("100Mb/s")));
      clientHelper3.SetAttribute
        ("PacketSize", UintegerValue (1000));

      ApplicationContainer clientApps3;
      AddressValue remoteAddress3
        (InetSocketAddress (i5i6.GetAddress (1), port3));
      clientHelper3.SetAttribute ("Remote", remoteAddress3);
      clientApps3.Add (clientHelper3.Install (n3n5.Get (0)));
      clientApps3.Start (Seconds (0.4));
      clientApps3.Stop (Seconds (client_stop_time));

    
      OnOffHelper clientHelper4 ("ns3::TcpSocketFactory", Address ());
      clientHelper4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      clientHelper4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      clientHelper4.SetAttribute
        ("DataRate", DataRateValue (DataRate ("100Mbps")));
      clientHelper4.SetAttribute
        ("PacketSize", UintegerValue (1000)); 

      ApplicationContainer clientApps4;
      AddressValue remoteAddress4
        (InetSocketAddress (i5i6.GetAddress (1), port4));
      clientHelper4.SetAttribute ("Remote", remoteAddress4);
      clientApps4.Add (clientHelper4.Install (n4n5.Get (0)));
      clientApps4.Start (Seconds (0.6));
      clientApps4.Stop (Seconds (client_stop_time));


}


int main (int argc, char *argv[])
{
  SeedManager::SetSeed(1);
  
  std::string redLinkDataRate = "45Mbps";
  std::string redLinkDelay = "2ms";
  std::string pathOut;
  bool writeForPlot = false;
  bool writePcap = false;
  bool flowMonitor = false;

  bool printRedStats = true;
  global_start_time = 0.0;
  global_stop_time = 1.2;
  sink_start_time = global_start_time;
  sink_stop_time = global_stop_time;
  client_start_time = sink_start_time;
  client_stop_time = global_stop_time;
  
  pathOut = "."; 
  CommandLine cmd;
  
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
  cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);

  cmd.Parse (argc, argv);

  NodeContainer c;
  c.Create (6);
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  Names::Add ( "N4", c.Get (4));
  Names::Add ( "N5", c.Get (5));
  n1n5 = NodeContainer (c.Get (0), c.Get (4));
  n2n5 = NodeContainer (c.Get (1), c.Get (4));
  n3n5 = NodeContainer (c.Get (2), c.Get (4));
  n4n5 = NodeContainer (c.Get (3), c.Get (4));
  n5n6 = NodeContainer (c.Get (4), c.Get (5));

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000 - 42));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));

  uint32_t meanPktSize = 500;

 
  NS_LOG_INFO ("Set RED params");
  Config::SetDefault ("ns3::RedQueueDisc::Mode", StringValue ("QUEUE_MODE_PACKETS"));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (meanPktSize));
  Config::SetDefault ("ns3::RedQueueDisc::Wait", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (0.002));
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (5));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (15));
  Config::SetDefault ("ns3::RedQueueDisc::QueueLimit", UintegerValue (1000));

  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));

  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc ("ns3::RedQueueDisc", "LinkBandwidth", StringValue (redLinkDataRate),
                           "LinkDelay", StringValue (redLinkDelay));

  NS_LOG_INFO ("Create channels");
  PointToPointHelper p2p;

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer devn1n5 = p2p.Install (n1n5);
  tchPfifo.Install (devn1n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("4ms"));
  NetDeviceContainer devn2n5 = p2p.Install (n2n5);
  tchPfifo.Install (devn2n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("8ms"));
  NetDeviceContainer devn3n5 = p2p.Install (n3n5);
  tchPfifo.Install (devn3n5);


  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  NetDeviceContainer devn4n5 = p2p.Install (n4n5);
  tchPfifo.Install (devn4n5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (redLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (redLinkDelay));
  NetDeviceContainer devn5n6 = p2p.Install (n5n6);
  QueueDiscContainer queueDiscs = tchRed.Install (devn5n6);
  Ptr<QueueDisc> redQueue = queueDiscs.Get (0);

  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i1i5 = ipv4.Assign (devn1n5);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i2i5 = ipv4.Assign (devn2n5);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i3i5 = ipv4.Assign (devn3n5);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i4i5 = ipv4.Assign (devn4n5);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  i5i6 = ipv4.Assign (devn5n6);

  redQueue->TraceConnectWithoutContext("Enqueue", MakeCallback(&EnqueueAtRed));
  redQueue->TraceConnectWithoutContext("Dequeue", MakeCallback(&DequeueAtRed));
  redQueue->TraceConnectWithoutContext("Drop", MakeCallback(&DroppedAtRed));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  BuildAppsTest ();

  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/red";
      ptp.EnablePcapAll (stmp.str ().c_str ());
    }

  Ptr<FlowMonitor> flowmon;
  if (flowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  if (writeForPlot)
    {
      filePlotQueue << pathOut << "/" << "red-queue.plotme";
      filePlotQueueAvg << pathOut << "/" << "red-queue_avg.plotme";

      filePlotEnqueue << pathOut << "/" << "EnqueueAtRed.plotme";
      filePlotDequeue << pathOut << "/" << "DequeueAtRed.plotme";
     filePlotDropped << pathOut << "/" << "DroppedAtRed.plotme";

      remove (filePlotQueue.str ().c_str ());
      remove (filePlotQueueAvg.str ().c_str ());
      remove (filePlotEnqueue.str ().c_str ());
      remove (filePlotDequeue.str ().c_str ());
      remove (filePlotDropped.str ().c_str ());
      Ptr<QueueDisc> queue = queueDiscs.Get (0);
      Simulator::ScheduleNow (&CheckQueueSize, queue);
    }

  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();

  if (flowMonitor)
    {
      std::stringstream stmp;
      stmp << pathOut << "/red.flowmon";

      flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
    }

 if (printRedStats)
    {
      RedQueueDisc::Stats st = StaticCast<RedQueueDisc> (queueDiscs.Get (0))->GetStats ();
      std::cout << "*** RED stats from Node 2 queue ***" << std::endl;
      std::cout << "\t " << st.unforcedDrop << " drops due prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due hard mark" << std::endl;
      std::cout << "\t " << st.qLimDrop << " drops due queue full" << std::endl;

      st = StaticCast<RedQueueDisc> (queueDiscs.Get (1))->GetStats ();
      std::cout << "*** RED stats from Node 3 queue ***" << std::endl;
      std::cout << "\t " << st.unforcedDrop << " drops due prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due hard mark" << std::endl;
      std::cout << "\t " << st.qLimDrop << " drops due queue full" << std::endl;
    }

  Simulator::Destroy ();

  return 0;
}
