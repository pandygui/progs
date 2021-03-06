#include <unistd.h>

#ifdef FIND_SO_LIB
#define _GNU_SOURCE 
#include <link.h> // must put the head line, if not get link error.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/mman.h>



#include "type.h"
#include "elf.h"
#include "mype.h"

#define STR_NUM 30


// coff global variable

#define COFF_SECTION_NUM 10

IMAGE_SYMBOL *coff_sym_addr;
int coff_sec_num;
IMAGE_SECTION_HEADER *coff_sections[COFF_SECTION_NUM];

IMAGE_SECTION_HEADER *get_coff_section_by_secname(const char *sec_name);

//
//  elf global variable

u32 printf_addr;
u32 next_i_addr;
s32 call_offset;

u32 symbol_table_addr;
u32 hello_val;
u32 text_offset;

typedef struct StrTabData_
{
  char  *str_;
  u32 len_;
}StrTabData;

StrTabData symbol_name[STR_NUM];

int sec_num;

u8 *section_string;
u8 *symbol_string;


char *sec_names[STR_NUM];
u32 sec_offset[STR_NUM];

int get_section_index(const char *section_name)
{
  for (int i = 0 ; i < sec_num ; ++i)
  {
    if (strcmp(section_name, sec_names[i]) == 0)
    {
      return i;	      
    }
  }
  return -1;
}

char *get_rel_section_name(char *rel_section_name)
{
  const char *pattern_len = ".rel";
  if (strstr(rel_section_name, pattern_len))
  {
    return rel_section_name + strlen(pattern_len);
  }
  else
    return 0;
}

u8 *hello_addr;

int shstrndx;

typedef struct SymbolData_
{
  u32 offset;
  u32 addr;
  u32 section_index;
}SymbolData;


int lookup_string_section(u8 *section_addr, u32 section_num)
{
  StrTabData *str_tab;
  Elf32Shdr *shdr = (Elf32Shdr*)(section_addr);
  for (int i=0 ; i < section_num ; ++i)
  {
    if (shdr->sh_type == 3) // string section
    {
      if (shstrndx == i) 
      {
        printf("section name string table\n");
        //str_tab = section_name;
        section_string = hello_addr+shdr->sh_offset;
      }
      else
      {
        //str_tab = symbol_name;
        symbol_string = hello_addr+shdr->sh_offset;
      }

      char *ptr = hello_addr+shdr->sh_offset;
      int ptr_index=0;
      int total_len=0;

      printf("found string section: %d\n", i);
      printf("shdr->sh_size: %#x\n", shdr->sh_size);
      printf("shdr->sh_offset: %#x\n", shdr->sh_offset);
      while(1)
      {
        int len = strlen(ptr);
        printf("ptr: %s ## ptr index: %x ## len: %u\n", ptr, total_len, len);
        total_len += len + 1;


        if (total_len >= shdr->sh_size)
          break;

        ptr += (len + 1);

      }
      //printf("shdr->sh_entsize: %#x\n", shdr->sh_entsize);

      //int ent_num = shdr->sh_size/shdr->sh_entsize;
    }
    ++shdr;
  }
}

Elf32Sym *lookup_symbol(u8 *section_addr, u32 section_num, const char* symbol_name)
{
  Elf32Shdr *shdr = (Elf32Shdr*)(section_addr);
  for (int i=0 ; i < section_num ; ++i)
  {
    if (shdr->sh_type == 2) // symbol table
    {
      printf("found symbol section: %d\n", i);
      int ent_num = shdr->sh_size/shdr->sh_entsize;
      Elf32Sym *sym = (Elf32Sym*)(hello_addr + shdr->sh_offset);
      for (int j=0 ; j < ent_num ; ++j)
      {
        if (symbol_name != 0)
        {
          if (strcmp(symbol_name, symbol_string + sym->st_name) == 0)
          {
            //printf("st_shndx: %#x ", sym->st_shndx);
            return sym;
          }
        }
        printf("index: %d ", j);
        printf("name[%#x]: %s ", sym->st_name, symbol_string + sym->st_name);
        printf("val: %#x ", sym->st_value);
        printf("st_shndx: %#x ", sym->st_shndx);
        printf("size: %#x\n", sym->st_size);

        ++sym;
      }
    }

    ++shdr;
  }
  return 0;
}

