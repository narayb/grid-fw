#include "grid_ui.h"



void grid_port_process_ui(struct grid_port* por){
	
	// Priorities: Always process local, try to process direct, broadcast messages are last. 
	
	
	uint8_t message_broadcast_action_available = 0;
	uint8_t message_local_action_available = 0;

	
	// UI STATE
	for (uint8_t i=0; i<grid_ui_state.bank_list_length; i++){
		
		for (uint8_t j=0; j<grid_ui_state.bank_list[i].element_list_length; j++){
			
			for (uint8_t k=0; k<grid_ui_state.bank_list[i].element_list[j].event_list_length; k++){
				
				if (grid_ui_event_istriggered(&grid_ui_state.bank_list[i].element_list[j].event_list[k])){

					message_broadcast_action_available++;

					
				}
                
				if (grid_ui_event_istriggered_local(&grid_ui_state.bank_list[i].element_list[j].event_list[k])){
                    
                    message_local_action_available++;

					
				}
				
			}
			
		}	
	}		
	
	// CORE SYSTEM
	for (uint8_t i=0; i<grid_core_state.bank_list[0].element_list_length; i++){
		
		for (uint8_t j=0; j<grid_core_state.bank_list[0].element_list[i].event_list_length; j++){
			
			if (grid_ui_event_istriggered(&grid_core_state.bank_list[0].element_list[i].event_list[j])){
				
				message_broadcast_action_available++;
			}
			
		}
		
	}	
	
	
	
	//NEW PING
	struct grid_port* port[4] = {&GRID_PORT_N, &GRID_PORT_E, &GRID_PORT_S, &GRID_PORT_W};
	
	for (uint8_t k = 0; k<4; k++){
		
		if (port[k]->ping_flag == 1){
		
			if (grid_buffer_write_init(&port[k]->tx_buffer, port[k]->ping_packet_length)){
				//Success
				for(uint32_t i = 0; i<port[k]->ping_packet_length; i++){
					grid_buffer_write_character(&port[k]->tx_buffer, port[k]->ping_packet[i]);
				}
				grid_buffer_write_acknowledge(&port[k]->tx_buffer);
			}
			port[k]->ping_flag = 0;
		}		
			
	}			

	
	
		

	
	//LOCAL MESSAGES
	if (message_local_action_available){
		
	
		struct grid_msg message;
		grid_msg_init(&message);
		grid_msg_init_header(&message, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_ROTATION);
			
			
		// Prepare packet header
		uint8_t payload[GRID_PARAMETER_PACKET_maxlength] = {0};				
		uint32_t offset=0;
		
		
		
		// UI STATE
		for (uint8_t i=0; i<grid_ui_state.bank_list_length; i++)
		{
			for (uint8_t j=0; j<grid_ui_state.bank_list[i].element_list_length; j++){
				
                for (uint8_t k=0; k<grid_ui_state.bank_list[i].element_list[j].event_list_length; k++){
				
					if (offset>GRID_PARAMETER_PACKET_marign){
						continue;
					}		
					else{
						
						CRITICAL_SECTION_ENTER()
						if (grid_ui_event_istriggered_local(&grid_ui_state.bank_list[i].element_list[j].event_list[k])){
							
                            offset += grid_ui_event_render_action(&grid_ui_state.bank_list[i].element_list[j].event_list[k], &payload[offset]);
                            grid_ui_event_reset(&grid_ui_state.bank_list[i].element_list[j].event_list[k]);
                        
						}
						CRITICAL_SECTION_LEAVE()
						
					}
				
				}
                
				

				
			}
		}
		
		
		grid_msg_body_append_text(&message, payload, offset);
		grid_msg_packet_close(&message);
			
		uint32_t message_length = grid_msg_packet_get_length(&message);
			
		// Put the packet into the UI_TX buffer
		if (grid_buffer_write_init(&GRID_PORT_U.tx_buffer, message_length)){
				
			for(uint32_t i = 0; i<message_length; i++){
					
				grid_buffer_write_character(&GRID_PORT_U.tx_buffer, grid_msg_packet_send_char(&message, i));
			}
				
			grid_buffer_write_acknowledge(&GRID_PORT_U.tx_buffer);
			
// 			uint8_t debug_string[200] = {0};
// 			sprintf(debug_string, "Space: RX: %d/%d  TX: %d/%d", grid_buffer_get_space(&GRID_PORT_U.rx_buffer), GRID_BUFFER_SIZE, grid_buffer_get_space(&GRID_PORT_U.tx_buffer), GRID_BUFFER_SIZE);
// 			grid_debug_print_text(debug_string);


		}
		else{
			// LOG UNABLE TO WRITE EVENT
		}
			
	}
	
	
	
	
	// Bandwidth Limiter for Broadcast messages
	
	if (por->cooldown > 0){
		por->cooldown--;
	}
	
	
	if (por->cooldown > 10){
				
		return;
	}
		
	
	//BROADCAST MESSAGES		
	if (message_broadcast_action_available){
			
		struct grid_msg message;
		grid_msg_init(&message);
		grid_msg_init_header(&message, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_ROTATION);
		

		// CORE SYSTEM
		for (uint8_t i=0; i<grid_core_state.bank_list[0].element_list_length; i++){
			
			for (uint8_t j=0; j<grid_core_state.bank_list[0].element_list[i].event_list_length; j++){
				
				if (grid_msg_packet_get_length(&message)>GRID_PARAMETER_PACKET_marign){
					continue;
				}
				else{
					
					CRITICAL_SECTION_ENTER()
					if (grid_ui_event_istriggered(&grid_core_state.bank_list[0].element_list[i].event_list[j])){
						
						uint32_t offset = grid_msg_body_get_length(&message); 
						message.body_length += grid_ui_event_render_action(&grid_core_state.bank_list[0].element_list[i].event_list[j], &message.body[offset]);
						grid_ui_event_reset(&grid_core_state.bank_list[0].element_list[i].event_list[j]);
						
					}
					CRITICAL_SECTION_LEAVE()
									
					
				}						
				

				
			}
			
		}
		
		// UI STATE
		for (uint8_t i=0; i<grid_ui_state.bank_list_length; i++){
			for (uint8_t j=0; j<grid_ui_state.bank_list[i].element_list_length; j++){
			
				for (uint8_t k=0; k<grid_ui_state.bank_list[i].element_list[j].event_list_length; k++){ //j=1 because init is local
				
					if (grid_msg_packet_get_length(&message)>GRID_PARAMETER_PACKET_marign){
						continue;
					}		
					else{
						
						CRITICAL_SECTION_ENTER()
						if (grid_ui_event_istriggered(&grid_ui_state.bank_list[i].element_list[j].event_list[k])){
							
							uint32_t offset = grid_msg_body_get_length(&message); 
							message.body_length += grid_ui_event_render_action(&grid_ui_state.bank_list[i].element_list[j].event_list[k], &message.body[offset]);
							grid_ui_event_reset(&grid_ui_state.bank_list[i].element_list[j].event_list[k]);
							
						}
						CRITICAL_SECTION_LEAVE()
						
					}
				
				}
			
			}
		}
			

		
		//por->cooldown += (2+por->cooldown/2);
		por->cooldown += 10;
		//por->cooldown = 3;
		
		

		grid_msg_packet_close(&message);
		uint32_t length = grid_msg_packet_get_length(&message);
		

		// Put the packet into the UI_RX buffer
		if (grid_buffer_write_init(&GRID_PORT_U.rx_buffer, length)){
	
			for(uint16_t i = 0; i<length; i++){
				
				grid_buffer_write_character(&GRID_PORT_U.rx_buffer, grid_msg_packet_send_char(&message, i));
			}
			
			grid_buffer_write_acknowledge(&GRID_PORT_U.rx_buffer);

			
		}
		else{
			// LOG UNABLE TO WRITE EVENT
		}
		
		
		
	
	}
	
}


