from sys import argv

script, filename, filename2 = argv

print(f"We're going to erase {filename}.")
print("If you don't want that, hit (^C).")
print("If you want that, hit RETURN.")

input('?')

print("Openning the file...")
target = open(filename, 'w')

print("Truncating the file. See you")
target.truncate()

print("Now I'm going to ask you for three lines.")

line1 = input("line 1: ")
line2 = input("line 2: ")
line3 = input("line 3: ")

print("I'm going to write these to the file.")

target.write(line1)
target.write("\n")
target.write(line2)
target.write("\n")
target.write(line3)
target.write("\n")

target2 = open(filename2)

print("The line1 is:")
# target3 = target2.readline(filename2)
print("The line3 is:")
target2.readline()

print("And finally, we close it.")
target.close()

