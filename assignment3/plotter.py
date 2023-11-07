from scapy.all import *
import matplotlib.pyplot as plt
import statistics

pcap_file = "/home/akshat/Downloads/CNAssignments/assignment3/PCAPfiles/epoll/500.pcap"
packets = rdpcap(pcap_file)
all_throughput = []
all_latency = []
throughput = {}
latency = {}
for packet in packets:
    if TCP in packet:
        src_ip = packet[IP].src
        dst_ip = packet[IP].dst
        src_port = packet[TCP].sport
        dst_port = packet[TCP].dport
        flow_key = f"{src_ip}:{src_port}-{dst_ip}:{dst_port}"
        if flow_key not in throughput:
            throughput[flow_key] = []
        if flow_key not in latency:
            latency[flow_key] = []
        payload_length = len(packet[TCP].payload)
        time = packet.time
        if len(throughput[flow_key]) > 0:
            throughput[flow_key].append((payload_length, time - throughput[flow_key][-1][1]))
        else:
            throughput[flow_key].append((payload_length, 0))

        if packet[TCP].ack in latency[flow_key]:
            latency[flow_key].append(packet.time - packet.sent_time)

for flow_key in throughput:      # Computing and printing average throughput and latency for each TCP flow
    total_bytes = sum(p[0] for p in throughput[flow_key])
    total_time = sum(p[1] for p in throughput[flow_key])
    if total_time > 0:
        average_throughput = (total_bytes * 8) / total_time
    else:
        average_throughput = 0
    if latency[flow_key]:
        avg_latency = statistics.mean(latency[flow_key]) * 1000  # Convert to milliseconds
    else:
        avg_latency = 0

    print(f"Flow: {flow_key}")
    print(f"Average Throughput: {average_throughput:.2f} bps")
    print(f"Average Latency: {avg_latency:.2f} ms")
    print()

    if average_throughput > 0:
        all_throughput.append(average_throughput)
    if avg_latency > 0:
        all_latency.append(avg_latency)

# Compute overall average throughput and latency
overall_avg_throughput = statistics.mean(all_throughput) if all_throughput else 0
overall_avg_latency = statistics.mean(all_latency) if all_latency else 0

print(f"Overall Average Throughput: {overall_avg_throughput:.2f} bps")
print(f"Overall Average Latency: {overall_avg_latency:.2f} ms")

# Create a plot for overall average throughput
plt.figure()
plt.bar(["Overall Average Throughput"], [overall_avg_throughput])
plt.title("Overall Average Throughput")
plt.ylabel("Throughput (bps)")

# Create a plot for overall average latency
plt.figure()
plt.bar(["Overall Average Latency"], [overall_avg_latency])
plt.title("Overall Average Latency")
plt.ylabel("Latency (ms)")

# Show the plots
plt.show()
