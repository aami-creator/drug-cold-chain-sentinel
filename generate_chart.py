# ============================================================
#   DRUG COLD-CHAIN SENTINEL
#   Chain-of-Custody Log Visualizer
#   IEEE International MYOSA Event 5.0 — Team Amrut
# ============================================================
#
#   HOW TO USE:
#   1. Connect your laptop to "ColdChainSentinel" Wi-Fi
#   2. Open Command Prompt (Windows) or Terminal (Mac)
#   3. Run: pip install matplotlib requests
#   4. Run: python generate_chart.py
#   5. Chart image saved as "ColdChain_Report.png"
#
#   NO INTERNET NEEDED — reads directly from your board!
# ============================================================

import json
import requests
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
import sys

# ── SETTINGS ────────────────────────────────────────────────
BOARD_IP    = "http://192.168.4.1"
LOG_URL     = f"{BOARD_IP}/log"
DATA_URL    = f"{BOARD_IP}/data"
OUTPUT_FILE = "ColdChain_Report.png"

# Colors for each event type
EVENT_COLORS = {
    "TEMP_ALERT":     "#FF6B6B",
    "HUMIDITY_ALERT": "#4ECDC4",
    "SHOCK_ALERT":    "#FF9F43",
    "TAMPER_ALERT":   "#FFD93D",
    "PRESSURE_ALERT": "#A29BFE",
    "SYSTEM_START":   "#55EFC4",
    "LOG_CLEARED":    "#B2BEC3",
}

# ── STEP 1: FETCH DATA FROM BOARD ───────────────────────────
print("=" * 55)
print("  DRUG COLD-CHAIN SENTINEL — Log Visualizer")
print("  Team Amrut | GEC Thrissur | IEEE MYOSA 5.0")
print("=" * 55)
print()

print("Connecting to board at 192.168.4.1...")
print("Make sure your laptop is connected to 'ColdChainSentinel' Wi-Fi!")
print()

# Fetch log file
try:
    response = requests.get(LOG_URL, timeout=5)
    raw_log = response.text.strip()
    print("✅ Connected! Downloaded log from board.")
except Exception as e:
    print("❌ Could not connect to board!")
    print(f"   Error: {e}")
    print()
    print("   Make sure:")
    print("   1. Board is powered on")
    print("   2. Laptop is connected to 'ColdChainSentinel' Wi-Fi")
    print("   3. Try opening 192.168.4.1 in browser first")
    sys.exit(1)

# Fetch live sensor data
try:
    live = requests.get(DATA_URL, timeout=5).json()
except:
    live = {}

# ── STEP 2: PARSE LOG ENTRIES ───────────────────────────────
entries = []

for line in raw_log.split('\n'):
    line = line.strip()

    if not line:
        continue

    try:
        entry = json.loads(line)
        entries.append(entry)
    except:
        continue

print(f"✅ Parsed {len(entries)} log entries.")
print()

if not entries:
    print("⚠️ No log entries found!")
    sys.exit(1)

# ── STEP 3: EXTRACT DATA ────────────────────────────────────

def ts_to_seconds(ts):
    try:
        parts = ts.split(":")
        return int(parts[0]) * 3600 + int(parts[1]) * 60 + int(parts[2])
    except:
        return 0

times   = [ts_to_seconds(e.get("t", "0:0:0")) for e in entries]
events  = [e.get("e", "UNKNOWN") for e in entries]
details = [e.get("d", "") for e in entries]
values  = [float(e.get("v", 0)) for e in entries]

# Temperature data
temp_times = [
    times[i] for i, e in enumerate(events)
    if "TEMP" in e
]

temp_values = [
    values[i] for i, e in enumerate(events)
    if "TEMP" in e
]

# Shock data
shock_times = [
    times[i] for i, e in enumerate(events)
    if "SHOCK" in e
]

shock_values = [
    values[i] for i, e in enumerate(events)
    if "SHOCK" in e
]

# Event counts
event_counts = {}

for e in events:
    event_counts[e] = event_counts.get(e, 0) + 1

# ── STEP 4: CREATE FIGURE ───────────────────────────────────
fig = plt.figure(figsize=(14, 10), facecolor='#0D1117')

fig.suptitle(
    "DRUG COLD-CHAIN SENTINEL — Digital Chain-of-Custody Report",
    fontsize=16,
    fontweight='bold',
    color='white',
    y=0.98
)

