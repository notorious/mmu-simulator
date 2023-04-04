# mmu-simulator
The application helps to understand how the memory management unit works.

NOTES:

Optional arguments:

-n N: Processes only the first N memory accesses/references. If not present, the program processes all addresses. If an out-of-range number (<0) is specified, the program prints "Number of memory accesses must be a number, greater than or equal to 0" to the standard output (or standard error stderr) and exits.

-c N: Sets the cache capacity of the TLB, i.e., the maximum number of page mapping entries (N) in the TLB. Default is 0 if not specified, meaning NO TLB caching. If an out-of-range number (<0) is specified, the program prints "Cache capacity must be a number, greater than or equal to 0" to the standard output (or standard error stderr) and exits.

-p mode: Selects a print mode to display different information. The available modes are:

levelbitmasks: Writes out the bitmasks for each level starting with the lowest tree level (root node is at level 0), one per line. In this mode, you do not need to actually process any addresses. The program prints bitmasks and exits. (Use report_levelbitmasks.)

va2pa: Shows virtual address translation to physical address for every address, one address translation per line. (Use report_virtualAddr2physicalAddr.)

va2pa_tlb_ptwalk: Shows virtual to physical translation for every address, lookup TLB then pagetable walk if TLB misses, one address translation per line. (Use report_va2pa_TLB_PTwalk.)

vpn2pfn: For every virtual address, shows its virtual page numbers for each level followed by the frame number, one address per line. (Use report_pagetable_map.)

offset: Shows offsets of virtual addresses (excluding the virtual page number), one address offset per line. (Use hexnum.)

summary: Shows summary statistics. This is the default argument if -p is not specified. (Use report_summary.) Statistics reported include the page size, number of addresses processed, hit and miss rates for tlb and pagetable walk, number of frames allocated, total bytes required for page table (hint: use sizeof). You should get a roughly accurate estimate of the total bytes used for the page table, including data used in all page tree levels. Note that your calculated number may not match the number of total bytes in sample_output.txt (should be close though), as you may not have strictly the same data members in your structures as in the solution code, which is fine. But you should be aware that, in general, with more paging levels, less total bytes would normally be used.

Mandatory arguments:

The first mandatory argument is the name of the trace file consisting of memory reference traces for simulating a series of attempts of accessing virtual/logical addresses.

trace.tr is given for your testing.

Appropriate error handling should be present if the file is nonexistent or cannot be opened. The program must print "Unable to open <<trace.tr>>" to standard error output (stderr) in this case.

The remaining mandatory arguments are the number of bits to be used for each level. The number of bits for any level MUST be greater than or equal to 1. If not, the program prints "Level 0 page table must be at least 1 bit" (or a similar message for other levels) to standard error output (stderr) with a line feed and exits. The total number of bits from all levels should be less than or equal to 28 (<= 28). If not, the program prints "Too many bits used in page tables" to standard error output (stderr) and exits.
