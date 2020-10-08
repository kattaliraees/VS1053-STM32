# VS1053-STM32
VS1053, VS1063 MP3 decoder STM32 Example code with 7 line interface (SCK, MOSI, MISO, RESET, DREQ, CS, DSC)

Driver APIs
```C
//For writing data to VS10xx registers at addr
void WriteSci(uint8_t addr, uint16_t data) {

	uint8_t d = 2;

	while(!HAL_GPIO_ReadPin(VS1053_DREQ_GPIO_Port, VS1053_DREQ_Pin)); //Wait until DREQ is high

	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 0); //Activate xCS (Chip select)
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //Write command code
	HAL_SPI_Transmit(&hspi2, &addr, 1, 50); //SCI Register number
	d = ((uint8_t)(data >> 8) & 0xFF);
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //LSB
	d = ((uint8_t)(data & 0xFF));
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //LSB
	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 1); //De-Activate xCS (Chip select off)
}

//For reading VS10xx register at addr
uint16_t ReadSci(uint8_t addr) {

	uint16_t res;

	uint8_t r;

	uint8_t d = 3;

	while(!HAL_GPIO_ReadPin(VS1053_DREQ_GPIO_Port, VS1053_DREQ_Pin)); //Wait until DREQ is high


	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 0); //Activate xCS (Chip select)
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //Read command code
	HAL_SPI_Transmit(&hspi2, &addr, 1, 50); //SCI Register number
	HAL_SPI_Receive(&hspi2, &r, 1, 50);
	res = ((uint16_t)(r << 8)) & 0xFF00;
	HAL_SPI_Receive(&hspi2, &r, 1, 50);
	res |= ((uint16_t)r & 0x00FF);
	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 1); //De-Activate xCS (Chip select off)

	return res;
}

//Sending MP3 raw bytes. Max 32 bytes at time without checking DREQ each time
int WriteSdi(uint8_t *data, uint8_t bytes){


	if(bytes > 32) {
		return -1;//Error - too many bytes to transfer
	}

	while(!HAL_GPIO_ReadPin(VS1053_DREQ_GPIO_Port, VS1053_DREQ_Pin));


	HAL_GPIO_WritePin(VS1053_DCS_GPIO_Port, VS1053_DCS_Pin, 0); //De-Activate xCS (Chip select off)


	HAL_SPI_Transmit(&hspi2, data, bytes, 50); //Read command code
	data++;

	HAL_GPIO_WritePin(VS1053_DCS_GPIO_Port, VS1053_DCS_Pin, 1);
	return 0;
}
