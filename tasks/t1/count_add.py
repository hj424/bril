# input: .json file
# output: total number of add instructions

import argparse

def main(args):

  file_name_base = args.name
  file_name      = file_name_base + ".json"

  if file_name_base == "":
    print("Analyzing default file: "+file_name+"...")
  
  # open the file
  f_ptr = open(file_name, "r")
  if (not f_ptr):
    print("! Can't open file: "+file_name)

  cnt_add = 0
  line = f_ptr.readline()
  while line:
    element = line.split(":")
    if len(element) >= 2:
      sub_ele = element[1].split("\"")
      if len(sub_ele) >= 2:
        if sub_ele[1] == "add":
          cnt_add = cnt_add + 1
    # read new line
    line = f_ptr.readline()

  print("Total # of add: "+str(cnt_add))
  print("DONE!")

# "real" main function
if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Count total # of add instructions.')
  parser.add_argument("--name", type=str, default="add",
                      help="name of dataset")
  parser.add_argument("--info", type=int, default=1,
                      help="Naive parser for counting total # of add instructions")
  args = parser.parse_args()
  print(args)

  main(args)
