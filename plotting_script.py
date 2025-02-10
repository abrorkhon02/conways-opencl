import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('simulation_results.csv')
plt.figure(figsize=(10,6))
plt.plot(df['Width'], df['Elapsed Time (s)'], marker='o')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('Grid Size (width=height)')
plt.ylabel('Time (seconds)')
plt.title('OpenCL Evolution Performance (n=1)')
plt.grid(True)
plt.savefig('performance_plot.png')
