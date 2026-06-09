//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef ROUTER_H
#define ROUTER_H

#include <omnetpp.h>
#include "RoutingMessage_m.h"
#include <map>
#include <vector>

using namespace omnetpp;
using namespace std;

class Router : public cSimpleModule
{
  private:
    string routerId;

    map<string, int> ripTable;
    map<string, int> lsdb;
    map<string, map<string, double>> topology;

    int seqNum;

    cMessage *event;  // timer for LSU

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void sendLSU();
    void processRIP(RoutingMessage *msg);
    void processAHRP(RoutingMessage *msg);
    void recomputeRoutes();

    string signMessage(string msg);
    bool verifyMessage(string msg, string sig, string origin);
};

#endif
