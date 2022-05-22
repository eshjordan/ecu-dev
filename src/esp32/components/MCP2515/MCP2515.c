#include "MCP2515.h"
#include "ecu-pins.h"

/* Pin 설정에 맞게 수정필요. Modify below items for your SPI configurations */
// extern SPI_HandleTypeDef        hspi4;
// #define SPI_CAN                 &hspi4
#define SPI_TIMEOUT             10
#define MCP2515_CS_HIGH()   // HAL_GPIO_WritePin(CAN_CS_GPIO_Port, CAN_CS_Pin, GPIO_PIN_SET)
#define MCP2515_CS_LOW()    // HAL_GPIO_WritePin(CAN_CS_GPIO_Port, CAN_CS_Pin, GPIO_PIN_RESET)

/* Prototypes */
static spi_device_handle_t can_spi_dev_handle = {0};
static void SPI_Tx(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length);
static uint8_t SPI_Rx(void);
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length);

/* MCP2515 초기화 */
uint8_t MCP2515_Initialize(void)
{
  MCP2515_CS_HIGH();    

  /* SPI Ready 확인 */

  // Configuration for the SPI bus
  spi_bus_config_t can_spi_buscfg = {
      .mosi_io_num   = ECU_CAN_SPI_MOSI,
      .miso_io_num   = ECU_CAN_SPI_MISO,
      .sclk_io_num   = ECU_CAN_SPI_SCLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };

  // Configuration for the SPI device interface
  spi_device_interface_config_t can_spi_devcfg = {
    .mode = 1,
    .clock_speed_hz = SPI_MASTER_FREQ_10M,
    .spics_io_num = ECU_CAN_SPI_CS,
    .queue_size = 3,
    .flags = 0,
    .pre_cb = NULL,
    .post_cb = NULL,
  };


  // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
  ESP_ERROR_CHECK(gpio_set_pull_mode(ECU_CAN_SPI_MOSI, GPIO_PULLUP_ONLY));
  ESP_ERROR_CHECK(gpio_set_pull_mode(ECU_CAN_SPI_SCLK, GPIO_PULLUP_ONLY));
  ESP_ERROR_CHECK(gpio_set_pull_mode(ECU_CAN_SPI_CS, GPIO_PULLUP_ONLY));

  // ESP_ERROR_CHECK(spi_slave_initialize(ECU_CAN_SPI_RCV_HOST, &can_spi_buscfg, &can_spi_slvcfg, SPI_DMA_CH1));

  esp_err_t status;

  status = spi_bus_initialize(ECU_CAN_SPI_RCV_HOST, &can_spi_buscfg, SPI_DMA_CH2);
  ESP_ERROR_CHECK(status);
  status = spi_bus_add_device(ECU_CAN_SPI_RCV_HOST, &can_spi_devcfg, &can_spi_dev_handle);
  ESP_ERROR_CHECK(status);

  return 1;
}

/* MCP2515 를 설정모드로 전환 */
uint8_t MCP2515_SetConfigMode(void)
{
  /* CANCTRL Register Configuration 모드 설정 */  
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x80);
  
  uint8_t loop = 10;
  
  do {    
    /* 모드전환 확인 */    
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x80)
      return 1;
    
    loop--;
  } while(loop > 0); 
  
  return 0;
}

/* MCP2515 를 Normal모드로 전환 */
uint8_t MCP2515_SetNormalMode(void)
{
  /* CANCTRL Register Normal 모드 설정 */  
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x00);
  
  uint8_t loop = 10;
  
  do {    
    /* 모드전환 확인 */    
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x00)
      return 1;
    
    loop--;
  } while(loop > 0);
  
  return 0;
}

/* MCP2515 Loopback Mode*/
uint8_t MCP2515_SetLoopbackMode(void)
{
  /* CANCTRL Register Loopback 모드 설정 */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x40);
  
  uint8_t loop = 10;
  
  do {    
    /* 모드전환 확인 */    
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x40)
      return 1;
    
    loop--;
  } while(loop > 0);
  
  return 0;
}

/* MCP2515 를 Sleep 모드로 전환 */
uint8_t MCP2515_SetSleepMode(void)
{
  /* CANCTRL Register Sleep 모드 설정 */  
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x20);
  
  uint8_t loop = 10;
  
  do {    
    /* 모드전환 확인 */    
    if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x20)
      return 1;
    
    loop--;
  } while(loop > 0);
  
  return 0;
}

/* MCP2515 SPI-Reset */
void MCP2515_Reset(void)
{    
  MCP2515_CS_LOW();
      
  SPI_Tx(MCP2515_RESET);
      
  MCP2515_CS_HIGH();
}

