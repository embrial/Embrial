import math

with open("keywords.txt", "r") as file:
	lines = file.readlines()

lines = sorted([line.strip().lower() for line in lines if line.strip()])
sideLen = math.ceil(math.sqrt(len(lines)))

with open("keywords.txt", "w") as file:
	file.writelines(f"{line}\n" for line in lines)

letters = [False for i in range(0,26)]
for line in lines:
    for charachter in line:
        if charachter.isalpha():
            letters[ord(charachter) - ord('a')] = True
ordinalOffset = 0
for letter in letters:
    print(chr(ord('a') + ordinalOffset))
    ordinalOffset += 1
