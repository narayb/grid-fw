/*
 * grid_sys.c
 *
 * Created: 4/12/2019 5:27:13 PM
 * Author : SUKU WC
*/

#include "grid_sys.h"
#include "grid_buf.h"
#include "stdio.h"
#include "string.h"




uint8_t grid_sys_error_intensity(struct grid_sys_model* mod){
	
	if (mod->error_style == 0){ // TRIANGLE
		
		return (125-abs(mod->error_state/2-125))/2;
	}
	else if (mod->error_style == 1){ // SQUARE
		
		return 255*(mod->error_state/250%2);
	}
	else if (mod->error_style == 2){ // CONST
		
		return 255*(mod->error_state>100);
	}
	
	
}

void grid_sys_error_set_color(struct grid_sys_model* mod, uint8_t red, uint8_t green, uint8_t blue){
	
	mod->color_red = red;
	mod->color_green = green;
	mod->color_blue = blue;
		
}

void grid_sys_error_set_alert(struct grid_sys_model* mod, uint8_t red, uint8_t green, uint8_t blue, uint8_t style, uint8_t duration){
	
	mod->color_red = red;
	mod->color_green = green;
	mod->color_blue = blue;
	
	mod->error_state = duration;
	mod->error_style = style;
	
}

uint8_t grid_sys_error_get_color_r(struct grid_sys_model* mod){
	
	return mod->color_red;
}

uint8_t grid_sys_error_get_color_g(struct grid_sys_model* mod){
	
	return mod->color_green;
}

uint8_t grid_sys_error_get_color_b(struct grid_sys_model* mod){
	
	return mod->color_blue;
}

uint8_t grid_sys_read_hex_char_value(uint8_t ascii, uint8_t* error_flag){
		
	uint8_t result = 0;
	
	if (ascii>47 && ascii<58){
		result = ascii-48;
	}
	else if(ascii>96 && ascii<103){
		result = ascii - 97 + 10;
	}
	else{
		// wrong input
		if (error_flag != NULL){
			*error_flag = ascii;
		}
	}
	
	return result;	
}

uint32_t grid_sys_read_hex_string_value(uint8_t* start_location, uint8_t length, uint8_t* error_flag){
	
	uint32_t result  = 0;
	
	for(uint8_t i=0; i<length; i++){
		
		result += grid_sys_read_hex_char_value(start_location[i], error_flag) << (length-i-1)*4;

		
	}

	return result;
}

void grid_sys_write_hex_string_value(uint8_t* start_location, uint8_t size, uint32_t value){
	
	uint8_t str[10];
	
	sprintf(str, "%08x", value);
		
	for(uint8_t i=0; i<size; i++){	
		start_location[i] = str[8-size+i];	
	}

}




uint32_t grid_sys_get_id(uint32_t* return_array){
			
	return_array[0] = *(uint32_t*)(GRID_SYS_UNIQUE_ID_ADDRESS_0);
	return_array[1] = *(uint32_t*)(GRID_SYS_UNIQUE_ID_ADDRESS_1);
	return_array[2] = *(uint32_t*)(GRID_SYS_UNIQUE_ID_ADDRESS_2);
	return_array[3] = *(uint32_t*)(GRID_SYS_UNIQUE_ID_ADDRESS_3);
	
	return 1;
	
}

uint32_t grid_sys_get_hwcfg(){
	
	// Read the register for the first time, then later just return the saved value

	if (grid_sys_hwfcg == -1){

		gpio_set_pin_direction(HWCFG_SHIFT, GPIO_DIRECTION_OUT);
		gpio_set_pin_direction(HWCFG_CLOCK, GPIO_DIRECTION_OUT);
		gpio_set_pin_direction(HWCFG_DATA, GPIO_DIRECTION_IN);
			
		// LOAD DATA
		gpio_set_pin_level(HWCFG_SHIFT, 0);
		delay_ms(1);
			
			
			
		uint8_t hwcfg_value = 0;
			
			
		for(uint8_t i = 0; i<8; i++){ // now we need to shift in the remaining 7 values
				
			// SHIFT DATA
			gpio_set_pin_level(HWCFG_SHIFT, 1); //This outputs the first value to HWCFG_DATA
			delay_ms(1);
				
				
			if(gpio_get_pin_level(HWCFG_DATA)){
					
				hwcfg_value |= (1<<i);
					
				}else{
					
					
			}
				
			if(i!=7){
					
				// Clock rise
				gpio_set_pin_level(HWCFG_CLOCK, 1);
					
				delay_ms(1);
					
				gpio_set_pin_level(HWCFG_CLOCK, 0);
			}
							
		}
		
		grid_sys_hwfcg = hwcfg_value;
		
	}

	
	return grid_sys_hwfcg;

}