Elf32Sym *lookup_symbol_by_index(u32 symbol_index)
{
// need check symbol_index is valid.
  //Elf32_Sym *sym = (Elf32_Sym*)(symbol_table_addr) + symbol_index;
  return (Elf32Sym*)(symbol_table_addr) + symbol_index;
}

SymbolData lookup_symbol_data(u8 *section_addr, u32 section_num, u32 symbol_index)
{
  // find symbol section first.

  Elf32Shdr *shdr = (Elf32Shdr*)(section_addr);
  for (int i=0 ; i < section_num ; ++i)
  {
    if (shdr->sh_type == 2) // symbol table
    {
#if 0
    printf("#%d type : %x\n", i, shdr->sh_type);
    printf("#%d shoff: %x\n", i, shdr->sh_offset);
    printf("#%d size: %x\n", i, shdr->sh_size);
    printf("#%d entsize: %x\n", i, shdr->sh_entsize);
#endif
      int ent_num = shdr->sh_size/shdr->sh_entsize;
      Elf32Sym *sym = (Elf32Sym*)(hello_addr + shdr->sh_offset) + symbol_index;
      SymbolData symbol_data;

      symbol_data.addr = sym->st_value;
      symbol_data.section_index = sym->st_shndx;
      return symbol_data;

#if 0
      for (int j=0 ; j < ent_num ; ++j)
      {
	printf("#%d st_value: %x\n", j, sym->st_value);
	printf("st_size: %x\n", sym->st_size);
	printf("st_shndx: %x\n", sym->st_shndx);
        ++sym;
      }
#endif
    }
    ++shdr;
  }


}


#define SECTION_NUM 100
u32 section_offset[SECTION_NUM];

u32 libc_addr;

#ifdef FIND_SO_LIB
int print_callback(struct dl_phdr_info *info, size_t size, void *data)
{
  if (strstr(info->dlpi_name, "libc"))
  {
    printf("%08x %s\n", info->dlpi_addr, info->dlpi_name);
    libc_addr = info->dlpi_addr;
  }
  return 0;
}
#endif

u32 get_symbol_offset_by_name(IMAGE_SYMBOL *sym)
{
  //printf("get %s offset\n", sym_name);
}

IMAGE_SECTION_HEADER *get_coff_section_by_index(int index)
{
  return coff_sections[index];
}

int size;

