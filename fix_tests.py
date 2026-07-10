import os
import glob

def clean_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    for line in lines:
        # Replace the multibyte character
        line = line.replace('─', '-')
        
        # Handle inline comments (not inside quotes - simplistic approach)
        # We know our test files don't have ';' inside strings.
        idx = line.find(';')
        if idx > 0:
            # Check if it's not the first character (which is a full line comment)
            # Just strip everything after ';'
            line = line[:idx] + '\n'
            
        new_lines.append(line)
        
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

for filepath in glob.glob('tests/*.as'):
    clean_file(filepath)
    print(f"Cleaned {filepath}")
