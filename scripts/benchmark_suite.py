#!/usr/bin/env python3
import subprocess
import json
import matplotlib.pyplot as plt
import os
import argparse
import sys

def run_benchmark(executable, model, threads, requests, warmup):
    """Run inferbench and return the result dictionary."""
    output_json = f"result_t{threads}.json"
    cmd = [
        executable,
        "--model", model,
        "--threads", str(threads),
        "--requests", str(requests),
        "--warmup", str(warmup),
        "--json", output_json,
        "--optimization", "all" # Ensure best performance
    ]
    
    print(f"Running: {' '.join(cmd)}")
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print(f"Error running benchmark: {e.stderr.decode()}")
        return None

    if not os.path.exists(output_json):
        print(f"Error: {output_json} not generated.")
        return None

    with open(output_json, 'r') as f:
        data = json.load(f)
    
    # Cleanup
    os.remove(output_json)
    return data['result']

def plot_results(threads_list, qps_list, lat_list, output_dir):
    """Generate QPS and Latency plots."""
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 1. Throughput (QPS)
    plt.figure(figsize=(10, 6))
    plt.plot(threads_list, qps_list, marker='o', linewidth=2, color='#28a745')
    plt.title('Throughput vs Threads')
    plt.xlabel('Threads')
    plt.ylabel('QPS (Queries Per Second)')
    plt.grid(True, linestyle='--', alpha=0.7)
    for x, y in zip(threads_list, qps_list):
        plt.text(x, y, f'{y:.1f}', ha='center', va='bottom')
    plt.savefig(os.path.join(output_dir, 'throughput.png'))
    plt.close()

    # 2. Latency
    plt.figure(figsize=(10, 6))
    plt.plot(threads_list, lat_list, marker='s', linewidth=2, color='#dc3545')
    plt.title('Average Latency vs Threads')
    plt.xlabel('Threads')
    plt.ylabel('Latency (ms)')
    plt.grid(True, linestyle='--', alpha=0.7)
    for x, y in zip(threads_list, lat_list):
        plt.text(x, y, f'{y:.1f}', ha='center', va='bottom')
    plt.savefig(os.path.join(output_dir, 'latency.png'))
    plt.close()
    
    print(f"Plots saved to {output_dir}/")

def main():
    parser = argparse.ArgumentParser(description="InferBench Automation Suite")
    parser.add_argument("--bin", default="./build/bin/inferbench", help="Path to inferbench executable")
    parser.add_argument("--model", required=True, help="Path to ONNX model")
    parser.add_argument("--requests", type=int, default=100, help="Requests per thread")
    args = parser.parse_args()

    if not os.path.exists(args.bin):
        print(f"Error: Executable {args.bin} not found.")
        sys.exit(1)
        
    if not os.path.exists(args.model):
        print(f"Error: Model {args.model} not found.")
        sys.exit(1)

    threads_config = [1, 2, 4, 8, 16]
    results_qps = []
    results_lat = []

    print("=== Starting Benchmark Suite ===")
    
    for t in threads_config:
        # Scale requests with threads to keep load sufficient, or keep per-thread constant?
        # Runner logic: total requests are distributed.
        # Let's keep total requests proportional to threads to ensure longer running time?
        # Or simple constant total requests? 
        # For meaningful throughput test, total requests should be large.
        # Let's use user arg --requests as "requests per thread" roughly
        total_reqs = args.requests * t
        
        res = run_benchmark(args.bin, args.model, t, total_reqs, warmup=10)
        if res:
            results_qps.append(res['qps'])
            results_lat.append(res['avg_latency_ms'])
            print(f"  [Threads: {t}] QPS: {res['qps']:.2f}, Latency: {res['avg_latency_ms']:.2f} ms")
        else:
            print(f"  [Threads: {t}] Failed")
            results_qps.append(0)
            results_lat.append(0)

    plot_results(threads_config, results_qps, results_lat, ".")

if __name__ == "__main__":
    main()
