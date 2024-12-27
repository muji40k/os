#! /bin/python
import sys
import json
from dataclasses import dataclass
from threading import Thread, Event

import networkx as nx
import matplotlib.pyplot as plt

from cli import Cli

SCHED_DESC = {0: "SCHED_NORMAL",
              1: "SCHED_FIFO",
              2: "SCHED_RR",
              3: "SCHED_BATCH",
              5: "SCHED_IDLE",
              6: "SCHED_DEADLINE"}

PF_VCPU = 0x00000001
PF_IDLE = 0x00000002
PF_EXITING = 0x00000004
PF_POSTCOREDUMP = 0x00000008
PF_IO_WORKER = 0x00000010
PF_WQ_WORKER = 0x00000020
PF_FORKNOEXEC = 0x00000040
PF_MCE_PROCESS = 0x00000080
PF_SUPERPRIV = 0x00000100
PF_DUMPCORE = 0x00000200
PF_SIGNALED = 0x00000400
PF_MEMALLOC = 0x00000800
PF_NPROC_EXCEEDED = 0x00001000
PF_USED_MATH = 0x00002000
PF_NOFREEZE = 0x00008000
PF_KSWAPD = 0x00020000
PF_MEMALLOC_NOFS = 0x00040000
PF_MEMALLOC_NOIO = 0x00080000
PF_LOCAL_THROTTLE = 0x00100000
PF_KTHREAD = 0x00200000
PF_RANDOMIZE = 0x00400000
PF_NO_SETAFFINITY = 0x04000000
PF_MCE_EARLY = 0x08000000
PF_MEMALLOC_PIN = 0x10000000
PF_SUSPEND_TASK = 0x80000000

FLAGS_DESCRIPTION = {
    PF_VCPU: "I'm a virtual CPU",
    PF_IDLE: "I am an IDLE thread",
    PF_EXITING: "Getting shut down",
    PF_POSTCOREDUMP: "Coredumps should ignore this task",
    PF_IO_WORKER: "Task is an IO worker",
    PF_WQ_WORKER: "I'm a workqueue worker",
    PF_FORKNOEXEC: "Forked but didn't exec",
    PF_MCE_PROCESS: "Process policy on mce errors",
    PF_SUPERPRIV: "Used super-user privileges",
    PF_DUMPCORE: "Dumped core",
    PF_SIGNALED: "Killed by a signal",
    PF_MEMALLOC: "Allocating memory",
    PF_NPROC_EXCEEDED: "set_user() noticed that RLIMIT_NPROC was exceeded",
    PF_USED_MATH: "If unset the fpu must be initialized before use",
    PF_NOFREEZE: "This thread should not be frozen",
    PF_KSWAPD: "I am kswapd",
    PF_MEMALLOC_NOFS: "All allocation requests will inherit GFP_NOFS",
    PF_MEMALLOC_NOIO: "All allocation requests will inherit GFP_NOIO",
    PF_LOCAL_THROTTLE: "Throttle writes only against the bdi I write to, I am cleaning dirty pages from some other bdi.",
    PF_KTHREAD: "I am a kernel thread",
    PF_RANDOMIZE: "Randomize virtual address space",
    PF_NO_SETAFFINITY: "Userland is not allowed to meddle with cpus_mask",
    PF_MCE_EARLY: "Early kill for mce process policy",
    PF_MEMALLOC_PIN: "Allocation context constrained to zones which allow long term pinning.",
    PF_SUSPEND_TASK: "This thread called freeze_processes() and should not be frozen"
}

TASK_STATE = {
        0x00000000: "TASK_RUNNING",
        0x00000001: "TASK_INTERRUPTIBLE",
        0x00000002: "TASK_UNINTERRUPTIBLE",
        0x00000004: "__TASK_STOPPED",
        0x00000008: "TASK_TRACED",
        0x00000010: "EXIT_DEAD",
        0x00000020: "EXIT_ZOMBIE",
        0x00000030: "EXIT_TRACE",
        0x00000040: "TASK_PARKED",
        0x00000080: "TASK_DEAD",
        0x00000100: "TASK_WAKEKILL",
        0x00000200: "TASK_WAKING",
        0x00000400: "TASK_NOLOAD",
        0x00000800: "TASK_NEW",
        0x00001000: "TASK_RTLOCK_WAIT",
        0x00002000: "TASK_FREEZABLE",
        0x00008000: "TASK_FROZEN",
        0x00010000: "TASK_STATE_MAX",
        0x0000ffff: "TASK_ANY",
        0x00000102: "TASK_KILLABLE",
        0x00000104: "TASK_STOPPED",
        0x00000402: "TASK_IDLE",
        0x00000003: "TASK_NORMAL",
        0x0000007f: "TASK_REPORT"
}