int load_coff(const char *fn)
{
  IMAGE_FILE_HEADER *coff_hdr = (IMAGE_FILE_HEADER*)hello_addr;
  printf("Machine: %#x\n", coff_hdr->Machine);
  printf("NumberOfSections: %#x\n", coff_hdr->NumberOfSections);
  printf("PointerToSymbolTable: %#x\n", coff_hdr->PointerToSymbolTable);
  printf("NumberOfSymbols: %#x\n", coff_hdr->NumberOfSymbols);

  IMAGE_SYMBOL *sym = (IMAGE_SYMBOL*)(hello_addr + coff_hdr->PointerToSymbolTable);
  coff_sym_addr = sym;
  for (int i=0 ; i < coff_hdr->NumberOfSymbols ; ++i)
  {
    printf("#%d\n", i);
    if (strcmp(sym->N.ShortName, "_hello")==0)
      hello_val = sym->Value;
    if (sym->N.Name.Short != 0)
      printf("ShortName: %s\n", sym->N.ShortName);
    else
      printf("use long name\n");
    printf("Value: %#x\n", sym->Value);
    printf("SectionNumber %#x\n", sym->SectionNumber);
    printf("Type %#x\n", sym->Type);
    ++sym;
  }

  IMAGE_SECTION_HEADER *coff_section_hdr = (IMAGE_SECTION_HEADER*)(hello_addr + sizeof(IMAGE_FILE_HEADER) );
  coff_sec_num = coff_hdr->NumberOfSections;
  for (int i=0 ; i < coff_hdr->NumberOfSections ; ++i)
  {
    coff_sections[i] = coff_section_hdr;
    if (strcmp(".text", coff_section_hdr->Name)==0)
      text_offset = coff_section_hdr->PointerToRawData;
    ++coff_section_hdr;
  }

  coff_section_hdr = (IMAGE_SECTION_HEADER*)(hello_addr + sizeof(IMAGE_FILE_HEADER) );

  for (int i=0 ; i < coff_hdr->NumberOfSections ; ++i)
  {
    printf("section name: %s\n", coff_section_hdr->Name);
    printf("SizeOfRawData: %#x\n", coff_section_hdr->SizeOfRawData);
    printf("PointerToRawData: %#x\n", coff_section_hdr->PointerToRawData);
    printf("PointerToRelocations: %#x\n", coff_section_hdr->PointerToRelocations);
    printf("VirtualAddress: %#x\n", coff_section_hdr->VirtualAddress);

    if (coff_section_hdr->PointerToRelocations != 0)
    {
      printf("NumberOfRelocations: %d\n", coff_section_hdr->NumberOfRelocations);
      IMAGE_RELOCATION *rel = (IMAGE_RELOCATION*)(hello_addr + coff_section_hdr->PointerToRelocations);
      for (int i=0 ; i < coff_section_hdr->NumberOfRelocations ; ++i)
      {
        printf("#%d ## DUMMYUNIONNAME.VirtualAddress: %#x\n", i, rel->DUMMYUNIONNAME.VirtualAddress);
        printf("SymbolTableIndex: %#x\n", rel->SymbolTableIndex);
        printf("Type: %#x\n", rel->Type);
        IMAGE_SYMBOL *sym_offset = coff_sym_addr + rel->SymbolTableIndex;
        printf("rel symbol name: %s\n", sym_offset->N.ShortName);
        printf("StorageClass: %#x\n", sym_offset->StorageClass);
        printf("sym type: %#x\n", sym_offset->Type);
        printf("sym value: %#x\n", sym_offset->Value);
        printf("sym NumberOfAuxSymbols: %#x\n", sym_offset->NumberOfAuxSymbols);

        u32 modify_addr = (u32)(hello_addr + coff_section_hdr->PointerToRawData + rel->DUMMYUNIONNAME.VirtualAddress);
        printf("modify offset: %#x\n", coff_section_hdr->PointerToRawData + rel->DUMMYUNIONNAME.VirtualAddress);
        printf("modify addr: %#x\n", modify_addr);


        IMAGE_SYMBOL *sym = coff_sym_addr + rel->SymbolTableIndex;
        printf("sym in %d section\n", sym->SectionNumber);

        if (strcmp(sym_offset->N.ShortName, "_printf") == 0)
        {
          next_i_addr = modify_addr + 4;
          call_offset = printf_addr - next_i_addr;
          *((u32*)modify_addr) = call_offset;

          printf("relocation coff win32 printf addr to linux glibc printf\n");
        }
        else
        {
          if (sym_offset->StorageClass == 3)
          {
            //IMAGE_SECTION_HEADER *modify_sec = get_coff_section_by_secname(sym_offset->N.ShortName);
            IMAGE_SECTION_HEADER *sec_hdr = get_coff_section_by_index(sym->SectionNumber - 1);
            #if 0
            if (modify_sec != 0)
            {
              printf("modify_sec off: %#x\n", modify_sec->PointerToRawData);
              *((u32*)modify_addr) = (hello_addr + modify_sec->PointerToRawData);
            }
            #endif

                 if (rel->Type == 0x6)
                 {
                   s32 modify_val = (hello_addr + sec_hdr->PointerToRawData + sym->Value);
                   *((u32*)modify_addr) = modify_val;
                 }

          }
          else if (sym_offset->StorageClass == 2)
               {
                 // find _func address and _i
                 printf("sym val  %#x\n", sym->Value);
                 IMAGE_SECTION_HEADER *sec_hdr = get_coff_section_by_index(sym->SectionNumber - 1);
                 printf("%s section offset is : %#x\n", sec_hdr->Name, sec_hdr->PointerToRawData);
                 // next instruct addr - func addr 

                 if (rel->Type == 0x14)
                 {
                   next_i_addr = modify_addr + 4;
                   s32 modify_val = (hello_addr + sec_hdr->PointerToRawData + sym->Value) - next_i_addr;
                   *((u32*)modify_addr) = modify_val;
                 }

                 if (rel->Type == 0x6)
                 {
                   s32 modify_val = (hello_addr + sec_hdr->PointerToRawData + sym->Value);
                   *((u32*)modify_addr) = modify_val;
                 }

               }

        }

        ++rel;
      }
    }

    ++coff_section_hdr;
  }
}


