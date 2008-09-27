/**
 * Mupen64 - r4300/x86_64/gregimm.c
 * Copyright (C) 2007 Richard Goedeken, Hacktarux
 * Based on code written by Hacktarux, Copyright (C) 2002
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * Forum homepage: http://www.emutalk.net/forumdisplay.php?f=50
 * 
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/


#include <stdio.h>
#include "../recomph.h"
#include "../recomp.h"
#include "../r4300.h"
#include "assemble.h"
#include "../ops.h"
#include "../../memory/memory.h"
#include "../macros.h"
#include "interpret.h"

void genbltz_test()
{
  int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   
  if (rs_64bit == 0)
  {
    int rs = allocate_register_32((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs, 0);
    setl_m8abs((unsigned char *) &branch_taken);
  }
  else if (rs_64bit == -1)
  {
    cmp_m32abs_imm32(((unsigned int *)dst->f.i.rs)+1, 0);
    setl_m8abs((unsigned char *) &branch_taken);
  }
  else
  {
    int rs = allocate_register_64((unsigned long long *)dst->f.i.rs);

    cmp_reg64_imm8(rs, 0);
    setl_m8abs((unsigned char *) &branch_taken);
  }
}

void genbltz()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[47]);
#endif
#ifdef INTERPRET_BLTZ
   gencallinterp((unsigned long long)BLTZ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZ, 1);
    return;
     }
   
   genbltz_test();
   gendelayslot();
   gentest();
#endif
}

void genbltz_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[47]);
#endif
#ifdef INTERPRET_BLTZ_OUT
   gencallinterp((unsigned long long)BLTZ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZ_OUT, 1);
    return;
     }
   
   genbltz_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbltz_idle()
{
#ifdef INTERPRET_BLTZ_IDLE
   gencallinterp((unsigned long long)BLTZ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZ_IDLE, 1);
    return;
     }
   
   genbltz_test();
   gentest_idle();
   genbltz();
#endif
}

void genbgez_test()
{
  int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   
  if (rs_64bit == 0)
  {
    int rs = allocate_register_32((unsigned int *)dst->f.i.rs);
    cmp_reg32_imm32(rs, 0);
    setge_m8abs((unsigned char *) &branch_taken);
  }
  else if (rs_64bit == -1)
  {
    cmp_m32abs_imm32(((unsigned int *)dst->f.i.rs)+1, 0);
    setge_m8abs((unsigned char *) &branch_taken);
  }
  else
  {
    int rs = allocate_register_64((unsigned long long *)dst->f.i.rs);
    cmp_reg64_imm8(rs, 0);
    setge_m8abs((unsigned char *) &branch_taken);
  }
}

void genbgez()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[48]);
#endif
#ifdef INTERPRET_BGEZ
   gencallinterp((unsigned long long)BGEZ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZ, 1);
    return;
     }
   
   genbgez_test();
   gendelayslot();
   gentest();
#endif
}

void genbgez_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[48]);
#endif
#ifdef INTERPRET_BGEZ_OUT
   gencallinterp((unsigned long long)BGEZ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZ_OUT, 1);
    return;
     }
   
   genbgez_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbgez_idle()
{
#ifdef INTERPRET_BGEZ_IDLE
   gencallinterp((unsigned long long)BGEZ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZ_IDLE, 1);
    return;
     }
   
   genbgez_test();
   gentest_idle();
   genbgez();
#endif
}

void genbltzl()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[49]);
#endif
#ifdef INTERPRET_BLTZL
   gencallinterp((unsigned long long)BLTZL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZL, 1);
    return;
     }
   
   genbltz_test();
   free_all_registers();
   gentestl();
#endif
}

void genbltzl_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[49]);
#endif
#ifdef INTERPRET_BLTZL_OUT
   gencallinterp((unsigned long long)BLTZL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZL_OUT, 1);
    return;
     }
   
   genbltz_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbltzl_idle()
{
#ifdef INTERPRET_BLTZL_IDLE
   gencallinterp((unsigned long long)BLTZL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZL_IDLE, 1);
    return;
     }
   
   genbltz_test();
   gentest_idle();
   genbltzl();
#endif
}

void genbgezl()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[50]);
#endif
#ifdef INTERPRET_BGEZL
   gencallinterp((unsigned long long)BGEZL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZL, 1);
    return;
     }
   
   genbgez_test();
   free_all_registers();
   gentestl();
#endif
}

void genbgezl_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[50]);
#endif
#ifdef INTERPRET_BGEZL_OUT
   gencallinterp((unsigned long long)BGEZL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZL_OUT, 1);
    return;
     }
   
   genbgez_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbgezl_idle()
{
#ifdef INTERPRET_BGEZL_IDLE
   gencallinterp((unsigned long long)BGEZL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZL_IDLE, 1);
    return;
     }
   
   genbgez_test();
   gentest_idle();
   genbgezl();
#endif
}

void genbranchlink()
{
   int r31_64bit = is64((unsigned int*)&reg[31]);
   
   if (r31_64bit == 0)
     {
    int r31 = allocate_register_32_w((unsigned int *)&reg[31]);
    
    mov_reg32_imm32(r31, dst->addr+8);
     }
   else if (r31_64bit == -1)
     {
    mov_m32abs_imm32((unsigned int *)&reg[31], dst->addr + 8);
    if (dst->addr & 0x80000000)
      mov_m32abs_imm32(((unsigned int *)&reg[31])+1, 0xFFFFFFFF);
    else
      mov_m32abs_imm32(((unsigned int *)&reg[31])+1, 0);
     }
   else
     {
    int r31 = allocate_register_64_w((unsigned long long *)&reg[31]);
    
    mov_reg32_imm32(r31, dst->addr+8);
    movsxd_reg64_reg32(r31, r31);
     }
}

void genbltzal()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[51]);
#endif
#ifdef INTERPRET_BLTZAL
   gencallinterp((unsigned long long)BLTZAL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZAL, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gendelayslot();
   gentest();
#endif
}

void genbltzal_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[51]);
#endif
#ifdef INTERPRET_BLTZAL_OUT
   gencallinterp((unsigned long long)BLTZAL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZAL_OUT, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gendelayslot();
   gentest_out();
#endif
}

void genbltzal_idle()
{
#ifdef INTERPRET_BLTZAL_IDLE
   gencallinterp((unsigned long long)BLTZAL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZAL_IDLE, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gentest_idle();
   genbltzal();
#endif
}

void genbgezal()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[52]);
#endif
#ifdef INTERPRET_BGEZAL
   gencallinterp((unsigned long long)BGEZAL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZAL, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gendelayslot();
   gentest();
#endif
}

void genbgezal_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[52]);
#endif
#ifdef INTERPRET_BGEZAL_OUT
   gencallinterp((unsigned long long)BGEZAL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZAL_OUT, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gendelayslot();
   gentest_out();
#endif
}

void genbgezal_idle()
{
#ifdef INTERPRET_BGEZAL_IDLE
   gencallinterp((unsigned long long)BGEZAL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZAL_IDLE, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gentest_idle();
   genbgezal();
#endif
}

void genbltzall()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[53]);
#endif
#ifdef INTERPRET_BLTZALL
   gencallinterp((unsigned long long)BLTZALL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZALL, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   free_all_registers();
   gentestl();
#endif
}

void genbltzall_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[53]);
#endif
#ifdef INTERPRET_BLTZALL_OUT
   gencallinterp((unsigned long long)BLTZALL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZALL_OUT, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   free_all_registers();
   gentestl_out();
#endif
}

void genbltzall_idle()
{
#ifdef INTERPRET_BLTZALL_IDLE
   gencallinterp((unsigned long long)BLTZALL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BLTZALL_IDLE, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gentest_idle();
   genbltzall();
#endif
}

void genbgezall()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[54]);
#endif
#ifdef INTERPRET_BGEZALL
   gencallinterp((unsigned long long)BGEZALL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZALL, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   free_all_registers();
   gentestl();
#endif
}

void genbgezall_out()
{
#if defined(COUNT_INSTR)
   inc_m32abs(&instr_count[54]);
#endif
#ifdef INTERPRET_BGEZALL_OUT
   gencallinterp((unsigned long long)BGEZALL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZALL_OUT, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   free_all_registers();
   gentestl_out();
#endif
}

void genbgezall_idle()
{
#ifdef INTERPRET_BGEZALL_IDLE
   gencallinterp((unsigned long long)BGEZALL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)BGEZALL_IDLE, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gentest_idle();
   genbgezall();
#endif
}

