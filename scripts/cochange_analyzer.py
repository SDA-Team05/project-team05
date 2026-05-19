import re
from collections import defaultdict
from itertools import combinations
import pandas as pd

def parse_git_log(filename):
    with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # splitting commits using header pattern (--hash--data--author)
    commits = content.split('\n--')
    cochanges = defaultdict(int)
    file_commit_counts = defaultdict(int)
    
    for commit in commits:
        # find all file paths modified in the commit
        files = re.findall(r'(?:^\d+\s+\d+\s+|\s+)([^\s\n]+)', commit, re.MULTILINE)
        # filter to keep only C/C++ source files and avoid empty lines
        files = list(set([f for f in files if f.endswith(('.c', '.cpp', '.h'))]))
        
        # count the presence of each file
        for f in files:
            file_commit_counts[f] += 1
            
        # generate all possible pairs of files modified together in the same commit
        if len(files) > 1 and len(files) < 30: # Ignore massive refactoring commits
            for file_a, file_b in combinations(sorted(files), 2):
                cochanges[(file_a, file_b)] += 1
                
    return cochanges, file_commit_counts

cochanges, file_counts = parse_git_log('git_history_wireshark.txt')

# create a structured list for the report
report_data = []
for (file_a, file_b), count in cochanges.items():
    if count >= 3: # consider only files that have changed together at least 3 times
        # calculate the coupling degree (Jaccard or simple percentage)
        # in how many commits where File A appears, does File B also appear?
        prob_b_given_a = (count / file_counts[file_a]) * 100
        prob_a_given_b = (count / file_counts[file_b]) * 100
        max_coupling = max(prob_b_given_a, prob_a_given_b)
        
        report_data.append({
            'File A': file_a,
            'File B': file_b,
            'Commits together': count,
            'Coupling %': round(max_coupling, 2)
        })

# save to a dataframe sorted by coupling in descending order
df = pd.DataFrame(report_data)
if not df.empty:
    df = df.sort_values(by='Coupling %', ascending=False)
    df.to_csv('cochange_anomalies.csv', index=False)
    print("Analysis completed! Open the file 'cochange_anomalies.csv'")
else:
    print("No significant coupling found. Try lowering the filters or extending the git history.")