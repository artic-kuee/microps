#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

#include "util.h"
#include "net.h"

#include "driver/null.h"
#include "driver/loopback.h"

#include "test.h"

static volatile sig_atomic_t terminate;

static void
on_signal(int s)
{
    (void)s;
    terminate = 1;
}

int
main(int argc, char *argv[])
{
    int opt, noop = 0;
    struct net_device *dev;

    /*
     * Parse command line parameters
     */
    while ((opt = getopt(argc, argv, "n")) != -1) {
        switch (opt) {
        case 'n':
            noop = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-n]\n", argv[0]);
            return -1;
        }
    }
    switch (argc - optind) {
    case 0:
        break;
    default:
        fprintf(stderr, "Usage: %s [-n]\n", argv[0]);
        return -1;
    }
    /*
     * Setup protocol stack
     */
    signal(SIGINT, on_signal);
    if (net_init() == -1) {
        errorf("net_init() failure");
        return -1;
    }
    dev = null_init();
    if (!dev) {
        errorf("null_init() failure");
        return -1;
    }
    dev = loopback_init();
    if (!dev) {
        errorf("loopback_init() failure");
        return -1;
    }
    if (net_run() == -1) {
        errorf("net_run() failure");
        return -1;
    }
    /*
     * Test Code
     */
    while (!terminate) {
        if (!noop) {
            if (net_device_output(dev, 0x0800, test_data, sizeof(test_data), NULL) == -1) {
                errorf("net_device_output() failure");
                break;
            }
        }
        sleep(1);
    }
    /*
     * Cleanup protocol stack
     */
    net_shutdown();
    return 0;
}