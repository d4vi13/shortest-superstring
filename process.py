import pandas as pd

def process_multiple_csv(inputs, output_file, value_columns):
    df = pd.concat([pd.read_csv(path) for path in inputs], ignore_index=True)

    grouped = df.groupby(['tam', 'threads'])[value_columns].agg(['mean', 'std'])

    grouped.columns = [f"{col}_{stat}" for col, stat in grouped.columns]

    grouped.reset_index().to_csv(output_file, index=False)


if __name__ == "__main__":
    input_files = [
        "results/1.csv",
        "results/4.csv",
        "results/8.csv",
        "results/16.csv",
        "results/28.csv",
    ]

    metrics = ["total", "sequencial", "paralelo"]

    process_multiple_csv(input_files, "processed_results.csv", metrics)

