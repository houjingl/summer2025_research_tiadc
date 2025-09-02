/* butils.c
 * Concise, table-driven UART command handler.
 */

#include "butils.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "peripherals.h"
#include "bjesdphy.h"
#include "bjesdlink.h"
#include "ad9695_api.h"
#include "xspips.h"
#include "xaxidma.h"
#include "baxidma.h"
#include "sleep.h"
#include "ad9695_registers.h"

extern XSpiPs spi_inst;
extern XAxiDma dma_inst;
extern u8 *RxBufferPtr;

extern uint8_t uart_send_flag; //Send flag enabled by the uart

#define ERR(fmt, ...) xil_printf("Command Error: " fmt "\r\n", ##__VA_ARGS__)

static int next_tok(char **ctx, char *out, size_t len) {
    char *t = strtok(NULL, " ");
    if (!t) return 0;
    strncpy(out, t, len - 1);
    out[len - 1] = '\0';
    return 1;
}

static void parse_cmd_args(char *line, char *option, size_t opt_len, char *addr_str, size_t addr_len, char *data_str, size_t data_len, const char *cmd_name) {
    char *ctx = line;
    strtok(ctx, " "); // skip command name
    if (!next_tok(&ctx, option, opt_len)) { ERR("Missing option (-r / -w)"); return; }
    if (!next_tok(&ctx, addr_str, addr_len)) { ERR("Missing address"); return; }
    if (!strcmp(option, "-w") && !next_tok(&ctx, data_str, data_len)) { ERR("Missing write data"); return; }
}

// Handler for SPI commands
void handle_spi_cmd(char *line) {
    char option[4], addr_str[8], data_str[4];
    uint16_t addr;
    uint8_t data;

    parse_cmd_args(line, option, sizeof(option), addr_str, sizeof(addr_str), data_str, sizeof(data_str), "spi");
    addr = (uint16_t)strtol(addr_str, NULL, 0);

    if (!strcmp(option, "-r")) {
        ad9695_read_register(&spi_inst, addr, &data);
        xil_printf("Value at 0x%04X = 0x%02X\r\n", addr, data);
    } else if (!strcmp(option, "-w")) {
        data = (uint8_t)strtol(data_str, NULL, 0);
        ad9695_write_register(&spi_inst, addr, data);
        xil_printf("Command Success: Wrote 0x%02X to 0x%04X\r\n", data, addr);
    } else ERR("Invalid option '%s' (use -r or -w)", option);
}

// Handler for JESD204 PHY commands
void handle_phy_cmd(char *line) {
    char option[4], addr_str[12], data_str[12];
    uint32_t addr, data, tmp_reg;

    parse_cmd_args(line, option, sizeof(option), addr_str, sizeof(addr_str), data_str, sizeof(data_str), "phy");
    addr = (uint32_t)strtoul(addr_str, NULL, 0);

    if (!strcmp(option, "-r")) {
        jesdphy_read(addr, &tmp_reg);
        xil_printf("Value at 0x%08X = 0x%08X\r\n", XPAR_JESD204_PHY_0_BASEADDR + addr, tmp_reg);
    } else if (!strcmp(option, "-w")) {
        data = (uint32_t)strtoul(data_str, NULL, 0);
        jesdphy_write(addr, data);
        xil_printf("Wrote 0x%08X to 0x%08X\r\n", data, XPAR_JESD204_PHY_0_BASEADDR + addr);
    } else ERR("Invalid option '%s' (use -r or -w)", option);
}

// Handler for JESD204 Link-layer commands
void handle_link_cmd(char *line) {
    char option[4], addr_str[12], data_str[12];
    uint32_t addr, data, tmp_reg;

    parse_cmd_args(line, option, sizeof(option), addr_str, sizeof(addr_str), data_str, sizeof(data_str), "link");
    addr = (uint32_t)strtoul(addr_str, NULL, 0);

    if (!strcmp(option, "-r")) {
        jesdlink_read(addr, &tmp_reg);
        xil_printf("Value at 0x%08X = 0x%08X\r\n", XPAR_JESD204C_0_BASEADDR + addr, tmp_reg);
    } else if (!strcmp(option, "-w")) {
        data = (uint32_t)strtoul(data_str, NULL, 0);
        jesdlink_write(addr, data);
        xil_printf("Wrote 0x%08X to 0x%08X\r\n", data, XPAR_JESD204C_0_BASEADDR + addr);
    } else ERR("Invalid option '%s' (use -r or -w)", option);
}

