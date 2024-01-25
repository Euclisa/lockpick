import random
import argparse
import os

CASES_PER_SET_DEFAULT = 1000

DRY_FILE_NAME = "uint_test_files.txt"
BASE = 1 << (1 << 11)

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

def gen_shift(size):
    return random.randint(0,size)


def pad_uint_to_size(uint, size):
    return min(2**size-1,uint)

def get_pair_fn(first_size, second_size):
    return f"uint_{first_size}_{second_size}.txt"

def get_pair_addition_fn(first_size, second_size):
    return f"uint_{first_size}_{second_size}_addition.txt"

def gen_unary(size, path, cases_per_test):
    f_samp = open(f"{path}/uint_{size}.txt", 'w')
    f_samp_shifts = open(f"{path}/uint_{size}_shifts.txt", 'w')
    f_lshift = open(f"{path}/uint_{size}_lshift.txt", 'w')
    f_rshift = open(f"{path}/uint_{size}_rshift.txt", 'w')

    for test_i in range(cases_per_test):
        uint = gen_single(size)
        shift = gen_shift(size)

        uint_hex = hex(uint)[2:]
        res_lshift = uint << shift
        res_rshift = uint >> shift

        lshift_hex = hex(res_lshift & (BASE-1))[2:]
        rshift_hex = hex(res_rshift)[2:]
        f_samp.write(f"{uint_hex}\n")
        f_samp_shifts.write(f"{shift}\n")
        f_lshift.write(f"{lshift_hex}\n")
        f_rshift.write(f"{rshift_hex}\n")
    
    f_lshift.close()
    f_rshift.close()


def gen_pairs(first_size, second_size, path, cases_per_test):
    f_samp = open(f"{path}/uint_{first_size}_{second_size}.txt", 'w')
    f_add = open(f"{path}/uint_{first_size}_{second_size}_add.txt", 'w')
    f_sub = open(f"{path}/uint_{first_size}_{second_size}_sub.txt", 'w')
    f_mul = open(f"{path}/uint_{first_size}_{second_size}_mul.txt", 'w')
    f_comp = open(f"{path}/uint_{first_size}_{second_size}_comparison.txt", 'w')
    f_and = open(f"{path}/uint_{first_size}_{second_size}_and.txt", 'w')
    f_or = open(f"{path}/uint_{first_size}_{second_size}_or.txt", 'w')
    f_xor = open(f"{path}/uint_{first_size}_{second_size}_xor.txt", 'w')
    for test_i in range(cases_per_test):
        first_uint = gen_single(first_size)
        if(random.randint(1,10) > 8):
            second_uint = pad_uint_to_size(first_uint,second_size)
        else:
            second_uint = gen_single(second_size)
        res_add = first_uint + second_uint
        res_sub = first_uint - second_uint
        res_mul = first_uint * second_uint
        res_comp = 0 if first_uint == second_uint else (1 if first_uint < second_uint else 2)
        res_and = first_uint & second_uint
        res_or = first_uint | second_uint
        res_xor = first_uint ^ second_uint

        first_hex = hex(first_uint)[2:]
        second_hex = hex(second_uint)[2:]
        add_hex = hex(res_add)[2:]
        sub_hex = hex(res_sub if res_sub > 0 else BASE + res_sub)[2:]
        mul_hex = hex(res_mul)[2:]
        and_hex = hex(res_and)[2:]
        or_hex = hex(res_or)[2:]
        xor_hex = hex(res_xor)[2:]
        f_samp.write(f"{first_hex} {second_hex}\n")
        f_add.write(f"{add_hex}\n")
        f_sub.write(f"{sub_hex}\n")
        f_mul.write(f"{mul_hex}\n")
        f_comp.write(f"{res_comp}\n")
        f_and.write(f"{and_hex}\n")
        f_or.write(f"{or_hex}\n")
        f_xor.write(f"{xor_hex}\n")
        
    
    f_samp.close()
    f_add.close()
    f_sub.close()
    f_mul.close()
    f_and.close()
    f_or.close()
    f_xor.close()

if __name__ == "__main__":

    arg_parser = argparse.ArgumentParser(description="lp_uint(N) test cases generator")

    arg_parser.add_argument('tests',nargs='*',type=str)
    arg_parser.add_argument('--cases_num','-c',type=int,default=CASES_PER_SET_DEFAULT)
    arg_parser.add_argument('--path','-p',type=str,required=True)
    arg_parser.add_argument('--dry','-d',action="store_true")

    args = arg_parser.parse_args()

    path = args.path

    if not os.path.isdir(path):
        raise RuntimeError(f"No such directory: '{path}'")

    if args.tests:
        pairs_set = set()
        unary_set = set()
        for test in args.tests:
            if '_' in test:
                first_size, second_size = test.split("_")
                first_size = int(first_size)
                second_size = int(second_size)
                if ((first_size & (first_size-1) != 0)) or first_size < 16 or ((second_size & (second_size-1)) != 0) or second_size < 16:
                    raise RuntimeError(f"Invalid argument: {test}")
                pairs_set.add((first_size,second_size))
            else:
                size = int(test)
                if (size & (size-1) != 0) or size < 16:
                    raise RuntimeError(f"Invalid argument: {test}")
                unary_set.add(size)
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
            for size in unary_set:
                gen_unary(size,path,args.cases_num)
