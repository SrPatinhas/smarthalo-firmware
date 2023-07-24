

    /*
     * ACCELEROMETER REGISTER SETTINGS DUMP TO SERIAL SEQUENCE
     * for debugging purposes
     **/
//        uint8_t dump_data;
//        uint8_t  test_string_register[] = "\r\n reg ";
//        uint8_t  test_string_equals[] = " = ";
//        for(uint8_t iter= 0; iter < 0x6D; ++iter)
//        {
//        	dump_data = check_register(ACCELEROMETER_DRIVER_ADDRESS, iter);
//        	nrf_delay_ms(10);
//        	printf("%s", test_string_register);
//        	printf("%x", iter);
//        	nrf_delay_ms(10);
//        	printf("%s", test_string_equals);
//        	printf("%x", dump_data);
//
//        }


    /*
     * MAGNETOMETER REGISTER SETTINGS DUMP TO SERIAL SEQUENCE
     * for debugging purposes
     * */
    //uint8_t dump_data;
   // uint8_t  test_string_register[] = "\r\n reg ";
   // uint8_t  test_string_equals[] = " = ";
//	for(uint8_t iter= 0; iter < 0x6D; ++iter)
//	{
//		dump_data = check_register(MAGNETOMETER_DRIVER_ADDRESS, iter);
//		nrf_delay_ms(10);
//		printf("%s", test_string_register);
//		printf("%x", iter);
//		nrf_delay_ms(10);
//		printf("%s", test_string_equals);
//		printf("%x", dump_data);
//	}

    /*
     * MAGNETOMETER TEST CODE
     * for debugging purposes
     * */
//	while(1)
//	{
//		 NRF_WDT->RR[0] = WDT_RR_RR_Reload;
//		 nrf_delay_ms(2000);
//		 //read raw data
//		 SH_Magnetometer_read_data();
//	}


    /*
     * ACCELEROMETER TEMPERATURE SENSOR TEST CODE
     * for debugging purposes
     * */
//    int8_t temp_data;
//    uint8_t  test_string_temperature[] = "\r\n temperature = ";
//    uint8_t dump_data;
//    uint8_t  test_string_regtemp[] = "\r\n temp register = ";
//    uint8_t  test_string_ctrl4[] = "\r\n CTRL4 register = ";
//    uint8_t  test_string_tempcfg[] = "\r\n TEMP_CFG_REG_A  register = ";
//    uint8_t  test_string_jump[] = "\r\n";
//
//    //  uint8_t  test_string_equals[] = " = ";
//	while(1)
//	{
//		 NRF_WDT->RR[0] = WDT_RR_RR_Reload;
//
//
//		 temp_data = ACCELEROMETER_MAGNETOMETER_retrieve_temperature_sensor_data();
//		 //temp_data = ~temp_data;
//		 nrf_delay_ms(10);
//		 printf("%s", test_string_temperature);
//		 printf("%d", (temp_data));
//
//		 dump_data = check_register(ACCELEROMETER_DRIVER_ADDRESS, 0x0D);
//		 nrf_delay_ms(10);
//	 	 printf("%s", test_string_regtemp);
//	 	 printf("%x", dump_data);
//
//		 dump_data = check_register(ACCELEROMETER_DRIVER_ADDRESS, 0x23);
//		 nrf_delay_ms(10);
//	 	 printf("%s", test_string_ctrl4);
//	 	 printf("%x", dump_data);
//
//		 dump_data = check_register(ACCELEROMETER_DRIVER_ADDRESS, 0x1F);
//		 nrf_delay_ms(10);
//	 	 printf("%s", test_string_tempcfg);
//	 	 printf("%x", dump_data);
//
//		 nrf_delay_ms(10);
//	 	 printf("%s", test_string_jump);
//	 	 nrf_delay_ms(2000);
//	}

