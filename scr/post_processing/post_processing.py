import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv('../report.csv')

mpi = df[df['metodo'] == 'MPI']
openMP = df[df['metodo'] == 'OPEN_MP']
serial = df[df['metodo'] == 'SERIAL']
print(serial)
plt.plot(mpi['cores'], mpi['tempo_execucao'],'ro-', 
         openMP['cores'], openMP['tempo_execucao'], 'b^--', 
         serial['cores'], serial['tempo_execucao'], 'gs')
plt.legend(['MPI', 'OpenMP', 'Serial'])
plt.xlabel("Número de cores ")
plt.ylabel("Tempo de execução(s)")
plt.title("100 timesteps, Grid 1024x1024")
plt.savefig("tempos_de_Execucao.png") 

print(df)
plt.show()

