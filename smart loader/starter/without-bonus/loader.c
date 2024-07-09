#include "loader.h"

#include <signal.h>
extern int _start();
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;


// assuming the max segmets would not go beyond 128
#define MAX_SEGMENTS 128

// defining the page size
#define PAGE_SIZE 4096

// this is the array of segments used to store various things
struct segment {
    unsigned long start;
    unsigned long end;
    int number_of_pages;
    int loaded;
    int qwerty;
    unsigned char *data;
    int on_the_segment;
};

// this is the array of segments
struct segment segments[MAX_SEGMENTS];

// these are for showing the results
int page_faults = 0;
int page_allocations = 0;
float internal_fragmentation = 0;

/*
 * release memory and other cleanups
 */
void loader_cleanup()
{
    // free the value of ehdr and phdr if they are not null
    free(ehdr);
    free(phdr);
    // close the file if it is still open
    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }

    // Unmaping the memory mapped using mmap for loaded segments
    for (int i = 0; i < MAX_SEGMENTS; ++i) {
        if (segments[i].loaded == 1) {
            if (segments[i].data != NULL) {
                munmap(segments[i].data, segments[i].number_of_pages * PAGE_SIZE);
                segments[i].data = NULL;
            }
        }
    }
    // all the cleaning has been done
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **argv)
{
    // Open the ELF file
    fd = open(argv[1], O_RDONLY);

    // error handling - checking if the file is opened correctly
    if (fd < 0)
    {
        printf("Error in opening of ELF file\n");
        return;
    }

    // assigning memory to the ehdr
    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));

    // assigning memory for program headers
    phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);

    // reading the ehdr using the read function
    size_t check1 = read(fd, ehdr, sizeof(Elf32_Ehdr));

    // error handling -- checking if the elf file has been read correctly
    if (check1 != (size_t)sizeof(Elf32_Ehdr))
    {
        printf("Error reading ELF header\n");
        return;
    }

    //  ehdr->e_phoff gives the offset from where the array of segments start
    lseek(fd, ehdr->e_phoff , SEEK_SET);

    // reading the program headers
    ssize_t f = read(fd, phdr, ehdr->e_phnum*sizeof(Elf32_Phdr));

    // error handling - if the size read is not equal to Elf32_Phdr then throw error
    if (f!=(ssize_t)(ehdr->e_phnum*sizeof(Elf32_Phdr)))
    {
        printf("error reading the elf file\n");
        return;
    }


    // storing the addresses of the sphdr in the segments array
    for (int i = 0; i < ehdr->e_phnum; i++) {
        segments[i].start = phdr[i].p_vaddr;
        segments[i].end = phdr[i].p_vaddr + phdr[i].p_memsz;
    }

    // calling the entry point
    int (*entry_point)() = (int (*)())((char *)ehdr->e_entry);
    int result = entry_point();
    printf("User _start return value = %d\n", result);



}
// this is for like first time using mmap arguments different for different cases using this for that
int fgh = 0;
void segfault_handler(int signo, siginfo_t *siginfo, void *context) {
    // updating the page faults and the page allocations
    page_faults++;
//    printf("start of seg fault\n");
    page_allocations++;
    fgh++;

    // error handling
    if (siginfo == NULL) {
        printf("siginfo is NULL\n");
        exit(1);
    }
    // seeing where the seg gault occured
    unsigned long  fault_addr = (unsigned long)siginfo->si_addr;

    // finding in which segment the seg fault occured
    int ind_of_seg = -1;
    for (int i = 0; i < MAX_SEGMENTS; i++)
    {
        if (segments[i].start <= fault_addr && fault_addr < segments[i].end) {
            ind_of_seg = i;
            break;
        }
    }
    segments[ind_of_seg].loaded = 1;

//    printf("index of segment: %d\n\n", ind_of_seg);

    int dtu = segments[ind_of_seg].number_of_pages;
    Elf32_Phdr *current_phdr = &phdr[ind_of_seg];

    // Calculating the page range that have the fault address
    long double page_start = (long double)(current_phdr->p_vaddr);
    long double page_end = page_start + PAGE_SIZE;

    while (fault_addr >= page_end) {
        page_start = page_end;
        dtu++;
        segments[ind_of_seg].qwerty++;
        segments[ind_of_seg].number_of_pages++;
        page_end = page_start + PAGE_SIZE;
//        printf(("inside of while loop \n"));
    }

    // this was for debugging
    //    printf("index inside the segment which needs to loaded: %d\n", dtu);
    //    printf("fault address: %ld  \n", fault_addr);
    //    printf("pg start %Lf \n", page_start);
    //    printf("pg end %Lf \n", page_end);


    if (fgh == 1)
    {
        // these calculations were of without bonus but now have done bonus
        current_phdr = &phdr[ind_of_seg];
        long double s1 = current_phdr->p_vaddr;
        long double s2 = current_phdr->p_vaddr+current_phdr->p_memsz;
        int noa = ((s2-s1)/PAGE_SIZE)+1;
        int len = noa*PAGE_SIZE;

        // doing mmap
        void *segment_addr = mmap((void *)current_phdr->p_vaddr+segments[ind_of_seg].qwerty*PAGE_SIZE,PAGE_SIZE,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_FIXED,fd, current_phdr->p_offset+segments[ind_of_seg].qwerty*PAGE_SIZE);
        segments[ind_of_seg].number_of_pages++;
        // increasing the number of pages alloated to that segment
        dtu++;

        // error handling -- checking if the coying has been done right
        if (segment_addr == MAP_FAILED)
        {
            printf("segment not being able to get copied");
            loader_cleanup();
            return;
        }
        // calculating the internal fragmentation
        if (PAGE_SIZE*dtu > current_phdr->p_memsz)
        {
            long double ert = current_phdr->p_memsz - ((current_phdr->p_memsz)/PAGE_SIZE)*PAGE_SIZE;
            internal_fragmentation+=PAGE_SIZE-ert;
        }
    }
    else
    {
        // similarly as in the if part
        long double s1 = current_phdr->p_vaddr;
        long double s2 = current_phdr->p_vaddr+current_phdr->p_memsz;
        int noa = ((s2-s1)/PAGE_SIZE)+1;
        int len = noa*PAGE_SIZE;
        Elf32_Phdr *current_phdr = &phdr[ind_of_seg];
        void *segment_addr = mmap((void *)current_phdr->p_vaddr+segments[ind_of_seg].qwerty*PAGE_SIZE,PAGE_SIZE,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,fd, current_phdr->p_offset+segments[ind_of_seg].qwerty*PAGE_SIZE);
        segments[ind_of_seg].number_of_pages++;
        dtu++;
        if (PAGE_SIZE*dtu >current_phdr->p_memsz)
        {
            long double ert = current_phdr->p_memsz - ((current_phdr->p_memsz)/PAGE_SIZE)*PAGE_SIZE;
            internal_fragmentation+=PAGE_SIZE-ert;
        }

        // error handling -- checking if the copying has been done right
        if (segment_addr == MAP_FAILED)
        {
            printf("segment not being able to get copied");
            loader_cleanup();
            return;
        }
    }
    segments[ind_of_seg].qwerty = 0;
}



