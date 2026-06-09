network AHRPNetwork
{
    submodules:
        R0: Router {
            parameters:
                routerId = "R0";
                @display("p=213,73");
        }
        R1: Router {
            parameters:
                routerId = "R1";
                @display("p=500,120");
        }
        R2: Router {
            parameters:
                routerId = "R2";
                @display("p=697,81");
        }
        R3: Router {
            parameters:
                routerId = "R3";
                @display("p=632,346");
        }
        R4: Router {
            parameters:
                routerId = "R4";
                @display("p=384,346");
        }
        R5: Router {
            parameters:
                routerId = "R5";
                @display("p=191,215");
        }

    connections allowunconnected:

        // Main chain
        R0.gate++ <--> {  delay = 10ms; datarate = 100Mbps; } <--> R1.gate++;
        R1.gate++ <--> {  delay = 15ms; datarate = 80Mbps; } <--> R2.gate++;
        R2.gate++ <--> {  delay = 20ms; datarate = 70Mbps; } <--> R3.gate++;
        R3.gate++ <--> {  delay = 10ms; datarate = 90Mbps; } <--> R4.gate++;
        R4.gate++ <--> {  delay = 15ms; datarate = 60Mbps; } <--> R5.gate++;

        // Extra links
        R0.gate++ <--> {  delay = 50ms; datarate = 50Mbps; } <--> R5.gate++;
        R1.gate++ <--> {  delay = 25ms; datarate = 75Mbps; } <--> R4.gate++;
        R1.gate++ <--> {  delay = 40ms; datarate = 50Mbps; } <--> R5.gate++;
        R1.gate++ <--> {  delay = 25ms; datarate = 100Mbps; } <--> R3.gate++;
}
