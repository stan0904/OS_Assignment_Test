# How to Run the OS Simulation

## Prerequisites

- **GCC compiler** (GNU Compiler Collection)
- **Make** utility
- **pthread library** (usually included with GCC on Linux/Unix systems)

## Building the Project

### Step 1: Clean previous builds (optional)
```bash
make clean
```

### Step 2: Build the OS simulation
```bash
make
```

This will:
- Create the `obj/` directory for object files
- Generate `syscalltbl.lst` from `syscall.tbl`
- Compile all source files
- Link everything into the `os` executable

## Running the Simulation

### Basic Usage
```bash
./os [configuration_file]
```

The configuration file path should be relative to the `input/` directory (without the `input/` prefix).

### Example 1: Run with paging enabled
```bash
./os os_1_mlq_paging
```

### Example 2: Run with single CPU
```bash
./os os_1_singleCPU_mlq_paging
```

### Example 3: Run with small memory (1K)
```bash
./os os_1_mlq_paging_small_1K
```

## Input File Format

The configuration file format depends on whether `MM_FIXED_MEMSZ` is defined in `include/os-cfg.h`:

### With MM_FIXED_MEMSZ (Backward Compatibility Mode)
```
[time_slice] [N_CPUs] [M_processes]
[time_0] [path_0] [priority_0]
[time_1] [path_1] [priority_1]
...
[time_M-1] [path_M-1] [priority_M-1]
```

### Without MM_FIXED_MEMSZ (Custom Memory Sizes)
```
[time_slice] [N_CPUs] [M_processes]
[RAM_SZ] [SWP_SZ0] [SWP_SZ1] [SWP_SZ2] [SWP_SZ3]
[time_0] [path_0] [priority_0]
[time_1] [path_1] [priority_1]
...
[time_M-1] [path_M-1] [priority_M-1]
```

**Example** (from `input/os_1_mlq_paging`):
```
2 4 8
268435456 16777216 0 0 0
1 p1s  15
5 m0s  120
7 p0s  130
9 s0 38
11 s3  39
14 m1s  15
16 s2  120
16 s1 0
```

Where:
- Line 1: `2` = time slice, `4` = number of CPUs, `8` = number of processes
- Line 2: Memory sizes (RAM=268435456, SWP0=16777216, SWP1-3=0)
- Lines 3-10: Process entries (start_time, process_file, priority)

## Configuration Options

Before running, you can modify `include/os-cfg.h` to control features:

- **`MM_PAGING`**: Enable/disable paging memory management
- **`MM_FIXED_MEMSZ`**: Enable backward compatibility (fixed memory sizes)
- **`MM64`**: Enable 64-bit memory management structure
- **`MLQ_SCHED`**: Multi-Level Queue Scheduler
- **`IODUMP`**: Enable I/O dump output
- **`PAGETBL_DUMP`**: Enable page table dump

## Troubleshooting

### Build Errors
- **"pthread not found"**: Install pthread development libraries
- **"make: command not found"**: Install make utility
- **"gcc: command not found"**: Install GCC compiler

### Runtime Errors
- **"Cannot find configure file"**: Check that the input file exists in `input/` directory
- **"Error: MEMRAM size must be positive"**: Invalid memory configuration in input file
- **"Error: At least one MEMSWP must be active"**: Need at least one swap with size > 0

## Output

The simulation will output:
- CPU dispatch messages
- Process loading messages
- Memory management operations (if debug flags are enabled)
- Page table dumps (if `PAGETBL_DUMP` is enabled)

Output files may be generated in the `output/` directory depending on configuration.

