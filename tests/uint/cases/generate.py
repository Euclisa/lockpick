import random
import argparse
import os

CASES_PER_SET_DEFAULT = 10000

DRY_FILE_NAME = "uint_test_files.txt"
BASE = 1 << (1 << 10)

def gen_single(size):
    uint_mode = random.randint(0,100)
    if uint_mode >= 10:
        uint = random.randint(0,int(2**size)-1)
    elif uint_mode >= 7:
        uint = 0
    elif uint_mode >= 3:
        uint = 2**size-1
    else:
        uint = random.randint(0,int(2**(size/2))-1)
    
    return uint

def get_pair_fn(first_size, second_size):
    return f"uint_{first_size}_{second_size}.txt"

def get_pair_addition_fn(first_size, second_size):
    return f"uint_{first_size}_{second_size}_addition.txt"

def gen_pairs(first_size, second_size, path, cases_per_test):
    f_samp = open(f"{path}/uint_{first_size}_{second_size}.txt", 'w')
    f_add = open(f"{path}/uint_{first_size}_{second_size}_addition.txt", 'w')
    f_sub = open(f"{path}/uint_{first_size}_{second_size}_subtraction.txt", 'w')
    f_mul = open(f"{path}/uint_{first_size}_{second_size}_multiplication.txt", 'w')
    for test_i in range(cases_per_test):
        first_uint = gen_single(first_size)
        second_uint = gen_single(second_size)
        res_add = first_uint + second_uint
        res_sub = first_uint - second_uint
        res_mul = first_uint * second_uint

        first_hex = hex(first_uint)[2:]
        second_hex = hex(second_uint)[2:]
        add_hex = hex(res_add)[2:]
        sub_hex = hex(res_sub if res_sub > 0 else BASE + res_sub)[2:]
        mul_hex = hex(res_mul)[2:]
        f_samp.write(f"{first_hex} {second_hex}\n")
        f_add.write(f"{add_hex}\n")
        f_sub.write(f"{sub_hex}\n")
        f_mul.write(f"{mul_hex}\n")
        
    
    f_samp.close()
    f_add.close()
    f_sub.close()
    f_mul.close()

if __name__ == "__main__":

    arg_parser = argparse.ArgumentParser(description="uint(N) test cases generator")

    arg_parser.add_argument('pairs',nargs='*',type=str)
    arg_parser.add_argument('--cases_num','-c',type=int,default=CASES_PER_SET_DEFAULT)
    arg_parser.add_argument('--path','-p',type=str,required=True)
    arg_parser.add_argument('--dry','-d',action="store_true")

    args = arg_parser.parse_args()

    path = args.path

    if not os.path.isdir(path):
        raise RuntimeError(f"No such directory: '{path}'")

    if args.pairs:
        pairs_set = set()
        for pair in args.pairs:
            first_size, second_size = pair.split("_")
            first_size = int(first_size)
            second_size = int(second_size)
            if ((first_size & (first_size-1) != 0)) or first_size < 16 or ((second_size & (second_size-1)) != 0) or second_size < 16:
                raise RuntimeError(f"Invalid argument: {pair}")
            pairs_set.add((first_size,second_size))
        if args.dry:
            with open(f"{path}/{DRY_FILE_NAME}",'w') as dry_f:
                for f,s in pairs_set:
                    fn_samp = get_pair_fn(f,s)
                    fn_add = get_pair_addition_fn(f,s)
                    #dry_f.write(f"{path}/{fn_samp}\n")
                    #dry_f.write(f"{path}/{fn_add}\n")
                    print(f"{path}/{fn_samp}",end='')
                    print(f" {path}/{fn_add}",end='')
        else:
            for f,s in pairs_set:
                gen_pairs(f,s,path,args.cases_num)
