import glob
import pandas as pd
import matplotlib.pyplot as plt
csv_files = glob.glob('*.csv')

for file in csv_files:
    df = pd.read_csv(file, header=None)
    values = df.iloc[:, 0]
    frequencies = df.iloc[:, 1]
    non_zero_count = (frequencies != 0).sum()
    plt.figure(figsize=(10, 6))
    plt.bar(values, frequencies)
    plt.title(f'Histogram for {file}')
    plt.xlabel('Value')
    plt.ylabel('Frequency')
    plt.yscale('log')
    plt.text(
        0.95, 0.95, f'NonZero entries: {non_zero_count}',
        transform=plt.gca().transAxes,
        ha='right', va='top', fontsize=12, bbox=dict(boxstyle='round', facecolor='white', alpha=0.8)
    )
    plt.show()