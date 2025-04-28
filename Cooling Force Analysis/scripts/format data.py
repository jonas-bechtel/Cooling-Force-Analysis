import os

def process_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
     
    lines = [line.replace('"', '') for line in lines]
    
    updated_lines = []  # A new list to store modified lines
    for line in lines:
        if line.startswith('0'):
            updated_lines.append(line)
            break
        else:

            # Replace ',T' with ',Theta' if 'Theta' is not found
            if "Theta" not in line:
                line = line.replace(",T", ",Theta")

            # Add '#' to the start if not already commented
            if not line.startswith('#'):
                line = '# ' + line

            # Append modified line to new list
            updated_lines.append(line)

    # Add remaining lines that start with '0' and onwards.
    updated_lines.extend(lines[len(updated_lines):])
    
    with open(file_path, 'w') as file:
        file.writelines(updated_lines)

def process_directory(directory_path):
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            if file.endswith('.CSV') or file.endswith('.csv'):  # Check file extension for CSV files
                process_file(file_path)
            else:
                os.remove(file_path)

if __name__ == "__main__":
    directory_to_process = '.\\..\\data' 
    process_directory(directory_to_process)