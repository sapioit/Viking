def read_file(file_name):
    file = open(file_name)
    lines = file.readlines()
    lines = [line.rstrip('\n') for line in lines]
    file.close()
    return lines

def write_dictionary(dict, file_name):
    file = open(file_name, 'w')
    for key in dict:
        file.write(key + ' ' + dict[key] + '\n')
    file.close()

def make_dictionary(lines):
    result = dict()
    for line in lines:
        split = line.split()
        mime_type = split[0]
        for extension in split[1:]:
            result[extension] = mime_type
    return result

def write_unordered_map(dict, file_name):
    file = open(file_name, 'w')
    file.write("#include <unordered_map>\n#include <string>\n\n")
    file.write("static std::unordered_map<std::string, std::string> mime_types {\n")
    i = 0
    for key in dict:
        i = i+1
        file.write("{\"" + key + "\"" + ", \"" + dict[key] + "\"}")
        if i < len(dict):
            file.write(",")
        file.write("\n")
    file.write("};\n")
    file.close()

raw_lines = read_file("mime_types.txt")
dictionary = make_dictionary(raw_lines)
write_dictionary(dictionary, "mime_types_map.txt")
write_unordered_map(dictionary, "mime_types.h")