#define GRID_SYS_NORTH	0
#define GRID_SYS_EAST	1
#define GRID_SYS_SOUTH	2
#define GRID_SYS_WEST	3

volatile uint8_t grid_sys_ping_counter[4] = {0, 0, 0, 0};

volatile uint32_t grid_sys_rx_counter[4] = {0, 0, 0, 0};
volatile uint32_t grid_sys_tx_counter[4] = {0, 0, 0, 0};



//=============================== USART TX COMPLETE ==============================//

static void tx_cb_USART_GRID_N(const struct usart_async_descriptor *const descr)
{
	tx_cb_USART_GRID(&GRID_PORT_N);
}

static void tx_cb_USART_GRID_E(const struct usart_async_descriptor *const descr)
{
	tx_cb_USART_GRID(&GRID_PORT_E);
}

static void tx_cb_USART_GRID_S(const struct usart_async_descriptor *const descr)
{
	tx_cb_USART_GRID(&GRID_PORT_S);
}

static void tx_cb_USART_GRID_W(const struct usart_async_descriptor *const descr)
{
	tx_cb_USART_GRID(&GRID_PORT_W);
}

void tx_cb_USART_GRID(GRID_PORT_t* const por){
	
	grid_sys_tx_counter[por->direction - 0x11]++;

	
	if(por->tx_double_buffer[1] == GRID_MSG_BROADCAST){
		// SHOULD WAIT FOR ACKNOWLEDGE
		
		// BUT WE DON'T
		for(uint8_t i=0; i<GRID_DOUBLE_BUFFER_TX_SIZE; i++){
			por->tx_double_buffer[i] = 0;
		}
		
		por->tx_double_buffer_status = 0;
	}
	else if (por->tx_double_buffer[1] == GRID_MSG_DIRECT){
		// NO NEED TO WAIT FOR ACKNOWLEDGE
		for(uint8_t i=0; i<GRID_DOUBLE_BUFFER_TX_SIZE; i++){
			por->tx_double_buffer[i] = 0;
		}
		
		por->tx_double_buffer_status = 0;
	}
	else{
		//TRAP
		for(uint8_t i=0; i<GRID_DOUBLE_BUFFER_TX_SIZE; i++){
			por->tx_double_buffer[i] = 0;
		}
		
		por->tx_double_buffer_status = 0;
	}
	
}





static void rx_cb_USART_GRID_N(const struct usart_async_descriptor *const descr)
{
	rx_cb_USART_GRID(&GRID_PORT_N);
}

static void rx_cb_USART_GRID_E(const struct usart_async_descriptor *const descr)
{
	rx_cb_USART_GRID(&GRID_PORT_E);
}

static void rx_cb_USART_GRID_S(const struct usart_async_descriptor *const descr)
{
	rx_cb_USART_GRID(&GRID_PORT_S);
}

static void rx_cb_USART_GRID_W(const struct usart_async_descriptor *const descr)
{
	rx_cb_USART_GRID(&GRID_PORT_W);
}

void rx_cb_USART_GRID(GRID_PORT_t* const por){
			
}





void grid_port_process_inbound_all(){
	
// 	grid_port_process_inbound(&GRID_PORT_N);
// 	grid_port_process_inbound(&GRID_PORT_E);
// 	grid_port_process_inbound(&GRID_PORT_S);
// 	grid_port_process_inbound(&GRID_PORT_W);
		
}	

