#include "uart_printf.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"

// 入数据到fifo
static uint8_t uart_fifo_put(const void *data, uint16_t dataLen);
static uint8_t uart_fifo_get(uint8_t *data, uint16_t dataLen);

static void HEX_TO_STR(uint8_t data, char *str);
static HANDLE_LOG_FIFO log_fifo_handle;

void uart_printf_init()
{
    log_fifo_handle.fifo_len = FIFO_LOG_LEN;
    log_fifo_handle.uart_error_code = UART_TX_READY;
#if uart_IT

    log_fifo_handle.fifo_log.in = 0;
    log_fifo_handle.fifo_log.out = 0;
#else
    log_fifo_handle.fifo_log[0].in = 0;
    log_fifo_handle.fifo_log[1].in = 0;

    log_fifo_handle.fifo_log[0].out = 0;
    log_fifo_handle.fifo_log[1].out = 0;

    log_fifo_handle.fifo_index = 0;
#endif
}

static uint8_t uart_fifo_put(const void *data, uint16_t dataLen)
{

    // 关闭os调度

    // 此次进fifo的最长长度

    uint16_t maxLength = 0;
    // 从in到fifo_len的长度
    uint16_t forwardLength = 0;
    uint8_t *buff;
    uint16_t *in = 0;
    uint16_t *out = 0;
#if uart_IT

    in = &(log_fifo_handle.fifo_log.in);
    out = &(log_fifo_handle.fifo_log.out);
    buff = &(log_fifo_handle.fifo_log.buff[0]);

#else

#endif

#if uart_test
    char tempdata[100] = "";
    snprintf(tempdata, 100, "the in is %d, the out is %d\r\n", *in, *out);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);

#endif

    // 检查buff是否已经满
    if ((*in == (log_fifo_handle.fifo_len - 1)) && (*out == 0))
    {
#if uart_test
        snprintf(tempdata, 100, "the buff is full, the in is maxlen -1, the out is 0\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
        return FIFO_IN_OR_OUT_FAIL;
    }
    else if ((*in + 1) == *out)
    {
#if uart_test
        snprintf(tempdata, 100, "the buff is full, the in + 1 = out\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
        return FIFO_IN_OR_OUT_FAIL;
    }

    // 计算出buff的容量
    if (*in > *out)
    {
        maxLength = log_fifo_handle.fifo_len - *in + *out;
#if uart_test
        snprintf(tempdata, 100, "the in %d is bigger than out %d\r\n", *in, *out);
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    }
    else if (*out == *in)
    {
        maxLength = log_fifo_handle.fifo_len;
#if uart_test
        snprintf(tempdata, 100, "the in = out =  %d\r\n", *out);
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    }
    else
    {
        maxLength = *out - *in;
#if uart_test
        snprintf(tempdata, 100, "the in %d less than out =  %d\r\n", *in, *out);
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    }

#if uart_test
    snprintf(tempdata, 100, "the maxLength %d,the dataLen %d\r\n", maxLength, dataLen);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif

    /*此次写入buff的长度。maxLength - 1 是为了确保 buff最大只能写 buff_len -1,
    在这种情况下，才能使用in +1 = out，判断buff写满了*/
    maxLength = MIN((maxLength - 1), dataLen);

    // 计算出从fifo[in]到fifo[fifo_len - 1]的长度
    forwardLength = log_fifo_handle.fifo_len - *in;
#if uart_test
    snprintf(tempdata, 100, "the forwardLength %d\r\n", forwardLength);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    // 复制进buff

    if (maxLength > forwardLength)
    {
        memcpy(&buff[*in], data, forwardLength);
        memcpy(&buff[0], &((uint8_t *)data)[forwardLength], maxLength - forwardLength);

        *in = maxLength - forwardLength;
    }
    else
    {
        memcpy(&buff[*in], data, maxLength);
        *in = *in + maxLength;
    }
#if uart_test
    snprintf(tempdata, 100, "the new in is %d the out is %d ,\r\n", *in, *out);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    // 开启os调度

    return FIFO_IN_OR_OUT_COMPLETE;
}

static uint8_t uart_fifo_get(uint8_t *data, uint16_t dataLen)
{
    // 此次进fifo的最长长度
    uint16_t maxLength = 0;
    // 从in到fifo_len的长度
    uint16_t forwardLength = 0;
    uint8_t *buff;
    uint16_t *in = 0;
    uint16_t *out = 0;
#if uart_IT

    in = &(log_fifo_handle.fifo_log.in);
    out = &(log_fifo_handle.fifo_log.out);
    buff = &(log_fifo_handle.fifo_log.buff[0]);

#else

#endif

#if uart_test
    char tempdata[100] = "";
    snprintf(tempdata, 100, "the in is %d, the out is %d the request length is%d\r\n", *in, *out, dataLen);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);

#endif
    // 关闭OS调度

    // buf内是否有数据
    if (*in == *out)
    {

#if uart_test
        char tempdata[100] = "";
        snprintf(tempdata, 100, "the in = the out = %d \r\n", *out);
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
        return FIFO_IN_OR_OUT_FAIL;
    }

    // 计算已经入buff数据长度
    if (*in < *out)
    {
        maxLength = log_fifo_handle.fifo_len - *out + *in;
#if uart_test
        char tempdata[100] = "";
        snprintf(tempdata, 100, "the in %d < the out %d\r\n", *in, *out);
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    }
    else
    {
        maxLength = *in - *out;
#if uart_test
        char tempdata[100] = "";
        snprintf(tempdata, 100, "the in %d > the out %d\r\n", *in, *out);
        HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    }

    // 计算出从fifo[out]到fifo[fifo_len - 1]的长度
    forwardLength = log_fifo_handle.fifo_len - *out;

    /*
    此次出buff的长度*/
    maxLength = MIN(maxLength, dataLen);
#if uart_test
    snprintf(tempdata, 100, "the maxLength %d , the forwardLength %d\r\n", maxLength, forwardLength);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);