void grid_ui_model_init(struct grid_ui_model* mod, uint8_t bank_list_length){
	
	mod->status = GRID_UI_STATUS_INITIALIZED;
	
	mod->bank_list_length = bank_list_length;	
	mod->bank_list = malloc(mod->bank_list_length*sizeof(struct grid_ui_bank));
	
	for(uint8_t i=0; i<bank_list_length; i++){
		
		mod->bank_list[i].status = GRID_UI_STATUS_UNDEFINED;		
		mod->bank_list[i].element_list_length = 0;
		
	}
	
}

void grid_ui_bank_init(struct grid_ui_model* parent, uint8_t index, uint8_t element_list_length){
	
	struct grid_ui_bank* bank = &parent->bank_list[index];
	bank->parent = parent;
	bank->index = index;
	
	
	bank->status = GRID_UI_STATUS_INITIALIZED;
	
	bank->element_list_length = element_list_length;
	bank->element_list = malloc(bank->element_list_length*sizeof(struct grid_ui_element));
	
	for(uint8_t i=0; i<element_list_length; i++){
		
		bank->element_list[i].status = GRID_UI_STATUS_UNDEFINED;
		bank->element_list[i].event_list_length = 0;
		
	}
	
}



void grid_ui_element_init(struct grid_ui_bank* parent, uint8_t index, enum grid_ui_element_t element_type){
	
	struct grid_ui_element* ele = &parent->element_list[index];
	ele->parent = parent;
	ele->index = index;


	ele->status = GRID_UI_STATUS_INITIALIZED;
	
	ele->type = element_type;
	
	// initialize all of the A template parameter values
	for(uint8_t i=0; i<GRID_TEMPLATE_UI_PARAMETER_LIST_LENGTH; i++){
		ele->template_parameter_list[i] = 0;
	}

	
	if (element_type == GRID_UI_ELEMENT_SYSTEM){
		
		ele->event_list_length = 6;
		
		ele->event_list = malloc(ele->event_list_length*sizeof(struct grid_ui_event));
		grid_ui_event_init(ele, 0, GRID_UI_EVENT_INIT); // Element Initialization Event
		grid_ui_event_init(ele, 1, GRID_UI_EVENT_HEARTBEAT); // Heartbeat
		grid_ui_event_init(ele, 2, GRID_UI_EVENT_MAPMODE_PRESS); // Mapmode press
		grid_ui_event_init(ele, 3, GRID_UI_EVENT_MAPMODE_RELEASE); // Mapmode release
		grid_ui_event_init(ele, 4, GRID_UI_EVENT_CFG_RESPONSE); //
		grid_ui_event_init(ele, 5, GRID_UI_EVENT_CFG_REQUEST); //
		
	}
	else if (element_type == GRID_UI_ELEMENT_POTENTIOMETER){
		
		ele->event_list_length = 2;
		
		ele->event_list = malloc(ele->event_list_length*sizeof(struct grid_ui_event));
		
		grid_ui_event_init(ele, 0, GRID_UI_EVENT_INIT); // Element Initialization Event
		grid_ui_event_init(ele, 1, GRID_UI_EVENT_AVC7); // Absolute Value Change (7bit)
		
		
	}
	else if (element_type == GRID_UI_ELEMENT_BUTTON){
		
		ele->event_list_length = 3;
		
		ele->event_list = malloc(ele->event_list_length*sizeof(struct grid_ui_event));
		
		grid_ui_event_init(ele, 0, GRID_UI_EVENT_INIT); // Element Initialization Event
		grid_ui_event_init(ele, 1, GRID_UI_EVENT_DP);	// Press
		grid_ui_event_init(ele, 2, GRID_UI_EVENT_DR);	// Release
		
	}
	else if (element_type == GRID_UI_ELEMENT_ENCODER){
		
		ele->event_list_length = 5;
		
		ele->event_list = malloc(ele->event_list_length*sizeof(struct grid_ui_event));
		
		grid_ui_event_init(ele, 0, GRID_UI_EVENT_INIT); // Element Initialization Event
		grid_ui_event_init(ele, 1, GRID_UI_EVENT_DP);	// Press
		grid_ui_event_init(ele, 2, GRID_UI_EVENT_DR);	// Release
		grid_ui_event_init(ele, 3, GRID_UI_EVENT_AVC7); // Absolute Value Change (7bit)
		grid_ui_event_init(ele, 4, GRID_UI_EVENT_ENCPUSHROT); // Absolute Value Change (7bit)
		
	}
	else{
		//UNKNOWN ELEMENT TYPE
	}
	
}

