import numpy as np

fichero_csv = open("results_latency.csv",'w+')
secuencial = 0
paralelo = 1
justo = 2

def get_data(file, mode, n_subs):
    latency = []
    with open(file, 'r') as f:
        for linea in f:
            try:
                latency.append(float(linea))
            except:
                print("ERROR", linea)
                break


    # Modo, N, min, max, avg, std
    data = str(mode) + ", " + str(n_subs) + ", " + str(np.max(latency)) + ", " + str(np.min(latency)) + ", " + str(np.mean(latency)) + ", " + str(np.std(latency)) + "\n"

    fichero_csv.write(data)

secuencial_50 = "secuencial_50.txt"
secuencial_500 = "secuencial_500.txt"
secuencial_900 = "secuencial_900.txt"
paralelo_50 = "paralelo_50.txt"
paralelo_500 = "paralelo_500.txt"
paralelo_900 = "paralelo_900.txt"
justo_50 = "justo_50.txt"
justo_500 = "justo_500.txt"
justo_900 = "justo_900.txt"

get_data(secuencial_50, secuencial, 50)
get_data(secuencial_500, secuencial, 500)

"""
get_data(secuencial_900, secuencial, 900)

get_data(paralelo_50, paralelo, 50)
get_data(paralelo_500, paralelo, 500)


get_data(paralelo_900, paralelo, 900)

get_data(justo_50, justo, 50)
get_data(justo_500, justo, 500)
get_data(justo_900, justo, 900)
"""