import matplotlib.pyplot as plt

# 시뮬레이션 결과 데이터 정의
results = {
    "0%": {
        "total_time": 60,
        "avg_response_time": 12.000000,
        "avg_turnaround_time": 37.000000,
        "throughput": 0.027027,
        "cpu_utilization": 100.000000,
        "io_utilization": 0.000000
    },
    "10%": {
        "total_time": 53,
        "avg_response_time": 11.800000,
        "avg_turnaround_time": 39.300000,
        "throughput": 0.025445,
        "cpu_utilization": 98.113208,
        "io_utilization": 37.735849
    },
    "50%": {
        "total_time": 81,
        "avg_response_time": 8.500000,
        "avg_turnaround_time": 50.700000,
        "throughput": 0.019724,
        "cpu_utilization": 70.370370,
        "io_utilization": 93.827160
    },
    "25%": {
        "total_time": 48,
        "avg_response_time": 11.000000,
        "avg_turnaround_time": 26.200000,
        "throughput": 0.038168,
        "cpu_utilization": 95.833333,
        "io_utilization": 20.833333
    },
    "33%": {
        "total_time": 63,
        "avg_response_time": 8.700000,
        "avg_turnaround_time": 48.000000,
        "throughput": 0.020833,
        "cpu_utilization": 100.000000,
        "io_utilization": 85.714286
    },
    "66%": {
        "total_time": 79,
        "avg_response_time": 9.300000,
        "avg_turnaround_time": 41.600000,
        "throughput": 0.024038,
        "cpu_utilization": 84.810127,
        "io_utilization": 92.405063
    },
    "75%": {
        "total_time": 92,
        "avg_response_time": 8.200000,
        "avg_turnaround_time": 49.300000,
        "throughput": 0.020284,
        "cpu_utilization": 61.956522,
        "io_utilization": 94.565217
    },
    "100%": {
        "total_time": 165,
        "avg_response_time": 6.900000,
        "avg_turnaround_time": 94.900000,
        "throughput": 0.010537,
        "cpu_utilization": 38.787879,
        "io_utilization": 96.363636
    }
}

# 그래프 생성
plt.figure(figsize=(12, 14))

# CPU Utilization 그래프
plt.subplot(4, 2, 1)
plt.plot(results.keys(), [result["cpu_utilization"] for result in results.values()], marker='o')
plt.title('CPU Utilization')
plt.xlabel('I/O Occurrence Rate')
plt.ylabel('Percentage (%)')

# I/O Utilization 그래프
plt.subplot(4, 2, 2)
plt.plot(results.keys(), [result["io_utilization"] for result in results.values()], marker='o', color='orange')
plt.title('I/O Utilization')
plt.xlabel('I/O Occurrence Rate')
plt.ylabel('Percentage (%)')

# Average Response Time 그래프
plt.subplot(4, 2, 3)
plt.plot(results.keys(), [result["avg_response_time"] for result in results.values()], marker='o', color='g')
plt.title('Average Response Time')
plt.xlabel('I/O Occurrence Rate')
plt.ylabel('Seconds')

# Average Turnaround Time 그래프
plt.subplot(4, 2, 4)
plt.plot(results.keys(), [result["avg_turnaround_time"] for result in results.values()], marker='o', color='purple')
plt.title('Average Turnaround Time')
plt.xlabel('I/O Occurrence Rate')
plt.ylabel('Seconds')

# Throughput 그래프
plt.subplot(4, 2, 5)
plt.plot(results.keys(), [result["throughput"] for result in results.values()], marker='o', color='r')
plt.title('Throughput')
plt.xlabel('I/O Occurrence Rate')
plt.ylabel('Jobs per Second')

# Total Time 그래프
plt.subplot(4, 2, 6)
plt.plot(results.keys(), [result["total_time"] for result in results.values()], marker='o', color='brown')
plt.title('Total Time')
plt.xlabel('I/O Occurrence Rate')
plt.ylabel('Seconds')

plt.tight_layout()
plt.show()