void grid_ui_event_init(struct grid_ui_element* parent, uint8_t index, enum grid_ui_event_t event_type){
	
	struct grid_ui_event* eve = &parent->event_list[index];
	eve->parent = parent;
	eve->index = index;

	eve->cfg_changed_flag = 0;
	
	eve->status = GRID_UI_STATUS_INITIALIZED;
	
	eve->type   = event_type;	
	eve->status = GRID_UI_STATUS_READY;


	// Initializing Event String	
	for (uint32_t i=0; i<GRID_UI_EVENT_STRING_maxlength; i++){
		eve->event_string[i] = 0;
	}
	
	eve->event_string_length = 0;
	

	// Initializing Action String
	for (uint32_t i=0; i<GRID_UI_ACTION_STRING_maxlength; i++){
		eve->action_string[i] = 0;
	}	
	
	eve->action_string_length = 0;
	

			
	grid_ui_event_generate_eventstring(eve->parent, event_type);
	grid_ui_event_generate_actionstring(eve->parent, event_type);	
	
	eve->cfg_changed_flag = 0;
	eve->cfg_default_flag = 1;
	eve->cfg_flashempty_flag = 1;
	
}


void grid_ui_reinit(struct grid_ui_model* ui){
	
	for(uint8_t i = 0; i<ui->bank_list_length; i++){
		
		struct grid_ui_bank* bank = &ui->bank_list[i];
		
		for (uint8_t j=0; j<bank->element_list_length; j++){
			
			struct grid_ui_element* ele = &bank->element_list[j];
			
			for (uint8_t k=0; k<ele->event_list_length; k++){
				
				struct grid_ui_event* eve = &ele->event_list[k];
				
				grid_ui_event_generate_actionstring(ele, eve->type);
				
				
				grid_ui_event_reset(eve);
                
                
                //grid_ui_smart_trigger_local(ui, i, j, eve->type);
				
			}
			
		}
		
	}
	
	grid_sys_state.bank_active_changed = 1;
	
}


void grid_ui_reinit_local(struct grid_ui_model* ui){
	
	for(uint8_t i = 0; i<ui->bank_list_length; i++){
		
		struct grid_ui_bank* bank = &ui->bank_list[i];
		
		for (uint8_t j=0; j<bank->element_list_length; j++){
			
			struct grid_ui_element* ele = &bank->element_list[j];
			
			for (uint8_t k=0; k<ele->event_list_length; k++){
				
				struct grid_ui_event* eve = &ele->event_list[k];
				
				grid_ui_event_generate_actionstring(ele, eve->type);
				
				
				grid_ui_event_reset(eve);
                
                
                grid_ui_smart_trigger_local(ui, i, j, eve->type);
				
			}
			
		}
		
	}
	
	grid_sys_state.bank_active_changed = 1;
	
}



