idf_component_register(SRCS "winston_main.cpp"
                            "hall_effect_controller.cpp"
                            "htu21d_controller.cpp"
                            "display_controller.cpp"
                            "locking.cpp"
                            "mongoose_server.cpp"
                            "mqtt_service.cpp"
                            "reed_controller.cpp"
                            "relay_controller.cpp"
                            "request_handler.cpp"
                            "server.cpp"
                            "system_controller.cpp"
                            "temp_controller.cpp"
                            "time_controller.cpp"
                            "ui_controller.cpp"
                            "wifi.cpp"
                       INCLUDE_DIRS ""
                       REQUIRES main
                                nvs_flash
                                esp_http_server
                                esp32-i2c-ssd1306-oled
                                mongoose
                                mqtt)