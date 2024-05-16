#include <gtest/gtest.h>

extern "C"
{
    #include "module/fifo/fifo.h"

    void app_main_init(void)
    {

    }

    void board_init(void)
    {
        
    }
}

TEST(fifo_fifo, invalid_initialization)
{
    fifo_t fifo;

    EXPECT_EQ(fifo_init(&fifo, 0, NULL, 0), FIFO_ELEMENTSIZE_INVALID);
    EXPECT_EQ(fifo_init(&fifo, 2, NULL, 32768), FIFO_BUFFERSIZE_INVALID);
}

TEST(fifo_fifo, element_size_byte)
{
    fifo_t fifo;
    uint8_t buffer[5];
    uint8_t test_byte = 0xFE;

    EXPECT_EQ(fifo_init(&fifo, sizeof(uint8_t), buffer, 5), FIFO_OK);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_TRUE(fifo_put(&fifo, &test_byte));
    EXPECT_TRUE(fifo_put8(&fifo, 0xDC));
    EXPECT_TRUE(fifo_put8(&fifo, 0xBA));
    EXPECT_TRUE(fifo_put8(&fifo, 0x98));
    //TODO: Capacity is off by one, if fifo module is changed the test can be adapted too

    EXPECT_EQ(fifo_freespace(&fifo), 0);
    
    EXPECT_FALSE(fifo_put8(&fifo, 0x54));

    EXPECT_EQ(fifo_data_available(&fifo), 4);
    EXPECT_TRUE(fifo_is_full(&fifo));

    EXPECT_EQ(fifo_get_ptr(&fifo), buffer);

    EXPECT_EQ(fifo_data_available(&fifo), 3);

    EXPECT_EQ(fifo_get8(&fifo), 0xDC);
    EXPECT_EQ(fifo_get8(&fifo), 0xBA);

    EXPECT_EQ(fifo_data_available(&fifo), 1);
    EXPECT_EQ(fifo_freespace(&fifo), 3);

    EXPECT_EQ(fifo_get8(&fifo), 0x98);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_EQ(fifo_get8(&fifo), 0);
    EXPECT_FALSE(fifo_get(&fifo, &test_byte));
}

TEST(fifo_fifo, element_size_word)
{
    fifo_t fifo;
    uint8_t buffer[10];
    uint16_t test_word = 0xFEFE;

    EXPECT_EQ(fifo_init(&fifo, sizeof(uint16_t), buffer, 5), FIFO_OK);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_TRUE(fifo_put(&fifo, (uint8_t*)&test_word));
    EXPECT_TRUE(fifo_put16(&fifo, 0xDCDC));
    EXPECT_TRUE(fifo_put16(&fifo, 0xBABA));
    EXPECT_TRUE(fifo_put16(&fifo, 0x9898));
    //TODO: Capacity is off by one, if fifo module is changed the test can be adapted too

    EXPECT_EQ(fifo_freespace(&fifo), 0);
    
    EXPECT_FALSE(fifo_put16(&fifo, 0x5454));

    EXPECT_EQ(fifo_data_available(&fifo), 4);
    EXPECT_TRUE(fifo_is_full(&fifo));

    EXPECT_EQ(fifo_get_ptr(&fifo), buffer);

    EXPECT_EQ(fifo_data_available(&fifo), 3);

    EXPECT_EQ(fifo_get16(&fifo), 0xDCDC);
    EXPECT_EQ(fifo_get16(&fifo), 0xBABA);

    EXPECT_EQ(fifo_data_available(&fifo), 1);
    EXPECT_EQ(fifo_freespace(&fifo), 3);

    EXPECT_EQ(fifo_get16(&fifo), 0x9898);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_EQ(fifo_get16(&fifo), 0);
    EXPECT_FALSE(fifo_get(&fifo, (uint8_t*)&test_word));
}

TEST(fifo_fifo, element_size_dword)
{
    fifo_t fifo;
    uint8_t buffer[20];
    uint32_t test_word = 0xFEFEFEFE;

    EXPECT_EQ(fifo_init(&fifo, sizeof(uint32_t), buffer, 5), FIFO_OK);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_TRUE(fifo_put(&fifo, (uint8_t*)&test_word));
    EXPECT_TRUE(fifo_put32(&fifo, 0xDCDCDCDC));
    EXPECT_TRUE(fifo_put32(&fifo, 0xBABABABA));
    EXPECT_TRUE(fifo_put32(&fifo, 0x98989898));
    //TODO: Capacity is off by one, if fifo module is changed the test can be adapted too

    EXPECT_EQ(fifo_freespace(&fifo), 0);
    
    EXPECT_FALSE(fifo_put32(&fifo, 0x54545454));

    EXPECT_EQ(fifo_data_available(&fifo), 4);
    EXPECT_TRUE(fifo_is_full(&fifo));

    EXPECT_EQ(fifo_get_ptr(&fifo), buffer);

    EXPECT_EQ(fifo_data_available(&fifo), 3);

    EXPECT_EQ(fifo_get32(&fifo), 0xDCDCDCDC);
    EXPECT_EQ(fifo_get32(&fifo), 0xBABABABA);

    EXPECT_EQ(fifo_data_available(&fifo), 1);
    EXPECT_EQ(fifo_freespace(&fifo), 3);

    EXPECT_EQ(fifo_get32(&fifo), 0x98989898);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_EQ(fifo_get32(&fifo), 0);
    EXPECT_FALSE(fifo_get(&fifo, (uint8_t*)&test_word));
}

TEST(fifo_fifo, element_size_custom)
{
    fifo_t fifo;
    uint8_t test_data[][7] = 
    {
        {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE},
        {0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC, 0xDC},
        {0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA},
        {0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98},
        {0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54},
    };
    uint8_t buffer[35];

    EXPECT_EQ(fifo_init(&fifo, sizeof(test_data[0]), buffer, 5), FIFO_OK);
    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_TRUE(fifo_put(&fifo, test_data[0]));
    EXPECT_TRUE(fifo_put(&fifo, test_data[1]));
    EXPECT_TRUE(fifo_put(&fifo, test_data[2]));
    EXPECT_TRUE(fifo_put(&fifo, test_data[3]));
    //TODO: Capacity is off by one, if fifo module is changed the test can be adapted too

    EXPECT_EQ(fifo_freespace(&fifo), 0);
    
    EXPECT_FALSE(fifo_put(&fifo, test_data[4]));

    EXPECT_EQ(fifo_data_available(&fifo), 4);
    EXPECT_TRUE(fifo_is_full(&fifo));

    EXPECT_EQ(fifo_get_ptr(&fifo), buffer);

    EXPECT_EQ(fifo_data_available(&fifo), 3);

    uint8_t element[7];

    EXPECT_TRUE(fifo_get(&fifo, element));
    EXPECT_EQ(memcmp(element, test_data[1], sizeof(test_data[1])), 0);

    EXPECT_TRUE(fifo_get(&fifo, element));
    EXPECT_EQ(memcmp(element, test_data[2], sizeof(test_data[2])), 0);

    EXPECT_EQ(fifo_data_available(&fifo), 1);
    EXPECT_EQ(fifo_freespace(&fifo), 3);

    EXPECT_TRUE(fifo_get(&fifo, element));
    EXPECT_EQ(memcmp(element, test_data[3], sizeof(test_data[3])), 0);

    EXPECT_EQ(fifo_data_available(&fifo), 0);
    EXPECT_EQ(fifo_freespace(&fifo), 4);

    EXPECT_EQ(fifo_get32(&fifo), 0);
    EXPECT_FALSE(fifo_get(&fifo, element));
}