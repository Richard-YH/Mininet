import subprocess
from mininet.net import Mininet
from mininet.node import OVSSwitch, RemoteController
from mininet.cli import CLI
from mininet.log import setLogLevel
from mininet.link import TCLink

def startRyuController(ip='127.0.0.1', port=6633):
    ryu_cmd = ['ryu-manager', '--ofp-tcp-listen-port', str(port), '--ofp-listen-host', ip, 'ryu.app.simple_switch']
    ryu_process = subprocess.Popen(ryu_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return ryu_process

# 3. “runIperf” function 
def runIperf(net, src, dst, bandwidth_limit):
    print(f"Starting iperf session from {src} to {dst} with bandwidth limit {bandwidth_limit} Mbps")
    server = net.getNodeByName(dst)
    client = net.getNodeByName(src)
    
    # Start iperf server
    server.cmd('iperf -s &')
    
    # Execute iperf client command
    result = client.cmd(f'iperf -c {server.IP()} -b {bandwidth_limit}M')
    print(result)
    # Check for bandwidth limit exceedance
    if "exceeding" in result:  # Modify this condition based on actual output
        print(f"Bandwidth limit exceeded for session from {src} to {dst}. Terminating session.")
        server.cmd('kill %iperf')
        client.cmd('kill %iperf')


def createTopo():

    # 1. Generate Topology
    net = Mininet(controller=RemoteController, switch=OVSSwitch)

    print(f"Creating node")
    switches = [net.addSwitch('s%d' % i) for i in range(1, 7)]
    hosts = [net.addHost('h%d' % i) for i in range(1, 10)]

    print(f"Creating Ryu controller")
    ryu_process = startRyuController()
    c0 = net.addController('c0', controller=RemoteController, ip='127.0.0.1', port=6633)
    
    # Add links
    print(f"Creating link")
    net.addLink(hosts[0], switches[0])  # h1 to s1
    net.addLink(hosts[1], switches[0])  # h2 to s1
    net.addLink(switches[0], switches[1])  # s1 to s2
    net.addLink(switches[1], hosts[2])  # s2 to h3
    net.addLink(switches[1], switches[2])  # s2 to s3
    net.addLink(switches[2], hosts[3])  # s3 to h4
    net.addLink(switches[2], hosts[4])  # s3 to h5
    net.addLink(switches[0], switches[3])  # s1 to s4
    net.addLink(switches[3], hosts[5])  # s4 to h6
    net.addLink(switches[3], switches[4])  # s4 to s5
    net.addLink(switches[4], hosts[6])  # s5 to h7
    net.addLink(switches[4], switches[5])  # s5 to s6
    net.addLink(switches[5], hosts[7])  # s6 to h8
    net.addLink(switches[5], hosts[8])  # s6 to h9

    # Starting network
    print("Starting network")
    net.start()

    # 2. Set up iperf sessions
    runIperf(net, 'h1', 'h2', 5)
    runIperf(net, 'h1', 'h3', 10)
    runIperf(net, 'h4', 'h5', 15)
    runIperf(net, 'h6', 'h8', 20)

    print("Running CLI")
    CLI(net)

    print("Stopping network")
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    createTopo()
