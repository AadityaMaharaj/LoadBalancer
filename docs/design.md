# Load Balancer — Design Notes

## Why C?
Low-level control over sockets and memory. Shows OS fundamentals.

## Algorithms (Phase 3)
- Round robin: simple, fair, stateless
- Least connections: smarter, tracks active connections per backend

## Phases
1. Listen for connections
2. Forward traffic to backends
3. Routing algorithms
4. Health checks
5. Demo layer