import os

dir_path = os.path.dirname(os.path.realpath(__file__))
build_dir = dir_path + "/../build"

os.makedirs(build_dir, exist_ok=True)

for file in os.listdir(dir_path + "/../"):
    if file.endswith(".frag") or file.endswith(".vert"):
        compiled_file = f"{build_dir}/{file}.spv"
        source_file = f"{dir_path}/../{file}"
        if not os.path.exists(compiled_file) or os.path.getmtime(source_file) > os.path.getmtime(compiled_file):
            os.system(f"glslc {dir_path}/../{file} -o {build_dir}/{file}.spv")
            print(f"Compiled {file}")