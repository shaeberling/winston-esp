idf_component_register(SRCS "sk6812_controller.cpp" "winston_main.cpp"
                            "controller_factory.cpp"
                            "control_hub.cpp"
                            "display_controller.cpp"
                            "esp_temp_controller.cpp"
                            "hall_effect_controller.cpp"
                            "htu21d_controller.cpp"
                            "bh1750_controller.cpp"
                            "bme280_controller.cpp"
                            "sk6812_controller.cpp"
                            "locking.cpp"
                            "mongoose_server.cpp"
                            "mqtt_service.cpp"
                            "pir_controller.cpp"
                            "reed_controller.cpp"
                            "relay_controller.cpp"
                            "request_handler.cpp"
                            "server.cpp"
                            "settings_loader.cpp"
                            "system_controller.cpp"
                            "time_controller.cpp"
                            "ui_controller.cpp"
                            "wifi.cpp"
                            "proto/pb_common.c"
                            "proto/pb_decode.c"
                            "proto/device_settings.pb.c"
                       INCLUDE_DIRS ""
                                    "proto/include"
                       EMBED_FILES "node_config.bin"
                       REQUIRES main
                                nvs_flash
                                esp_http_server
                                esp32-i2c-ssd1306-oled
                                bh1750
                                bme280
                                esp_utils
                                mongoose
                                mqtt
                                neopixel)