void handle_dma_cmd(char* line) {
    char copy[MAX_UART_LINE_LENGTH];
    char option[4];

    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char* token = strtok(copy, " ");
    if (!token || strcmp(token, "dma") != 0) { ERR("Expected \"dma\""); return; }

    token = strtok(NULL, " ");
    if (!token) { ERR("Missing option (-r or -w)"); return; }
    strncpy(option, token, sizeof(option) - 1);
    option[sizeof(option) - 1] = '\0';

    if (strcmp(option, "-w") == 0) {
        xil_printf("Starting DMA capture of %d bytes...\r\n", DMA_CMD_BUF_SIZE);
        Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, DMA_CMD_BUF_SIZE);
        int res =XAxiDma_SimpleTransfer(&dma_inst, (UINTPTR) RxBufferPtr,
                        DMA_CMD_BUF_SIZE, XAXIDMA_DEVICE_TO_DMA);

        if (res != XST_SUCCESS) { ERR("XAxiDma_SimpleTransfer failed. Error Code: %d.", res); return; }
        u32 timeout = 1000;
        int busy;
        do {
            busy = XAxiDma_Busy(&dma_inst, XAXIDMA_DEVICE_TO_DMA);
            if(!busy){ break; }
            timeout --;
            usleep(1);
        }while(timeout > 0);
        if (busy) { xil_printf("DMA was still busy and timed out.\r\n"); }
        else { xil_printf("DMA Finished Successfully.\r\n"); }
        xil_printf("dma -w complete.\r\n");
    } else if (strcmp(option, "-r") == 0) {
        xil_printf("Reading back %d bytes:\r\n", DMA_CMD_BUF_SIZE);
        for (uint32_t i = 0; i < DMA_CMD_BUF_SIZE; i+=16) {
            xil_printf("@0x%02X = 0x%02X ", i, RxBufferPtr[i]);
            xil_printf("\r\n");
        }
        xil_printf("\r\n");
    } else if (strcmp(option, "-d") == 0) {
        XAxiDma_Reset(&dma_inst);
        xil_printf("reset completed!\r\n");
    } else if (strcmp(option, "-c") == 0) {
        XAxiDma_Resume(&dma_inst);
        xil_printf("resume completed!\r\n");
    } else { ERR("Invalid option \"%s\" (use -r or -w or -d)", option); }
}

void handle_mem_cmd(char* line) {
    char option[4], addr_str[12], data_str[12];
    uint32_t addr, data, tmp_reg;

    parse_cmd_args(line, option, sizeof(option), addr_str, sizeof(addr_str), data_str, sizeof(data_str), "mem");
    addr = (uint32_t)strtoul(addr_str, NULL, 0);

    if (strcmp(option, "-r") == 0) {
        tmp_reg = Xil_In32(addr);
        xil_printf("Command Success: Value at 0x%08X = 0x%08X\r\n", addr, tmp_reg);
    } else if (strcmp(option, "-w") == 0) {
        data = (uint32_t)strtoul(data_str, NULL, 0);
        Xil_Out32(addr, data);
        xil_printf("Command Success: Wrote 0x%08X to 0x%08X\r\n", data, addr);
    } else { ERR("Invalid option \"%s\" (use -r or -w)", option); }
}

#define DMA_CTRL_BASE XPAR_AXI_DMA_0_BASEADDR // Unused macro

void handle_dma_dbg_cmd(char* line) {
    char option[4], addr_str[12], data_str[12];
    uint32_t offset, data, reg_val;

    parse_cmd_args(line, option, sizeof(option), addr_str, sizeof(addr_str), data_str, sizeof(data_str), "dbg");
    offset = (uint32_t)strtoul(addr_str, NULL, 0);
    uint32_t addr = RxBufferPtr + offset;

    if (strcmp(option, "-r") == 0) {
        reg_val = Xil_In32(addr);
        xil_printf("Command Success: DMA[0x%08X] = 0x%08X\r\n", offset, reg_val);
    } else if (strcmp(option, "-w") == 0) {
        data = (uint32_t)strtoul(data_str, NULL, 0);
        Xil_Out32(addr, data);
        xil_printf("Command Success: Wrote 0x%08X to DMA[0x%08X]\r\n", data, offset);
    } else { ERR("Invalid option \"%s\" (use -r or -w)", option); }
} 

void handle_udp_cmd(char* line)
{
    uart_send_flag = 1;
}

void handle_adc_cmd(char* line)
{
    char copy[MAX_UART_LINE_LENGTH];
    char option[4];

    strncpy(copy, line, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char* token = strtok(copy, " ");
    if (!token || strcmp(token, "adc") != 0) { ERR("Expected \"adc\""); return; }

    token = strtok(NULL, " ");
    if (!token) { ERR("Missing option"); return; }
    strncpy(option, token, sizeof(option) - 1);
    option[sizeof(option) - 1] = '\0';

    if(strcmp(option, "-c") == 0){
        int timeout = 10;
        uint8_t pll_stat;

        do {
            usleep(10000);
            ad9695_jesd_get_pll_status(&pll_stat);
        } while (!(pll_stat & AD9695_JESD_PLL_LOCK_STAT) && timeout--);

        xil_printf("ad9695 PLL %s\r\n", (pll_stat & AD9695_JESD_PLL_LOCK_STAT) ? "LOCKED" : "UNLOCKED");
        jesdphy_check_pll_status(&pll_stat);
    }
    else {ERR("Invalid option \"%s\" (use -c)", option);}

}

typedef void (*cmd_fn)(char *line);
static const struct { const char *name; cmd_fn fn; } cmd_table[] = {
    { "spi",  handle_spi_cmd  },
    { "phy",  handle_phy_cmd  },
    { "link", handle_link_cmd },
    { "dma",  handle_dma_cmd  },
    { "dbg",  handle_dma_dbg_cmd  },
    { "mem",  handle_mem_cmd  },
    { "udp",  handle_udp_cmd  },
    { "adc",  handle_adc_cmd  }
};

void handle_cmd(char *line) {
    if (!line || !*line) { ERR("empty command"); return; }

    char cmd[8];
    strncpy(cmd, line, sizeof cmd - 1);
    cmd[sizeof cmd - 1] = '\0';
    char *space = strchr(cmd, ' ');
    if (space) *space = '\0';

    for (size_t i = 0; i < sizeof cmd_table / sizeof cmd_table[0]; ++i) {
        if (!strcmp(cmd, cmd_table[i].name)) { cmd_table[i].fn(line); return; }
    }
    xil_printf("Invalid command type: %s\r\n", cmd);
}