void grid_ui_nvm_store_all_configuration(struct grid_ui_model* ui, struct grid_nvm_model* nvm){
	
    grid_nvm_ui_bulk_store_init(nvm, ui);

}

void grid_ui_nvm_load_all_configuration(struct grid_ui_model* ui, struct grid_nvm_model* nvm){
	
	grid_nvm_ui_bulk_read_init(nvm, ui);

		
	
}

void grid_ui_nvm_clear_all_configuration(struct grid_ui_model* ui, struct grid_nvm_model* nvm){
	
	grid_nvm_ui_bulk_clear_init(nvm, ui);

}


uint8_t grid_ui_recall_event_configuration(struct grid_ui_model* ui, uint8_t bank, uint8_t element, enum grid_ui_event_t event_type){
	
	struct grid_ui_element* ele = NULL;
	struct grid_ui_event* eve = NULL;
	uint8_t event_index = 255;
	
	if (bank < ui->bank_list_length){
		
		if (element < ui->bank_list[bank].element_list_length){
			
			ele = &ui->bank_list[bank].element_list[element];
			
			for(uint8_t i=0; i<ele->event_list_length; i++){
				if (ele->event_list[i].type == event_type){
					event_index = i;
					eve = &ele->event_list[event_index];
				}
			}	
			
		}
		
		
	}
	
	
	if (event_index != 255){ // OK
		
		struct grid_msg message;

		grid_msg_init(&message);
		grid_msg_init_header(&message, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_ROTATION);


		uint8_t payload[GRID_PARAMETER_PACKET_maxlength] = {0};
		uint8_t payload_length = 0;
		uint32_t offset = 0;



		// BANK ENABLED
		offset = grid_msg_body_get_length(&message);

		sprintf(payload, GRID_CLASS_CONFIGURATION_frame_start);
		payload_length = strlen(payload);

		grid_msg_body_append_text(&message, payload, payload_length);

		grid_msg_text_set_parameter(&message, offset, GRID_INSTR_offset, GRID_INSTR_length, GRID_INSTR_REPORT_code);
		grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_BANKNUMBER_offset, GRID_CLASS_CONFIGURATION_BANKNUMBER_length, eve->parent->parent->index);
		grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_ELEMENTNUMBER_offset, GRID_CLASS_CONFIGURATION_ELEMENTNUMBER_length, eve->parent->index);
		grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_EVENTTYPE_offset, GRID_CLASS_CONFIGURATION_EVENTTYPE_length, eve->type);

		offset = grid_msg_body_get_length(&message);
		grid_msg_body_append_text_escaped(&message, eve->action_string, eve->action_string_length);





		sprintf(payload, GRID_CLASS_CONFIGURATION_frame_end);
		payload_length = strlen(payload);

		grid_msg_body_append_text(&message, payload, payload_length);


		grid_msg_packet_close(&message);
		grid_msg_packet_send_everywhere(&message);
		
	}
	else{ // INVALID REQUEST
		
		struct grid_msg message;

		grid_msg_init(&message);
		grid_msg_init_header(&message, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_POSITION, GRID_SYS_DEFAULT_ROTATION);


		uint8_t payload[GRID_PARAMETER_PACKET_maxlength] = {0};
		uint8_t payload_length = 0;
		uint32_t offset = 0;

		// BANK ENABLED
		offset = grid_msg_body_get_length(&message);

		sprintf(payload, GRID_CLASS_CONFIGURATION_frame_start);
		payload_length = strlen(payload);

		grid_msg_body_append_text(&message, payload, payload_length);

		grid_msg_text_set_parameter(&message, offset, GRID_INSTR_offset, GRID_INSTR_length, GRID_INSTR_NACKNOWLEDGE_code);
		grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_BANKNUMBER_offset, GRID_CLASS_CONFIGURATION_BANKNUMBER_length, bank);
		grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_ELEMENTNUMBER_offset, GRID_CLASS_CONFIGURATION_ELEMENTNUMBER_length, element);
		grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_EVENTTYPE_offset, GRID_CLASS_CONFIGURATION_EVENTTYPE_length, event_type);


		sprintf(payload, GRID_CLASS_CONFIGURATION_frame_end);
		payload_length = strlen(payload);

		grid_msg_body_append_text(&message, payload, payload_length);


		grid_msg_packet_close(&message);
		grid_msg_packet_send_everywhere(&message);		
		
		
	}

	
}



