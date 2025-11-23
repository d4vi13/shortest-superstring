import pandas as pd

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

if __name__ == "__main__":
    generate_speedup_table("processed_results.csv", "sequential_time_percentage.tex")