int load_elf(const char *fn)
{
#if 0
  FILE *fs;
  fs = fopen("./hello.o", "r");
  fseek(fs, 0, SEEK_END);
  size = ftell(fs);
  fseek(fs, 0, SEEK_SET);

  u32 align_addr;

  hello_addr = (u8*)malloc(size);
  printf("hello addr: %p\n", hello_addr);
  align_addr = (u32)hello_addr;
  printf("align addr: %x\n", align_addr);

  align_addr &= 0xfffff000;
  align_addr += 0x1000;
  hello_addr = align_addr;
  printf("align 0x1000 hello addr: %p\n", hello_addr);
  fread(hello_addr, 1, size, fs);
  fclose(fs);
#endif
  u8 *ptr = hello_addr;

  Elf32Ehdr *elf_hdr = (Elf32Ehdr*)ptr;

  printf("shoff: %x\n", elf_hdr->e_shoff);
  printf("shnum: %x\n", elf_hdr->e_shnum);
  printf("e_shstrndx: %d\n", elf_hdr->e_shstrndx);

  if (1 == elf_hdr->e_ident[4])
  {
    printf("32bit elf\n");
  }
  else if (2 == elf_hdr->e_ident[4])
       {
         printf("64bit elf\n");
         printf("don't support 64bit elf\n");
         return -1;
       }
       else
       {
         printf("unknow elf\n");
         return -1;
       }

  sec_num = elf_hdr->e_shnum;

  shstrndx = elf_hdr->e_shstrndx;

  Elf32Shdr *shdr = (Elf32Shdr*)(hello_addr + elf_hdr->e_shoff);
  Elf32Shdr *shdr_addr = (Elf32Shdr*)(hello_addr + elf_hdr->e_shoff);

  lookup_string_section((u8*)shdr_addr, elf_hdr->e_shnum);
  lookup_symbol((u8*)shdr_addr, elf_hdr->e_shnum, 0);

  lookup_symbol((u8*)shdr_addr, elf_hdr->e_shnum, "printf");

  Elf32Sym *sym = lookup_symbol((u8*)shdr_addr, elf_hdr->e_shnum, "hello");

  hello_val = sym->st_value;
  printf("hello_val: %#x\n", hello_val);

  text_offset;
  for (int i=0 ; i < elf_hdr->e_shnum ; ++i)
  {
    printf("#%d section_name[%#x]: %s\n", i, shdr->sh_name, section_string+(shdr->sh_name));
    sec_names[i] = section_string+(shdr->sh_name);
    sec_offset[i] = shdr->sh_offset;
    section_offset[i] = shdr->sh_offset;
    if (shdr->sh_type == 2) // symbol table
      symbol_table_addr = hello_addr + shdr->sh_offset;



    //printf("section_offset[%d]: %#x\n", i, section_offset[i]);
    ++shdr;
  }

  for (int i=0 ; i < elf_hdr->e_shnum ; ++i)
  {
    printf("sec name[%d]: %s\n", i, sec_names[i]);
  }

  shdr = (Elf32Shdr*)(hello_addr + elf_hdr->e_shoff);

  for (int i=0 ; i < elf_hdr->e_shnum ; ++i)
  {
    //printf("#%d shdr : %p\n", i, (u8*)shdr-hello_addr);
    

#if 0
    if (i == 1) // .text section
    {
      printf("text offset: %#x\n", shdr->sh_offset);
      text_offset = shdr->sh_offset;
    }
#endif

    // fixed me: the version do relocation for .text, do nothing for .rel.eh_frame/.eh_frame.
    // I don't know what is .rel.eh_frame/.eh_frame.
    if ( (shdr->sh_type == 9) && (strcmp(".rel.text", section_string+shdr->sh_name)==0) )
    {
      printf("rel section name: %s\n", section_string+shdr->sh_name); 
      char *rel_section_name = get_rel_section_name(section_string+shdr->sh_name);
      printf("need rel section name: %s\n", rel_section_name);

      int rel_sec_idx = get_section_index(rel_section_name);

      if (rel_sec_idx == -1)
      {
        printf("rel_sec_idx = -1\n");
        exit(1);	       
      }
      printf("rel_sec_idx: %d\n", rel_sec_idx);
      printf("rel_sec_offset: %#x\n", sec_offset[rel_sec_idx]);
      text_offset = sec_offset[rel_sec_idx];

      //exit(-1);

      printf("section_offset[1]: %#x\n", section_offset[1]);
      printf("section_offset[3]: %#x\n", section_offset[3]);
    printf("#%d name : %x\n", i, shdr->sh_name);
    printf("#%d type : %x\n", i, shdr->sh_type);
    printf("#%d shoff: %x\n", i, shdr->sh_offset);
    printf("#%d size: %x\n", i, shdr->sh_size);
    printf("#%d entsize: %x\n", i, shdr->sh_entsize);


      int ent_num = shdr->sh_size/shdr->sh_entsize;
      Elf32Rel *rel = (Elf32Rel*)(hello_addr + shdr->sh_offset);
      for (int j=0 ; j < ent_num ; ++j)
      {
        printf("#j: %d\n", j);
	printf("r_offset: %x\n", rel->r_offset);
	printf("r_info: %x\n", rel->r_info);
	u8 class = rel->r_info & 0xff;
	u8 symbo_index = ((rel->r_info >> 8 ) & 0xffffff);
        SymbolData symbol_data = lookup_symbol_data((u8*)shdr_addr, elf_hdr->e_shnum, symbo_index);
	symbol_data.offset = rel->r_offset;
	printf("class: %x\n", class);
	printf("symbo_index: %x\n", symbo_index);
	//printf("symbo name: %s\n", symbol_string+symbo_index);
	printf("symbol_data.offset: %x\n", symbol_data.offset);
	printf("symbol_data.addr: %x\n", symbol_data.addr);

	printf("section_offset[1]: %x\n", section_offset[1]);
	printf("section_offset[3]: %x\n", section_offset[3]);

	printf("symbol_data.section_index: %x\n", symbol_data.section_index);
	printf("modify addr: %#x\n", text_offset + symbol_data.offset);
        printf("modify value: %#x\n", section_offset[symbol_data.section_index] + symbol_data.addr);

        Elf32Sym *sym  = lookup_symbol_by_index(symbo_index);
        printf("sym st_info: %#x\n", sym->st_info);
        printf("sym st_info bind: %#x\n", sym->st_info >> 4);
        printf("sym st_info type: %#x\n", sym->st_info & 0xf);

        if ((sym->st_info & 0xf) != 3)
          printf("rel symbol name: %s\n", symbol_string + sym->st_name);
        // ref: http://docs.oracle.com/cd/E19683-01/816-1386/6m7qcoblj/index.html#chapter6-tbl-21
        switch (sym->st_info & 0xf)
        {
          case 0:
          {
            printf("notype\n");
            break;
          }
          case 1:
          {
            printf("object\n");
            break;
          }
          case 2:
          {
            printf("func\n");
            break;
          }
          case 3:
          {
            printf("section\n");
            //printf("rel symbol name: %s\n", section_string + ((Elf32Shdr*)(hello_addr+section_offset[symbol_data.section_index]))->sh_name );
            break;
          }
          case 4:
          {
            printf("file\n");
            break;
          }
          default:
          {
            printf("current not support");
            break;
          }
        }



        if (strcmp(symbol_string + sym->st_name, "printf") == 0)
        {
          printf("relocate printf\n");
            s32 org_val = *(u32*)(hello_addr + text_offset + symbol_data.offset);
	    printf("org val: %d\n", org_val);
            next_i_addr = (u32)((hello_addr + text_offset + symbol_data.offset) - org_val);
            printf("next instruct addr: %#x\n", next_i_addr);
            printf("printf addr: %#x\n", printf_addr);
            call_offset = printf_addr - next_i_addr;
            *(u32*)(hello_addr + text_offset + symbol_data.offset) = call_offset;
            printf("call offset: %#x\n", call_offset);
            //exit(0);


        }
        else
        {



	switch (class)
	{
          case 1:
	  {
            *(u32*)(hello_addr + text_offset + symbol_data.offset) += hello_addr + section_offset[symbol_data.section_index] + symbol_data.addr;
            break;
	  }
          case 2:
	  {
            s32 org_val = *(u32*)(hello_addr + text_offset + symbol_data.offset);

            s32 modify_val = hello_addr + section_offset[symbol_data.section_index] + symbol_data.addr + org_val - (u32)(hello_addr + text_offset + symbol_data.offset);
            printf("modify_val : %#x\n", modify_val);

            *(u32*)(hello_addr + text_offset + symbol_data.offset) = hello_addr + section_offset[symbol_data.section_index] + symbol_data.addr + org_val - (u32)(hello_addr + text_offset + symbol_data.offset);
            printf("text_offset: %#x\n", text_offset);

            //*(s32*)(hello_addr + text_offset + symbol_data.offset) = 0xffffffde;
            printf("1 %#x\n", (u32)(hello_addr + text_offset + symbol_data.offset));

            printf("func addr %#x\n", hello_addr + section_offset[symbol_data.section_index] + symbol_data.addr);
            printf("func offset %#x\n", section_offset[symbol_data.section_index] + symbol_data.addr);
	    printf("org val: %d\n", org_val);

            printf("val %#x\n", hello_addr + section_offset[symbol_data.section_index] + symbol_data.addr + org_val - (u32)(hello_addr + text_offset + symbol_data.offset));
            //*(u32*)(hello_addr + text_offset + symbol_data.offset) = 0xdeffffff;
            break;
	  }
          default:
          {
            printf("not support class: %x\n", class);
            break;
          }


          
	}

        }


        ++rel;
      }
      #if 0
        SymbolData symbol_data = lookup_symbol_data((u8*)shdr_addr, elf_hdr->e_shnum, 9);
	printf("sym9 symbol_data.addr: %x\n", symbol_data.addr);
	printf("sym9 symbol_data.section_index: %x\n", symbol_data.section_index);
        #endif
    }
    ++shdr;
  }

  //unsigned int addr = 0;

}