uint8_t grid_ui_nvm_store_event_configuration(struct grid_ui_model* ui, struct grid_nvm_model* nvm, struct grid_ui_event* eve){
	

	struct grid_msg message;

	grid_msg_init(&message);
	grid_msg_init_header(&message, GRID_SYS_LOCAL_POSITION, GRID_SYS_LOCAL_POSITION, GRID_SYS_DEFAULT_ROTATION);


	uint8_t payload[GRID_PARAMETER_PACKET_maxlength] = {0};
	uint8_t payload_length = 0;
	uint32_t offset = 0;



	// BANK ENABLED
	offset = grid_msg_body_get_length(&message);

	sprintf(payload, GRID_CLASS_CONFIGURATION_frame_start);
	payload_length = strlen(payload);

	grid_msg_body_append_text(&message, payload, payload_length);

	grid_msg_text_set_parameter(&message, offset, GRID_INSTR_offset, GRID_INSTR_length, GRID_INSTR_EXECUTE_code);
	grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_BANKNUMBER_offset, GRID_CLASS_CONFIGURATION_BANKNUMBER_length, eve->parent->parent->index);
	grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_ELEMENTNUMBER_offset, GRID_CLASS_CONFIGURATION_ELEMENTNUMBER_length, eve->parent->index);
	grid_msg_text_set_parameter(&message, offset, GRID_CLASS_CONFIGURATION_EVENTTYPE_offset, GRID_CLASS_CONFIGURATION_EVENTTYPE_length, eve->type);

	offset = grid_msg_body_get_length(&message);
	grid_msg_body_append_text_escaped(&message, eve->action_string, eve->action_string_length);





	sprintf(payload, GRID_CLASS_CONFIGURATION_frame_end);
	payload_length = strlen(payload);

	grid_msg_body_append_text(&message, payload, payload_length);


	grid_msg_packet_close(&message);

	grid_nvm_clear_write_buffer(nvm);

	uint32_t message_length = grid_msg_packet_get_length(&message);

	if (message_length){

		nvm->write_buffer_length = message_length;
	
		for(uint32_t i = 0; i<message_length; i++){
		
			nvm->write_buffer[i] = grid_msg_packet_send_char(&message, i);
		}

	}

	uint32_t event_page_offset = grid_nvm_calculate_event_page_offset(nvm, eve);
	nvm->write_target_address = GRID_NVM_LOCAL_BASE_ADDRESS + GRID_NVM_PAGE_OFFSET*event_page_offset;

	int status = 0;
	
	
	uint8_t debugtext[200] = {0};

	if (eve->cfg_default_flag == 1 && eve->cfg_flashempty_flag == 0){
		
		//sprintf(debugtext, "Cfg: Default B:%d E:%d Ev:%d => Page: %d Status: %d", eve->parent->parent->index, eve->parent->index, eve->index, event_page_offset, status);
		flash_erase(nvm->flash, nvm->write_target_address, 1);
		eve->cfg_flashempty_flag = 1;
		status = 1;
	}
	
	
	if (eve->cfg_default_flag == 0 && eve->cfg_changed_flag == 1){
		
		//sprintf(debugtext, "Cfg: Store B:%d E:%d Ev:%d => Page: %d Status: %d", eve->parent->parent->index, eve->parent->index, eve->index, event_page_offset, status);		
		flash_write(nvm->flash, nvm->write_target_address, nvm->write_buffer, GRID_NVM_PAGE_SIZE);
		status = 1;
	}


	//grid_debug_print_text(debugtext);

	eve->cfg_changed_flag = 0;
	
	return status;
	
}



uint8_t grid_ui_nvm_load_event_configuration(struct grid_ui_model* ui, struct grid_nvm_model* nvm, struct grid_ui_event* eve){
	
		
	grid_nvm_clear_read_buffer(nvm);
	
	uint32_t event_page_offset = grid_nvm_calculate_event_page_offset(nvm, eve);	
	nvm->read_source_address = GRID_NVM_LOCAL_BASE_ADDRESS + GRID_NVM_PAGE_OFFSET*event_page_offset;	
	

	int status = flash_read(nvm->flash, nvm->read_source_address, nvm->read_buffer, GRID_NVM_PAGE_SIZE);	
		
	uint8_t copydone = 0;
	
	uint8_t cfgfound = 0;
		
	for (uint16_t i=0; i<GRID_NVM_PAGE_SIZE; i++){
			
			
		if (copydone == 0){
				
			if (nvm->read_buffer[i] == '\n'){ // END OF PACKET, copy newline character
				GRID_PORT_U.rx_double_buffer[i] = nvm->read_buffer[i];
				GRID_PORT_U.rx_double_buffer_status = i+1;
				GRID_PORT_U.rx_double_buffer_read_start_index = 0;
				copydone = 1;
				
				cfgfound=2;
					
			}
			else if (nvm->read_buffer[i] == 255){ // UNPROGRAMMED MEMORY, lets get out of here
				copydone = 1;
			}
			else{ // NORMAL CHARACTER, can be copied
				GRID_PORT_U.rx_double_buffer[i] = nvm->read_buffer[i];
				
				cfgfound=1;
			}
				
				
		}
			
			
	}
	
	return cfgfound;
	
	
}
uint8_t grid_ui_nvm_clear_event_configuration(struct grid_ui_model* ui, struct grid_nvm_model* nvm, struct grid_ui_event* eve){
		
		uint32_t event_page_offset = grid_nvm_calculate_event_page_offset(nvm, eve);
		
		

		flash_erase(nvm->flash, GRID_NVM_LOCAL_BASE_ADDRESS + GRID_NVM_PAGE_OFFSET*event_page_offset, 1);

		
		
		return 1;
		
}



