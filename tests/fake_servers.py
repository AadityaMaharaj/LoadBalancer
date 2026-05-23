import socket
import threading
import time
import sys

REQUEST_COUNT = {1: 0, 2: 0, 3: 0}
START_TIME = time.time()

def run_server(port, name, number):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('localhost', port))
    s.listen(5)
    while True:
        try:
            conn, addr = s.accept()
            data = conn.recv(1024)
            REQUEST_COUNT[number] += 1
            total = sum(REQUEST_COUNT.values())
            elapsed = round(time.time() - START_TIME, 1)
            print(
                f"  [{name}] request #{REQUEST_COUNT[number]} "
                f"| total: {total} | uptime: {elapsed}s"
            )
            response = (
                f"HTTP/1.1 200 OK\r\n"
                f"Content-Type: text/plain\r\n\r\n"
                f"Hello from {name} (port {port})\n"
            )
            conn.send(response.encode())
            conn.close()
        except Exception:
            pass

servers = [
    (8081, "Backend-1", 1),
    (8082, "Backend-2", 2),
    (8083, "Backend-3", 3),
]

print("\n  Load Balancer Demo — Backend Servers")
print("  ─────────────────────────────────────")
for port, name, _ in servers:
    print(f"  {name} listening on port {port}")
print("  ─────────────────────────────────────")
print("  Waiting for requests...\n")

threads = [
    threading.Thread(target=run_server, args=(p, n, num), daemon=True)
    for p, n, num in servers
]
for t in threads:
    t.start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    total = sum(REQUEST_COUNT.values())
    print("\n  ─────────────────────────────────────")
    print(f"  Session summary")
    print(f"  ─────────────────────────────────────")
    for i, (_, name, num) in enumerate(servers):
        pct = round(REQUEST_COUNT[num] / total * 100) if total else 0
        bar = "█" * (pct // 5)
        print(f"  {name}: {REQUEST_COUNT[num]} reqs ({pct}%) {bar}")
    print(f"  Total: {total} requests")
    print("  ─────────────────────────────────────\n")