def parse_flags(flags: int) -> list[str]:
    out = []

    for flag in FLAGS_DESCRIPTION:
        if (flags & flag):
            out.append(FLAGS_DESCRIPTION[flag])

    return out

def parse_state(state: int) -> list[str]:
    out = []

    for s in TASK_STATE:
        if (s == (state & s)):
            out.append(TASK_STATE[s])

    return out

@dataclass
class Process:
    name : str
    pid : int
    ppid : int
    state : int
    flags : int
    prio : int
    policy : int


def get_process(js : dict[str, str]) -> Process:
    return Process(js["self"], int(js["pid"]), int(js["ppid"]), int(js["state"]),
                   int(js["flags"]), int(js["prio"]), int(js["policy"]))

def print_process(proc: Process) -> None:
    print("name: {:s}\npid: {:d}\nppid: {:d}\nstate: {:d}\n"
          .format(proc.name, proc.pid, proc.ppid, proc.state)
          + "\n".join(["\t" + i for i in parse_state(proc.state)])
          +"\nflags: {:d}\n".format(proc.flags)
          + "\n".join(["\t" + i for i in parse_flags(proc.flags)])
          + "\nprio: {:d}\npolicy: {:s}".format(proc.prio, SCHED_DESC[proc.policy]))


def show_prop(args : list[str], proc_dict : dict[int, Process]) -> int:
    func = {"pid": lambda x: [str(x.pid)],
            "ppid": lambda x: [str(x.ppid)],
            "prio": lambda x: [str(x.prio)],
            "policy": lambda x: [SCHED_DESC[x.policy]],
            "flags" : lambda x: parse_flags(x.flags),
            "state" : lambda x: parse_state(x.state)}

    if (args[1] not in func):
        return 0

    f = func[args[1]]

    for proc in proc_dict.values():
        print(proc.name)

        for desc in f(proc):
            print("\t- {:s}".format(desc))

        print()

    return 0

def group_process(args : list[str], proc_dict : dict[int, Process]) -> int:
    func = {"prio": lambda x: x.prio,
            "policy": lambda x: SCHED_DESC[x.policy]}

    if (args[1] not in func.keys()):
        return 0

    dct = {}
    f = func[args[1]]

    for i in [f(proc) for proc in proc_dict.values()]:
        dct[i] = []

    for p in proc_dict.values():
        dct[f(p)].append(p.name)

    for p in sorted(dct.keys()):
        print(p, "\n", ", ".join(dct[p]), sep="", end="\n\n")

    return 0


def build_tree_cli(_ : list[str], start : Event) -> int:
    start.set()

    return 0


def show_pid(args : list[str], proc_dict : dict[int, Process]) -> int:
    try:
        n = int(args[1])

        if (n in proc_dict):
            print_process(proc_dict[n])
        else:
            print("Информация отсутствует\n")
    except:
        print("Ошибка")

    return 0


def show_name(args : list[str], proc_dict : dict[int, Process]) -> int:
    for proc in proc_dict.values():
        if (proc.name == args[1]):
            print_process(proc)

            return 0

    print("Информация отсутствует\n")

    return 0


def cli_thread(cli: Cli, end : Event) -> None:
    cli.mainloop()
    end.set()


def build_tree(proc_dict : dict[int, Process]) -> None:
    edges = []
    labels = {}

    for proc in proc_dict.values():
        edges.append([proc.ppid, proc.pid])
        labels[proc.pid] = proc.name

    G = nx.Graph()
    G.add_edges_from(edges)

    # pos = nx.spring_layout(G)
    pos = nx.kamada_kawai_layout(G)

    colors = [proc_dict[n].prio if n in proc_dict else 0 for n in G.nodes]

    nx.draw(G, pos, labels=labels, node_color=colors, font_size=8)
    sm = plt.cm.ScalarMappable(norm=plt.Normalize(vmin = 0, vmax=140))

    plt.colorbar(sm)
    plt.show()


def main() -> int:
    proc_dict = {}
    js_file = open(sys.argv[1])

    for proc in json.load(js_file):
        tmp = get_process(proc)
        proc_dict[tmp.pid] = tmp

    js_file.close()

    end = Event()
    start = Event()

    cli = Cli()
    cli.add("show", "", show_prop, proc_dict)
    cli.add("group", "", group_process, proc_dict)
    cli.add("build_tree", "", build_tree_cli, start)
    cli.add("show_pid", "", show_pid, proc_dict)
    cli.add("show_name", "", show_name, proc_dict)

    Thread(target=cli_thread, args=(cli, end,)).start()
    run = True

    while (run):
        end.wait(timeout=0.005)

        if (end.is_set()):
            run = False
        elif (start.is_set()):
            start.clear()
            build_tree(proc_dict)

    return 0


if __name__ == "__main__":
    exit(main())

