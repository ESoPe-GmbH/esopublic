/**
 * @file crc32.h
 * @author Tim Koczwara
 * 
 * @brief 
 * 
 * @version 1.00 (24.04.2025)
 * 	- Intial release
 * 
 * @par References
 * 
 **/

#ifndef __MODULE_CRC32_H_
#define __MODULE_CRC32_H_

#include "module.h"
#if MODULE_ENABLE_CRC
#include "module/enum/function_return.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Version of the crc32 module
#define CRC32_STR_VERSION "1.00"	

/// Default polynom used for CRC32 calculation
#define CRC32_POLYNOM_DEFAULT	0xedb88320
/// Default initial value used for CRC32 calculation
#define CRC32_INITIAL_DEFAULT	0xffffffff
/// Default final value used for CRC32 calculation
#define CRC32_FINAL_XOR_DEFAULT	0xffffffff
/// Initializes the crc32_t structure with default values
/// @param crc Pointer to the crc32_t structure that needs to be initialized
#define CRC32_INIT_DEFAULT(crc)	\
    crc32_init(crc, CRC32_POLYNOM_DEFAULT, CRC32_INITIAL_DEFAULT, CRC32_FINAL_XOR_DEFAULT);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Enumeration
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Structure
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * 	@struct crc32_t
 *
 * The structure is used for calculating the crc. It contains the initial value, a polynom used for calculation and
 * a final value that is used XOR the calculated crc.
 */
typedef struct crc32_s
{
	uint32_t polynom;		///< Generator polynom used in the calculation. For CRC-CCITT use 0x1021.
	uint32_t initial;		///< Initial value for the crc word before adding the first byte.
	uint32_t final_xor;		///< Final value that is calculated xor the calculated crc.
	bool reverse;			///< Reverses the crc calculation
	uint32_t crc;			///< Current crc value
    uint32_t* table;            ///< Table used for the calculation. If the table is not used, this value is NULL.
}crc32_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * Intializes the module with the given parameters. The default values are used if the parameters are not set.
**/
void crc32_init(crc32_t* crc, uint32_t polynom, uint32_t initial, uint32_t final_xor);
/**
 * @brief Start a CRC calculation. The initial value is set and the crc is reset to the initial value.
 * 
 * @param crc   Pointer to the crc32_t structure that needs to be initialized.
 */
void crc32_start(crc32_t* crc);
/**
 * @brief Updates the CRC with the given data. The data is processed byte by byte.
 * 
 * @param crc   Pointer to the crc32_t structure that needs to be updated.
 * @param data  Pointer to the data that needs to be processed.
 * @param length Length of the data that needs to be processed.
 */
void crc32_update(crc32_t* crc, const uint8_t* data, size_t length);
/**
 * @brief Finishes the CRC calculation and returns the calculated CRC value. The final value is xor the calculated crc.
 * 
 * @param crc   Pointer to the crc32_t structure that needs to be finished.
 * @return uint32_t  The calculated CRC value.
 */
uint32_t crc32_finish(crc32_t* crc);

#endif // MODULE_ENABLE_CRC32

#endif /* __MODULE_CRC32_H_ */