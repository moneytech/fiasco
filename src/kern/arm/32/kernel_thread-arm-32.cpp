IMPLEMENTATION [arm && mp]:

#include "io.h"
#include "platform_control.h"
#include "outer_cache.h"
#include "scu.h"
#include "paging.h"
#include <cstdio>

PUBLIC
static void
Kernel_thread::boot_app_cpus()
{
  if (Config::Max_num_cpus <= 1)
    return;

  extern char _tramp_mp_entry[];
  extern volatile Mword _tramp_mp_startup_cp15_c1;
  extern volatile Mword _tramp_mp_startup_pdbr;
  extern volatile Mword _tramp_mp_startup_dcr;
  extern volatile Mword _tramp_mp_startup_ttbcr;
  extern volatile Mword _tramp_mp_startup_mair0;
  extern volatile Mword _tramp_mp_startup_mair1;

  if (Scu::Available)
    {
      unsigned num_ap_cpus = Cpu::scu->config() & 3;
      printf("Number of CPUs: %d\n", num_ap_cpus + 1);
    }

  _tramp_mp_startup_cp15_c1 = Config::Cache_enabled
                              ? Cpu::Cp15_c1_cache_enabled : Cpu::Cp15_c1_cache_disabled;
  _tramp_mp_startup_pdbr
    = Kmem_space::kdir()->virt_to_phys((Address)Kmem_space::kdir()) | Page::Ttbr_bits;
  _tramp_mp_startup_ttbcr   = Page::Ttbcr_bits;
  _tramp_mp_startup_mair0   = Page::Mair0_prrr_bits;
  _tramp_mp_startup_mair1   = Page::Mair1_nmrr_bits;
  _tramp_mp_startup_dcr     = 0x55555555;

  __asm__ __volatile__ ("" : : : "memory");
  Mem_unit::clean_dcache();

  Outer_cache::clean(Kmem_space::kdir()->virt_to_phys((Address)&_tramp_mp_startup_cp15_c1));
  Outer_cache::clean(Kmem_space::kdir()->virt_to_phys((Address)&_tramp_mp_startup_pdbr));
  Outer_cache::clean(Kmem_space::kdir()->virt_to_phys((Address)&_tramp_mp_startup_dcr));
  Outer_cache::clean(Kmem_space::kdir()->virt_to_phys((Address)&_tramp_mp_startup_ttbcr));
  Outer_cache::clean(Kmem_space::kdir()->virt_to_phys((Address)&_tramp_mp_startup_mair0));
  Outer_cache::clean(Kmem_space::kdir()->virt_to_phys((Address)&_tramp_mp_startup_mair1));

  Platform_control::boot_ap_cpus(Kmem_space::kdir()->virt_to_phys((Address)_tramp_mp_entry));
}
