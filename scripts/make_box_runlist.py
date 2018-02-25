
run_list = open("run_list", "w")



seed = 4001

# selection = "resource"
dist = ".1"
cost = "1"
tournament_size = "2"
mutation = ".05"
inflow = "100"
frac = ".0025"
max_res = "5"
recomb = "1"

run_list.write("set email dolsonem@msu.edu\nset freq_email Crash\nset description box\nset mem_request 2\nset walltime 10\nset config_dir config\nset dest_dir /mnt/home/dolsonem/eco-ea-box/final"+"\n\n")

for selection in ["resource","lexicase", "tournament"]:
    for good in range(11,-1,-1):
        for bad in range(11,-1,-1):
            if good + bad > 10:
                continue
            name = "box_" + str(good) + "g_" + str(bad) + "b_0n_" + selection
            line = " ".join([str(seed)+".."+str(seed+9), name, "./100DBox -N_GOOD", str(good), "-N_BAD", str(bad), "-DISTANCE_CUTOFF", dist, "-SELECTION", selection.upper(), "-SEED $seed -UPDATES 50000 -RESOURCE_INFLOW", inflow, "-COST", cost, "-TOURNAMENT_SIZE", tournament_size,
                             "-MUTATION_SIZE", mutation, "-POP_SIZE 5000 -FRAC", frac, "-MAX_RES_USE", max_res, "-RECOMBINATION", recomb])
            seed += 10
            run_list.write(line+"\n\n")

run_list.close()
