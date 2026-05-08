#!/usr/bin/env python3
"""Plot accelerometer peak per hit from a rig log, marking the failure point.

Optional helper (needs matplotlib). For a pass/fail number use parse_log.py instead.

Usage:
    python3 plot_log.py run.csv
    python3 plot_log.py --out run.png run.csv
"""
import argparse
import sys


def read_rows(path):
    rows = []
    with open(path) as f:
        for line in f:
            parts = line.strip().split(",")
            rows.append(tuple(int(p) for p in parts))
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("log")
    ap.add_argument("--out", help="save PNG here instead of showing a window")
    args = ap.parse_args()

    try:
        import matplotlib
        if args.out:
            matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed: pip install matplotlib", file=sys.stderr)
        sys.exit(2)

    rows = read_rows(args.log)
    hit_counts = [r[1] for r in rows]
    accel = [r[2] for r in rows]
    failure = next((r for r in rows if r[3] == 0), None)

    plt.figure(figsize=(9, 4))
    plt.plot(hit_counts, accel, lw=0.8, marker=".", ms=3)
    plt.xlabel("hit #")
    plt.ylabel("accel peak (ADC counts)")
    plt.title("Impact magnitude per hit")
    if failure:
        plt.axvline(failure[1], color="crimson", ls="--",
                    label=f"beep lost @ hit {failure[1]}")
        plt.legend()
    plt.tight_layout()

    if args.out:
        plt.savefig(args.out, dpi=120)
        print(f"wrote {args.out}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
