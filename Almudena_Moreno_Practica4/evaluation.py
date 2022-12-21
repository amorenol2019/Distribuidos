import numpy as np

fichero_csv = open("results_latency.csv",'w+')
secuencial = 0
paralelo = 1
justo = 2

def get_data(file, mode, n_subs):
    latency = []
    with open(file, 'r') as f:
        for linea in f:
            linea = linea.rstrip(".\n")
            print(linea)
            try:
                latency.append(int(linea))
            except:
                continue

    # Modo, N, min, max, avg, std
    data = str(mode) + ", " + str(n_subs) + ", " + str(np.max(latency)) + ", " + str(np.min(latency)) + ", " + str(np.mean(latency)) + ", " + str(np.std(latency)) + "\n"

    fichero_csv.write(data)

secuencial_50 = "secuencial_50_grep.txt"
secuencial_500 = "secuencial_500_grep.txt"
secuencial_900 = "secuencial_900_grep.txt"
paralelo_50 = "paralelo_50_grep.txt"
paralelo_500 = "paralelo_500_grep.txt"
paralelo_900 = "paralelo_900_grep.txt"
justo_50 = "justo_50_grep.txt"
justo_500 = "justo_500_grep.txt"
justo_900 = "justo_900_grep.txt"

get_data(secuencial_50, secuencial, 50)
get_data(secuencial_500, secuencial, 500)
get_data(secuencial_900, secuencial, 900)

get_data(paralelo_50, paralelo, 50)
get_data(paralelo_500, paralelo, 500)

"""
get_data(paralelo_900, paralelo, 900)

get_data(justo_50, justo, 50)
get_data(justo_500, justo, 500)
get_data(justo_900, justo, 900)
"""