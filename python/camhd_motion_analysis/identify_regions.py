import json


def load_regions( data_file ):

    with open(data_file) as input:
        j = json.load(input)


    return j