plt.figtext(
    0.5,
    0.955,
    "IEEE MYOSA Event 5.0  |  Team Amrut  |  GEC Thrissur, Kerala",
    ha='center',
    fontsize=10,
    color='#14B8A6'
)

gs = GridSpec(
    3,
    2,
    figure=fig,
    hspace=0.45,
    wspace=0.35,
    top=0.92,
    bottom=0.08,
    left=0.08,
    right=0.95
)

# ── CHART 1: TEMPERATURE TIMELINE ───────────────────────────
ax1 = fig.add_subplot(gs[0, :])

ax1.set_facecolor('#161B22')

for spine in ax1.spines.values():
    spine.set_color('#30363D')

ax1.tick_params(colors='#8B949E')

if temp_times and temp_values:

    ax1.plot(
        temp_times,
        temp_values,
        'o-',
        color='#FF6B6B',
        linewidth=2,
        markersize=6
    )

    # Safe zone
    ax1.axhspan(
        2,
        8,
        alpha=0.15,
        color='#00B894',
        label='Safe zone (2–8°C)'
    )

    ax1.axhline(
        y=8,
        color='#FF6B6B',
        linestyle='--',
        alpha=0.6
    )

    ax1.axhline(
        y=2,
        color='#74B9FF',
        linestyle='--',
        alpha=0.6
    )

    # Above safe range
    ax1.fill_between(
        temp_times,
        temp_values,
        8,
        where=[v > 8 for v in temp_values],
        alpha=0.3,
        color='#FF6B6B'
    )

    # Below safe range
    ax1.fill_between(
        temp_times,
        temp_values,
        2,
        where=[v < 2 for v in temp_values],
        alpha=0.3,
        color='#74B9FF'
    )

    ax1.legend(
        facecolor='#161B22',
        labelcolor='white',
        fontsize=8
    )

else:
    ax1.text(
        0.5,
        0.5,
        'No temperature data yet',
        transform=ax1.transAxes,
        ha='center',
        va='center',
        color='#8B949E',
        fontsize=12
    )

ax1.set_title(
    'Temperature Timeline',
    color='white',
    fontsize=12
)

ax1.set_xlabel(
    'Time (seconds)',
    color='#8B949E'
)

ax1.set_ylabel(
    'Temperature (°C)',
    color='#8B949E'
)

# ── CHART 2: EVENT TIMELINE ─────────────────────────────────
ax2 = fig.add_subplot(gs[1, 0])

ax2.set_facecolor('#161B22')

for spine in ax2.spines.values():
    spine.set_color('#30363D')

ax2.tick_params(colors='#8B949E', labelsize=7)

unique_events = list(set(events))
event_y = {e: i for i, e in enumerate(unique_events)}

for t, e in zip(times, events):

    color = EVENT_COLORS.get(e, '#FFFFFF')

    ax2.scatter(
        t,
        event_y[e],
        color=color,
        s=80,
        zorder=3
    )

    ax2.axvline(
        x=t,
        color=color,
        alpha=0.2,
        linewidth=0.8
    )

ax2.set_yticks(range(len(unique_events)))
ax2.set_yticklabels(unique_events, color='#8B949E')

ax2.set_title(
    'Event Timeline',
    color='white',
    fontsize=11
)

ax2.set_xlabel(
    'Time (seconds)',
    color='#8B949E'
)

# ── CHART 3: EVENT COUNT SUMMARY ────────────────────────────
ax3 = fig.add_subplot(gs[1, 1])

ax3.set_facecolor('#161B22')

for spine in ax3.spines.values():
    spine.set_color('#30363D')

ax3.tick_params(colors='#8B949E', labelsize=7)

bar_labels = list(event_counts.keys())
bar_values = list(event_counts.values())
bar_colors = [
    EVENT_COLORS.get(e, '#FFFFFF')
    for e in bar_labels
]

bars = ax3.barh(
    bar_labels,
    bar_values,
    color=bar_colors,
    alpha=0.85
)

ax3.set_title(
    'Event Count Summary',
    color='white',
    fontsize=11
)

ax3.set_xlabel(
    'Count',
    color='#8B949E'
)

for bar, val in zip(bars, bar_values):

    ax3.text(
        bar.get_width() + 0.1,
        bar.get_y() + bar.get_height() / 2,
        str(val),
        va='center',
        color='white',
        fontsize=9,
        fontweight='bold'
    )

