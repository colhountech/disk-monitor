/*
 * disk-monitor.c
 * Lightweight disk usage monitor with change tracking.
 * Auto-detects physical filesystems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define STATE_DIR      "/var/lib/disk-monitor"
#define STATE_FILE     STATE_DIR "/state.log"
#define CRITICAL_PCT   75
#define SPIKE_PCT      5
#define MIN_SIZE_GB    5
#define MAX_DISKS      16

typedef struct {
    char label[40];
    int  pct;
    char used[32];
    char avail[32];
    int  delta;
    char trend[16];
} DiskInfo;

static void human_size(unsigned long long bytes, char *buf, size_t len) {
    const char *units[] = {"B", "K", "M", "G", "T"};
    int i = 0;
    double size = (double)bytes;

    while (size >= 1024.0 && i < 4) {
        size /= 1024.0;
        i++;
    }
    snprintf(buf, len, "%.1f%s", size, units[i]);
}

static int is_physical_fs(const char *fstype) {
    static const char *ignore[] = {
        "tmpfs", "devtmpfs", "overlay", "squashfs", "proc", "sysfs", NULL
    };
    for (int i = 0; ignore[i]; i++) {
        if (strcmp(fstype, ignore[i]) == 0)
            return 0;
    }
    return 1;
}

int main(void) {
    DiskInfo disks[MAX_DISKS] = {0};
    int count = 0;
    time_t now = time(NULL);
    char date[11];

    strftime(date, sizeof(date), "%Y-%m-%d", localtime(&now));

    if (mkdir(STATE_DIR, 0755) != 0 && errno != EEXIST) {
        perror("Failed to create state directory");
        return 1;
    }

    /* Gather current disks */
    FILE *mf = fopen("/proc/mounts", "r");
    if (!mf) {
        fprintf(stderr, "Cannot open /proc/mounts\n");
        return 1;
    }

    char line[512], device[256], mount[256], fstype[64];
    while (fgets(line, sizeof(line), mf) && count < MAX_DISKS) {
        if (sscanf(line, "%s %s %s", device, mount, fstype) != 3)
            continue;
        if (!is_physical_fs(fstype))
            continue;

        struct statvfs vfs;
        if (statvfs(mount, &vfs) != 0)
            continue;

        unsigned long long total = vfs.f_blocks * vfs.f_frsize;
        if (total < (unsigned long long)MIN_SIZE_GB * 1024ULL * 1024ULL * 1024ULL)
            continue;

        unsigned long long used = total - vfs.f_bavail * vfs.f_frsize;

        disks[count].pct = total ? (int)((used * 100ULL) / total) : 0;
        human_size(used, disks[count].used, sizeof(disks[count].used));
        human_size(vfs.f_bavail * vfs.f_frsize, disks[count].avail, sizeof(disks[count].avail));

        strncpy(disks[count].label,
                (strcmp(mount, "/") == 0) ? "Root (/):" : "Data disk:",
                sizeof(disks[count].label) - 1);
        disks[count].label[sizeof(disks[count].label) - 1] = '\0';

        count++;
    }
    fclose(mf);

    /* Load previous state */
    int prev_pct[MAX_DISKS] = {0};
    int has_prev = 0;

    FILE *sf = fopen(STATE_FILE, "r");
    if (sf) {
        char last_line[256] = "";
        char buf[256];
        while (fgets(buf, sizeof(buf), sf)) {
            strncpy(last_line, buf, sizeof(last_line) - 1);
            last_line[sizeof(last_line) - 1] = '\0';
        }
        fclose(sf);

        if (strlen(last_line) > 0) {
            has_prev = 1;
            char *token = strtok(last_line, " ");
            token = strtok(NULL, " ");   // skip timestamp
            for (int i = 0; token && i < count; i++) {
                prev_pct[i] = atoi(token);
                token = strtok(NULL, " ");
            }
        }
    }

    /* Compute deltas and trends */
    for (int i = 0; i < count; i++) {
        disks[i].delta = has_prev ? (disks[i].pct - prev_pct[i]) : disks[i].pct;

        if (disks[i].delta > 0)
            snprintf(disks[i].trend, sizeof(disks[i].trend), "▲+%d%%", disks[i].delta);
        else if (disks[i].delta < 0)
            snprintf(disks[i].trend, sizeof(disks[i].trend), "▼%d%%", disks[i].delta);
        else
            strcpy(disks[i].trend, "→ 0%%");
    }

    /* Save current state */
    sf = fopen(STATE_FILE, "a");
    if (sf) {
        char ts[20];
        strftime(ts, sizeof(ts), "%Y%m%d%H%M%S", localtime(&now));
        fprintf(sf, "%s ", ts);
        for (int i = 0; i < count; i++) {
            fprintf(sf, "%d ", disks[i].pct);
        }
        fprintf(sf, "\n");
        fclose(sf);
    }

    /* Print */
    printf("🖥️  Disk Monitor — %s\n\n", date);
    for (int i = 0; i < count; i++) {
        printf("%-12s %3d%% used (%s used, %s free) %s\n",
               disks[i].label,
               disks[i].pct,
               disks[i].used,
               disks[i].avail,
               disks[i].trend);
    }

    return 0;
}
