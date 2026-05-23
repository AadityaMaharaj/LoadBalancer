import socket
import time
import sys

def send_request(i):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(3)
        s.connect(('localhost', 8080))
        msg = f"GET / HTTP/1.0\r\nHost: localhost\r\n\r\n"
        s.send(msg.encode())
        response = s.recv(1024).decode().strip()
        s.close()
        # grab just the last line
        last_line = [l for l in response.split('\n') if l.strip()][-1]
        print(f"  [{i:02d}] {last_line}")
    except Exception as e:
        print(f"  [{i:02d}] error: {e}")

count = int(sys.argv[1]) if len(sys.argv) > 1 else 10
delay = float(sys.argv[2]) if len(sys.argv) > 2 else 0.3

print(f"\n  Sending {count} requests to load balancer...")
print("  ─────────────────────────────────────")
for i in range(1, count + 1):
    send_request(i)
    time.sleep(delay)
print("  ─────────────────────────────────────")
print("  Done.\n")