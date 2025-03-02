def get_level_power(level):
    if level < 100:
        multi = 2.5
    elif level < 250:
        multi = 3
    elif level < 450:
        multi = 3.5
    elif level < 700:
        multi = 4
    else:
        multi = 4.5

    power = (level / 10) ** multi

    if level < 10:
        return round(power, 2)
    elif level < 24:
        return round(power, 1)
    else:
        return round(power)

def get_num_per_singularity(level):
    if level < 10:
        return 0
    elif level < 100:
        multi = 1.038
    elif level < 250:
        multi = 1.031
    elif level < 450:
        multi = 1.025
    elif level < 700:
        multi = 1.019
    elif level < 1000:
        multi = 1.010
    else:
        return 1

    return round(multi ** (1000 - level))

def print_levels():
    for i in range(0, 1001):
        print("{:4}:{:10}".format(i, get_level_power(i)), end="")

        if i % 10 == 9:
            print()

        if i == 99 or i == 249 or i == 449 or i == 699 or i == 999:
            print()
            
    print()

def print_nums_per_singularity():
    num_mortals = sum([get_num_per_singularity(i) for i in range(0, 100)])
    num_trancendants = sum([get_num_per_singularity(i) for i in range(100, 250)])
    num_demigods = sum([get_num_per_singularity(i) for i in range(250, 450)])
    num_gods = sum([get_num_per_singularity(i) for i in range(450, 750)])
    num_constellations = sum([get_num_per_singularity(i) for i in range(750, 1000)])
    num_singularities = get_num_per_singularity(1000)

    print("singularities:  {:20}".format(num_singularities))
    print("constellations: {:20}".format(num_constellations))
    print("gods:           {:20}".format(num_gods))
    print("demigods:       {:20}".format(num_demigods))
    print("trancendants:   {:20}".format(num_trancendants))
    print("num_mortals:    {:20}".format(num_mortals))
    print("max_int:        {:20}".format(2**64))

    print()
    
    print("constellations per singularity:", num_constellations / num_singularities)
    print("gods per constellation:", num_gods / num_constellations)
    print("demigods per god:", num_demigods / num_gods)
    print("trancendants per demigod:", num_trancendants / num_demigods)
    print("mortals per trancendant:", num_mortals / num_trancendants)
          
    print()
    
    for i in range(0, 100):
        c = sum([get_num_per_singularity(i2) for i2 in range(i * 10, (i+1) * 10)])
        print("{:4}:{:20}".format(i, c))

print_levels()
print_nums_per_singularity()