# ── CHART 4: SHOCK EVENTS ───────────────────────────────────
ax4 = fig.add_subplot(gs[2, 0])

ax4.set_facecolor('#161B22')

for spine in ax4.spines.values():
    spine.set_color('#30363D')

ax4.tick_params(colors='#8B949E', labelsize=7)

if shock_times:

    width = max(times) / 50 if max(times) > 0 else 1

    ax4.bar(
        shock_times,
        shock_values,
        color='#FF9F43',
        alpha=0.8,
        width=width
    )

    ax4.axhline(
        y=7,
        color='#FF6B6B',
        linestyle='--',
        alpha=0.7,
        label='Alert threshold'
    )

    ax4.legend(
        facecolor='#161B22',
        labelcolor='white',
        fontsize=7
    )

else:

    ax4.text(
        0.5,
        0.5,
        'No shock events recorded',
        transform=ax4.transAxes,
        ha='center',
        va='center',
        color='#8B949E',
        fontsize=10
    )

ax4.set_title(
    'Shock Events',
    color='white',
    fontsize=11
)

ax4.set_xlabel(
    'Time (seconds)',
    color='#8B949E'
)

ax4.set_ylabel(
    'Acceleration (m/s²)',
    color='#8B949E'
)

# ── CHART 5: LIVE SUMMARY ───────────────────────────────────
ax5 = fig.add_subplot(gs[2, 1])

ax5.set_facecolor('#161B22')

for spine in ax5.spines.values():
    spine.set_color('#30363D')

ax5.axis('off')

# Stats
total_events   = len(entries)
tamper_count   = sum(1 for e in events if "TAMPER" in e)
shock_count    = sum(1 for e in events if "SHOCK" in e)
temp_count     = sum(1 for e in events if "TEMP" in e)
pressure_count = sum(1 for e in events if "PRESSURE" in e)

uptime  = live.get("uptime", "N/A")
pressure = live.get("pressure", "N/A")

stats = [
    ("Total Events Logged", str(total_events), "#FFFFFF"),
    ("Temperature Alerts", str(temp_count), "#FF6B6B"),
    ("Shock Alerts", str(shock_count), "#FF9F43"),
    ("Tamper Events", str(tamper_count), "#FFD93D"),
    ("Pressure Alerts", str(pressure_count), "#A29BFE"),
    ("Current Pressure", f"{pressure} hPa", "#74B9FF"),
    ("Device Uptime", str(uptime), "#55EFC4"),
]

ax5.text(
    0.5,
    0.97,
    "Live Device Summary",
    transform=ax5.transAxes,
    ha='center',
    va='top',
    color='white',
    fontsize=11,
    fontweight='bold'
)

for idx, (label, value, color) in enumerate(stats):

    y = 0.82 - idx * 0.12

    ax5.text(
        0.05,
        y,
        label + ":",
        transform=ax5.transAxes,
        color='#8B949E',
        fontsize=9,
        va='center'
    )

    ax5.text(
        0.95,
        y,
        value,
        transform=ax5.transAxes,
        color=color,
        fontsize=10,
        fontweight='bold',
        va='center',
        ha='right'
    )

    # Divider line (FIXED)
    ax5.plot(
        [0.03, 0.97],
        [y - 0.045, y - 0.045],
        color='#30363D',
        linewidth=0.5,
        transform=ax5.transAxes
    )

# ── FOOTER ──────────────────────────────────────────────────
plt.figtext(
    0.5,
    0.02,
    "Generated automatically from on-device SPIFFS log  |  "
    "Zero internet required  |  Complete digital chain-of-custody passport",
    ha='center',
    fontsize=8,
    color='#444C56'
)

# ── SAVE FIGURE ─────────────────────────────────────────────
plt.savefig(
    OUTPUT_FILE,
    dpi=150,
    bbox_inches='tight',
    facecolor='#0D1117',
    edgecolor='none'
)

print(f"✅ Chart saved as: {OUTPUT_FILE}")
print()

print("📊 Summary:")
print(f"   Total events logged : {total_events}")
print(f"   Temperature alerts  : {temp_count}")
print(f"   Shock alerts        : {shock_count}")
print(f"   Tamper events       : {tamper_count}")
print(f"   Current pressure    : {pressure} hPa")

print()
print(f"🎉 Open '{OUTPUT_FILE}' to see your digital passport chart!")
print("   Show this in Step 5 of your demo video!")
print()

print("=" * 55)