void grid_ui_event_register_eventstring(struct grid_ui_element* ele, enum grid_ui_event_t event_type, uint8_t* event_string, uint32_t event_string_length){
	
	grid_debug_print_text("Register Action");
	uint8_t event_index = 255;
	
	for(uint8_t i=0; i<ele->event_list_length; i++){
		if (ele->event_list[i].type == event_type){
			event_index = i;
		}
	}
	
	if (event_index == 255){
		grid_debug_print_text("Event Not Found");
		return; // EVENT NOT FOUND
	}
	
	
	
	// Clear Action String
	for(uint32_t i=0; i<GRID_UI_EVENT_STRING_maxlength; i++){
		ele->event_list[event_index].event_string[i] = 0;
	}
	ele->event_list[event_index].event_string_length = 0;
	
		
	// TEMPLATE MAGIC COMING UP!
	
	uint8_t parameter_list_length = 0;

	
	for (uint32_t i=0; i<event_string_length; i++){
		
		// Copy Action
		ele->event_list[event_index].event_string[i] = event_string[i];
		
		// Check if STX or ETX was escaped, if so fix it!
		if (ele->event_list[event_index].event_string[i] > 127){
			
			grid_debug_print_text(" Escaped Char Found ");
			ele->event_list[event_index].event_string[i] -= 128;
			
		}
	
		
	}
	

	ele->event_list[event_index].event_string_length = event_string_length;
	
	grid_ui_smart_trigger(ele->parent->parent, ele->parent->index, ele->index, event_type);
	
	
}

void grid_ui_event_generate_eventstring(struct grid_ui_element* ele, enum grid_ui_event_t event_type){
	
	uint8_t event_index = 255;
	
	for(uint8_t i=0; i<ele->event_list_length; i++){
		if (ele->event_list[i].type == event_type){
			event_index = i;
		}
	}
	
	if (event_index == 255){
		return; // EVENT NOT FOUND
	}
	
	
	
	
	uint8_t event_string[GRID_UI_EVENT_STRING_maxlength] = {0};	
	
	
	if (ele->type == GRID_UI_ELEMENT_BUTTON){
		
		if (event_type == GRID_UI_EVENT_INIT){
			
			sprintf(event_string, GRID_EVENTSTRING_INIT_BUT); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
						
		}
		else if (event_type == GRID_UI_EVENT_DP){
			
			sprintf(event_string, GRID_EVENTSTRING_DP_BUT); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
					
		}
		else if (event_type == GRID_UI_EVENT_DR){
			
			sprintf(event_string, GRID_EVENTSTRING_DR_BUT); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
				
		}
		
	}
	else if (ele->type == GRID_UI_ELEMENT_POTENTIOMETER){
		
		if (event_type == GRID_UI_EVENT_INIT){
			
			sprintf(event_string, GRID_EVENTSTRING_INIT_POT); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
			
		}
		else if (event_type == GRID_UI_EVENT_AVC7){
			
			sprintf(event_string, GRID_EVENTSTRING_AVC7_POT); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
			
		}
		
	}
	else if (ele->type == GRID_UI_ELEMENT_ENCODER){
			
		if (event_type == GRID_UI_EVENT_INIT){
				
			sprintf(event_string, GRID_EVENTSTRING_INIT_ENC); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
				
		}
		else if (event_type == GRID_UI_EVENT_AVC7){
		
			sprintf(event_string, GRID_EVENTSTRING_AVC7_ENC); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
		
		}
		else if (event_type == GRID_UI_EVENT_ENCPUSHROT){
		
			sprintf(event_string, GRID_EVENTSTRING_PUSHROT_ENC); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
		
		}
		else if (event_type == GRID_UI_EVENT_DP){
				
			sprintf(event_string, GRID_EVENTSTRING_DP_ENC); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
				
		}
		else if (event_type == GRID_UI_EVENT_DR){
				
			sprintf(event_string, GRID_EVENTSTRING_DR_ENC); // !!
			grid_ui_event_register_eventstring(ele, event_type, event_string, strlen(event_string));
				
		}
			
	}
	
		
	
}




