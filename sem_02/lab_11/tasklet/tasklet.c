#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>
#include <asm/io.h>
#include <linux/ktime.h>

#include <linux/interrupt.h>

#define IRQ_NUM 1

MODULE_LICENSE("GPL");


static int start_pressed = 0x01;
static const char *const press_codes[] = {
    "escape pressed", "1 pressed", "2 pressed", "3 pressed", "4 pressed",
    "5 pressed", "6 pressed", "7 pressed", "8 pressed", "9 pressed",
    "0 (zero) pressed", "- pressed", "= pressed", "backspace pressed",
    "tab pressed", "Q pressed", "W pressed", "E pressed", "R pressed",
    "T pressed", "Y pressed", "U pressed", "I pressed",
    "O pressed", "P pressed", "[ pressed", "] pressed",
    "enter pressed", "left control pressed", "A pressed", "S pressed",
    "D pressed", "F pressed", "G pressed", "H pressed",
    "J pressed", "K pressed", "L pressed", "; pressed",
    "' (single quote) pressed", "` (back tick) pressed",
    "left shift pressed", "\\ pressed",
    "Z pressed", "X pressed", "C pressed", "V pressed",
    "B pressed", "N pressed", "M pressed", ", pressed",
    ". pressed", "/ pressed", "right shift pressed", "(keypad) * pressed",
    "left alt pressed", "space pressed", "CapsLock pressed", "F1 pressed",
    "F2 pressed", "F3 pressed", "F4 pressed", "F5 pressed",
    "F6 pressed", "F7 pressed", "F8 pressed", "F9 pressed",
    "F10 pressed", "NumberLock pressed", "ScrollLock pressed", "(keypad) 7 pressed",
    "(keypad) 8 pressed", "(keypad) 9 pressed", "(keypad) - pressed",
    "(keypad) 4 pressed",
    "(keypad) 5 pressed", "(keypad) 6 pressed", "(keypad) + pressed",
    "(keypad) 1 pressed",
    "(keypad) 2 pressed", "(keypad) 3 pressed", "(keypad) 0 pressed",
    "(keypad) . pressed",
    "F11 pressed", "F12 pressed"
};

static int start_released = 0x81;
static const char *const release_codes[] = {
    "escape released", "1 released", "2 released",
    "3 released", "4 released", "5 released", "6 released",
    "7 released", "8 released", "9 released", "0 (zero) released",
    "- released", "= released", "backspace released", "tab released",
    "Q released", "W released", "E released", "R released",
    "T released", "Y released", "U released", "I released",
    "O released", "P released", "[ released", "] released",
    "enter released", "left control released", "A released", "S released",
    "D released", "F released", "G released", "H released",
    "J released", "K released", "L released", "; released",
    "' (single quote) released", "` (back tick) released", "left shift released", "\\ released",
    "Z released", "X released", "C released", "V released",
    "B released", "N released", "M released", ", released",
    ". released", "/ released", "right shift released", "(keypad) * released",
    "left alt released", "space released", "CapsLock released", "F1 released",
    "F2 released", "F3 released", "F4 released", "F5 released",
    "F6 released", "F7 released", "F8 released", "F9 released",
    "F10 released", "NumberLock released", "ScrollLock released", "(keypad) 7 released",
    "(keypad) 8 released", "(keypad) 9 released", "(keypad) - released", "(keypad) 4 released",
    "(keypad) 5 released", "(keypad) 6 released", "(keypad) + released", "(keypad) 1 released",
    "(keypad) 2 released", "(keypad) 3 released", "(keypad) 0 released", "(keypad) . released",
    "F11 released", "F12 released"
};

static const size_t codes_size = sizeof(release_codes) / sizeof(release_codes[0]);

const char *get_scan_code(unsigned long code)
{
    int pos = code;
    const char **buf;

    if (start_released < code)
    {
        buf = release_codes;
        pos -= start_released;
    }
    else
    {
        buf = press_codes;
        pos -= start_pressed;
    }

    if (codes_size <= pos)
        return "unknown";

    return buf[pos];
}

static int dev_id;
static struct tasklet_struct tasklet;

#define print_tasklet_info(where, how) \
{ \
    ktime_t time = ktime_get_real(); \
    long long h = (time / 3600000000000 + 3) % 24; \
    long long m = time / 60000000000 % 60; \
    long long s = time / 1000000000 % 60; \
    printk("[myirq][" where "][" how "] tasklet info: .state = %lu; .count = %d | time : %lld:%lld:%lld", \
           tasklet.state, atomic_read(&tasklet.count), h, m, s); \
}

void tasklet_handler(struct tasklet_struct *t)
{
    print_tasklet_info("tasklet_handler", "");
    printk("[myirq][tasklet_handler] keyboard scan code: %lu (%s)", t->data,
           get_scan_code(t->data));
}

irqreturn_t irq_handler(int line, void *data)
{
    printk("[myirq][irq_handler] handler line - %d", line);

    if (IRQ_NUM == line)
    {
        tasklet.data = inb(0x60);
        print_tasklet_info("irq_handler", "before schedule");
        tasklet_schedule(&tasklet);
        print_tasklet_info("irq_handler", "after schedule");

        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}

static int __init init_md(void)
{
    int rc = request_irq(IRQ_NUM, irq_handler, IRQF_SHARED, "mykbd-handler-tasklet", &dev_id);

    if (0 != rc)
    {
        printk("[myirq] Unable to request IRQ");
        return 1;
    }

    tasklet_setup(&tasklet, tasklet_handler);
    printk("[myirq] init");
    return 0;
}

static void __exit exit_md(void)
{
    tasklet_kill(&tasklet);
    free_irq(IRQ_NUM, &dev_id);
    printk("[myirq] exit");
}

module_init(init_md);
module_exit(exit_md);

