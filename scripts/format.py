import argparse


arg_parser = argparse.ArgumentParser(description="Formatter for C code",epilog="UNRELIABLE!!! COMMIT CHANGES BEFORE USE")

arg_parser.add_argument('-f','--file',type=str,help="Path to the target file")
arg_parser.add_argument('-e','--escape',type=str,help="Escape '\\' characters would be added at the end of each line from 'start' to 'end' (example: '--escape 26-130')")

args = arg_parser.parse_args()

def add_escapes(lines_range: str, file_path: str):
    if file_path is None:
        print("File is not specified")
        exit(1)

    with open(file_path,'r') as f:
        content = f.readlines()
    
    start_line, end_line = list(map(lambda x: int(x)-1,lines_range.split('-')))
    if(start_line < 0 or end_line >= len(content)):
        print(f"Specified lines range '{lines_range}' does not fit in lines range of file '{file}' with line number of {len(content)}")
        exit(1)
    
    max_line_length = 0
    for i in range(start_line,end_line+1):
        content[i] = content[i].rstrip("\n \t")
        length = len(content[i])
        if length > max_line_length:
            max_line_length = length
    
    ESCAPE_OFFSET = 10
    target_length = max_line_length+ESCAPE_OFFSET
    for i in range(start_line,end_line+1):
        content[i] = content[i].ljust(target_length,' ') + '\\\n'
    
    with open('temp.c', 'w') as f:
        f.writelines(content)


if args.escape is not None:
    add_escapes(args.escape,args.file)


