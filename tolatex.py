def latex_table_from_csv(input_file, output_file):
    import pandas as pd

    df = pd.read_csv(input_file)

    with open(output_file, "w") as f:
        f.write(df.to_latex(index=False))

latex_table_from_csv("processed_results.csv", "table.tex")

