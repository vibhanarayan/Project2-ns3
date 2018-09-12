#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("p2c");

uint32_t checkTimes;
double avgQueueSize;

double start_time;
double stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

NodeContainer n1na;
NodeContainer n2na;
NodeContainer n3na;
NodeContainer n4na;
NodeContainer nanb;
NodeContainer nbn5;
NodeContainer nbn6;
NodeContainer nbn7;
NodeContainer nbn8;

Ipv4InterfaceContainer i1ia;
Ipv4InterfaceContainer i2ia;
Ipv4InterfaceContainer i3ia;
Ipv4InterfaceContainer i4ia;
Ipv4InterfaceContainer iaib;
Ipv4InterfaceContainer ibi5;
Ipv4InterfaceContainer ibi6;
Ipv4InterfaceContainer ibi7;
Ipv4InterfaceContainer ibi8;

std::stringstream plotQueue;
std::stringstream plotQueueAvg;
std::stringstream plotEnqueue;
std::stringstream plotDequeue;
std::stringstream plotDropped;



void CheckQueueSize (Ptr<QueueDisc> queue, int gw)
{
  uint32_t qSize = StaticCast<RedQueueDisc> (queue)->GetQueueSize ();
  
  avgQueueSize += qSize;
  checkTimes++;
  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueSize, queue, gw);

  std::ofstream fPlotQueue;
  std::ofstream fplotQueueAvg;

  std::string ofile = std::to_string(gw) + "rq.csv";
    std::string ofileavg = std::to_string(gw) + "rqa.csv";

    fPlotQueue.open(ofile, std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << "," << qSize << std::endl;
  fPlotQueue.close ();

    fplotQueueAvg.open(ofileavg, std::ios::app);

  fplotQueueAvg << Simulator::Now ().GetSeconds () << "," << avgQueueSize / checkTimes << std::endl;
  fplotQueueAvg.close ();
}

void Loopsetup (uint32_t i, uint32_t j, Ptr<Node> SourceNode, Ptr<Node> SinkNode, Ipv4Address SinkAddr, double start, double duration, std::string drate, uint32_t maxp)
{
    uint32_t port = 50000 + 10*i + j;

    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
    ApplicationContainer sinkApp = sinkHelper.Install (SinkNode);
    sinkApp.Start (Seconds (sink_start_time));
    sinkApp.Stop (Seconds (sink_stop_time));
    
    OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (drate)));
    clientHelper.SetAttribute ("PacketSize", UintegerValue (maxp));

    ApplicationContainer clientApps;
    AddressValue remoteAddress (InetSocketAddress (SinkAddr, port));
    clientHelper.SetAttribute ("Remote", remoteAddress);
    clientApps.Add (clientHelper.Install (SourceNode));
    clientApps.Start (Seconds (start));
    clientApps.Stop (Seconds (start + duration));
}

void EnqueueAtRed(Ptr<const QueueItem> item) {
    TcpHeader tcp;
    Ptr<Packet> pkt = item->GetPacket();
    pkt->PeekHeader(tcp);

    std::ofstream outfile;
    outfile.open("Enqueue.csv",std::ios::app);
    outfile<<Simulator::Now().GetSeconds()<<","<<tcp.GetDestinationPort()<<","<<tcp.GetSequenceNumber()<<std::endl;

}

void DequeueAtRed(Ptr<const QueueItem> item) {
    TcpHeader tcp;
    Ptr<Packet> pkt = item->GetPacket();
    pkt->PeekHeader(tcp);

    std::ofstream outfile;
    outfile.open("Dequeue.csv",std::ios::app);
    outfile<<Simulator::Now().GetSeconds()<<","<<tcp.GetDestinationPort()<<","<<tcp.GetSequenceNumber()<<std::endl;
}

void DroppedAtRed(Ptr<const QueueItem> item) {
    TcpHeader tcp;
    Ptr<Packet> pkt = item->GetPacket();
    pkt->PeekHeader(tcp);

    std::ofstream outfile;
    outfile.open("DroppedAtRed.csv",std::ios::app);
    outfile<<Simulator::Now().GetSeconds()<<","<<tcp.GetDestinationPort()<<","<<tcp.GetSequenceNumber()<<std::endl;

}

