import matplotlib
matplotlib.use("Agg")

import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

BG = "#0F1117"
GRID = "#222"
WHITE = "#fff"
SUB = "#aaa"
BLUE = "#4F8EF7"

# -----------------------------
# UPDATE THESE VALUES
# -----------------------------

python_re = 414
grep_ms = 70
ripgrep_ms = 48

logfire_api = 151
logfire_cli = 93

throughput = {
    "Python":107,
    "grep":631,
    "ripgrep":925,
    "API":293,
    "CLI":476
}

pipeline = {
    "mmap":7,
    "scan+filter":58,
    "serialize":28
}

timeline = [
    ("baseline",136),
    ("serializer",72),
    ("escape opt",55),
    ("current",42)
]

improvement = round(((136-93)/136)*100)

# -----------------------------

fig = plt.figure(
    figsize=(18,12),
    facecolor=BG
)

gs = gridspec.GridSpec(
    3,2,
    hspace=.55,
    wspace=.35
)

ax1 = fig.add_subplot(gs[0,0])
ax2 = fig.add_subplot(gs[0,1])
ax3 = fig.add_subplot(gs[1,:])
ax4 = fig.add_subplot(gs[2,:])

for ax in [ax1,ax2,ax3,ax4]:
    ax.set_facecolor(BG)
    ax.grid(True,color=GRID)
    ax.tick_params(colors=WHITE)

    for s in ax.spines.values():
        s.set_color("#333")

# ===================================================
# LATENCY
# ===================================================

labels = [
"Python re",
"ripgrep",
"grep",
"Python API",
"C++ Engine"
]

vals = [
python_re,
ripgrep_ms,
grep_ms,
logfire_api,
logfire_cli
]

colors = [
"#888",
"#aaa",
"#ccc",
"#7aa6ff",
BLUE
]

bars = ax1.bar(labels,vals,color=colors)

ax1.set_title(
"Query Latency\n44.3MB • median(10) • warm cache",
color=WHITE,
fontweight="bold"
)

ax1.set_ylabel("Latency (ms) ↓",color=SUB)

for b,v in zip(bars,vals):
    ax1.text(
        b.get_x()+b.get_width()/2,
        v+6,
        f"{v}",
        ha="center",
        color=WHITE
    )

# ===================================================
# THROUGHPUT
# ===================================================

labs = list(throughput.keys())
vals = list(throughput.values())

bars = ax2.bar(
labs,
vals,
color=["#888","#bbb","#ddd","#7aa6ff",BLUE]
)

ax2.set_title(
"Throughput\nHigher is better",
color=WHITE,
fontweight="bold"
)

ax2.set_ylabel("MB/s ↑",color=SUB)

for b,v in zip(bars,vals):
    ax2.text(
        b.get_x()+b.get_width()/2,
        v+12,
        str(v),
        ha="center",
        color=WHITE
    )

# ===================================================
# PIPELINE BREAKDOWN
# ===================================================

parts = list(pipeline.keys())
vals = list(pipeline.values())

ax3.barh(
["C++ Engine"],
[vals[0]],
color="#55ee99",
label="mmap"
)

left = vals[0]

for name,val,col in zip(
parts[1:],
vals[1:],
["#6fa8ff",BLUE]
):
    ax3.barh(
        ["C++ Engine"],
        [val],
        left=left,
        color=col,
        label=name
    )

    ax3.text(
        left + val/2,
        0,
        f"{name}\n{val} ms",
        ha="center",
        va="center",
        color=WHITE,
        fontsize=11,
        fontweight="bold"
    )

    left += val

ax3.text(
vals[0]/2,
0,
f"mmap\n{vals[0]} ms",
ha="center",
va="center",
color="black",
fontweight="bold"
)

ax3.legend(
facecolor=BG,
labelcolor=WHITE
)

ax3.set_title(
"Pipeline Breakdown (current median)",
color=WHITE,
fontweight="bold"
)

# ===================================================
# OPTIMIZATION TIMELINE
# ===================================================

steps=[x[0] for x in timeline]
times=[x[1] for x in timeline]

ax4.plot(
steps,
times,
marker="o",
linewidth=3
)

for x,y in zip(steps,times):
    ax4.text(
        x,
        y+3,
        str(y),
        color=WHITE,
        ha="center"
    )

ax4.set_title(
"Optimization Journey",
color=WHITE,
fontweight="bold"
)

ax4.set_ylabel(
"Latency (ms) ↓",
color=SUB
)

ax4.text(
1.4,
118,
f"{improvement}% lower latency",
fontsize=16,
fontweight="bold",
color=BLUE
)

# ===================================================
# GLOBAL TITLES
# ===================================================

fig.suptitle(
"Logfire — Performance Dashboard",
fontsize=24,
fontweight="bold",
color=WHITE
)

fig.text(
0.5,
0.92,
"32% lower latency • ~476 MB/s throughput • ~4.5× faster than Python re",
ha="center",
fontsize=13,
fontweight="bold",
color=BLUE
)

fig.text(
0.5,
0.02,
"Ryzen 5600H • WSL2 • GCC15 • -O3 • -march=native • median benchmarks • warm cache",
ha="center",
color="#666"
)

plt.savefig(
"benchmark_dashboard.png",
dpi=240,
bbox_inches="tight",
facecolor=BG
)

print("saved benchmark_dashboard.png")