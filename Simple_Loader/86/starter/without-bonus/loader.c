#include "loader.h"
extern int _start();
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;


// // both the functions are to check the validity of the elf file ( from the refernce given in the lecture slides)
// static inline void *elf_load_rel(Elf32_Ehdr *hdr) {
// 	int result;
// 	result = elf_load_stage1(hdr);
// 	if(result == ELF_RELOC_ERR) {
// 		ERROR("Unable to load ELF file.\n");
// 		return NULL;
// 	}
// 	result = elf_load_stage2(hdr);
// 	if(result == ELF_RELOC_ERR) {
// 		ERROR("Unable to load ELF file.\n");

// 		return NULL;
// 	}
// 	// TODO : Parse the program header (if present)
// 	return (void *)hdr->e_entry;
// }
 
// void *elf_load_file(void *file) {
// 	Elf32_Ehdr *hdr = (Elf32_Ehdr *)file;
// 	if(!elf_check_supported(hdr)) {
// 		ERROR("ELF File cannot be loaded.\n");
// 		return;
// 	}
// 	switch(hdr->e_type) {
// 		case ET_EXEC:
// 			// TODO : Implement
// 			return NULL;
// 		case ET_REL:
// 			return elf_load_rel(hdr);
// 	}
// 	return NULL;
// }

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

    // elf_load_file(fd);
    // elf_load_rel(ehdr);

    // error handling -- checking if the elf file has been read correctly
    if (check1 != (size_t)sizeof(Elf32_Ehdr))
    {
        printf("Error reading ELF header\n");
        return;
    }

    // Iterating through the program headers
    int i = 0;
    int flag1 = 0;
    int flag2 = 0;
    while (i < ehdr->e_phnum)
    {
        //  ehdr->e_phoff gives the offset from where the array of segments start 
        lseek(fd, ehdr->e_phoff + i * ehdr->e_phentsize, SEEK_SET);
        
        // reading the segment one by one 
        ssize_t f = read(fd, phdr, sizeof(Elf32_Phdr));

        // error handling - if the size read is not equal to Elf32_Phdr then throw error
        if (f!=(ssize_t)sizeof(Elf32_Phdr))
        {
           printf("error reading the elf file\n");
           return;
        }

        // we have to see if the type of segment is of pt_load
        if (phdr->p_type == PT_LOAD)
        {
            flag1 = 1;

            // we are seeing if the ehdr->e_entry lies within the segment
            if ((ehdr->e_entry >= phdr->p_vaddr) &&
                (ehdr->e_entry < phdr->p_vaddr + phdr->p_memsz))
            {
                
                flag2 = 1;
                break;
            }
        }
        i++;
    }
    // if both the conditions get true then copy the segment using 
    if (flag1 == 1 && flag2 == 1)
    {

        // copying the segment in the segment addr 
        void *segment_addr = mmap((void *)phdr->p_vaddr,phdr->p_memsz,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_PRIVATE | MAP_FIXED,fd, phdr->p_offset);

        // error handling -- checking if the coying has been done right 
        if (segment_addr == MAP_FAILED)
        {
            printf("segment not being able to get copied");
            loader_cleanup();
            return;
        }

        // Calculate the offset within the segment
        size_t ows = ehdr->e_entry - phdr->p_vaddr;

        // Typecast the address to function pointer and call _start 
        int (*entry_point)() = (int (*)())((char *)segment_addr + ows);
        entry_point();
        int result = entry_point();
        printf("User _start return value = %d\n", result);
    }
}

int main(int argc, char **argv)
{
    printf("The Execution of the main function has started\n");
    if (argc != 2)
    {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }
    printf("Exexcuting file\n");

    // Load and run the ELF executable
    load_and_run_elf(argv);

    // Clean up allocated memory and resources
    // closing the file descriptor
    loader_cleanup();
    return 0;
}