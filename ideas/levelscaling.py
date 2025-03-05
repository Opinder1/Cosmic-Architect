import math

def get_level_step(level):
    return level / 10

def get_level_class_stage(level):
    return level / 50

def get_level_realm(level):
    if level >= 1000: # Perfection
        return 6
    elif level >= 750: # Three quarters perfection
        return 5
    elif level >= 500: # Half way perfection
        return 4
    elif level >= 250: # Quarter perfection
        return 3
    elif level >= 100: # Tenth perfection
        return 2
    else:
        return 1

def get_level_xp(level):
    return level ** 2

def get_level_total_xp(level):
    return sum([get_level_xp(i) for i in range(0, level)])

def get_level_power_multi(level):
    step = get_level_step(level)
    realm = get_level_realm(level)
    multi = 1 + (0.5 * realm)

    power = step ** multi

    if power < 100:
        return round(power, 1)
    else:
        return round(power)

def approx_num_per_singularity(level):
    singularity_realm = get_level_realm(1000)
    realm = get_level_realm(level)

    increase_per_level = 12
    increase_per_realm = 1.3

    t = (1000 - level) / increase_per_level

    p = t ** ((singularity_realm - realm) ** increase_per_realm)

    return round(p)

def get_level_lifespan_multi(level):
    if level >= 500:
        return math.inf # Immortal at half perfection
    elif level >= 250:
        return 1000
    elif level >= 100:
        return 10
    else:
        return 1

def get_median_child_level(parent1_level, parent2_level):
    level = (parent1_level + parent2_level) / 3
    
    if parent1_level == 1000 or parent2_level == 1000:
        return round(level * 1.2) # Can give birth to eldrich if parent is singularity
    else:
        return round(min(level, 499)) # Can't give birth to immortals

def print_levels():
    for i in range(0, 1000, 10):
        print("{:4}-{:<4}: {:10}-{:<10}".format(
            i, i + 9,
            get_level_power_multi(i), get_level_power_multi(i + 9)
        ))
            
    print()

def print_level_xps():
    for i in range(0, 1000, 10):
        print("{:4}-{:<4}: {:10}-{:<10} {:10}-{:<10}".format(
            i, i + 9,
            get_level_xp(i), get_level_xp(i + 9),
            get_level_total_xp(i), get_level_total_xp(i + 9)
        ))

def print_nums_per_singularity():
    num_mortals = sum([approx_num_per_singularity(i) for i in range(0, 100)])
    num_trancendants = sum([approx_num_per_singularity(i) for i in range(100, 250)])
    num_demigods = sum([approx_num_per_singularity(i) for i in range(250, 500)])
    num_gods = sum([approx_num_per_singularity(i) for i in range(500, 750)])
    num_constellations = sum([approx_num_per_singularity(i) for i in range(750, 1000)])
    num_singularities = approx_num_per_singularity(1000)

    print("singularities:  {:20}".format(num_singularities))
    print("constellations: {:20}".format(num_constellations))
    print("gods:           {:20}".format(num_gods))
    print("demigods:       {:20}".format(num_demigods))
    print("trancendants:   {:20}".format(num_trancendants))
    print("mortals:        {:20}".format(num_mortals))
    print("max_int:        {:20}".format(2**64))

    print()
    
    print("constellations per singularity:", num_constellations / num_singularities)
    print("gods per constellation:", num_gods / num_constellations)
    print("demigods per god:", num_demigods / num_gods)
    print("trancendants per demigod:", num_trancendants / num_demigods)
    print("mortals per trancendant:", num_mortals / num_trancendants)
          
    print()
    
    for i in range(0, 100):
        c = sum([approx_num_per_singularity(i2) for i2 in range(i * 10, (i+1) * 10)])
        print("{:4}:{:20}".format(i, c))

def print_level_children():
    levels = [10, 99, 100, 249, 250, 499, 500, 749, 750, 999, 1000]
    
    for level in levels:
        for level2 in levels:
            print("{} + {} = {}".format(level, level2, get_median_child_level(level, level2)))

print_levels()
#print_level_xps()
#print_nums_per_singularity()
#print_level_children()
