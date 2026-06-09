import hmac
import hashlib
import networkx as nx
import os
import matplotlib.pyplot as plt
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.log import setLogLevel, info
from mininet.cli import CLI

# -----------------------------
# 1. AHRP SECURITY & LOGIC
# -----------------------------
# HMAC shared keys using University Index number for uniqueness
SECRET_KEYS = {f"s{i}": f"secret_key_230548R_{i}".encode() for i in range(6)}
METRIC_WEIGHTS = {"w1_latency": 0.6, "w2_bandwidth": 0.4}

class AHRP_Controller:
    def __init__(self):
        self.topology = nx.Graph()

    def calculate_cost(self, latency, bandwidth):
        """ AHRP Cost Model: Weighted Latency and Normalized Bandwidth """
        w1, w2 = METRIC_WEIGHTS["w1_latency"], METRIC_WEIGHTS["w2_bandwidth"]
        cost = (w1 * (latency / 50)) + (w2 * (100 / (bandwidth + 1)))
        return round(cost, 4)

    def compute_path(self, src, dst):
        """ Dijkstra's Algorithm based on AHRP calculated weights """
        try:
            return nx.dijkstra_path(self.topology, src, dst, weight='weight')
        except nx.NetworkXNoPath:
            return None
        except Exception as e:
            print(f"Routing Error: {e}")
            return None

    def verify_update(self, u, v, lat, bw, received_sig):
        """ Cryptographic verification of Link State Updates (LSU) """
        cost = self.calculate_cost(lat, bw)
        msg = f"{u}-{v}-{cost}"
        expected_sig = hmac.new(SECRET_KEYS.get(u, b"key"), msg.encode(), hashlib.sha256).hexdigest()
        is_valid = hmac.compare_digest(expected_sig, received_sig)
        return is_valid, expected_sig

# -----------------------------
# 2. TOPOLOGY DEFINITION
# -----------------------------
class AHRP_Mesh_Topo(Topo):
    def build(self):
        # Create 6 Core Routers
        switches = [self.addSwitch(f's{i}') for i in range(6)]
        
        # Create End-User Hosts
        h1 = self.addHost('h1', ip='10.0.0.1/24')
        h2 = self.addHost('h2', ip='10.0.0.2/24')
        
        # Edge Connections
        self.addLink(h1, 's0')
        self.addLink(h2, 's5')

        # Heterogeneous Links (u, v, latency_ms, bandwidth_mbps)
        # Link s0-s5 is purposely slow (200ms) to test 'Adaptive' logic
        links = [
            ('s0', 's1', 10, 100), ('s1', 's2', 15, 80), ('s2', 's3', 20, 70),
            ('s3', 's4', 10, 90), ('s4', 's5', 15, 60), ('s0', 's5', 200, 50),
            ('s1', 's4', 25, 75), ('s1', 's3', 25, 100)
        ]
        for u, v, lat, bw in links:
            self.addLink(u, v, cls=TCLink, delay=f'{lat}ms', bw=bw)

# -----------------------------
# 3. FLOW INSTALLATION (Bridge between Logic and Hardware)
# -----------------------------
def install_path(net, path, target_host):
    """ Installs OpenFlow rules to switches along the computed path """
    if not path: return
    for i in range(len(path)):
        sw = net.get(path[i])
        next_node = net.get(path[i+1]) if i < len(path)-1 else target_host
        
        # Find local port connecting to the next hop
        intf = sw.connectionsTo(next_node)[0][0]
        port = sw.ports[intf]
        
        # Add IP and ARP flows to avoid packet drops
        sw.cmd(f'ovs-ofctl add-flow {sw.name} "priority=100,ip,nw_dst={target_host.IP()},actions=output:{port}"')
        sw.cmd(f'ovs-ofctl add-flow {sw.name} "priority=100,arp,arp_tpa={target_host.IP()},actions=output:{port}"')

