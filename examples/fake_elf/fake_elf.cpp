/*
Copyright (C) 2001-present by Serge Lamikhov-Center

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
 * This example shows how to create ELF executable file for Linux on x86-64
 *
 * Instructions:
 * 1. Compile and link this file with ELFIO library
 *    g++ writer.cpp -o writer
 * 2. Execute result file writer
 *    ./writer
 * 3. Add executable flag for the output file
 *    chmod +x hello_x86_64
 * 4. Run the result file:
 *    ./hello_x86_64
 */

#include <elfio/elfio.hpp>

using namespace ELFIO;

const Elf64_Addr CODE_ADDR = 0x001000;
const Elf_Xword PAGE_SIZE = 0x10;

int main(void) {
    // Create an elfio reader
    elfio writer;

    writer.create(ELFCLASS64, ELFDATA2LSB);

    writer.set_os_abi(ELFOSABI_ARM);
    writer.set_type(ET_DYN);
    writer.set_machine(EM_AARCH64);

    // .text section
    section *text_sec = writer.sections.add(".text");
    text_sec->set_type(SHT_PROGBITS);
    text_sec->set_flags(SHF_ALLOC | SHF_EXECINSTR);
    text_sec->set_addr_align(0x10);

    char text[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    text_sec->set_data(text, sizeof(text));

    // HEADER segment
    segment *header_seg = writer.segments.add();
    header_seg->set_type(PT_PHDR);
    header_seg->set_flags(PF_R);
    header_seg->set_file_size(writer.get_segment_entry_size() * 2);
    header_seg->set_virtual_address(writer.get_segments_offset());
    header_seg->set_physical_address(writer.get_segments_offset());

    // LOAD segment
    segment *text_seg = writer.segments.add();
    text_seg->set_type(PT_LOAD);
    text_seg->set_virtual_address(CODE_ADDR);
    text_seg->set_physical_address(CODE_ADDR);
    text_seg->set_file_size(sizeof(text));
    text_seg->set_flags(PF_X | PF_R);
    text_seg->set_align(PAGE_SIZE);

    text_seg->add_section(text_sec, text_sec->get_addr_align());

    // strtab section
    section *strtab_sec = writer.sections.add(".strtab");
    strtab_sec->set_type(SHT_STRTAB);
    string_section_accessor strtaba(strtab_sec);
    Elf_Word str_index = strtaba.add_string("msg");

    // symtab section
    section *symtab_sec = writer.sections.add(".symtab");
    symtab_sec->set_type(SHT_SYMTAB);
    symtab_sec->set_info(1);
    symtab_sec->set_addr_align(0x4);
    symtab_sec->set_link(strtab_sec->get_index());
    symtab_sec->set_entry_size(writer.get_default_entry_size(SHT_SYMTAB));

    symbol_section_accessor symtaba(writer, symtab_sec);
    symtaba.add_symbol(str_index, CODE_ADDR, sizeof(text)/2, STB_GLOBAL, STT_FUNC, 0, text_sec->get_index());
    symtaba.add_symbol(strtaba, "aaa", CODE_ADDR+10, 10, STB_GLOBAL, STT_FUNC, 0, text_sec->get_index());
//
//    // dynstr section
//    section *dynstr_sec = writer.sections.add(".dynstr");
//    dynstr_sec->set_type(SHT_STRTAB);
//    string_section_accessor dynstra(dynstr_sec);
//
//    // dynsym section
//    section *dynsym_sec = writer.sections.add(".dynsym");
//    dynsym_sec->set_type(SHT_DYNSYM);
//    dynsym_sec->set_info(1);
//    dynsym_sec->set_addr_align(0x4);
//    dynsym_sec->set_link(dynstr_sec->get_index());
//    dynsym_sec->set_entry_size(writer.get_default_entry_size(SHT_DYNSYM));
//    symbol_section_accessor dynsyma(writer, dynsym_sec);
//    dynsyma.add_symbol(dynstra, "exp_a", CODE_ADDR, 0, STB_GLOBAL, STT_FUNC, 0, text_sec->get_index());


    // Create ELF file
    writer.save("libart.dump");

    return 0;
}
