import os
import csv

def process_file(file_path):
    
    with open(file_path, 'r') as file:
        lines = file.readlines()
            
    # Collect header lines and extract the last header line for column names
    header_lines = [line for line in lines if line.startswith('#')]
    #print(header_lines)
    with open(file_path, 'r') as infile:
        # output dict needs a list for new column ordering
        fieldnames = header_lines[-1].strip('\n').split(',')
        #print(fieldnames)
        
        rows = []
        for row in csv.DictReader(infile, fieldnames=fieldnames):
            rows.append(row)
    
    with open(file_path, 'w', newline='') as outfile:
        for line in header_lines[:-1]:
            #print(line)
            outfile.write(line)
        
        desired_fieldnames = ['# Time', 'R', 'Theta', 'X', 'Y'][:len(fieldnames)]
        
        writer = csv.DictWriter(outfile, fieldnames=desired_fieldnames)
        # reorder the header first
        writer.writeheader()
        skippedRows = 0
        for row in rows:
            if(skippedRows < len(header_lines)):
                skippedRows += 1
                continue
                
            # writes the reordered rows to the new file
            writer.writerow(row)
            
    return 

def process_directory(directory_path):
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            if file.endswith('.CSV') or file.endswith('.csv'):  # Check file extension for CSV files
                process_file(file_path)
                print(file)
            else:
                os.remove(file_path)
                
if __name__ == "__main__":
    directory_to_process = '.\\..\\data' 
    process_directory(directory_to_process)