# -----------------------------
# 4. VISUALIZATION
# -----------------------------
def visualize_ahrp_network(ahrp_controller, path=None):
    """ Matplotlib visualization for report documentation """
    G = ahrp_controller.topology
    pos = nx.spring_layout(G, seed=42)
    
    plt.figure(figsize=(10, 7))
    nx.draw_networkx_nodes(G, pos, node_size=900, node_color='#90ee90', edgecolors='black')
    nx.draw_networkx_labels(G, pos, font_size=12, font_weight='bold')
    nx.draw_networkx_edges(G, pos, width=1.5, alpha=0.5, edge_color='gray')
    
    if path:
        path_edges = list(zip(path, path[1:]))
        nx.draw_networkx_edges(G, pos, edgelist=path_edges, width=4, edge_color='#1f78b4', label='AHRP Path')
    
    edge_labels = nx.get_edge_attributes(G, 'weight')
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_color='red')
    
    plt.title("AHRP Adaptive Path Selection (Cost-Based)", fontsize=15)
    plt.legend(loc='upper left')
    plt.axis('off')
    plt.savefig('mininet_ahrp_vis.png', dpi=300)
    print("\n[Visual] Topology visualization saved as 'mininet_ahrp_vis.png'")
    plt.show()

# -----------------------------
# 5. MAIN EXECUTION
# -----------------------------
def run_simulation():
    # Cleanup previous Mininet sessions
    os.system('sudo mn -c > /dev/null 2>&1')
    
    topo = AHRP_Mesh_Topo()
    net = Mininet(topo=topo, link=TCLink, controller=None)
    net.start()
    
    ahrp = AHRP_Controller()
    h1, h2 = net.get('h1', 'h2')

    # Security Validation (Appendix Figure 4)
    print("\n" + "="*60)
    print("  FIGURE 4: MANUAL SECURITY VERIFICATION (Node S1)")
    print("="*60)
    test_lat, test_bw = 10, 100
    cost = ahrp.calculate_cost(test_lat, test_bw)
    origin_sig = hmac.new(SECRET_KEYS['s0'], f"s0-s1-{cost}".encode(), hashlib.sha256).hexdigest()
    valid, local_sig = ahrp.verify_update('s0', 's1', test_lat, test_bw, origin_sig)
    print(f"R1 Received Update: origin=R0, cost={cost}, sig={origin_sig[:16]}...")
    print(f"R1 Local Calculation: sig={local_sig[:16]}...")
    print(f"R1 Verification Status: {'[ VERIFIED ]' if valid else '[ FAILED ]'}")
    print("="*60 + "\n")

    # Populate Controller Topology
    links = [
        ('s0', 's1', 10, 100), ('s1', 's2', 15, 80), ('s2', 's3', 20, 70),
        ('s3', 's4', 10, 90), ('s4', 's5', 15, 60), ('s0', 's5', 200, 50),
        ('s1', 's4', 25, 75), ('s1', 's3', 25, 100)
    ]
    for u, v, l, b in links:
        ahrp.topology.add_edge(u, v, weight=ahrp.calculate_cost(l, b))

    # Path Computation
    path = ahrp.compute_path('s0', 's5')
    
    if path:
        print("\n" + "="*60)
        print("  FIGURE 5: ICMP CONNECTIVITY & PATH VERIFICATION")
        print("="*60)
        print(f"Computed AHRP Path: {' -> '.join(path)}")
        
        # Install flows for both directions (Forward & Return)
        install_path(net, path, h2)
        install_path(net, list(reversed(path)), h1)
        
        print("\n[Action] Testing Connectivity (Ping)...")
        print(h1.cmd(f'ping -c 3 {h2.IP()}'))
        
        print("\n[Action] Traceroute (Hop Verification)...")
        print(h1.cmd(f'tracepath -n {h2.IP()}'))
        print("="*60)
        
        visualize_ahrp_network(ahrp, path)
    else:
        print("Error: AHRP could not find a path.")

    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run_simulation()
