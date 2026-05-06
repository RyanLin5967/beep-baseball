#!/usr/bin/env python3
"""Parse a rig CSV log and report hits-to-failure.

Data rows are: millis,hit_count,accel_peak,beeping
Failure = the first data row with beeping == 0.

Usage:
    python3 parse_log.py run.csv
    python3 parse_log.py --target 2000 run.csv
"""
import argparse
import sys


def parse(path):
    hits = []  # (millis, hit_count, accel_peak, beeping)
    with open(path, newline="") as f:
        for raw in f:
            line = raw.strip()
            if not line:
                continue
            parts = line.split(",")
            millis, hit_count, accel_peak, beeping = (int(p) for p in parts)
            hits.append((millis, hit_count, accel_peak, beeping))
    return hits


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("log", help="CSV log file from the rig")
    ap.add_argument("--target", type=int, default=2000, help="pass threshold in hits")
    args = ap.parse_args()

    rows = parse(args.log)
    total_hits = max(r[1] for r in rows)
    failure = next((r for r in rows if r[3] == 0), None)

    print(f"rows parsed : {len(rows)}")
    print(f"total hits  : {total_hits}")
    if failure is None:
        verdict = "PASS" if total_hits >= args.target else "INCONCLUSIVE"
        print(f"failure     : none recorded (still beeping at last row)")
        print(f"verdict     : {verdict} (target {args.target})")
        sys.exit(0 if verdict == "PASS" else 1)
    fail_hit = failure[1]
    verdict = "PASS" if fail_hit >= args.target else "FAIL"
    print(f"failure at  : hit {fail_hit} (t={failure[0]} ms)")
    print(f"verdict     : {verdict} (target {args.target})")
    sys.exit(0 if verdict == "PASS" else 1)


if __name__ == "__main__":
    main()
