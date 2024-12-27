#! /bin/python
FILENAME = "heap2.log"
INTERVALS = 3
PAGES = 2489


def check(values, i) -> bool:
    for j in range(INTERVALS - 1):
        if values[j][i] != values[j + 1][i]:
            return True

    return False


def main():
    file = open(FILENAME, "r");
    values = []

    for j in range(INTERVALS):
        file.readline()
        values.append([])

        for i in range(PAGES):
            values[j].append(file.readline())

    file.close()

    for i in range(PAGES):
        if check(values, i):
            print(i);
            for j in range(INTERVALS):
                print(values[j][i])


if __name__ == "__main__":
    main()

