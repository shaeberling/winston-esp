idf_component_register(SRCS "winston_main.cpp"
                            "hall_effect_controller.cpp"
                            "display_controller.cpp"
                            "reed_controller.cpp"
                            "relay_controller.cpp"
                            "server.cpp"
                            "temp_controller.cpp"
                            "ui_controller.cpp"
                            "wifi.cpp"
                            "htu21d_controller.cpp"
                       INCLUDE_DIRS ""
                       REQUIRES main
                                nvs_flash
                                esp_http_server
                                esp32-i2c-ssd1306-oled)