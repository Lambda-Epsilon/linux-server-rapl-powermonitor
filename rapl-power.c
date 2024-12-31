#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MSR_RAPL_POWER_UNIT 0x606
#define MSR_PKG_ENERGY_STATUS 0x611

//arbitrary value, modify at will, only 2 was tested
#define MAX_PACKAGES 2

static int open_msr(int cpu) {
    char msr_filename[32];
    snprintf(msr_filename, sizeof(msr_filename), "/dev/cpu/%d/msr", cpu);
    int fd = open(msr_filename, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open MSR file");
        exit(1);
    }
    return fd;
}

static uint64_t read_msr(int fd, int which) {
    uint64_t data;
    if (pread(fd, &data, sizeof(data), which) != sizeof(data)) {
        perror("Failed to read MSR");
        exit(1);
    }
    return data;
}

static int detect_packages(int *package_map) {
    int total_packages = 0;
    for (int i = 0; i < MAX_PACKAGES; i++) {
        char filename[64];
        snprintf(filename, sizeof(filename), "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", i);
        FILE *fp = fopen(filename, "r");
        if (!fp) {
            break; // No more CPUs to check
        }
        int package_id;
        fscanf(fp, "%d", &package_id);
        fclose(fp);

        if (package_map[package_id] == -1) {
            package_map[package_id] = i;
            total_packages++;
        }
    }
    return total_packages;
}

int main() {
    int package_map[MAX_PACKAGES];
    double energy_unit[MAX_PACKAGES];
    for (int i = 0; i < MAX_PACKAGES; i++) {
        package_map[i] = -1;
        energy_unit[i] = 0.0;
    }

    int total_packages = detect_packages(package_map);
    if (total_packages == 0) {
        fprintf(stderr, "No CPU packages detected. Exiting.\n");
        return 1;
    }

    printf("Detected %d packages:\n", total_packages);
    for (int i = 0; i < total_packages; i++) {
        printf("Package %d mapped to CPU %d\n", i, package_map[i]);
    }

    // --------------------------------
    // Compute the energy units per package
    // --------------------------------
    for (int i = 0; i < total_packages; i++) {
        int fd = open_msr(package_map[i]);
        uint64_t msr_value = read_msr(fd, MSR_RAPL_POWER_UNIT);
        close(fd);

        // We extract bits 12:8 for energy status unit
        // 1/(2^raw_unit). Alternatively, one could use pow(0.5, raw_unit).
        uint64_t raw_unit = (msr_value >> 8) & 0x1F;  // bits [12:8]
        energy_unit[i] = 1.0 / (1 << raw_unit);

        printf("Package %d: Energy unit = %.10f Joules\n", i, energy_unit[i]);
    }

    double initial_energy[MAX_PACKAGES], final_energy[MAX_PACKAGES], power[MAX_PACKAGES];
    int sampling_time = 5;

    for (int i = 0; i < total_packages; i++) {
        int fd = open_msr(package_map[i]);
        initial_energy[i] = read_msr(fd, MSR_PKG_ENERGY_STATUS) * energy_unit[i];
        close(fd);
    }

    printf("Sampling for %d seconds...\n", sampling_time);
    sleep(sampling_time);

    for (int i = 0; i < total_packages; i++) {
        int fd = open_msr(package_map[i]);
        final_energy[i] = read_msr(fd, MSR_PKG_ENERGY_STATUS) * energy_unit[i];
        close(fd);
    }

    printf("Power Consumption:\n");
    double total_power = 0.0;
    for (int i = 0; i < total_packages; i++) {
        power[i] = (final_energy[i] - initial_energy[i]) / sampling_time;
        total_power += power[i];
        printf("Package %d: %.6f Watts\n", i, power[i]);
    }

    printf("Total Power: %.6f Watts\n", total_power);

    return 0;
}
