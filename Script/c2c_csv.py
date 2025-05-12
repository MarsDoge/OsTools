import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import argparse
import os

def load_data(filename):
    m = np.array(pd.read_csv(filename, header=None))
    m = np.tril(m) + np.tril(m).T - np.diag(np.diag(m))  # 构造对称矩阵
    median = np.nanmedian(m)
    #np.fill_diagonal(m, median)  # 填补对角线
    np.fill_diagonal(m, 0)  # 填补对角线
    m = np.where(np.isnan(m), median, m)  # 替换残余 NaN
    return m

def show_heapmap(m, title=None, subtitle=None, vmin=None, vmax=None, yticks=True, figsize=None):
    vmin = np.nanmin(m) if vmin is None else vmin
    vmax = np.nanmax(m) if vmax is None else vmax
    black_at = (vmin + 3*vmax) / 4
    subtitle = "Core-to-core latency" if subtitle is None else subtitle
    
    isnan = np.isnan(m)

    plt.rcParams['xtick.bottom'] = plt.rcParams['xtick.labelbottom'] = False
    plt.rcParams['xtick.top'] = plt.rcParams['xtick.labeltop'] = True

    figsize = np.array(m.shape) * 0.3 + np.array([6, 1]) if figsize is None else figsize
    fig, ax = plt.subplots(figsize=figsize, dpi=130)
    fig.patch.set_facecolor('w')
    
    plt.imshow(np.full_like(m, 0.7), vmin=0, vmax=1, cmap='gray')  # 灰底
    plt.imshow(m, cmap='viridis', vmin=vmin, vmax=vmax)

    fontsize = 9 if vmax >= 100 else 10
    for (i, j) in np.ndindex(m.shape):
        t = "" if isnan[i,j] else f"{m[i,j]:.1f}" if vmax < 10.0 else f"{m[i,j]:.0f}"
        c = "w" if m[i,j] < black_at else "k"
        plt.text(j, i, t, ha="center", va="center", color=c, fontsize=fontsize)

    plt.xticks(np.arange(m.shape[1]), labels=[f"{i+1}" for i in range(m.shape[1])], fontsize=9)
    if yticks:
        plt.yticks(np.arange(m.shape[0]), labels=[f"CPU {i+1}" for i in range(m.shape[0])], fontsize=9)
    else:
        plt.yticks([])

    plt.title(f"{title}\n{subtitle}\nMin={vmin:.1f}ns Median={np.nanmedian(m):.1f}ns Max={vmax:.1f}ns",
              fontsize=11, linespacing=1.5)
    plt.tight_layout()


def main():
    parser = argparse.ArgumentParser(description="Draw a heatmap from CSV")
    parser.add_argument("--input", "-i", required=True, help="Input CSV file path")
    parser.add_argument("--output", "-o", default="heatmap.png", help="Output PNG file name")
    parser.add_argument("--title", "-t", default="Loongson Heatmap", help="Heatmap title")
    args = parser.parse_args()

    if not os.path.exists(args.input):
        print(f"Error: File '{args.input}' does not exist.")
        return

    m = load_data(args.input)
    show_heapmap(m, title=args.title)
    plt.savefig(args.output, bbox_inches="tight", dpi=150)
    print(f"Saved heatmap to '{args.output}'")
    plt.close()

if __name__ == "__main__":
    main()

