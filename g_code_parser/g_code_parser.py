import os

def remove_non_content(content):
    if '(' in content:
        start = content.find('(')
        end = content.find(')') + 1
        content = content[0:start] + content[end:]
        content = remove_non_content(content)
        return content
    else:
        return remove_whitespace(content)

def remove_whitespace(content):
    content = content.strip()
    content = ' '.join(content.split())
    return content


class Parser:

    def __init__(self,f):
        self.output_file = f
        return

    def parse_line(self, line):
        is_movement = False
        is_arc = False
        g_code = None
        x = None
        y = None
        z = None
        i = None
        j = None

        phrases = line.split()

        for phrase in phrases:
            code = phrase[0]
            argument = phrase[1:]

            if code == 'G':
                self.g_command(argument)
                g_code = int(argument)
                continue
            if code == 'X':
                x = float(argument)
            if code == 'Y':
                y = float(argument)
            if code == 'Z':
                z = float(argument)
            if code == 'I':
                i = float(argument)
            if code == 'J':
                j = float(argument)
            if phrase == 'M03':
                self.output_file.write("dropServo();\n")
            if phrase == 'M05':
                self.output_file.write("liftServo();\n")

        if z == None:
            z = "zPos"
        if y == None:
            y = "yPos"
        if x == None:
            x = "xPos"
        if g_code == 0 or g_code == 1:
            self.output_file.write(str(x)+","+str(y)+","+str(z)+");\n")
        if g_code == 2:
            self.output_file.write(str(x)+","+str(y)+","+str(z)+","+str(i)+","+str(j)+");\n")
        if g_code == 3:
            self.output_file.write(str(x)+","+str(y)+","+str(z)+","+str(i)+","+str(j)+");\n")

    def parse_file(self, input_file):
        self.instructions = []
        commands = []

        print("parse_file starting")
        script_dir = os.path.dirname(os.path.abspath(__file__))
        file_path = os.path.join(script_dir, input_file)

        with open(input_file) as file:
            for line in file:
                commands.append(line)

        for command in commands:
            command = remove_non_content(command)
            if not command:
                continue

            self.parse_line(command)

    def g_command(self, code):
        code = int(code)

        if code == 0:  # Go max speed
            self.output_file.write("moveToXYZ(")
            return
        if code == 1:  # Feed rate will be provided for speed
            self.output_file.write("moveToXYZ(")
            return
        if code == 2:
            self.output_file.write("moveArcCW(")
            return
        if code == 3:
            self.output_file.write("moveArcCCW(")
            return


        print("Unknown command G" + str(code))