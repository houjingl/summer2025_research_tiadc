/* ============================================================================
 *  console_cmds.h — Unified UART-command helpers for DMA + JESD204 debug      
 * ============================================================================
 *  This header combines the previously separate JESD‑centric command set       
 *  (spi/phy/link) with DMA & memory debugging commands.  Internal helpers are  
 *  kept inside the .c file; external code sees only the high‑level handlers.   
 *                                                                              
 *  Command grammar                                                             
 *  ──────────────────────────────────────────────────────────────────────────   
 *  Keyword Option Arguments                       Description                  
 *  ──────────────────────────────────────────────────────────────────────────   
 *  spi     -r    <reg16>                         Read AD9695 register          
 *          -w    <reg16> <data8>                Write AD9695 register         
 *                                                                              
 *  phy     -r    <off32>                         Read JESD PHY register        
 *          -w    <off32> <data32>               Write JESD PHY register       
 *                                                                              
 *  link    -r    <off32>                         Read JESD Link register       
 *          -w    <off32> <data32>               Write JESD Link register      
 *                                                                              
 *  dma     -r                                    Dump last DMA buffer          
 *          -w                                    Start DMA capture             
 *          -d                                    Reset DMA core                
 *          -c                                    Resume DMA core               
 *                                                                              
 *  dbg     -r    <off32>                         Read DMA S2MM register        
 *          -w    <off32> <data32>               Write DMA S2MM register       
 *                                                                              
 *  mem     -r    <addr32>                        Read arbitrary address        
 *          -w    <addr32> <data32>              Write arbitrary address       
 * --------------------------------------------------------------------------  
 *  © 2025 Your Project Name — MIT License                                      
 * ==========================================================================*/

#ifndef CONSOLE_CMDS_H
#define CONSOLE_CMDS_H

#include <stdint.h>

/* Top‑level dispatcher */
void handle_cmd(char *line);

/* Individual command families (exposed for legacy callers) */
void handle_spi_cmd (char *line);
void handle_phy_cmd (char *line);
void handle_link_cmd(char *line);
void handle_dma_cmd (char *line);
void handle_dma_dbg_cmd(char *line);
void handle_mem_cmd (char *line);

#endif /* CONSOLE_CMDS_H */
