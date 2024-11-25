import os
import sys
import argparse
import glob
import math

class Vec3:
    def __init__(self, v: list):
        self.x = float(v[0])
        self.y = float(v[1])
        self.z = float(v[2])

    def normalize(self):
        new = Vec3([self.x, self.y, self.z])

        magnitude = math.sqrt(new.x**2 + new.y**2 + new.z**2)
        new.x /= magnitude
        new.y /= magnitude
        new.z /= magnitude

        return new

    def dot(self, other):
        x = [self.x, self.y, self.z]
        y = [other.x, other.y, other.z]
        return sum([xn*yn for xn,yn in zip(x,y)])

    def size(self):
        return 3
    
    def to_string(self):
        return [f"{self.x}f", f"{self.y}f", f"{self.z}f"]

    def __repr__(self):
        return ", ".join(self.to_string())

    # def __add__(self, other):
    #     return Vec3([self.x + other.x, self.y + other.y, self.z + other.z])

class NormalAccumulator:
    def __init__(self, v: Vec3, simple: bool):
        self.count = 1
        self.normal = [v]
        self.simple = simple
    
    def add(self, v: Vec3):
        if self.simple and self.count == 1:
            return

        self.normal.append(v)
        self.count += 1

    def size(self):
        if self.simple:
            return 1
        else:
            return 3

    def faces(self, v: Vec3):
        x = self.normal[0].normalize()
        y = v.normalize()
        if x.dot(y) == -1:
            return True
        return False

    def to_string(self):
        if self.simple:
            # assume model faces up
            if self.faces(Vec3([0.0, 1.0, 0.0])):
                return ["0.8f"]
            else:
                return ["1.0f"]

        total_normals = [0.0, 0.0, 0.0]
        for n in self.normal:
            total_normals[0] += n.x
            total_normals[1] += n.y
            total_normals[2] += n.z

        nx = total_normals[0]/self.count
        ny = total_normals[1]/self.count
        nz = total_normals[2]/self.count
        return [f"{nx}f", f"{ny}f", f"{nz}f"]
    
    def __repr__(self):
        return ", ".join(self.to_string())

class VertexBufferObject:
    def __init__(self, v: Vec3):
        self.values = [v]

    def add_normal(self, v: Vec3, simple: bool):
        for i in self.values:
            if type(i).__name__ == "NormalAccumulator":
                return
        
        self.values.append(NormalAccumulator(v, simple))

    def size(self):
        total_count = 0
        for val in self.values:
            total_count += val.size()
        return total_count
    
    def to_string(self):
        str_values = []
        for val in self.values:
            str_values += val.to_string()
        return ", ".join(str_values)

    def __repr__(self):
        return self.to_string()

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)
    sys.exit(1)

def main():
    parser = argparse.ArgumentParser(prog=sys.argv[0])

    parser.add_argument("-s", "--search-path", dest="search_path", required=True, help="Directory to search for .obj files from")
    parser.add_argument("-c", "--cpp-output-path", dest="cpp_output_path", required=True, help="Directory to put the output .cpp file to")
    parser.add_argument("-i", "--header-output-path", dest="h_output_path", required=True, help="Directory to put the output .h file to")
    parser.add_argument("-z", "--simple-shading", dest="simple", action="store_true", default=False, required=False, help="Whether or not to read the OBJ as a 'simple' geometric object. \
                                                                                          The first normal vector found is used as the normal vector for the vertex, as opposed to an average.")

    args = parser.parse_args()

    for file in glob.glob(f"{args.search_path}/*.obj"):
        vbo = []
        veo = []
        normals = []
        lines = None

        with open(file, "r") as f:
            lines = f.read()
        
        if lines is None:
            eprint(f"Could not read contents of file {file}")
        
        lines = lines.strip().split("\n")

        if len(lines) == 0:
            eprint(f"Contents of file {file} are invalid")

        if lines[0].strip() != "# WaveFront *.obj file (generated by Autodesk ATF)":
            eprint(f"Contents of file {file} are not in the right format")

        for line in lines[1:]:
            if len(line.strip()) == 0:
                continue
            
            line_contents = line.split(" ")

            if line_contents[0] == "v":
                vbo.append(VertexBufferObject(Vec3(line_contents[1:4])))

            elif line_contents[0] == "vn":
                normals.append(Vec3(line_contents[1:4]))
                
            elif line_contents[0] == "f":
                # either looks like "f 1 2 3"
                # or "f 1//1 2//2 3//3" 
                # or "f 1/2/1 2/3/2 3/4/3" 

                single_case = False

                if len(line_contents[1].split("/")) == 1:
                    single_case = True
                    v1 = line_contents[1].split("/")
                    v2 = line_contents[2].split("/")
                    v3 = line_contents[3].split("/")
                else:
                    v1,_,n1 = line_contents[1].split("/")
                    v2,_,n2 = line_contents[2].split("/")
                    v3,_,n3 = line_contents[3].split("/")

                veo.append((v1, v2, v3))

                if not single_case:
                    n1_i = int(n1) - 1
                    n2_i = int(n2) - 1
                    n3_i = int(n3) - 1

                    v1_i = int(v1) - 1
                    v2_i = int(v2) - 1
                    v3_i = int(v3) - 1

                    vbo[v1_i].add_normal(normals[n1_i], args.simple)
                    vbo[v2_i].add_normal(normals[n2_i], args.simple)
                    vbo[v3_i].add_normal(normals[n3_i], args.simple)

        if len(vbo) == 0 or len(veo) == 0:
            eprint(f"Contents of file {file} resulted in no parseable data")

        just_file_name = file.split("/")[-1].replace(".obj", "").replace('-','_')

        vbo_buffer_size = len(vbo) * vbo[0].size()

        with open(f"{args.h_output_path}/{just_file_name}.h", "w") as f:
            f.write(f"extern float {just_file_name}_buffer_data[{vbo_buffer_size}];\n")
            f.write(f"extern unsigned int {just_file_name}_buffer_data_stride;\n")
            f.write(f"extern unsigned int {just_file_name}_elements_data[{len(veo)*len(veo[0])}];")

        with open(f"{args.cpp_output_path}/{just_file_name}.cpp", "w") as f:
            f.write(f"float {just_file_name}_buffer_data[{vbo_buffer_size}] = {{\n")
            for v in vbo:
                f.write(f"\t{str(v)},\n")
            f.write(f"}};\n\n")

            f.write(f"unsigned int {just_file_name}_buffer_data_stride = {vbo[0].size()};\n\n")

            f.write(f"unsigned int {just_file_name}_elements_data[{len(veo)*len(veo[0])}] = {{\n")
            for vv in veo:
                ui1,ui2,ui3 = vv
                f.write(f"\t{ui1}-1, {ui2}-1, {ui3}-1,\n")
            f.write(f"}};\n\n")

if __name__ == "__main__":
    main()