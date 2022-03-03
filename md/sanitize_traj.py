if __name__ == "__main__":
    with open("trajectory1.xyz") as f:
        for line in f.readlines():
            if not line:
                continue
            if line[0].isdigit():
                continue
            if line.startswith("#"):
                continue
            else:
                print(line.strip("\n"))
