#!/usr/bin/env python3
"""555 astable frequency helper for the beeper.

Forward:  given R1, R2, C1 -> oscillation frequency.
Solve:    given a target frequency, R1, C1 -> the R2 you need.

Astable frequency:  f = 1.44 / ((R1 + 2*R2) * C1)

Usage:
    python3 beep_frequency.py --r1 10000 --r2 18000 --c1 10e-9
    python3 beep_frequency.py --solve-r2 2700 --r1 10000 --c1 10e-9
"""
import argparse


def freq(r1, r2, c1):
    return 1.44 / ((r1 + 2 * r2) * c1)


def solve_r2(f, r1, c1):
    # f = 1.44 / ((R1 + 2 R2) C1)  ->  R2 = 1.44/(f*C1) - R1
    return 1.44 / (f * c1) - r1


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--r1", type=float, default=10000.0, help="R1 in ohms (default 10k)")
    ap.add_argument("--r2", type=float, help="R2 in ohms (forward mode)")
    ap.add_argument("--c1", type=float, default=10e-9, help="C1 in farads (default 10nF)")
    ap.add_argument("--solve-r2", type=float, metavar="HZ",
                    help="target frequency in Hz; prints the R2 needed")
    args = ap.parse_args()

    if args.solve_r2 is not None:
        r2 = solve_r2(args.solve_r2, args.r1, args.c1)
        print(f"target f   : {args.solve_r2:.0f} Hz")
        print(f"=> R2      : {r2:.0f} ohm")
        print(f"check f    : {freq(args.r1, r2, args.c1):.0f} Hz")
    elif args.r2 is not None:
        f = freq(args.r1, args.r2, args.c1)
        print(f"R1={args.r1:.0f}  R2={args.r2:.0f}  C1={args.c1*1e9:.1f}nF")
        print(f"=> f = {f:.0f} Hz")
        if not (2000 <= f <= 4500):
            print("note: typical 27mm piezo discs resonate ~2.7-4kHz; retune for loudness")
    else:
        ap.error("pass either --r2 (forward) or --solve-r2 HZ (solve)")


if __name__ == "__main__":
    main()
