#include "MCP2515.h"
// #include "ecu-pins.h"

/* Pin 설정에 맞게 수정필요. Modify below items for your SPI configurations */
// extern SPI_HandleTypeDef        hspi4;
// #define SPI_CAN                 &hspi4
#define SPI_TIMEOUT             10
#define MCP2515_CS_HIGH()   // ESP_ERROR_CHECK(gpio_set_level(ECU_CAN_SPI_CS, 0x0U))
#define MCP2515_CS_LOW()    // ESP_ERROR_CHECK(gpio_set_level(ECU_CAN_SPI_CS, 0x1U))




spi_device_handle_t handle = {0};

spi_device_interface_config_t devcfg = {
    // .command_bits = 10,
    .clock_speed_hz = SPI_MASTER_FREQ_8M,
    .mode = 0,          //SPI mode 0
    /*
        * The timing requirements to read the busy signal from the EEPROM cannot be easily emulated
        * by SPI transactions. We need to control CS pin by SW to check the busy signal manually.
        */
    // .spics_io_num = -1,
    .spics_io_num = ECU_CAN_SPI_CS,
    .queue_size = 1,
    // .flags = SPI_DEVICE_POSITIVE_CS,
    .pre_cb = NULL,
    .post_cb = NULL,
    .input_delay_ns = 0,  //the EEPROM output the data half a SPI clock behind.
};

gpio_config_t cs_cfg = {
    .pin_bit_mask = BIT64(ECU_CAN_SPI_CS),
    .mode = GPIO_MODE_OUTPUT,
};

spi_bus_config_t buscfg = {
    .miso_io_num = ECU_CAN_SPI_MISO,
    .mosi_io_num = ECU_CAN_SPI_MOSI,
    .sclk_io_num = ECU_CAN_SPI_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 32,
};








/* Prototypes */
// static spi_device_handle_t handle = {0};
static void SPI_Tx(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length);
static uint8_t SPI_Rx(void);
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length);








esp_err_t spi_init()
{
    esp_err_t err = ESP_OK;

    ecu_log("CALLING gpio_set_level");
    err = gpio_set_level(ECU_CAN_SPI_CS, 0);
    if  (err != ESP_OK) {
        goto cleanup;
    }

    ecu_log("CALLING gpio_config");
    err = gpio_config(&cs_cfg);
    if  (err != ESP_OK) {
        goto cleanup;
    }

    //Initialize the SPI bus
    ecu_log("CALLING spi_bus_initialize");
    err = spi_bus_initialize(ECU_CAN_SPI_RCV_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(err);

    //Attach the EEPROM to the SPI bus
    ecu_log("CALLING spi_bus_add_device");
    err = spi_bus_add_device(ECU_CAN_SPI_RCV_HOST, &devcfg, &handle);
    if  (err != ESP_OK) {
        goto cleanup;
    }

    // err = spi_device_acquire_bus(handle, portMAX_DELAY);
    // if  (err != ESP_OK) {
    //     goto cleanup;
    // }

    // spi_transaction_t t = {
    //     .cmd = 0x0,
    //     .user = NULL
    // };

    // ecu_log("CALLING spi_device_polling_transmit");
    // err = spi_device_polling_transmit(handle, &t);
    // ESP_ERROR_CHECK(err);

    ecu_log("CALLING SPI_Tx");
    SPI_Tx(0x0U);

    return ESP_OK;

cleanup:
    // ecu_log("CALLING spi_device_release_bus");
    // ESP_ERROR_CHECK(spi_device_release_bus(handle));
    ecu_log("CALLING spi_bus_remove_device");
    ESP_ERROR_CHECK(spi_bus_remove_device(handle));

    return err;
}








/* MCP2515 초기화 */
uint8_t MCP2515_Initialize(void)
{
  MCP2515_CS_HIGH();

  /* SPI Ready 확인 */

  ESP_ERROR_CHECK(spi_init());

  // printf("RXF2EID0: \x", MCP2515_ReadByte(MCP2515_RXF2EID0));
  // printf("CANSTAT: \x", MCP2515_ReadByte(MCP2515_CANSTAT));
  // printf("CANCTRL: \x", MCP2515_ReadByte(MCP2515_CANCTRL));

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
      .rx_buffer = NULL,
      .length = sizeof(data) * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  ecu_log("CALLING spi_device_transmit");
  esp_err_t status = spi_device_transmit(handle, &trans_desc);
  ESP_ERROR_CHECK(status);
}

/* SPI Tx Wrapper 함수 */
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
{
  // HAL_SPI_Transmit(SPI_CAN, buffer, length, SPI_TIMEOUT);    

  // clang-format off
  spi_transaction_t trans_desc = {
      .tx_buffer = buffer,
      .rx_buffer = NULL,
      .length = length * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(handle, &trans_desc);
  ESP_ERROR_CHECK(status);
}

/* SPI Rx Wrapper 함수 */
static uint8_t SPI_Rx(void)
{
  uint8_t retVal = 0;
  // HAL_SPI_Receive(SPI_CAN, &retVal, 1, SPI_TIMEOUT);

  // clang-format off
  spi_transaction_t trans_desc = {
      .tx_buffer = NULL,
      .rx_buffer = &retVal,
      .length = sizeof(retVal) * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(handle, &trans_desc);
  ESP_ERROR_CHECK(status);

  return retVal;
}

/* SPI Rx Wrapper 함수 */
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
{
  // HAL_SPI_Receive(SPI_CAN, buffer, length, SPI_TIMEOUT);

  // clang-format off
  spi_transaction_t trans_desc = {
      .tx_buffer = NULL,
      .rx_buffer = buffer,
      .length = length * 8
  };
  // clang-format on

  // Wait for the master to send a query, acknowledge we're here
  esp_err_t status = spi_device_transmit(handle, &trans_desc);
  ESP_ERROR_CHECK(status);
}