#endif
    if (maxLength > forwardLength)
    {
        memcpy(data, &buff[*out], forwardLength);
        memcpy(&data[forwardLength], buff, maxLength - forwardLength);
        *out = maxLength - forwardLength;
    }
    else
    {
        memcpy(data, &buff[*out], maxLength);
        *out = *out + maxLength;
    }
#if uart_test
    snprintf(tempdata, 100, "the in is %d, the new out is %d\r\n", *in, *out);
    HAL_UART_Transmit(&huart1, (uint8_t *)tempdata, strlen(tempdata), 100);

#endif

    // FIFO为空
    if (*in == *out)
    {
        return FIFO_IN_EQUAL_OUT;
    }
    return FIFO_IN_OR_OUT_COMPLETE;
    // 开启os调度
}

uint8_t uart_printf(const void *strData, const uint8_t *data, const uint8_t len)
{
    // strData 不能为NUll
    UART_ASSERT(strData != NULL);
    if (data != NULL)
    {
        char tempData[log_fifo_handle.fifo_len];

        //  初始化tempData
        sprintf(tempData, "%s ", (char *)strData);

        //  或者
        // memset(tempData, 0, log_fifo_handle.fifo_len);
        // strncat(tempData, strData, strlen((char *)strData));

        // 或者
        // memset(tempData, 0, log_fifo_handle.fifo_len);
        // memcpy(tempData,strData,strlen((char *)strData));

        char temp[12] = "";

#if capital_letter

        // 使用大写
        for (int i = 0; i < len; i++)
        {
            HEX_TO_STR(data[i], temp);

            if (i == 0)
            {
                strcat(tempData, &temp[1]);
            }
            else
            {
                strcat(tempData, temp);
            }
        }
#else
        // 使用小写
        for (int i = 0; i < len; i++)
        {
            if (data[i] < (uint8_t)'a')
            {
                sprintf(temp, "-0%x", data[i]);
            }
            else
            {
                sprintf(temp, "-%x", data[i]);
            }
            if (i == 0)
            {
                strncat(tempData, &temp[1], strlen((char *)strData));
            }
            else
            {
                strncat(tempData, temp, strlen((char *)strData));
            }
        }
#endif

        sprintf(temp, "\r\n");
        strncat(tempData, temp, strlen((char *)strData));

        uart_fifo_put(tempData, strlen(tempData));
    }
    else
    {
        uart_fifo_put(strData, strlen((char *)strData));
    }

    // 格式化输出

#if uart_IT
    if (log_fifo_handle.uart_error_code == UART_TX_READY)
    {
        // 打开串口中断
        ENABLE_UART_TX_DR_IT();
        log_fifo_handle.uart_error_code = UART_TX_BUSY;
    }
#else

#endif
    return 0;
}

// 仅操作DR寄存器
void uart_out()
{
    uint8_t data = 0;
    if (uart_fifo_get(&data, 1) == FIFO_IN_EQUAL_OUT)
    {
        UART_SEND_DATA_DR(data);
        DISABLE_UART_TX_DR_IT();
        log_fifo_handle.uart_error_code = UART_TX_READY;
    }
    else
    {
        UART_SEND_DATA_DR(data);
    }
}

static void HEX_TO_STR(uint8_t data, char *str)
{
    uint8_t temp = 0;
    memcpy(str, "-", 1);

    for (uint8_t i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            temp = (data & 0xf0) >> 4;
        }
        else
        {
            temp = data & 0x0f;
        }
        temp = temp < 0x0a ? (temp + '0') : (temp - 0x0a + 'A');

        memcpy(&str[i + 1], &temp, 1);
    }
}