void grid_ui_event_generate_actionstring(struct grid_ui_element* ele, enum grid_ui_event_t event_type){
	
	uint8_t event_index = 255;
	
	for(uint8_t i=0; i<ele->event_list_length; i++){
		if (ele->event_list[i].type == event_type){
			event_index = i;
		}
	}
	
	if (event_index == 255){
		return; // EVENT NOT FOUND
	}
	
	
	
	uint8_t action_string[GRID_UI_ACTION_STRING_maxlength] = {0};
	
	if (ele->type == GRID_UI_ELEMENT_BUTTON){
				
		switch(event_type){
			case GRID_UI_EVENT_INIT:	sprintf(action_string, GRID_ACTIONSTRING_INIT_BUT);		break;
			case GRID_UI_EVENT_DP:		sprintf(action_string, GRID_ACTIONSTRING_DP_BUT);		break;
			case GRID_UI_EVENT_DR:		sprintf(action_string, GRID_ACTIONSTRING_DR_BUT);		break;
		}
		
	}
	else if (ele->type == GRID_UI_ELEMENT_POTENTIOMETER){
		
		switch(event_type){
			case GRID_UI_EVENT_INIT:	sprintf(action_string, GRID_ACTIONSTRING_INIT_BUT);		break;
			case GRID_UI_EVENT_AVC7:	sprintf(action_string, GRID_ACTIONSTRING_AVC7_POT);		break;
		}
		
	}
	else if (ele->type == GRID_UI_ELEMENT_ENCODER){
		
		switch(event_type){
			case GRID_UI_EVENT_INIT:        sprintf(action_string, GRID_ACTIONSTRING_INIT_ENC);	break;
			case GRID_UI_EVENT_AVC7:        sprintf(action_string, GRID_ACTIONSTRING_AVC7_ENC);	break;
			case GRID_UI_EVENT_ENCPUSHROT:	sprintf(action_string, GRID_ACTIONSTRING_PUSHROT_ENC);	break;
			case GRID_UI_EVENT_DP:          sprintf(action_string, GRID_ACTIONSTRING_DP_ENC);	break;
			case GRID_UI_EVENT_DR:          sprintf(action_string, GRID_ACTIONSTRING_DR_ENC);	break;
		}
			
	}
	
	if (strlen(action_string)){
		
		grid_ui_event_register_actionstring(ele, event_type, action_string, strlen(action_string));
		
	}
	
	ele->event_list[event_index].cfg_changed_flag = 0;
	ele->event_list[event_index].cfg_default_flag = 1;	
	
}


void grid_ui_event_register_actionstring(struct grid_ui_element* ele, enum grid_ui_event_t event_type, uint8_t* action_string, uint32_t action_string_length){
		
	uint8_t event_index = 255;
	
	for(uint8_t i=0; i<ele->event_list_length; i++){
		if (ele->event_list[i].type == event_type){
			event_index = i;
		}
	}
	
	if (event_index == 255){
		return; // EVENT NOT FOUND
	}
	
	
	
	// Clear Action String
	for(uint32_t i=0; i<GRID_UI_ACTION_STRING_maxlength; i++){
		ele->event_list[event_index].action_string[i] = 0;
	}
	ele->event_list[event_index].action_string_length = 0;
	

	uint8_t escaped_characters = 0;
	
	for (uint32_t i=0; i<action_string_length; i++){
		
		// Copy Action
		ele->event_list[event_index].action_string[i] = action_string[i];
		
		// Check if STX or ETX was escaped, if so fix it!
		if (ele->event_list[event_index].action_string[i] > 127){
			
			escaped_characters++;
			ele->event_list[event_index].action_string[i] -= 128;
			
		}
		
	}
	

	ele->event_list[event_index].action_string_length = action_string_length;
	
	ele->event_list[event_index].cfg_changed_flag = 1;
	
	
}



uint8_t grid_ui_event_find(struct grid_ui_element* ele, enum grid_ui_event_t event_type){

	uint8_t event_index = 255;
		
	for(uint8_t i=0; i<ele->event_list_length; i++){
		if (ele->event_list[i].type == event_type){
			event_index = i;
		}
	}

		
		
	return event_index;
	
}

void grid_ui_event_trigger(struct grid_ui_element* ele, uint8_t event_index){

	if (event_index == 255){
		
		return;
	}
	
	struct grid_ui_event* eve = &ele->event_list[event_index];


		
	eve->trigger = GRID_UI_STATUS_TRIGGERED;

}

void grid_ui_event_trigger_local(struct grid_ui_element* ele, uint8_t event_index){

	if (event_index == 255){
		
		return;
	}
	
	struct grid_ui_event* eve = &ele->event_list[event_index];


		
	eve->trigger = GRID_UI_STATUS_TRIGGERED_LOCAL;

}


void grid_ui_smart_trigger(struct grid_ui_model* mod, uint8_t bank, uint8_t element, enum grid_ui_event_t event){

	uint8_t event_index = grid_ui_event_find(&mod->bank_list[bank].element_list[element], event);
	
	if (event_index == 255){
		
		return;
	}
	
	grid_ui_event_trigger(&mod->bank_list[bank].element_list[element], event_index);

}


void grid_ui_smart_trigger_local(struct grid_ui_model* mod, uint8_t bank, uint8_t element, enum grid_ui_event_t event){

	uint8_t event_index = grid_ui_event_find(&mod->bank_list[bank].element_list[element], event);
	
	if (event_index == 255){
		
		return;
	}

    grid_ui_event_trigger_local(&mod->bank_list[bank].element_list[element], event_index);
    
}


void grid_ui_event_reset(struct grid_ui_event* eve){
	
	eve->trigger = GRID_UI_STATUS_READY;
}

uint8_t grid_ui_event_istriggered(struct grid_ui_event* eve){
		
		
	if (eve->trigger == GRID_UI_STATUS_TRIGGERED){
		
					
		return 1;
				
	}
	else{
		
		return 0;
	}
			
}


uint8_t grid_ui_event_istriggered_local(struct grid_ui_event* eve){
		
		
	if (eve->trigger == GRID_UI_STATUS_TRIGGERED_LOCAL){
		
					
		return 1;
				
	}
	else{
		
		return 0;
	}
			
}

