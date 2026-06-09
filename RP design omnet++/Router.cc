#include "Router.h"
#include <functional>
#include <sstream>

Define_Module(Router);

string Router::signMessage(string msg)
{
    return to_string(hash<string>{}(msg + routerId));
}

bool Router::verifyMessage(string msg, string sig, string origin)
{
    return sig == signMessage(msg);
}
void Router::initialize()
{
    routerId = par("routerId").stringValue();
    seqNum = 0;

    event = new cMessage("LSU_TIMER");

    scheduleAt(simTime() + 1.0, event);
}

void Router::handleMessage(cMessage *msg)
{
    if (msg == event)
    {
        sendLSU();
        scheduleAt(simTime() + 5.0, event);
        return;
    }

    RoutingMessage *pkt = check_and_cast<RoutingMessage *>(msg);

    if (pkt->getType() == "RIP")
        processRIP(pkt);
    else if (pkt->getType() == "AHRP")
        processAHRP(pkt);

    delete pkt;
}
void Router::sendLSU()
{
    seqNum++;

    RoutingMessage *msg = new RoutingMessage();
    msg->setOrigin(routerId.c_str());
    msg->setSeq(seqNum);
    msg->setType("AHRP");

    string payload = "LINKSTATE";
    msg->setPayload(payload.c_str());

    string sig = signMessage(payload);
    msg->setSignature(sig.c_str());

    for (int i = 0; i < gateSize("gate"); i++)
    {
        RoutingMessage *copy = msg->dup();  // duplicate for each neighbor
        send(copy, "gate$o", i);
    }
    delete msg;
}
void Router::processAHRP(RoutingMessage *msg)
{
    string origin = msg->getOrigin();
    int seq = msg->getSeq();

    if (lsdb.find(origin) != lsdb.end() && lsdb[origin] >= seq)
        return;

    lsdb[origin] = seq;

    // update topology (simplified)
    topology[origin][routerId] = 1;

    recomputeRoutes();
}

void Router::recomputeRoutes()
{
    EV << "Recomputing routes for " << routerId << endl;

    // simplified version (you can extend to real Dijkstra)
    for (auto &node : topology)
    {
        EV << "Route info from " << routerId
           << " to " << node.first << endl;
    }
}
void Router::processRIP(RoutingMessage *msg)
{
    string origin = msg->getOrigin();

    if (ripTable.find(origin) == ripTable.end())
        ripTable[origin] = 1;
    else
        ripTable[origin] = min(ripTable[origin] + 1, 100);
}
