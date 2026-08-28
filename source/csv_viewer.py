import argparse
import time
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

def plot_sample(df, idx, freq_axis, max_freq):
    record = df.iloc[idx].values
    
    plt.clf()  # Clear
    plt.plot(freq_axis, record, color='#007acc', linewidth=1.2)
    plt.xlim(0, max_freq)
    plt.ylim(-100, 0)
    plt.xlabel('Frequency (kHz)')
    plt.ylabel('Level (dB)')
    plt.title(f'Espectre - Sample #{idx} (of {len(df)-1})')
    plt.grid(True, which='both', linestyle='--', alpha=0.5)

def process_file(file_path, sample=None, max_freq=24):
    df = pd.read_csv(file_path, header=None)
    total_records = len(df)

    if total_records == 0:
        print("CSV file empty.")
        return

    # X axis (0 a max_freq kHz)
    num_puntos = len(df.iloc[0].values)
    freq_axis = np.linspace(0, max_freq, num_puntos)

    plt.figure(figsize=(10, 5))

    # If sample number provided
    if sample is not None:
        if sample < -total_records or sample >= total_records:
            print(f"Error: Sample {sample} out of range (0 - {total_records - 1}).")
            return
            
        idx = sample if sample >= 0 else total_records + sample
        print(f"Show sample: {idx} of {file_path}")
        
        plot_sample(df, idx, freq_axis, max_freq)
        plt.tight_layout()
        plt.show()

    else:
        print(f"Bucle mode: Show {total_records} records 1 per 2 segundos.")
        plt.ion()
        
        try:
            while True:
                for idx in range(total_records):
                    plot_sample(df, idx, freq_axis, max_freq)
                    plt.tight_layout()
                    plt.draw()
                    plt.pause(2)
        except (KeyboardInterrupt, plt.TerminatedWorkerError):
            print("\nStopped")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="CSV FFT viewer.")
    
    parser.add_argument("file", type=str, help="Filename/Filepath")
    
    # Parámetro opcional: número de sample/registro
    parser.add_argument("-s", "--sample", type=int, default=None, 
                        help="Sample to show. Shows all if omitted.")
    
    # Parámetro opcional: frecuencia máxima (24 o 48 kHz)
    parser.add_argument("-f", "--freq", type=int, choices=[24, 48], default=24,
                        help="Frecuencia máxima en kHz (24 o 48). Por defecto: 24.")
    
    args = parser.parse_args()

    process_file(args.file, sample=args.sample, max_freq=args.freq)
