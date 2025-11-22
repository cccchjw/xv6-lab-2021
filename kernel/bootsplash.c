// bootsplash.c - Custom boot splash screen for xv6
#include "types.h"
#include "defs.h"

void
print_logo(void)
{
  printf("\n");
  printf("  /\\_____/\\    \n");
  printf(" /  o   o  \\   Welcome to\n");  
  printf("( ==  ^  == )   Bread'OS\n");
  printf(" )         (    \n");
  printf(" (         )   Version 1.0\n");
  printf(" ( (  ) ( ) )  \n");
  printf("(__(__)___(__) \n");
  printf("\n");
}

void
print_welcome_banner(void)
{
  printf("\n");
  printf("=================================================\n");
  printf("              WELCOME TO Bread'OS v1.0          \n");
  printf("             Custom xv6 Distribution            \n");
  printf("=================================================\n");
  printf("\n");
}

void
print_system_info(void)
{
  printf("System Information:\n");
  printf("-------------------\n");
  printf("OS Name: Bread'OS v1.0\n");
  printf("Kernel: xv6 Custom Edition\n");
  printf("Architecture: RISC-V\n");
  printf("Memory: 512MB configured\n");
  printf("-------------------\n");
  printf("\n");
}

void
show_boot_splash(void)
{
  // 清屏（通过多个换行）
  printf("\n\n\n\n\n");
  
  // 显示 Logo
  print_logo();
  
  // 显示欢迎横幅
  print_welcome_banner();
  
  // 显示系统信息
  print_system_info();
  
  printf("\nBooting completed successfully!\n");
  printf("Type 'help' for available commands\n");
  printf("=================================================\n");
  printf("\n");
}