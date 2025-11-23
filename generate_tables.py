import pandas as pd
import numpy as np

THREADS = [1, 4, 8, 16, 28]

def generate_seq_time_table(input_csv: str, output_csv: str):
    df = pd.read_csv(input_csv)

    # Compute ratio: seq_mean / total_mean
    df["ratio"] = df["sequencial_mean"] / df["total_mean"]

    # Pivot to form the desired structure
    table = df.pivot_table(
        index="tam",
        columns="threads",
        values="ratio"
    )

    table = table.reindex(columns=THREADS)

    table = table.reset_index()

    table.to_latex(output_csv, index=False)

WRITE_EMPTY_FOR_NAN = False  # if True, output will contain '' instead of NaN

def generate_speedup_table(input_csv: str, output_csv: str):
    df = pd.read_csv(input_csv)

    if not {"tam", "threads", "total_mean"}.issubset(df.columns):
        raise ValueError("Input CSV missing required columns: tam, threads, total_mean")

    # pivot: rows = tam, cols = threads
    pivot = df.pivot_table(index="tam", columns="threads", values="total_mean", aggfunc="mean")

    # ensure thread columns exist (may introduce NaNs)
    pivot = pivot.reindex(columns=THREADS)

    missing_thread_cols = [t for t in THREADS if t not in pivot.columns or pivot[t].isna().all()]

    # Prepare result DataFrame with same index and THREADS columns
    result = pd.DataFrame(index=pivot.index, columns=THREADS, dtype=float)

    missing_base_tams = []
    for tam in pivot.index:
        base = pivot.at[tam, 1] if 1 in pivot.columns else np.nan
        if pd.isna(base):
            missing_base_tams.append(tam)
            # leave whole row as NaN (or fill with something else)
            continue

        # For each requested thread, compute base / value if value exists
        for t in THREADS:
            val = pivot.at[tam, t] if t in pivot.columns else np.nan
            if pd.isna(val):
                result.at[tam, t] = np.nan
            else:
                # protect against division by zero
                if val == 0:
                    result.at[tam, t] = np.nan
                else:
                    result.at[tam, t] = float(base) / float(val)

        out = result.reset_index().rename(columns={"index": "tam"})
    out = out.rename_axis(None, axis=1)

    if WRITE_EMPTY_FOR_NAN:
        out = out.fillna("")

    out.to_latex(output_csv, index=False)


def generate_amdahl_table(input_csv: str, output_csv: str):
    df = pd.read_csv(input_csv)

    required = {"tam", "threads", "total_mean", "sequencial_mean"}
    if not required.issubset(df.columns):
        raise ValueError(f"Input CSV must contain: {required}")

    # Pivot into tam × threads matrices
    total_pivot = df.pivot_table(
        index="tam",
        columns="threads",
        values="total_mean",
        aggfunc="mean"
    ).reindex(columns=THREADS)

    seq_pivot = df.pivot_table(
        index="tam",
        columns="threads",
        values="sequencial_mean",
        aggfunc="mean"
    ).reindex(columns=THREADS)

    # Sequential_mean for 1 thread
    seq1 = seq_pivot[1]

    # Output amdahl table
    amdahl_table = pd.DataFrame(index=total_pivot.index, columns=THREADS, dtype=float)

    for tam in total_pivot.index:
        seq_val = seq1.loc[tam]

        for t in THREADS:
            total_val = total_pivot.loc[tam, t]

            if pd.isna(seq_val) or pd.isna(total_val):
                amdahl_table.loc[tam, t] = float("nan")
                continue

            # Sequential fraction f = seq(1 thread) / total(t threads)
            f = seq_val / total_val

            # Amdahl speedup
            S = 1.0 / (f + (1 - f) / t)

            amdahl_table.loc[tam, t] = S

    amdahl_table = amdahl_table.reset_index()
    amdahl_table.to_latex(output_csv, index=False)

def generate_efficiency_table(input_csv: str, output_csv: str):
    df = pd.read_csv(input_csv)

    if not {"tam", "threads", "total_mean"}.issubset(df.columns):
        raise ValueError("Input CSV missing required columns: tam, threads, total_mean")

    pivot = df.pivot_table(
        index="tam",
        columns="threads",
        values="total_mean",
        aggfunc="mean"
    ).reindex(columns=THREADS)

    result = pd.DataFrame(index=pivot.index, columns=THREADS, dtype=float)

    missing_base_tams = []

    for tam in pivot.index:
        base = pivot.at[tam, 1] if 1 in pivot.columns else np.nan
        if pd.isna(base):
            missing_base_tams.append(tam)
            continue

        for t in THREADS:
            val = pivot.at[tam, t] if t in pivot.columns else np.nan
            if pd.isna(val) or val == 0:
                result.at[tam, t] = np.nan
            else:
                # NEW FORMULA:
                # base / (val * t)
                result.at[tam, t] = float(base) / float(val * t)

    out = result.reset_index().rename(columns={"index": "tam"})
    out = out.rename_axis(None, axis=1)

    if WRITE_EMPTY_FOR_NAN:
        out = out.fillna("")

    out.to_latex(output_csv, index=False)


if __name__ == "__main__":
    generate_seq_time_table("processed_results.csv", "tables/sequential_time_percentage.tex")
    generate_speedup_table("processed_results.csv", "tables/speedup_percentage.tex")
    generate_amdahl_table("processed_results.csv", "tables/amdahl.tex")
    generate_efficiency_table("processed_results.csv", "tables/efficiency.tex")