int main(int argc, char **argv)
{
    // constructing the signal for the seg fault handler function for seegmentation fault
    struct sigaction seg_fault;
    seg_fault.sa_sigaction = segfault_handler;
    seg_fault.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &seg_fault, NULL);

    // initializing the argumetns of the struct segment as zero so that they dont creat prbolem
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        segments[i].loaded = 0;
        segments[i].number_of_pages = 0;
        segments[i].qwerty = 0;
        segments[i].data = NULL;
        segments[i].on_the_segment = 0;

    }


    printf("The Execution of the main function has started\n");
    // checking if the arguments in 2 only na
    if (argc != 2)
    {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }
    printf("Exexcuting file\n");

    // Load and run the ELF executable
    load_and_run_elf(argv);


    // showing the page faults , page allocations and the iternal fragmentation
    printf("Page faults: %d\n", page_faults);
    printf("Page allocations: %d\n", page_allocations);
    printf("Internal fragmentation (KB): %f\n", internal_fragmentation / 1024);



    // Clean up allocated memory and resources
    // closing the file descriptor
    loader_cleanup();
    return 0;
}



// if the full segment is to be loaded then the code is below


//#include "loader.h"
//
//#include <signal.h>
//extern int _start();
//Elf32_Ehdr *ehdr;
//Elf32_Phdr *phdr;
//int fd;
//
//
//
//#define MAX_SEGMENTS 32
//#define PAGE_SIZE 4096
//
//struct segment {
//    unsigned long start;
//    unsigned long end;
//    unsigned char *data;
//    int number_of_pages;
//    int loaded;
//    int iscalculated;
//};
//
//struct segment segments[MAX_SEGMENTS];
//int page_faults = 0;
//int page_allocations = 0;
//float internal_fragmentation = 0;
//float total_size_of_process = 0;
//
//
///*
// * release memory and other cleanups
// */
//void loader_cleanup()
//{
//    // free the value of ehdr and phdr if they are not null
//    free(ehdr);
//    free(phdr);
//    // close the file if it is still open
//    if (fd >= 0)
//    {
//        close(fd);
//        fd = -1;
//    }
//    for (int i = 0; i < MAX_SEGMENTS; ++i) {
//        if (segments[i].loaded == 1) {
//            if (segments[i].data != NULL) {
//                munmap(segments[i].data, segments[i].number_of_pages * PAGE_SIZE);
//                segments[i].data = NULL;
//            }
//        }
//    }
//
//
//    // all the cleaning has been done
//}
//
///*
// * Load and run the ELF executable file
// */
//void load_and_run_elf(char **argv)
//{
//    // Open the ELF file
//    fd = open(argv[1], O_RDONLY);
//
//    // error handling - checking if the file is opened correctly
//    if (fd < 0)
//    {
//        printf("Error in opening of ELF file\n");
//        return;
//    }
//
//    // assigning memory to the ehdr
//    ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
//
//    // assigning memory for program headers
//    phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
//
//    // reading the ehdr using the read function
//    size_t check1 = read(fd, ehdr, sizeof(Elf32_Ehdr));
//
//    // error handling -- checking if the elf file has been read correctly
//    if (check1 != (size_t)sizeof(Elf32_Ehdr))
//    {
//        printf("Error reading ELF header\n");
//        return;
//    }
//
//    //  ehdr->e_phoff gives the offset from where the array of segments start
//    lseek(fd, ehdr->e_phoff , SEEK_SET);
//
//    // reading the program headers
//    ssize_t f = read(fd, phdr, ehdr->e_phnum*sizeof(Elf32_Phdr));
//
//    // error handling - if the size read is not equal to Elf32_Phdr then throw error
//    if (f!=(ssize_t)(ehdr->e_phnum*sizeof(Elf32_Phdr)))
//    {
//        printf("error reading the elf file\n");
//        return;
//    }
//
//    for (int i = 0; i < ehdr->e_phnum; i++) {
//        if (phdr[i].p_type == PT_LOAD) {
//            segments[i].loaded = 1;
//            segments[i].start = phdr[i].p_vaddr;
//            segments[i].end = phdr[i].p_vaddr + phdr[i].p_memsz;
//        }
//        else
//        {
//            segments[i].loaded = 0;
//        }
//    }
//    int (*entry_point)() = (int (*)())((char *)ehdr->e_entry);
////    entry_point();
//    int result = entry_point();
//    printf("User _start return value = %d\n", result);
//
//
//
//}
//
//int fgh = 0;
//void segfault_handler(int signo, siginfo_t *siginfo, void *context) {
//    page_faults++;
//    page_allocations++;
//    fgh++;
//    if (siginfo == NULL) {
//        printf("siginfo is NULL\n");
//        exit(1);
//    }
//
//    unsigned long  fault_addr = (unsigned long)siginfo->si_addr;
//    int ind_of_seg = -1;
//
//    for (int i = 0; i < MAX_SEGMENTS; i++)
//    {
//        if (segments[i].start <= fault_addr && fault_addr < segments[i].end) {
//            ind_of_seg = i;
//            break;
//        }
//    }
//
//    int dtu = segments[ind_of_seg].number_of_pages;
//    Elf32_Phdr *current_phdr = &phdr[ind_of_seg];
//    int nsut = current_phdr->p_memsz;
//    if (segments[ind_of_seg].iscalculated == 0)
//    {
//        total_size_of_process+=current_phdr->p_memsz;
//        segments[ind_of_seg].iscalculated = 1;
//    }
//
////    internal_fragmentation=current_phdr->p_vaddr%PAGE_SIZE;
//
//    if (fgh == 1)
//    {
//        current_phdr = &phdr[ind_of_seg];
//        long double s1 = current_phdr->p_vaddr;
//        long double s2 = current_phdr->p_vaddr+current_phdr->p_memsz;
//        int noa = ((s2-s1)/PAGE_SIZE)+1;
//        int len = noa*PAGE_SIZE;
////        internal_fragmentation += len - current_phdr->p_memsz;
//
//
//
////        printf("if page_allocations %d \n", page_allocations);
////        printf("if size of phdr %Lf\n",(s2-s1));
////        printf("if size of PAGE_SIZE%d\n",PAGE_SIZE);
////        printf("if len: %d\n",len);
////        printf("if noa %d\n",noa);
//
//
//        void *segment_addr = mmap((void *)current_phdr->p_vaddr+dtu*PAGE_SIZE,PAGE_SIZE,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_FIXED,fd, current_phdr->p_offset+dtu*PAGE_SIZE);
//        segments[ind_of_seg].number_of_pages++;
//        dtu++;
//        // error handling -- checking if the coying has been done right
//        if (segment_addr == MAP_FAILED)
//        {
//            printf("segment not being able to get copied");
//            loader_cleanup();
//            return;
//        }
//        if (PAGE_SIZE*dtu > current_phdr->p_memsz)
//        {
//            // printf("inside if ocnditon\n");
//            internal_fragmentation+=dtu*PAGE_SIZE-current_phdr->p_memsz;
//        }
//
//        // printf("page size: %d\n",dtu*PAGE_SIZE);
//        // printf("current phdr memsz: %d\n",current_phdr->p_memsz);
//        //  printf("internal fragmentation: %f\n",internal_fragmentation);
//
////        if (noa == 1)
////        {
////            internal_fragmentation+=
////        }
////        internal_fragmentation += len - current_phdr->p_memsz;
////        printf("%f\n",internal_fragmentation);
////        printf("%d\n",len);
////        printf("%d\n",current_phdr->p_memsz);
//
//    }
//    else
//    {
//        long double s1 = current_phdr->p_vaddr;
//        long double s2 = current_phdr->p_vaddr+current_phdr->p_memsz;
//        int noa = ((s2-s1)/PAGE_SIZE)+1;
//        int len = noa*PAGE_SIZE;
//        Elf32_Phdr *current_phdr = &phdr[ind_of_seg];
//
////        printf("else page_allocations %d \n", page_allocations);
////        printf("else size of phdr%Lf\n",(s2-s1));
////        printf("else size of PAGE_SIZE%d\n",(s2-s1));
////        printf("else len: %d\n",len);
////        printf("else noa %d\n",noa);
//
//
//
//        void *segment_addr = mmap((void *)current_phdr->p_vaddr+dtu*PAGE_SIZE,PAGE_SIZE,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,fd, current_phdr->p_offset+dtu*PAGE_SIZE);
//        segments[ind_of_seg].number_of_pages++;
//        dtu++;
//
//        if (PAGE_SIZE*dtu >current_phdr->p_memsz)
//        {
//            //                 printf("inside if ocnditonof else part\n");
//
//            //    printf("page size: %d\n",PAGE_SIZE);
//            //     printf("current phdr memsz: %d\n",current_phdr->p_memsz);
//            //      printf("internal fragmentation: %f\n",internal_fragmentation);
//            internal_fragmentation+= (dtu*PAGE_SIZE-current_phdr->p_memsz);
//            //  printf("internal fragmentation: %f\n",internal_fragmentation);
//
//
//        }
//        // printf("page size: %d\n",PAGE_SIZE);
//        // printf("current phdr memsz: %d\n",current_phdr->p_memsz);
//        //  printf("internal fragmentation: %f\n",internal_fragmentation);
//
//
//        // error handling -- checking if the coying has been done right
//        if (segment_addr == MAP_FAILED)
//        {
//            printf("segment not being able to get copied");
//            loader_cleanup();
//            return;
//        }
//
//
////        internal_fragmentation += len - current_phdr->p_memsz;
////        printf("%f\n",internal_fragmentation);
////        printf("%d\n",len);
////        printf("%d\n",current_phdr->p_memsz);
//    }
////    internal_fragmentation = page_allocations*PAGE_SIZE-total_size_of_process;
//    // printf("total size of all segments: %f\n",total_size_of_process);
//    // printf("page_allocations*PAGE_SIZE %d\n",page_allocations*PAGE_SIZE);
//    printf("\n\none seg fault done\n\n");
//}
//
//
//
//int main(int argc, char **argv)
//{
//    struct sigaction seg_fault;
//    seg_fault.sa_sigaction = segfault_handler;
//    seg_fault.sa_flags = SA_SIGINFO;
//    sigaction(SIGSEGV, &seg_fault, NULL);
//
//
//    for (int i = 0; i < MAX_SEGMENTS; i++) {
//        segments[i].data = NULL;
//        segments[i].loaded = 0;
//        segments[i].number_of_pages = 0;
//        segments[i].iscalculated = 0;
//    }
//    printf("The Execution of the main function has started\n");
//    if (argc != 2)
//    {
//        printf("Usage: %s <ELF Executable>\n", argv[0]);
//        exit(1);
//    }
//    printf("Exexcuting file\n");
//
//    // Load and run the ELF executable
//    load_and_run_elf(argv);
//
//
//
//    printf("Page faults: %d\n", page_faults);
//    printf("Page allocations: %d\n", page_allocations);
//    printf("Internal fragmentation (KB): %f\n", internal_fragmentation / 1024);
//
//
//
//    // Clean up allocated memory and resources
//    // closing the file descriptor
//    loader_cleanup();
//    return 0;
//}
