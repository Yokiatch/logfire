import time
import re
import sys
import os
import subprocess
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build'))

SMALL_LOG = os.path.expanduser("~/bench.log")        # 44MB  / 1M lines
LARGE_LOG = os.path.expanduser("~/large_bench.log")  # 561MB / 10M lines
PATTERN   = "ERROR"
RUNS      = 5

def median_ms(times):
    return round(sorted(times)[len(times) // 2] * 1000, 1)

def mb_per_sec(ms, path):
    size_mb = os.path.getsize(path) / 1e6
    return round(size_mb / (ms / 1000), 1)

def warm(path):
    with open(path, 'rb') as f:
        f.read()

def run_logfire(path, limit=0):
    import _logfire, json
    times = []
    for _ in range(RUNS):
        t0 = time.perf_counter()
        result = json.loads(_logfire.query_file(
            path, pattern=PATTERN, limit=limit))
        times.append(time.perf_counter() - t0)
    return median_ms(times), len(result)

def run_python(path):
    pattern = re.compile(PATTERN)
    times = []
    count = 0
    for _ in range(RUNS):
        t0 = time.perf_counter()
        results = [l.strip() for l in open(path) if pattern.search(l)]
        count = len(results)
        times.append(time.perf_counter() - t0)
    return median_ms(times), count

def run_grep(path):
    times = []
    for _ in range(RUNS):
        t0 = time.perf_counter()
        r  = subprocess.run(["grep", "-c", PATTERN, path],
                            capture_output=True, text=True)
        times.append(time.perf_counter() - t0)
    return median_ms(times), int(r.stdout.strip())

def run_ripgrep(path):
    times = []
    for _ in range(RUNS):
        t0 = time.perf_counter()
        r  = subprocess.run(["rg", "-c", PATTERN, path],
                            capture_output=True, text=True)
        times.append(time.perf_counter() - t0)
    return median_ms(times), int(r.stdout.strip())

# ── run benchmarks ────────────────────────────────────────────────────────────
print("Warming caches...")
warm(SMALL_LOG)
warm(LARGE_LOG)

print("\n── Small file (44MB / 1M lines) ──")
print("  [1/4] logfire...")
lf_sm,  _ = run_logfire(SMALL_LOG)
print("  [2/4] Python re...")
py_sm,  _ = run_python(SMALL_LOG)
print("  [3/4] grep...")
gr_sm,  _ = run_grep(SMALL_LOG)
print("  [4/4] ripgrep...")
rg_sm,  _ = run_ripgrep(SMALL_LOG)

print("\n── Large file (561MB / 10M lines) — limit=1000 ──")
print("  [1/3] logfire limit=1000...")
lf_lg_1k,  _ = run_logfire(LARGE_LOG, limit=1000)
print("  [2/3] logfire limit=10000...")
lf_lg_10k, _ = run_logfire(LARGE_LOG, limit=10000)
print("  [3/3] logfire limit=100000...")
lf_lg_100k,_ = run_logfire(LARGE_LOG, limit=100000)

small_mb = round(os.path.getsize(SMALL_LOG) / 1e6, 1)
large_mb = round(os.path.getsize(LARGE_LOG) / 1e6, 1)

print(f"\n{'─'*50}")
print(f"Small file ({small_mb} MB):")
print(f"  logfire   : {lf_sm} ms  |  {mb_per_sec(lf_sm, SMALL_LOG)} MB/s")
print(f"  Python re : {py_sm} ms  |  {mb_per_sec(py_sm, SMALL_LOG)} MB/s")
print(f"  grep      : {gr_sm} ms  |  {mb_per_sec(gr_sm, SMALL_LOG)} MB/s")
print(f"  ripgrep   : {rg_sm} ms  |  {mb_per_sec(rg_sm, SMALL_LOG)} MB/s")
print(f"\nLarge file ({large_mb} MB) — logfire with early exit:")
print(f"  limit=1k   : {lf_lg_1k} ms")
print(f"  limit=10k  : {lf_lg_10k} ms")
print(f"  limit=100k : {lf_lg_100k} ms")

# ── colours ───────────────────────────────────────────────────────────────────
BLUE    = "#4F8EF7"
GRAY1   = "#888888"
GRAY2   = "#AAAAAA"
GRAY3   = "#CCCCCC"
BG      = "#0F1117"
GRID    = "#222222"
WHITE   = "#FFFFFF"
SUBTEXT = "#AAAAAA"

# ── figure ────────────────────────────────────────────────────────────────────
fig = plt.figure(figsize=(16, 10), facecolor=BG)
gs  = gridspec.GridSpec(2, 2, figure=fig,
                        hspace=0.45, wspace=0.3,
                        left=0.07, right=0.97,
                        top=0.88, bottom=0.10)

ax1 = fig.add_subplot(gs[0, 0])  # small file latency
ax2 = fig.add_subplot(gs[0, 1])  # small file throughput
ax3 = fig.add_subplot(gs[1, :])  # large file early exit

for ax in (ax1, ax2, ax3):
    ax.set_facecolor(BG)
    ax.tick_params(colors=WHITE, labelsize=10)
    for spine in ax.spines.values():
        spine.set_edgecolor("#333333")
    ax.yaxis.grid(True, color=GRID, linewidth=0.6, zorder=0)
    ax.set_axisbelow(True)

def label_bars(ax, bars, vals, suffix="", color=WHITE):
    ymax = max(vals)
    for bar, val in zip(bars, vals):
        ax.text(bar.get_x() + bar.get_width() / 2,
                bar.get_height() + ymax * 0.025,
                f"{val}{suffix}",
                ha="center", va="bottom",
                fontsize=9.5, color=color, fontweight="bold")

# ── Chart 1: small file latency ───────────────────────────────────────────────
tools_sm   = ["Python\nre", "ripgrep", "grep", "logfire"]
times_sm   = [py_sm, rg_sm, gr_sm, lf_sm]
colors_sm  = [GRAY1, GRAY2, GRAY3, BLUE]
x_sm       = np.arange(len(tools_sm))

bars1 = ax1.bar(x_sm, times_sm, width=0.5, color=colors_sm,
                edgecolor="#222222", linewidth=0.8, zorder=3)
ax1.set_xticks(x_sm)
ax1.set_xticklabels(tools_sm, color=WHITE, fontsize=10)
ax1.set_ylabel("Time (ms) — lower is better", color=SUBTEXT, fontsize=10)
ax1.set_title(f"Query Latency  ({small_mb} MB / 1M lines)",
              color=WHITE, fontsize=12, fontweight="bold", pad=10)
label_bars(ax1, bars1, times_sm, " ms")
ax1.get_xticklabels()[3].set_color(BLUE)
ax1.get_xticklabels()[3].set_fontweight("bold")

# speedup annotation
speedup = round(py_sm / lf_sm, 1)
ax1.annotate(f"{speedup}× faster\nthan Python",
             xy=(x_sm[3], lf_sm),
             xytext=(x_sm[3] - 0.6, lf_sm + max(times_sm) * 0.25),
             color=BLUE, fontsize=9, fontweight="bold",
             arrowprops=dict(arrowstyle="->", color=BLUE, lw=1.5))

# footnote
ax1.text(0.5, -0.18,
         "grep/ripgrep stream to stdout — logfire returns structured JSON",
         transform=ax1.transAxes, ha="center", fontsize=8, color="#666666")

# ── Chart 2: small file throughput ───────────────────────────────────────────
tput_sm = [mb_per_sec(t, SMALL_LOG) for t in times_sm]
bars2 = ax2.bar(x_sm, tput_sm, width=0.5, color=colors_sm,
                edgecolor="#222222", linewidth=0.8, zorder=3)
ax2.set_xticks(x_sm)
ax2.set_xticklabels(tools_sm, color=WHITE, fontsize=10)
ax2.set_ylabel("MB/s — higher is better", color=SUBTEXT, fontsize=10)
ax2.set_title(f"Throughput  ({small_mb} MB / 1M lines)",
              color=WHITE, fontsize=12, fontweight="bold", pad=10)
label_bars(ax2, bars2, tput_sm)
ax2.get_xticklabels()[3].set_color(BLUE)
ax2.get_xticklabels()[3].set_fontweight("bold")

# ── Chart 3: large file early exit ───────────────────────────────────────────
limits     = ["limit=1,000", "limit=10,000", "limit=100,000"]
times_lg   = [lf_lg_1k, lf_lg_10k, lf_lg_100k]
tput_lg    = [mb_per_sec(t, LARGE_LOG) for t in times_lg]
x_lg       = np.arange(len(limits))
blues      = ["#2E6FD4", "#3F7EE8", BLUE]

bars3 = ax3.bar(x_lg, times_lg, width=0.35, color=blues,
                edgecolor="#222222", linewidth=0.8, zorder=3, label="Time (ms)")

ax3.set_xticks(x_lg)
ax3.set_xticklabels(limits, color=WHITE, fontsize=11)
ax3.set_ylabel("Time (ms) — lower is better", color=SUBTEXT, fontsize=10)
ax3.set_title(
    f"Early Exit Performance  ({large_mb} MB / 10M lines)  —  "
    f"single-pass stops scanning once limit is reached",
    color=WHITE, fontsize=12, fontweight="bold", pad=10)

for bar, t_ms, tput in zip(bars3, times_lg, tput_lg):
    ax3.text(bar.get_x() + bar.get_width() / 2,
             bar.get_height() + max(times_lg) * 0.025,
             f"{t_ms} ms\n{tput:,} MB/s",
             ha="center", va="bottom",
             fontsize=10, color=WHITE, fontweight="bold")

ax3.text(0.5, -0.12,
         f"mmap maps full {large_mb} MB file — single-pass scans only pages needed to satisfy limit",
         transform=ax3.transAxes, ha="center", fontsize=9, color="#666666")

# ── title + footer ────────────────────────────────────────────────────────────
fig.suptitle("logfire  —  High-Performance Log Analytics Engine",
             fontsize=17, fontweight="bold", color=WHITE, y=0.97)

fig.text(0.5, 0.02,
         f"Benchmark: pattern='{PATTERN}'  •  median of {RUNS} runs  •  "
         f"warm cache  •  Ubuntu WSL2  •  Ryzen 5600H  •  logfire v0.1.0",
         ha="center", fontsize=8.5, color="#555555")

out = os.path.join(os.path.dirname(__file__), "benchmark.png")
plt.savefig(out, dpi=180, bbox_inches="tight", facecolor=BG)
print(f"\nSaved → {out}")
