import socket
import threading
import sys

def run_server(port, name):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('localhost', port))
    s.listen(5)
    print(f"[{name}] listening on port {port}")
    while True:
        conn, addr = s.accept()
        data = conn.recv(1024)
        print(f"[{name}] got request: {data.decode().strip()}")
        conn.send(f"Response from {name}\n".encode())
        conn.close()

servers = [
    (8081, "Backend-1"),
    (8082, "Backend-2"),
    (8083, "Backend-3"),
]

threads = [threading.Thread(target=run_server, args=(p, n)) for p, n in servers]
for t in threads:
    t.daemon = True
    t.start()

print("All backends running. Press Ctrl+C to stop.")
try:
    for t in threads:
        t.join()
except KeyboardInterrupt:
    print("\nShutting down.")