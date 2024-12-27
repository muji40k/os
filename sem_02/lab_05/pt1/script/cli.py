from typing import Callable, Any
from abc import ABC, abstractmethod

class Prompt(ABC):
    @abstractmethod
    def draw(self) -> None:
        pass

class DefaultPrompt(Prompt):
    def draw(self) -> None:
        print("$ ", end="")


class Cli:
    def __init__(self, header="", prompt : Prompt=DefaultPrompt(), silent=False):
        self.header : str = header
        self.entries : list[Callable[[list[str], Any], int]] = []
        self.names : list[str] = []
        self.descriptions : list[list[str]] = []
        self.args : list[Any] = []
        self.prompt : Prompt = prompt
        self.silent = silent


    def add(self, name : str, description : str, entry : Callable[[list[str], Any], int], arg : Any):
        name = name.strip()

        if (name in self.names or name in ["help", "quit"]):
            return

        if (1 != len(name.split())):
            return

        self.names.append(name)
        self.descriptions.append(description.split('\n'))
        self.entries.append(entry)
        self.args.append(arg)


    def mainloop(self) -> int:
        if (not self.silent):
            if (self.header):
                print(self.header)
            print("Меню:")
            self._print_help()
            print()

        run = True
        rc = 0

        while (run and not rc):
            try:
                self.prompt.draw()
                argv = self._get_input()

                i = 0
                while (len(self.names) > i and self.names[i] != argv[0]):
                    i += 1

                if (len(self.names) != i):
                    rc = self.entries[i](argv, self.args[i])
                elif ("help" == argv[0]):
                    self._print_help()
                elif ("quit" == argv[0]):
                    run = False
                else:
                    print("Неизвестная команда")
            except EOFError:
                run = False
                print()

        print("quit")

        return rc


    def _print_help(self):
        maxl = max([len(i) for i in self.names] + [4])

        for i in range(len(self.names)):
            print("{:<{}} | {}".format(self.names[i], maxl, self.descriptions[i][0]))

            for j in range(1, len(self.descriptions[i])):
                print("{:<{}} | {}".format("", maxl, self.descriptions[i][j]))


        print("{:<{}} | {}".format("help", maxl, "Выводит данное сообщение"))
        print("{:<{}} | {}".format("quit", maxl, "Выйти из программы"))


    def _get_input(self, prompt : str = "") -> list[str]:
        inp = input(prompt)
        out = []

        i = False
        for part in inp.split('"'):
            if not i:
                for token in part.strip().split():
                    token = token.strip()

                    if token:
                        out.append(token)
            elif part:
                out.append(part)

            i = not i

        return out

