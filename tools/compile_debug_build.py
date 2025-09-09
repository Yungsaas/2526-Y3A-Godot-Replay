import subprocess
import sys
import os
import shutil  # Import shutil for file operations

# Paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR = os.path.dirname(SCRIPT_DIR)

# This is only used for copying the gdextension to another godot project
SOURCE_FOLDER = os.path.join(PARENT_DIR, 'test_project/replay_qol')  # Replace with your source folder path
DESTINATION_FOLDER = os.path.join(PARENT_DIR, 'physics_platformer/replay_qol')  # Replace with your destination folder path

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')


def copy_folder():
    """Remove old content in the destination folder and copy new content from the source folder."""
    try:
        # Remove the destination folder if it exists
        if os.path.exists(DESTINATION_FOLDER):
            shutil.rmtree(DESTINATION_FOLDER)  # Remove the existing folder

        # Copy the source folder to the destination
        shutil.copytree(SOURCE_FOLDER, DESTINATION_FOLDER)
        print(f"Successfully copied '{SOURCE_FOLDER}' to '{DESTINATION_FOLDER}'.")
    except Exception as e:
        print(f"Error while copying folder: {e}")
        
def run_scons_build():
    """Run 'scons compiledb=yes' from the project root and show output in real-time."""
    try:
        process = subprocess.Popen(
            ["scons", "compiledb=yes"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            cwd=PARENT_DIR  # Run from root directory where SConstruct is
        )

        stdout_lines = []
        stderr_lines = []

        while True:
            output = process.stdout.readline()
            if output:
                print(output, end="")
                stdout_lines.append(output)
            elif process.poll() is not None:
                break

        remaining_out, remaining_err = process.communicate()
        if remaining_out:
            print(remaining_out, end="")
            stdout_lines.append(remaining_out)
        if remaining_err:
            stderr_lines.append(remaining_err)

        if process.returncode == 0:
            print("\nCompilation finished successfully.")
            print("A debug build for your current OS and architecture was added to the bin folder.")
            print("The compile_commands.json file was also updated to improve IntelliSense support.")

            copy_folder()
        else:
            print("\nCompilation FAILED:")
            print(''.join(stderr_lines).strip() or "Unknown error occurred.")

    except FileNotFoundError:
        print("Error: 'scons' command not found. Make sure SCons is installed and available in your PATH.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

    input("\nPress any key to continue...")


if __name__ == "__main__":
    clear_screen()
    run_scons_build()
