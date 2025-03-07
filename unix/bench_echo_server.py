import argparse
import socket
import time
import threading
import queue
import statistics
from collections import defaultdict

class EchoClient:
    def __init__(self, host, port, msg_size, num_requests):
        self.host = host
        self.port = port
        self.msg = b'A' * msg_size  # 生成指定大小的测试消息
        self.num_requests = num_requests
        self.latencies = []
        self.error_count = 0

    def run(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.host, self.port))
            
            for _ in range(self.num_requests):
                start = time.perf_counter()
                sock.sendall(self.msg)
                resp = sock.recv(len(self.msg))                
                if resp != self.msg:
                    self.error_count += 1
                else:
                    latency = (time.perf_counter() - start) * 1000  # 毫秒
                    self.latencies.append(latency)
            
            sock.close()
        except Exception as e:
            self.error_count += self.num_requests

def worker(task_queue, result_queue, host, port, msg_size, requests_per_client):
    while True:
        try:
            client_id = task_queue.get_nowait()
        except queue.Empty:
            break
            
        client = EchoClient(host, port, msg_size, requests_per_client)
        client.run()
        
        result_queue.put({
            'latencies': client.latencies,
            'errors': client.error_count
        })
        task_queue.task_done()

def analyze_results(results, duration):
    total_requests = 0
    total_errors = 0
    all_latencies = []
    
    for res in results:
        total_requests += len(res['latencies']) + res['errors']
        total_errors += res['errors']
        all_latencies.extend(res['latencies'])
    
    if not all_latencies:
        print("No successful requests!")
        return

    print(f"\n{' Benchmark Results ':=^60}")
    print(f"Total Requests  : {total_requests}")
    print(f"Successful      : {total_requests - total_errors}")
    print(f"Errors          : {total_errors} ({total_errors/total_requests:.2%})")
    print(f"Throughput      : {total_requests / duration:.2f} req/s")
    print(f"Avg Latency     : {statistics.mean(all_latencies):.2f} ms")
    print(f"P50 Latency     : {statistics.quantiles(all_latencies, n=100)[49]:.2f} ms")
    print(f"P90 Latency     : {statistics.quantiles(all_latencies, n=100)[89]:.2f} ms")
    print(f"P99 Latency     : {statistics.quantiles(all_latencies, n=100)[98]:.2f} ms")
    print(f"Max Latency     : {max(all_latencies):.2f} ms")

def main():
    parser = argparse.ArgumentParser(description='Echo Server Benchmark Tool')
    parser.add_argument('--host', default='localhost', help='Server hostname')
    parser.add_argument('--port', type=int, default=8080, help='Server port')
    parser.add_argument('-c', '--clients', type=int, default=100, help='Number of concurrent clients')
    parser.add_argument('-r', '--requests', type=int, default=10, 
                       help='Requests per client')
    parser.add_argument('-s', '--size', type=int, default=1024,
                       help='Message size in bytes')
    args = parser.parse_args()

    task_queue = queue.Queue()
    result_queue = queue.Queue()
    
    # 初始化任务队列
    for i in range(args.clients):
        task_queue.put(i)

    # 创建工作线程
    threads = []
    start_time = time.perf_counter()
    
    for _ in range(args.clients):
        t = threading.Thread(
            target=worker,
            args=(task_queue, result_queue, args.host, args.port, args.size, args.requests)
        )
        t.start()
        threads.append(t)

    # 等待所有任务完成
    task_queue.join()
    duration = time.perf_counter() - start_time

    # 收集结果
    results = []
    while not result_queue.empty():
        results.append(result_queue.get())

    # 分析结果
    analyze_results(results, duration)

if __name__ == '__main__':
    main()