//    int8_t temp_data;
//    float tempertuerere =0.0f;
//    float result =100;
//    uint8_t intresult = 0;
//   // uint8_t  test_string_space[] = "         \r";
//    while(1)
//    {
//    	result =100;
//    	tempertuerere = led_brightness_correction_for_temperature();
//    	intresult =  (uint8_t)(result * tempertuerere);
//    	//printf("%s",test_string_space);
//    	temp_data = ACCELEROMETER_MAGNETOMETER_retrieve_temperature_sensor_data();
//    	printf("%d     ", (temp_data));
//    	printf("%d \r\n",intresult);
//    	nrf_delay_ms(1000);
//    	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
//    }



     /*
     * ACCELEROMETER REGISTER SETTINGS DUMP TO SERIAL SEQUENCE
     * for debugging purposes
     **/
//   uint8_t dump_data;
//   uint8_t  test_string_register[] = "\r\n reg ";
//   uint8_t  test_string_equals[] = " = ";
//   for(uint8_t iter= 0; iter < 0x6D; ++iter)
//   {
//   	dump_data = check_register(ACCELEROMETER_DRIVER_ADDRESS, iter);
//   	nrf_delay_ms(10);
//   	printf("%s", test_string_register);
//   	printf("%x", iter);
//   	nrf_delay_ms(10);
//   	printf("%s", test_string_equals);
//   	printf("%x", dump_data);
//
//   }


    /*
     * MAGNETOMETER REGISTER SETTINGS DUMP TO SERIAL SEQUENCE
     * for debugging purposes
     * */
//	uint8_t dump_data;
//	uint8_t  test_string_register[] = "\r\n reg ";
//	uint8_t  test_string_equals[] = " = ";
//	for(uint8_t iter= 0; iter < 0x6E; ++iter)
//	{
//		dump_data = check_register(MAGNETOMETER_DRIVER_ADDRESS, iter);
//		nrf_delay_ms(10);
//		printf("%s", test_string_register);
//		printf("%x", iter);
//		nrf_delay_ms(10);
//		printf("%s", test_string_equals);
//		printf("%x", dump_data);
//	}
	//clear_bit_in_register( MAGNETOMETER_DRIVER_ADDRESS,  CFG_REG_A_M, FIRST_BIT);
	//clear_bit_in_register( MAGNETOMETER_DRIVER_ADDRESS,  CFG_REG_A_M, SECOND_BIT);

//	while(1)
//	{
//		NRF_WDT->RR[0] = WDT_RR_RR_Reload;
//		nrf_delay_ms(2000);
//		printf("\n\r");
//		for(uint8_t iter= 0x68; iter < 0x6E; iter++)
//		{
//			dump_data = check_register(MAGNETOMETER_DRIVER_ADDRESS, iter);
//			nrf_delay_ms(10);
//			printf("%s", test_string_register);
//			printf("%x", iter);
//			nrf_delay_ms(10);
//			printf("%s", test_string_equals);
//			printf("%x", dump_data);
//			nrf_delay_ms(10);
//		}
//	}
   // SH_magnetometer_data* temp_values;
//    int blurb = 0;
//    //uint8_t ii = 0;
//    while(1)
//    {
//    	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
//    	SH_Magnetometer_read_data();
//    	nrf_delay_ms(3000);
//    	//temp_values = magnetometer_get_local_stored_values();
//    	blurb = MAGNETOMETER_x_axis_data();
//    	printf("x = %d  \r\n",blurb);
//    	blurb = MAGNETOMETER_y_axis_data();
//    	printf("y = %d  \r\n",blurb);
//    	blurb = MAGNETOMETER_z_axis_data();
//    	printf("z = %d  \r\n",blurb);
//    	printf("\r\n");

    //	++ii;
    	//if(ii>30)
    	//{
    	//	ii = 0;
    	//}
   // }



 // SH_magnetometer_data average_data_magnetometer;
