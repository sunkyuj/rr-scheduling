import matplotlib.pyplot as plt

time_quantum = [2, 3, 4, 5]
total_time = [117, 81, 67, 57]

cpu_utilization = [53.846154, 70.370370, 79.104478, 96.491228]
io_utilization = [97.435897, 93.827160, 58.208955, 56.140351]
avg_response_time = [6.2, 8.5, 14.8, 12.5]
avg_turnaround_time = [63.4, 50.7, 36.7, 28.8]
throughput = [0.015773, 0.019724, 0.027248, 0.034722]

# CPU Utilization 그래프
plt.figure(figsize=(12, 10))
plt.subplot(4, 2, 1)
plt.plot(time_quantum, cpu_utilization, marker='o')
plt.title('CPU Utilization')
plt.xlabel('Time Quantum')
plt.ylabel('Percentage (%)')

# I/O Utilization 그래프
plt.subplot(4, 2, 2)
plt.plot(time_quantum, io_utilization, marker='o', color='orange')
plt.title('I/O Utilization')
plt.xlabel('Time Quantum')
plt.ylabel('Percentage (%)')

# Average Response Time 그래프
plt.subplot(4, 2, 3)
plt.plot(time_quantum, avg_response_time, marker='o', color='g')
plt.title('Average Response Time')
plt.xlabel('Time Quantum')
plt.ylabel('Seconds')

# Average Turnaround Time 그래프
plt.subplot(4, 2, 4)
plt.plot(time_quantum, avg_turnaround_time, marker='o', color='purple')
plt.title('Average Turnaround Time')
plt.xlabel('Time Quantum')
plt.ylabel('Seconds')

# Throughput 그래프
plt.subplot(4, 2, 5)
plt.plot(time_quantum, throughput, marker='o', color='r')
plt.title('Throughput')
plt.xlabel('Time Quantum')
plt.ylabel('Jobs per Second')

# Total Time 그래프
plt.subplot(4, 2, 6)
plt.plot(time_quantum, total_time, marker='o', color='brown')
plt.title('Total Time')
plt.xlabel('Time Quantum')
plt.ylabel('Seconds')

plt.tight_layout()
plt.show()