/* 1바이트 읽기 */
uint8_t MCP2515_ReadByte (uint8_t address)
{
  uint8_t retVal;
  
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_READ);
  SPI_Tx(address);
  retVal = SPI_Rx();
      
  MCP2515_CS_HIGH();
  
  return retVal;
}

/* Sequential Bytes 읽기 */
void MCP2515_ReadRxSequence(uint8_t instruction, uint8_t *data, uint8_t length)
{
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);        
  SPI_RxBuffer(data, length);
    
  MCP2515_CS_HIGH();
}

/* 1바이트 쓰기 */
void MCP2515_WriteByte(uint8_t address, uint8_t data)
{    
  MCP2515_CS_LOW();  
  
  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(address);
  SPI_Tx(data);  
    
  MCP2515_CS_HIGH();
}

/* Sequential Bytes 쓰기 */
void MCP2515_WriteByteSequence(uint8_t startAddress, uint8_t endAddress, uint8_t *data)
{    
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(startAddress);
  SPI_TxBuffer(data, (endAddress - startAddress + 1));
  
  MCP2515_CS_HIGH();
}

/* TxBuffer에 Sequential Bytes 쓰기 */
void MCP2515_LoadTxSequence(uint8_t instruction, uint8_t *idReg, uint8_t dlc, uint8_t *data)
{    
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);
  SPI_TxBuffer(idReg, 4);
  SPI_Tx(dlc);
  SPI_TxBuffer(data, dlc);
       
  MCP2515_CS_HIGH();
}

/* TxBuffer에 1 Bytes 쓰기 */
void MCP2515_LoadTxBuffer(uint8_t instruction, uint8_t data)
{
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);
  SPI_Tx(data);
        
  MCP2515_CS_HIGH();
}

/* RTS 명령을 통해서 TxBuffer 전송 */
void MCP2515_RequestToSend(uint8_t instruction)
{
  MCP2515_CS_LOW();
  
  SPI_Tx(instruction);
      
  MCP2515_CS_HIGH();
}

/* MCP2515 Status 확인 */
uint8_t MCP2515_ReadStatus(void)
{
  uint8_t retVal;
  
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_READ_STATUS);
  retVal = SPI_Rx();
        
  MCP2515_CS_HIGH();
  
  return retVal;
}

/* MCP2515 RxStatus 레지스터 확인 */
uint8_t MCP2515_GetRxStatus(void)
{
  uint8_t retVal;
  
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_RX_STATUS);
  retVal = SPI_Rx();
        
  MCP2515_CS_HIGH();
  
  return retVal;
}

/* 레지스터 값 변경 */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{    
  MCP2515_CS_LOW();
  
  SPI_Tx(MCP2515_BIT_MOD);
  SPI_Tx(address);
  SPI_Tx(mask);
  SPI_Tx(data);
        
  MCP2515_CS_HIGH();
}

/* SPI Tx Wrapper 함수 */
static void SPI_Tx(uint8_t data)
{
  // HAL_SPI_Transmit(SPI_CAN, &data, 1, SPI_TIMEOUT);    

  // clang-format off
  spi_transaction_t trans_desc = {
      .tx_buffer = &data,
      .length = sizeof(data) * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(can_spi_dev_handle, &trans_desc);
  ESP_ERROR_CHECK(status);
}

/* SPI Tx Wrapper 함수 */
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
{
  // HAL_SPI_Transmit(SPI_CAN, buffer, length, SPI_TIMEOUT);    

  // clang-format off
  spi_transaction_t trans_desc = {
      .tx_buffer = buffer,
      .length = length * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(can_spi_dev_handle, &trans_desc);
  ESP_ERROR_CHECK(status);
}

/* SPI Rx Wrapper 함수 */
static uint8_t SPI_Rx(void)
{
  uint8_t retVal = 0;
  // HAL_SPI_Receive(SPI_CAN, &retVal, 1, SPI_TIMEOUT);

  // clang-format off
  spi_transaction_t trans_desc = {
      .rx_buffer = &retVal,
      .length = sizeof(retVal) * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(can_spi_dev_handle, &trans_desc);
  ESP_ERROR_CHECK(status);

  return retVal;
}

/* SPI Rx Wrapper 함수 */
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
{
  // HAL_SPI_Receive(SPI_CAN, buffer, length, SPI_TIMEOUT);

  // clang-format off
  spi_transaction_t trans_desc = {
      .rx_buffer = buffer,
      .length = length * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(can_spi_dev_handle, &trans_desc);
  ESP_ERROR_CHECK(status);
}