uint32_t grid_ui_event_render_action(struct grid_ui_event* eve, uint8_t* target_string){

	
	uint8_t temp[500] = {0};

	uint32_t i=0;
	
	// copy event string
	for(true; i<eve->event_string_length; i++){
		temp[i] = eve->event_string[i];
	
	}

	// copy action string
	for(true; i<(eve->event_string_length + eve->action_string_length); i++){
		temp[i] = eve->action_string[i - eve->event_string_length];
	
	}
	
		
	uint32_t block_start = 0;
	uint32_t block_length = 0;


	uint32_t total_substituted_length = 0;

	uint8_t condition_state = 0;
	uint8_t condition_istrue = 0;


	for(i=0; i<(eve->event_string_length + eve->action_string_length) ; i++){

		target_string[i-total_substituted_length] = temp[i];


		if (temp[i] == '{'){
			
			block_start = i;

		}
		else if (temp[i] == '}'){

			block_length = i - block_start + 1;

			if (block_length){


				//printf("block_length %d \r\n", block_length);
				
				grid_expr_set_current_event(&grid_expr_state, eve);
				grid_expr_evaluate(&grid_expr_state, &temp[block_start+1], block_length-2); // -2 to not include {


				//printf("oslen %d\r\n", grid_expr_state.output_string_length);


				// Clear all of the copied expression
				for (uint8_t j = 0; j<block_length; j++){
					target_string[i-total_substituted_length-block_length+1+j] = 0;	
				}

				// Copy the result
				for (uint8_t j = 0; j<grid_expr_state.output_string_length; j++){
					target_string[i-total_substituted_length-block_length+j + 1] = grid_expr_state.output_string[GRID_EXPR_OUTPUT_STRING_MAXLENGTH-grid_expr_state.output_string_length+j];
					
					//printf("putc: %c\r\n", grid_expr_state.output_string[GRID_EXPR_OUTPUT_STRING_MAXLENGTH-grid_expr_state.output_string_length+j]);
				
				}

				total_substituted_length += block_length - grid_expr_state.output_string_length;

				//printf(" evaluated %s\r\n", &grid_expr_state.output_string[GRID_EXPR_OUTPUT_STRING_MAXLENGTH-grid_expr_state.output_string_length]);

				if (condition_state == 1){

					condition_istrue = grid_expr_state.return_value;
				}
			}
		}
		else if(temp[i] == '#'){

			if (temp[i+1] == 'I' && temp[i+2] == 'F' ){
				printf("#IF  \r\n");

				total_substituted_length += 3;
				i+=2;
				condition_state = 1;

			}else if (temp[i+1] == 'T' && temp[i+2] == 'H' ){
				printf("#THEN  \r\n");



				if (!condition_istrue){
					//SKIP
					printf("skip \r\n");
					for(uint8_t j=1; j<(eve->event_string_length + eve->action_string_length)-i; j++){

						if (temp[i+j] == '#'){


							total_substituted_length += j;
							i+=j-1;
							break;
						}
						else if(j==(eve->event_string_length + eve->action_string_length)-i-1){
							printf("Syntax Error!\r\n");
						}

					}

				}
				else{

					total_substituted_length += 3;
					i+=2;

					printf("no skip\r\n");
				}


			}else if (temp[i+1] == 'E' && temp[i+2] == 'L' ){
				printf("#ELSE  \r\n");

				if (condition_istrue){
					//SKIP
					printf("skip \r\n");
					for(uint8_t j=1; j<(eve->event_string_length + eve->action_string_length)-i; j++){

						if (temp[i+j] == '#'){
	
							total_substituted_length += j;
							i+=j-1;
							break;
						}
						else if(j==(eve->event_string_length + eve->action_string_length)-i-1){
							printf("Syntax Error!\r\n");
						}

					}

				}
				else{
					printf("no skip\r\n");


					total_substituted_length += 3;
					i+=2;

				}

			}else if (temp[i+1] == 'E' && temp[i+2] == 'N' ){
				printf("#ENDIF  \r\n");
				total_substituted_length += 3;
				i+=2;
				condition_istrue = 0;
				condition_state = 0;
			}
			



		}


	}
	
	
	// RESET ENCODER RELATIVE TEMPLATE PARAMETER VALUES
	if(eve->parent->type == GRID_UI_ELEMENT_ENCODER){	
		eve->parent->template_parameter_list[GRID_TEMPLATE_B_PARAMETER_LIST_LENGTH + GRID_TEMPLATE_E_PARAMETER_CONTROLLER_REL] = 255;	
		eve->parent->template_parameter_list[GRID_TEMPLATE_B_PARAMETER_LIST_LENGTH + GRID_TEMPLATE_E_PARAMETER_CONTROLLER_REL_VELOCITY_LOW] = 255;	
		eve->parent->template_parameter_list[GRID_TEMPLATE_B_PARAMETER_LIST_LENGTH + GRID_TEMPLATE_E_PARAMETER_CONTROLLER_REL_VELOCITY_HIGH] = 255;	
	
    }
	
	
	return eve->event_string_length + eve->action_string_length - total_substituted_length;
		
}