IMAGE_SECTION_HEADER *get_coff_section_by_secname(const char *sec_name)
{
  for (int i=0 ; i < coff_sec_num ; ++i)
  {
    if (strcmp(sec_name, coff_sections[i]->Name)==0)
      return coff_sections[i];
  }
  return 0;
}

void usage(const char *fn)
{
  printf("%s fn\n", fn);
}

int is_elf(const char *load_obj)
{
  u8 elf_pattern[] = {0x7f, 0x45, 0x4c, 0x46};
  if (memcmp(load_obj, elf_pattern, 4) == 0)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

int is_win32_coff(const char *load_obj)
{
  //u8 coff_pattern[] = {0x4c, 0x01};
  if (load_obj[0] == 0x4c && load_obj[1] == 0x01)
    return 0;
  else
    return 1;
}

int main(int argc, const char *argv[])
{
#if 1
  if (argc <= 1)
  {
    usage(argv[0]);
    return 1;
  }
#endif

#ifdef FIND_SO_LIB
  dl_iterate_phdr(print_callback, NULL);
  printf("libc addr: %x\n", libc_addr);

  if (libc_addr != 0)
  {
    Elf32Ehdr *elf_hdr = (Elf32Ehdr*)libc_addr;
    printf("shoff: %x\n", elf_hdr->e_shoff);
    printf("shnum: %x\n", elf_hdr->e_shnum);
    printf("e_shstrndx: %d\n", elf_hdr->e_shstrndx);
  }
#endif


  printf_addr = &printf;
  printf("printf addr: %x\n", printf_addr);
  //u32 hello_val = lookup_symbol((u8*)shdr_addr, elf_hdr->e_shnum, "hello");

  
  FILE *fs;
  fs = fopen(argv[1], "r");
  //fs = fopen("./hello.coff", "r");
  fseek(fs, 0, SEEK_END);
  size = ftell(fs);
  fseek(fs, 0, SEEK_SET);

  size += 4096;
  u32 align_addr;

  hello_addr = (u8*)malloc(size);
  printf("hello addr: %p\n", hello_addr);
  align_addr = (u32)hello_addr;
  printf("align addr: %x\n", align_addr);

  align_addr &= 0xfffff000;
  align_addr += 0x1000;
  hello_addr = align_addr;
  printf("align 0x1000 hello addr: %p\n", hello_addr);
  fread(hello_addr, 1, size, fs);
  fclose(fs);

  if (is_elf(hello_addr) == 0)
  {
    printf("load elf object: %s\n", argv[1]);
    if (-1 == load_elf(argv[1]))
    {
      return -1;
    }
  }
  else if (is_win32_coff(hello_addr) == 0)
       {
         printf("load win coff object: %s\n", argv[1]);
         load_coff(argv[1]);
       }

#if 1
  errno = 0;
  int pagesize = (int)sysconf(_SC_PAGESIZE);
  printf("pagesize: %d\n", pagesize);
  if (mprotect(hello_addr, size, PROT_EXEC|PROT_READ|PROT_WRITE) == 0)
  {
    //typedef void *Fptr();
    //Fptr fp;

    //(Fptr)(hello_addr + text_offset + hello_val);

    // function ptr, call instruct
    (*(void(*)())(hello_addr + text_offset + hello_val) )();
    
    // this not call instruct is jmp, can not return.
    //goto *(hello_addr + text_offset + hello_val);
  }
  else
  {
    switch (errno)
    {
      case EACCES:
      {
        printf("1\n");
        break;
      }
      case EINVAL:
      {
        printf("2\n");
        break;
      }
      case ENOMEM:
      {
        printf("3\n");
        break;
      }
    }
    perror("mprotect err\n");
  }
#endif
  return 0;
  
}

// find rel.text and rel.data
// inf rel.text/rel.data offset in object file.
// from 0x3d4, size: 0x10
// 16 00 00 00  r_offset
// 01 08 00 00  r_info
// 1e 00 00 00
// 02 09 00 00 

#if 0
I need know i, func symbol offset in hello.o
RELOCATION RECORDS FOR [.text]:
OFFSET   TYPE              VALUE 
00000016 R_386_32          i
0000001e R_386_PC32        func

     8: 00000000     4 OBJECT  GLOBAL DEFAULT    3 i
     9: 00000000    15 FUNC    GLOBAL DEFAULT    1 func

  [ 3] .data             PROGBITS        00000000 000058 000004 00  WA  0   0  4
  [ 1] .text             PROGBITS        00000000 000034 000024 00  AX  0   0  4


#endif
