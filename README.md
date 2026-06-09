# Routing-Protocol-Design
Adaptive Hybrid Routing Protocol (AHRP)
# Adaptive Hybrid Routing Protocol (AHRP)

[cite_start]An academic implementation of a secure, dynamic routing protocol that integrates distance-vector simplicity with link-state accuracy[cite: 484, 490]. [cite_start]Designed and validated using Python (**NetworkX**, **Matplotlib**) and high-fidelity containerized emulations inside **Mininet**[cite: 448, 450].

[cite_start]This project was developed for the module **EN2150: Communication Network Engineering** at the Department of Electronic & Telecommunication Engineering, University of Moratuwa[cite: 639, 643].

---

## 🚀 Overview

[cite_start]The **Adaptive Hybrid Routing Protocol (AHRP)** addresses the structural vulnerabilities of traditional interior gateway protocols (such as RIP's hop-count limits/slow convergence and OSPF's susceptibility to link-state poisoning)[cite: 449, 453, 459]. 

AHRP introduces two core elements:
1. [cite_start]**Dynamic Cost Evaluation:** Routes are prioritized using a weighted combination of real-time latency and available bandwidth[cite: 491, 496].
2. [cite_start]**Cryptographic Control Planes:** All Link State Updates (LSUs) are cryptographically signed using an HMAC-SHA256 signature scheme to eliminate unauthorized route injection[cite: 500, 556].

---

## 📑 Core Principles & Architecture

### 1. Dynamic Metric Equation
[cite_start]Instead of basic hop counting [cite: 541][cite_start], the link cost ($Cost_{ij}$) between two nodes $R_i$ and $R_j$ is calculated dynamically using normalized values[cite: 497, 499]:

$$Cost_{ij} = w_1 \cdot \frac{L_{ij}}{50} + w_2 \cdot \frac{100}{B_{ij} + 1}$$

* [cite_start]$L_{ij}$: Link Latency in milliseconds[cite: 497].
* [cite_start]$B_{ij}$: Link Bandwidth in Mbps[cite: 498].
* $w_1 / w_2$: Tunable importance metrics. (The baseline simulation operates at a stable **60/40 split** where $w_1 = 0.6$ and $w_2 = 0.4$) [cite_start][cite: 528, 551].

### 2. Message Verification Flow Chart