//
//    int blurb;
//   // int16_t blurb1;
//    while(1)
//    {
//    	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
//    	SH_Magnetometer_read_data();
//    	average_data_magnetometer.x_axis = MAGNETOMETER_x_axis_data();
//    	average_data_magnetometer.y_axis = MAGNETOMETER_y_axis_data();
//    	average_data_magnetometer.z_axis = MAGNETOMETER_z_axis_data();
//    	//temp_values = magnetometer_get_local_stored_values();
//    	blurb = SH_Magnetometer_heading(average_data_magnetometer);
//    	printf("heading = %d  \r\n",blurb);
//     	blurb = MAGNETOMETER_x_axis_data();
//    	printf("x = %d mGauss\r\n",((blurb*15)/10)); //1.5 mGauss per lsb
//    	blurb = MAGNETOMETER_y_axis_data();
//    	printf("y = %d mGauss\r\n",((blurb*15)/10));
//    	blurb = MAGNETOMETER_z_axis_data();
//    	printf("z = %d mGauss\r\n",((blurb*15)/10));
//    	printf("\r\n");
//    	//blurb = MAGNETOMETER_y_axis_data();
//    	//printf("y = %d  \r\n",blurb);
//    	//blurb = MAGNETOMETER_z_axis_data();
//    	//printf("z = %d  \r\n",blurb);
//    	printf("\r\n");
//    	nrf_delay_ms(1000);
//    }




//    HaloPixel_setPixelColor(19, 255,0,0);
//    HaloPixel_show();
//    nrf_delay_ms(3000);
//    HaloPixel_setPixelColor(19, 0,255,0);
//    HaloPixel_show();
//    nrf_delay_ms(3000);
//    HaloPixel_setPixelColor(19, 0,0,255);
//    HaloPixel_show();
//    nrf_delay_ms(3000);
//    HaloPixel_setPixelColor(21, 255,0,0);
//    HaloPixel_show();
//    nrf_delay_ms(3000);
//    HaloPixel_setPixelColor(21, 0,255,0);
//    HaloPixel_show();
//    nrf_delay_ms(3000);
//    HaloPixel_setPixelColor(21, 0,0,255);
//    HaloPixel_show();
//    nrf_delay_ms(3000);


//         /*
//     * BATMON REGISTER SETTINGS DUMP TO SERIAL SEQUENCE
//     * for debugging purposes
//     **/
//   uint8_t dump_data;
//   uint8_t  test_string_register[] = "\r\n reg ";
//   uint8_t  test_string_equals[] = " = ";
//   for(uint8_t iter= 0; iter < 0x6D; ++iter)
//   {
//	   dump_data = check_register(ADDRESS_OF_GAS_GAUGE, iter);
//	   nrf_delay_ms(10);
//	   printf("%s", test_string_register);
//	   printf("%x", iter);
//	   nrf_delay_ms(10);
//	   printf("%s", test_string_equals);
//	   printf("%x", dump_data);
//
//   }


    /*
     * ACCELEROMETER REGISTER SETTINGS DUMP TO SERIAL SEQUENCE
     * for debugging purposes
     **/
//        uint8_t dump_data;
//        uint8_t  test_string_register[] = "\r\n reg ";
//        uint8_t  test_string_equals[] = " = ";
//        for(uint8_t iter= 0; iter < 0x6D; ++iter)
//        {
//        	dump_data = check_register(MAGNETOMETER_DRIVER_ADDRESS, iter);
//        	nrf_delay_ms(10);
//        	printf("%s", test_string_register);
//        	printf("%x", iter);
//        	nrf_delay_ms(10);
//        	printf("%s", test_string_equals);
//        	printf("%x", dump_data);
//
//        }




//    float  vbat = 0;
//    float SOCbat = 0;
//    while(1)
//    {
//		vbat = current_voltage_of_the_battery();
//		SOCbat = current_soc_of_the_battery();
//		printf("Vbat = %f \r\n",vbat);
//		printf("SOCbat = %f \r\n",SOCbat);
//		nrf_delay_ms(3000);
//    }

