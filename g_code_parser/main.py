import g_code_parser
import os



f = open("output.c", "w")
f.write("#include \"motor.h\"\nextern float xPos;\nextern float yPos;\nextern float zPos;\nvoid run_g_code() {\n")

file_parser = g_code_parser.Parser(f)
file_parser.parse_file("input_g_code.ngc")

f.write("\n")

f.write("}")
f.close()