int main (int argc, char *argv[])
{
  LogComponentEnable ("RedQueueDisc", LOG_LEVEL_INFO);
  SeedManager::SetSeed(1);
  std::string redLinkDataRate = "45Mbps";
  std::string redLinkDelay = "2ms";

  std::string pathOut;
  bool writePcap = false;
  bool flowMonitor = false;

  bool printRedStats = true;

  start_time = 0.0;
  stop_time = 1.2; 
  sink_start_time = start_time;
  sink_stop_time = stop_time;
  client_start_time = sink_start_time;
  client_stop_time = stop_time;

  
  pathOut = "."; 
  CommandLine cmd;
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes");
  NodeContainer c;
  c.Create (10);
  Names::Add ( "N1", c.Get (0));
  Names::Add ( "N2", c.Get (1));
  Names::Add ( "N3", c.Get (2));
  Names::Add ( "N4", c.Get (3));
  Names::Add ( "N5", c.Get (4));
  Names::Add ( "N6", c.Get (5));
  Names::Add ( "N7", c.Get (6));
  Names::Add ( "N8", c.Get (7));
  Names::Add ( "Na", c.Get (8));
  Names::Add ( "Nb", c.Get (9));
  n1na = NodeContainer (c.Get (0), c.Get (8));
  n2na = NodeContainer (c.Get (1), c.Get (8));
  n3na = NodeContainer (c.Get (2), c.Get (8));
  n4na = NodeContainer (c.Get (3), c.Get (8));
  nanb = NodeContainer (c.Get (8), c.Get (9));
  nbn5 = NodeContainer (c.Get (9), c.Get (4));
  nbn6 = NodeContainer (c.Get (9), c.Get (5));
  nbn7 = NodeContainer (c.Get (9), c.Get (6));
  nbn8 = NodeContainer (c.Get (9), c.Get (7));

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

    /*if (redTest == 3) // test like 1, but with bad params
    {
      Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (10));
      Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (0.003));
    }
  else if (redTest == 5) // test 5, same of test 4, but in byte mode
    {
      Config::SetDefault ("ns3::RedQueueDisc::Mode", StringValue ("QUEUE_MODE_BYTES"));
      Config::SetDefault ("ns3::RedQueueDisc::Ns1Compat", BooleanValue (true));
      Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (5 * meanPktSize));
      Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (15 * meanPktSize));
      Config::SetDefault ("ns3::RedQueueDisc::QueueLimit", UintegerValue (1000 * meanPktSize));
    }
   */ 

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
  p2p.SetChannelAttribute ("Delay", StringValue ("0.5ms"));
  NetDeviceContainer d1da = p2p.Install (n1na);
  tchPfifo.Install (d1da);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer d2da = p2p.Install (n2na);
  tchPfifo.Install (d2da);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer d3da = p2p.Install (n3na);
  tchPfifo.Install (d3da);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  NetDeviceContainer d4da = p2p.Install (n4na);
  tchPfifo.Install (d4da);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (redLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (redLinkDelay));
  NetDeviceContainer dadb = p2p.Install (nanb);
  QueueDiscContainer queueDiscs = tchRed.Install (dadb);
  Ptr<QueueDisc> redQueue = queueDiscs.Get (0);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0.5ms"));
  NetDeviceContainer dbd5 = p2p.Install (nbn5);
  tchPfifo.Install (dbd5);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer dbd6 = p2p.Install (nbn6);
  tchPfifo.Install (dbd6);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  NetDeviceContainer dbd7 = p2p.Install (nbn7);
  tchPfifo.Install (dbd7);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer dbd8 = p2p.Install (nbn8);
  tchPfifo.Install (dbd8);


  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i1ia = ipv4.Assign (d1da);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i2ia = ipv4.Assign (d2da);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i3ia = ipv4.Assign (d3da);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i4ia = ipv4.Assign (d4da);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  iaib = ipv4.Assign (dadb);

  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  ibi5 = ipv4.Assign (dbd5);

  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  ibi6 = ipv4.Assign (dbd6);

  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  ibi7 = ipv4.Assign (dbd7);

  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  ibi8 = ipv4.Assign (dbd8);

  redQueue->TraceConnectWithoutContext("Enqueue", MakeCallback(&EnqueueAtRed));
  redQueue->TraceConnectWithoutContext("Dequeue", MakeCallback(&DequeueAtRed));
  redQueue->TraceConnectWithoutContext("Drop", MakeCallback(&DroppedAtRed)); 
 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ipv4InterfaceContainer ip4;

ip4.Add(i1ia.Get(0));
ip4.Add(i2ia.Get(0));
ip4.Add(i3ia.Get(0));
ip4.Add(i4ia.Get(0));
ip4.Add(ibi5.Get(1));
ip4.Add(ibi6.Get(1));
ip4.Add(ibi7.Get(1));
ip4.Add(ibi8.Get(1));

 double min1 = 0.0;
double max1 = 0.7;
Ptr<UniformRandomVariable> x1 = CreateObject<UniformRandomVariable> ();
x1->SetAttribute ("Min", DoubleValue (min1));
x1->SetAttribute ("Max", DoubleValue (max1));


double min2 = 0.1;
double max2 = 0.7;
Ptr<UniformRandomVariable> x2 = CreateObject<UniformRandomVariable> ();
x2->SetAttribute ("Min", DoubleValue (min2));
x2->SetAttribute ("Max", DoubleValue (max2));

  for(int j=1; j<=4; j++){
    for(int k=1; k<=8; k++){
      Loopsetup(j, k, c.Get(j-1), c.Get(j+3), ip4.GetAddress(j+3), x1->GetValue(),x2->GetValue(), "100Mbps", 600);
    }
  }

for(int j=5; j<=8; j++){
    for(int k=1; k<=8; k++){
      Loopsetup(j-1, k, c.Get(j-1), c.Get(j-5), ip4.GetAddress(j-5), x1->GetValue(),x2->GetValue(), "100Mbps", 600);

    }
  }
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


      Ptr<QueueDisc> queue1 = queueDiscs.Get (0);
      Ptr<QueueDisc> queue2 = queueDiscs.Get (1);

      Simulator::ScheduleNow (&CheckQueueSize, queue1, 1);
      Simulator::ScheduleNow (&CheckQueueSize, queue2, 2);

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
      std::cout << "*** RED stats from Node a queue ***" << std::endl;
      std::cout << "\t " << st.unforcedDrop << " drops due prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due hard mark" << std::endl;
      std::cout << "\t " << st.qLimDrop << " drops due queue full" << std::endl;

      st = StaticCast<RedQueueDisc> (queueDiscs.Get (1))->GetStats ();
      std::cout << "*** RED stats from Node b queue ***" << std::endl;
      std::cout << "\t " << st.unforcedDrop << " drops due prob mark" << std::endl;
      std::cout << "\t " << st.forcedDrop << " drops due hard mark" << std::endl;
      std::cout << "\t " << st.qLimDrop << " drops due queue full" << std::endl;
    }

  Simulator::Destroy ();

  return 0;
}
