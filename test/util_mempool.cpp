#include <gtest/gtest.h>

extern "C"
{
    #include "module/util/mem_pool.h"

    void app_main_init(void)
    {

    }

    void board_init(void)
    {
        
    }
}

class UtilMempoolTest : public ::testing::Test 
{
    protected:

    void SetUp() override 
    {
        ret = mem_pool_init(&pool, 12, 512);
    }

    void TearDown() override
    {
        mem_pool_free(pool, true);
    }

    FUNCTION_RETURN ret;
    mem_pool_handle_t pool;
   
};

TEST_F(UtilMempoolTest, AllocWriteRead)
{
    mem_pool_chunk_handle_t chunk, chunk2;
    uint8_t buffer[10];
    size_t count;

    ASSERT_EQ(ret, FUNCTION_RETURN_OK) << "Pool init failed\n";

	//------------------------------------------------
	// Test allocation of pool and chunk
	//------------------------------------------------

    ret = mem_pool_alloc_chunk(pool, &chunk, 3000);
    ASSERT_EQ(ret, FUNCTION_RETURN_OK) << "Chunk Alloc Failed";

    ASSERT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 6);
    ASSERT_EQ(mem_pool_get_chunk_free_size(chunk), 512*6);
    ASSERT_EQ(mem_pool_get_chunk_total_size(chunk), 512*6);

	//------------------------------------------------
	// Test writing in a chunk
	//------------------------------------------------

    count = mem_pool_append_chunk(chunk, (char*)"WTF", 3, true);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 6);
    EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 3);
    EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), (512 * 6) - 3);

    count = mem_pool_write_chunk(chunk, 1076, (char*)"WTF", 3, true);
    EXPECT_EQ(count, 3);
    EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 6);
    EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 1079);
    EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), (512 * 6) - 1079);

    count = mem_pool_write_chunk(chunk, 1022, (char*)"STFU", 4, true);
    EXPECT_EQ(count, 4);
    EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 6);
    EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 1079);
    EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), (512 * 6) - 1079);

    ret = mem_pool_realloc_chunk(chunk, mem_pool_get_chunk_used_size(chunk));
    ASSERT_EQ(ret, FUNCTION_RETURN_OK) << "Chunk Re-Alloc Failed";

    EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 3);
    EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 1079);
    EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), (512 * 3) - 1079);

    count = mem_pool_write_chunk(chunk, (512 * 3) - 1, (char*)"STFU", 4, true);
    EXPECT_EQ(count, 4);
    EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), (512 * 4));
    EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), (512 * 3) + 3);
    EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), (512 * 4) - (512 * 3 + 3));

    count = mem_pool_write_chunk(chunk, (512 * 4) - 2, (char*)"WTF", 3, false);
    EXPECT_EQ(count, 2);
    EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 4);
    EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 512 * 4);
    EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), 0);

    //------------------------------------------------
    // Test reading the written data from a chunk
    //----------------------------------------------
	const uint8_t at_0[] = {'W', 'T', 'F', 0};
	count = mem_pool_read_chunk(chunk, 0, buffer, 4);
	EXPECT_EQ(count, 4);
	EXPECT_EQ(memcmp(buffer, at_0, 4), 0);

    const uint8_t at_1076[] = {0, 'W', 'T', 'F', 0};
	count = mem_pool_read_chunk(chunk, 1075, buffer, 5);
	EXPECT_EQ(count, 5);
	EXPECT_EQ(memcmp(buffer, at_1076, 4), 0);

	const uint8_t at_1022[] = {0, 'S', 'T', 'F', 'U', 0};
	count = mem_pool_read_chunk(chunk, 1021, buffer, 6);
	EXPECT_EQ(count, 6);
	EXPECT_EQ(memcmp(buffer, at_1022, 4), 0);

	const uint8_t at_1535[] = {0, 'S', 'T', 'F', 'U', 0};
	count = mem_pool_read_chunk(chunk, (512 * 3) - 2, buffer, 6);
	EXPECT_EQ(count, 6);
	EXPECT_EQ(memcmp(buffer, at_1535, 4), 0);

	const uint8_t at_2046[] = {0, 'W', 'T', 'F', 0};
	count = mem_pool_read_chunk(chunk, (512 * 4) - 3, buffer, 5);
	EXPECT_EQ(count, 3);
	EXPECT_EQ(memcmp(buffer, at_2046, 3), 0);

    //------------------------------------------------
    // Test allocation of multiple chunks
	//------------------------------------------------
    ret = mem_pool_alloc_chunk(pool, &chunk2, 3000);
	ASSERT_EQ(ret, FUNCTION_RETURN_OK);
	EXPECT_EQ(mem_pool_get_chunk_total_size(chunk2), 512 * 6);
	EXPECT_EQ(mem_pool_get_chunk_free_size(chunk2), 512 * 6);
	EXPECT_EQ(mem_pool_get_chunk_used_size(chunk2), 0);

	ret = mem_pool_realloc_chunk(chunk, 512 * 6);
	ASSERT_EQ(ret, FUNCTION_RETURN_OK);
	EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 6);
	EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 512 * 4);
	EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), 512 * 2);

	EXPECT_EQ(mem_pool_get_chunk_total_size(chunk2), 512 * 6);
	EXPECT_EQ(mem_pool_get_chunk_free_size(chunk2), 512 * 6);
	EXPECT_EQ(mem_pool_get_chunk_used_size(chunk2), 0);

	ret = mem_pool_free_chunk(chunk2);
    ASSERT_EQ(ret, FUNCTION_RETURN_OK);
    
	ret = mem_pool_realloc_chunk(chunk, 512 * 8);
	ASSERT_EQ(ret, FUNCTION_RETURN_OK);

	EXPECT_EQ(mem_pool_get_chunk_total_size(chunk), 512 * 8);
	EXPECT_EQ(mem_pool_get_chunk_used_size(chunk), 512 * 4);
	EXPECT_EQ(mem_pool_get_chunk_free_size(chunk), 512 * 4);
    
	//------------------------------------------------
	// Test deallocation
	//------------------------------------------------

	ret = mem_pool_free(pool, false);
	ASSERT_EQ(ret, FUNCTION_RETURN_NOT_READY);

	ret = mem_pool_free_chunk(chunk);
	ASSERT_EQ(ret, FUNCTION_RETURN_OK);
}