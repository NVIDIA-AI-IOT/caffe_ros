'''
 parse_folders.py

 This script is used to convert a directory structure into the appropriate map files for use
 in DIGITS. The script randomizes and partitions the data, then outputs the appropriate label
 indices (using the name of the folder the image was found in as the image's label).
'''

import argparse
import os
import sys
import numpy as np
import random

def main(root_dir, path_to_output_files, labels, percent_train):

    train_map_path = os.path.join(path_to_output_files, 'train_map.txt')
    validate_map_path = os.path.join(path_to_output_files, 'val_map.txt')
    labels_map_path = os.path.join(path_to_output_files, 'labels_map.txt')
        
    with open(train_map_path, 'w') as train_map, \
         open(validate_map_path, 'w') as validate_map, \
         open(labels_map_path, 'w') as labels_map:
        
        # generate list of all files
        files = [os.path.join(dp, f) for dp, dn, file_names in os.walk(root_dir) for f in file_names]
        
        # enumerate all possible labels
        if not labels:
            subdirs = [name for name in os.listdir(root_dir)]
            labels = {subdirs[i] : i for i in range(len(subdirs))}
        else:
            labels = {labels[i] : i for i in range(len(labels))}
            
        # randomly split the data
        random.shuffle(files)
        num_in_train = int((percent_train/float(100))*len(files))
        train, validate = np.split(files, [num_in_train])
            
        for file in train:
            base_path, subdir = os.path.split(os.path.dirname(file))
            fileName = os.path.join(subdir, os.path.basename(file))
            train_map.write('{} {}\n'.format(fileName, labels[subdir]))
                
        for file in validate:
            base_path, subdir = os.path.split(os.path.dirname(file))
            fileName = os.path.join(subdir, os.path.basename(file))
            validate_map.write('{} {}\n'.format(fileName, labels[subdir]))

        # sort labels for printing them out
        labels = sorted(labels, key=labels.get)
        for label in labels:
            labels_map.write('{}\n'.format(label))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Create map files from custom directory structure.')
    parser.add_argument('root_dir')
    parser.add_argument('path_to_output_files')
    parser.add_argument('--labels')
    parser.add_argument('percent_train', type=int) # i.e. 80 to mean 80%
    args = parser.parse_args()
    labels = []
    if args.labels:
        labels = args.labels.split(",")
        print(labels)
    main(args.root_dir, args.path_to_output_files, labels, args.percent_train)