void grid_sys_ping_all(){
		
	grid_sys_ping(&GRID_PORT_N);
	grid_sys_ping(&GRID_PORT_E);
	grid_sys_ping(&GRID_PORT_S);
	grid_sys_ping(&GRID_PORT_W);
	
}

uint8_t grid_msg_get_checksum(uint8_t* str, uint32_t length){
	
	uint8_t checksum = 0;
	for (uint32_t i=0; i<length-3; i++){
		checksum ^= str[i];
	}
	
	return checksum;
	
}

uint8_t grid_msg_set_checksum(uint8_t* message, uint32_t length, uint8_t checksum){
	
	uint8_t checksum_string[4];

	sprintf(checksum_string, "%02x", checksum);

	message[length-3] = checksum_string[0];
	message[length-2] = checksum_string[1];
	
}


// MESSAGE PARAMETER FUNCTIONS

uint8_t grid_msg_get_id(uint8_t* message){
	
	uint8_t error = 0;
	return grid_sys_read_hex_string_value(&message[4], 2, &error);
	
}
uint8_t grid_msg_get_dx(uint8_t* message){
	
	uint8_t error = 0;
	return grid_sys_read_hex_string_value(&message[6], 2, &error);	
	
}
uint8_t grid_msg_get_dy(uint8_t* message){
	
	uint8_t error = 0;
	return grid_sys_read_hex_string_value(&message[8], 2, &error);	

}
uint8_t grid_msg_get_age(uint8_t* message){
	
	uint8_t error = 0;
	return grid_sys_read_hex_string_value(&message[10], 2, &error);	
	
}

void grid_msg_set_id(uint8_t* message, uint8_t param){
	
	grid_sys_write_hex_string_value(&message[4], 2, param);
	
}
void grid_msg_set_dx(uint8_t* message, uint8_t param){
	
	grid_sys_write_hex_string_value(&message[6], 2, param);
	
}
void grid_msg_set_dy(uint8_t* message, uint8_t param){

	grid_sys_write_hex_string_value(&message[8], 2, param);

}
void grid_msg_set_age(uint8_t* message, uint8_t param){
	
	grid_sys_write_hex_string_value(&message[10], 2, param);
	
}

// RECENT MESSAGES

uint8_t grid_msg_find_recent(struct grid_sys_model* model, uint32_t fingerprint){
	
	for(GRID_SYS_RECENT_MESSAGES_INDEX_T i = 0; i<GRID_SYS_RECENT_MESSAGES_LENGTH; i++){
		
		if (model->recent_messages[i%GRID_SYS_RECENT_MESSAGES_LENGTH] == fingerprint){
			
			return 1;
			
		}
		
	}
	
	return 0;
}

void grid_msg_push_recent(struct grid_sys_model* model, uint32_t fingerprint){
	
	model->recent_messages_index+=1;
	model->recent_messages_index%=GRID_SYS_RECENT_MESSAGES_LENGTH;
	
	model->recent_messages[model->recent_messages_index] = fingerprint;
	
}




void grid_sys_ping(GRID_PORT_t* por){
		
	char message[20];
	uint8_t length = 0;
	
	// Create the packet
	sprintf(message, "%c%c%c%c%08x%c00\n", GRID_MSG_START_OF_HEADING, GRID_MSG_DIRECT, GRID_MSG_BELL, por->direction ,grid_sys_get_hwcfg(), GRID_MSG_END_OF_TRANSMISSION);
	
	// Calculate packet length
	length = strlen(message);

	grid_msg_set_checksum(message, length, grid_msg_get_checksum(message, length));
		
	// Put the packet into the tx_buffer
	if (grid_buffer_write_init(&por->tx_buffer, length)){
		
		for(uint16_t i = 0; i<length; i++){
			
			grid_buffer_write_character(&por->tx_buffer, message[i]);
		}
		
		grid_buffer_write_acknowledge(&por->tx_buffer);
	}
				
	
}


	
	
	
	