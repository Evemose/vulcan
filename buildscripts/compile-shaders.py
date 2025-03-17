import os

script_dir = os.path.dirname(os.path.realpath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, ".."))

src_dir = os.path.join(project_root, "src")
build_dir = os.path.join(project_root, "build")

print(f"Compiling shaders from {src_dir} to {build_dir}")

total_shaders = 0

for root, dirs, files in os.walk(src_dir):
    for file in files:
        if file.endswith(".frag") or file.endswith(".vert"):
            # Full path to source shader
            source_file = os.path.join(root, file)

            # Path relative to ../src
            rel_path = os.path.relpath(source_file, start=src_dir)
            # Where we want the compiled .spv to go (mirror folders under ../build)
            output_folder = os.path.join(build_dir, os.path.dirname(rel_path))
            os.makedirs(output_folder, exist_ok=True)

            compiled_file = os.path.join(output_folder, file + ".spv")

            # Only compile if .spv doesn't exist or the shader is newer
            if (not os.path.exists(compiled_file) or
                    os.path.getmtime(source_file) > os.path.getmtime(compiled_file)):

                cmd = f'glslc "{source_file}" -o "{compiled_file}"'
                os.system(cmd)
                print(f"Compiled {rel_path}")
                total_shaders += 1

print(f"Compiled {total_shaders} shaders")
