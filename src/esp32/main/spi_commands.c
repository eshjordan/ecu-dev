#include "app.h"

void ecu_send_status(ESP32Msg_t *esp_status)
{
    static char msg[1024] = {0};

    static DMA_ATTR WORD_ALIGNED_ATTR Message_t rx_msg  = {0};
    static DMA_ATTR WORD_ALIGNED_ATTR ESP32Msg_t tx_msg = {0};
    static spi_slave_transaction_t trans_desc           = {
                  .rx_buffer = &rx_msg,
                  .tx_buffer = &tx_msg,
                  .length    = max(sizeof(Message_t), sizeof(ESP32Msg_t)) * 8UL,
    };

    // Copy the IO state to the message buffer, clear rx buffer
    memset(&rx_msg, 0, sizeof(rx_msg));
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg        = *esp_status;
    tx_msg.header = make_header(0, sizeof(ESP32Msg_t));
    calc_esp_checksum(&tx_msg);

    // Set up a transaction to send/receive
    ESP_ERROR_CHECK(spi_slave_transmit(ECU_SPI_RCV_HOST, &trans_desc, portMAX_DELAY));

    ESP32Msg_to_str(msg, &tx_msg);
    ecu_log("SPI - Sent message:\n%s", msg);

    msg_to_str(msg, &rx_msg);
    ecu_log("SPI - Received message:\n%s", msg);

    int msg_status = check_msg(&rx_msg);
    if (msg_status < 0)
    {
        msg_err_to_str(msg, msg_status);
        ecu_warn("SPI - Received invalid acknowledgement msg - %s", msg);
        return;
    }

    if (rx_msg.command != ACK_CMD)
    {
        ecu_warn("SPI - Received invalid ack command - %d", rx_msg.command);
        return;
    }
}
