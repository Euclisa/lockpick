import random
import argparse
import os

CASES_PER_SET_DEFAULT = 100
MAX_VALUE = 255

LIST_INSERT_AFTER_CODE = 0
LIST_INSERT_BEFORE_CODE = 1
LIST_PUSH_BACK_CODE = 2
LIST_PUSH_FRONT_CODE = 3
LIST_REMOVE_CODE = 4

def gen_single_push_back(curr_list):
    val = random.randint(0,MAX_VALUE)
    curr_list.append(str(val))
    list_str = ' '.join(curr_list)
    return f"{LIST_PUSH_BACK_CODE} {val}", f"{list_str}"

def gen_single_push_front(curr_list):
    val = random.randint(0,MAX_VALUE)
    curr_list.insert(0,str(val))
    list_str = ' '.join(curr_list)
    return f"{LIST_PUSH_FRONT_CODE} {val}", f"{list_str}"

def gen_single_insert_before_head(curr_list):
    val = random.randint(0,MAX_VALUE)
    curr_list.insert(0,str(val))
    list_str = ' '.join(curr_list)
    return f"{LIST_INSERT_BEFORE_CODE} {0} {val}", f"{list_str}"

def gen_single_insert_after_head(curr_list):
    val = random.randint(0,MAX_VALUE)
    curr_list.insert(1,str(val))
    list_str = ' '.join(curr_list)
    return f"{LIST_INSERT_AFTER_CODE} {0} {val}", f"{list_str}"

def gen_single_insert_before_tail(curr_list):
    val = random.randint(0,MAX_VALUE)
    curr_list.insert(len(curr_list)-1,str(val))
    list_str = ' '.join(curr_list)
    return f"{LIST_INSERT_BEFORE_CODE} {max(0,len(curr_list)-2)} {val}", f"{list_str}"

def gen_single_insert_after_tail(curr_list):
    val = random.randint(0,MAX_VALUE)
    curr_list.insert(len(curr_list),str(val))
    list_str = ' '.join(curr_list)
    return f"{LIST_INSERT_AFTER_CODE} {max(0,len(curr_list)-2)} {val}", f"{list_str}"

def gen_single_remove(curr_list):
    if not curr_list:
        raise RuntimeError()
    pos = random.randint(0,len(curr_list)-1)
    curr_list.pop(pos)
    list_str = ' '.join(curr_list)
    list_str = list_str if len(list_str) > 0 else "empty"
    return f"{LIST_REMOVE_CODE} {pos}", f"{list_str}"


def gen_cases_push_back(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_push_back.txt",'w') as f:
        for case_i in range(cases_per_test):
            sample, res = gen_single_push_back(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_push_front(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_push_front.txt",'w') as f:
        for case_i in range(cases_per_test):
            sample, res = gen_single_push_front(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_insert_before_head(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_insert_before_head.txt",'w') as f:
        for case_i in range(cases_per_test):
            sample, res = gen_single_insert_before_head(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_insert_after_head(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_insert_after_head.txt",'w') as f:
        for case_i in range(cases_per_test):
            sample, res = gen_single_insert_after_head(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_insert_before_tail(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_insert_before_tail.txt",'w') as f:
        for case_i in range(cases_per_test):
            sample, res = gen_single_insert_before_tail(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_insert_after_tail(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_insert_after_tail.txt",'w') as f:
        for case_i in range(cases_per_test):
            sample, res = gen_single_insert_after_tail(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_random(path,cases_per_test):
    curr_list = []
    with open(f"{path}/list_random.txt",'w') as f:
        for case_i in range(cases_per_test):
            if curr_list:
                action_code = random.randint(0,6)
            else:
                action_code = random.randint(0,5)
            
            if action_code == 0:
                sample, res = gen_single_push_back(curr_list)
            elif action_code == 1:
                sample, res = gen_single_push_front(curr_list)
            elif action_code == 2:
                sample, res = gen_single_insert_before_head(curr_list)
            elif action_code == 3:
                sample, res = gen_single_insert_after_head(curr_list)
            elif action_code == 4:
                sample, res = gen_single_insert_before_tail(curr_list)
            elif action_code == 5:
                sample, res = gen_single_insert_after_tail(curr_list)
            elif action_code == 6:
                sample, res = gen_single_remove(curr_list)

            f.write(f"{sample}\n{res}\n")

        while len(curr_list) > 1:
            sample, res = gen_single_remove(curr_list)
            f.write(f"{sample}\n{res}\n")


def gen_cases_all(path,cases_per_test):
    gen_cases_push_back(path,cases_per_test)
    gen_cases_push_front(path,cases_per_test)
    gen_cases_insert_before_head(path,cases_per_test)
    gen_cases_insert_after_head(path,cases_per_test)
    gen_cases_insert_before_tail(path,cases_per_test)
    gen_cases_insert_after_tail(path,cases_per_test)
    gen_cases_random(path,cases_per_test)


if __name__ == "__main__":
    random.seed(0)

    arg_parser = argparse.ArgumentParser(description="lp_list test cases generator")

    arg_parser.add_argument('--cases_num','-c',type=int,default=CASES_PER_SET_DEFAULT)
    arg_parser.add_argument('--path','-p',type=str,required=True)

    args = arg_parser.parse_args()

    path = args.path

    if not os.path.isdir(path):
        raise RuntimeError(f"No such directory: '{path}'")
    
    gen_cases_all(path,args.cases_num)