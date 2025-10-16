import pandas as pd

threads = [16, 8, 4, 2, 1]
n_t = 0

columns = ["tam", "threads", "media_tempo", "dp_tempo", "media_sequencial", "dp_sequencial", "media_paralelo", "dp_paralelo"]
res = pd.DataFrame(columns=columns)

df = pd.read_csv("result")

for start in range(0, len(df), 20):
    end = start + 20
    block = df.iloc[start:end]

    new_row = [
            block["tam"].iloc[0],
            threads[n_t],
            block["total"].mean(),
            block["total"].std(),
            block["sequencial"].mean(),
            block["sequencial"].std(),
            block["paralelo"].mean(),
            block["paralelo"].std()
            ]

    new_row = pd.DataFrame([new_row], columns=columns)
    res = pd.concat([res, new_row], ignore_index=True)

    n_t = (n_t + 1) % len(threads)

res.to_csv("processed_result.csv", index=False)


