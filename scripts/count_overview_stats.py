import os
import subprocess

def count_stats():
    # Define the repository path (update if necessary)
    repo_path = 'd:/GitHub/Polito/Wireshark/wireshark'
    os.chdir(repo_path)
    
    print("Calculating statistics... (this may take a few seconds for lines of code)")
    
    # 1. Calculate the number of files tracked by git
    files = subprocess.check_output(['git', 'ls-files']).decode('utf-8', errors='ignore').splitlines()
    num_files = len(files)
    
    # 2. Calculate the number of unique developers via git history
    devs = subprocess.check_output(['git', 'log', '--format=%aN']).decode('utf-8', errors='ignore').splitlines()
    num_devs = len(set(devs))
    
    # 3. Calculate Lines of Code (LOC) by iterating over all files and counting lines
    loc = 0
    for f in files:
        path = os.path.join(repo_path, f)
        if os.path.isfile(path) and not os.path.islink(path):
            try:
                with open(path, 'rb') as fd:
                    # Add 1 for each line in the file
                    loc += sum(1 for _ in fd)
            except Exception:
                pass
                
    print("-" * 30)
    print("WIRESHARK GLOBAL STATISTICS")
    print("-" * 30)
    print(f"Total Files:           {num_files:,}")
    print(f"Unique Developers:     {num_devs:,}")
    print(f"Lines of Code (LOC):   {loc:,}")
    print("-" * 30)

if __name__ == '__main__':
    count_stats()
