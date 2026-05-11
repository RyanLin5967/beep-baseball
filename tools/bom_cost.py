#!/usr/bin/env python3
"""Total the BOM and check it against the per-ball cost target.

Reads a bom.csv (columns include qty, unit_cost_usd) and sums qty*unit_cost.

Usage:
    python3 bom_cost.py ../hardware/bom.csv
    python3 bom_cost.py --target 6.00 ../hardware/bom.csv
"""
import argparse
import csv
import sys


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("bom", help="path to bom.csv")
    ap.add_argument("--target", type=float, default=6.00, help="per-ball cost target USD")
    args = ap.parse_args()

    total = 0.0
    n = 0
    with open(args.bom, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            try:
                qty = float(row["qty"])
                cost = float(row["unit_cost_usd"])
            except (KeyError, ValueError, TypeError):
                print(f"warning: skipping row with bad qty/cost: {row}", file=sys.stderr)
                continue
            total += qty * cost
            n += 1

    if n == 0:
        print("ERROR: no priced BOM rows parsed.", file=sys.stderr)
        sys.exit(2)

    print(f"priced lines : {n}")
    print(f"total cost   : ${total:.2f}")
    print(f"target       : ${args.target:.2f}")
    if total <= args.target:
        print(f"verdict      : OK (${args.target - total:.2f} under target)")
        sys.exit(0)
    else:
        print(f"verdict      : OVER by ${total - args.target:.2f}")
        sys.exit(1)


if __name__ == "__main__":
    main()
