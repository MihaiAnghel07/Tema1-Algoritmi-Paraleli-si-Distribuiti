# Tema1-Algoritmi-Paraleli-si-Distribuiti
Paralleling a genetic algorithm to solve the knapsack problem
***************************
Nume: Anghel Mihai-Gabriel*
Grupa: 336CC		          *
***************************


****Functii si structuri folosite****
(cele create de mine, nu cred ca are sens sa detaliez functiile din skelet)

 - Structura input (input.h) -
Am folosit o structura 'input' pentru a parsa datele catre functia asupra
careia actioneaza thread-urile.

 - compute_start() - 
Calculeaza indexul de start pentru un thread

 - compute_end() - 
Calculeaza indexul de sfarsit pentru un thread

 - check() - 
Verifica daca thread-urile acopera toata aria unei parcurgeri secventiale

 - oets() - 
Functie de sortare paralela care inlocuieste qsort-ul din schelet



****Paralelizarea****
In run_genetic_algorithm, aplic paralelizarea pentru fiecare pas secvential
din algoritmul genetic, calculand capetele "start" si "end" pentru fiecare 
thread. In plus, am paralelizat si functia compute_fitness_function() si
functia de sortare oets().
Logica arata in felul urmator:
 - se calculeaza fittnes-ul fiecarui individ
 - apoi aplic o bariera pentru ca sortarea trebuie sa se faca pe baza 
   fitness-ului(deci toate thread-urile trebuie sa termine calcularea
   fittness-ului)
 - folosesc un flag static care ma ajuta sa interschimb generatiile doar 
   o data (doar de catre un thread) si fiind o zona de racecondition
   folosesc mutex(flag-ul l-am adaugat atunci cand facea sortarea cu qsort,
   insa nu am mai schimbat logica)
 - apoi thread-urile calculeaza selectia, mutatia si crossover-ul paralele
 - apoi interchimb generatiile, dar aplic o bariera pentru ca toate
   thread-urile sa termine functiile genetice
 - in final, dupa ce toate thread-urile au terminat, calculeaz dinnou 
   fitness-ul, sortez indivizii si fac afisarea.
 - in final dealoc memoria, tot in mod paralel.
 
 
 
 
 
